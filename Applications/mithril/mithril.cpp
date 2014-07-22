// Copyright 2011 A.N.M. Imroz Choudhury
//
// mithril.cpp - Parses the output of the "dwarfdump" program, placing
// relevant data into a protobuffer for use by aeon and waxlamp (and
// others).

// MTV headers.
#include <Core/LexicalBlock.pb.h>
#include <Core/Subprogram.pb.h>
#include <Core/Type.pb.h>
#include <Core/Variable.pb.h>
#include <Core/Util/Util.h>
using MTV::Mithril::LexicalBlock;
using MTV::Mithril::LexicalBlockList;
using MTV::Mithril::Locator;
using MTV::Mithril::Loclist;
using MTV::Mithril::Subprogram;
using MTV::Mithril::SubprogramList;
using MTV::Mithril::Type;
using MTV::Mithril::TypeList;
using MTV::Mithril::Variable;
using MTV::Mithril::VariableList;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <iostream>
#include <limits>
#include <stack>
#include <stdint.h>

class LineBuffer{
public:
  LineBuffer(std::istream& in)
    : in(in),
      line_(0)
  {}

  std::string get(){
    ++line_;

    std::string result;
    if(buffer.size() > 0){
      result = buffer.top();
      buffer.pop();
    }
    else{
      std::getline(in, result);
    }
    std::cerr << "LINE: " << result << std::endl;
    return result;
  }

  void unget(const std::string& s){
    --line_;

    buffer.push(s);
  }

  bool eof() const {
    return in.eof();
  }

  long line() const {
    return line_;
  }

private:
  std::istream& in;
  std::stack<std::string> buffer;
  long line_;
};

class DwarfParser{
private:
  class RootInfo{
  public:
    RootInfo(const std::vector<std::string>& tokens)
      : indent_(-1),
        id_(0),
        tag_(""),
        whole(false)
    {
      // If the line is blank, or the first character of the line isn't
      // a less-than sign, then it's not a root tag.
      if(tokens.size() == 0 or tokens[0][0] != '<'){
        return;
      }

      // Construct the spec string - if the first token is a LONE
      // less-than-sign, then adjoin it to the second token (if this
      // can't be done, the line is not a root tag), and the third token
      // is the tag name; otherwise, the first token is the spec string,
      // and the second is the tag name.
      std::string spec;
      if(tokens[0].length() == 1){
        if(tokens.size() < 3){
          return;
        }
        else{
          spec = tokens[0] + tokens[1];
          tag_ = tokens[2];
        }
      }
      else if(tokens.size() < 2){
        return;
      }
      else{
        spec = tokens[0];
        tag_ = tokens[1];
      }

      // Examine the spec string.  It should have the form of
      // "<number><hex-number>" - if it doesn't, it's not a root tag.
      const size_t brk = spec.find("><");
      if(brk == std::string::npos){
        return;
      }

      // <12><0xdead>
      //
      // brk = 3
      // length = 12

      // Extract the two fields - one goes from the second character to
      // the "break", and the other from two past the "break" to the
      // next-to-last character.
      std::string indent_s, id_s;
      indent_s = spec.substr(1, brk - 1);
      id_s = spec.substr(brk + 2, spec.length() - (brk + 2) - 1);

      {
        // Try to read an int out of the first string - if we can't,
        // then this is not a root tag.
        std::stringstream ss(indent_s);
        if(!(ss >> indent_)){
          return;
        }
      }

      {
        // Try to read an unsigned hex value out of the second string -
        // if we can't, this this is not a root tag.
        std::stringstream ss(id_s);
        if(!(ss >> std::hex >> id_)){
          return;
        }
      }

      // std::cerr << "RootInfo constructed" << std::endl
      //           << "\t" << "indent level: " << indent_ << std::endl
      //           << "\t" << "id: 0x" << std::hex << id_ << std::dec << std::endl
      //           << "\t" << "tag: " << tag_ << std::endl;

      whole = true;
    }

    int indent() const {
      return indent_;
    }

    uint64_t id() const {
      return id_;
    }

    const std::string& tag() const {
      return tag_;
    }

    bool good() const {
      return whole;
    }

  private:
    int indent_;
    uint64_t id_;
    std::string tag_;
    bool whole;
  };

public:
  DwarfParser(std::istream& in)
    : buffer(in),
      in(in)
  {}

  void parse(){
    // Read lines of input and exit when there are no more to process.
    while(true){
      // Read a line of input.
      std::string line = buffer.get();

      // Finish out if nothing more to read.
      if(buffer.eof()){
        return;
      }

      // Only handle lines containing "DW_*" tags.
      if(!is_dw_tag(line)){
        continue;
      }

      // Split into tokens.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the tag is a "root" tag.
      RootInfo info(tokens);
      if(info.good()){
        // Dispatch a parsing routine based on the tag type.
        if(info.tag() == "DW_TAG_compile_unit"){
          parse_compile_unit(info.indent());
        }
        else{
          std::cerr << "tag at top level was: " << info.tag() << std::endl;
        }
      }
    }
  }

  void print() const {
    std::cout << "TYPES:" << std::endl;
    for(int i=0; i<types.type_size(); i++){
      const Type& t = types.type(i);

      std::cout << std::hex << t.id() << ": " << std::dec;

      if(t.has_base_type()){
        std::cout << "base type '" << t.name() << "' of size " << t.base_type().size() << std::endl;
      }
      else if(t.has_class_type()){
        std::cout << "class type '" << t.name() << "' of size " << t.class_type().size() << std::endl;
      }
      else if(t.has_typedef_type()){
        std::cout << "typedef type referring to type " << std::hex << t.typedef_type().ref() << std::dec << std::endl;
      }
      else if(t.has_const_type()){
        std::cout << "const type referring to type " << std::hex << t.const_type().ref() << std::dec << std::endl;
      }
      else if(t.has_pointer_type()){
        std::cout << "pointer type of size " << t.pointer_type().size() << ", referring to type " << std::hex << t.pointer_type().ref() << std::dec << std::endl;
      }
      else if(t.has_array_type()){
        std::cout << "array type: not implemented quite yet" << std::endl;
      }
    }

    std::cout << "VARIABLES:" << std::endl;
    for(int i=0; i<vars.var_size(); i++){
      const Variable& v = vars.var(i);

      std::cout << v.name() << " declared at " << v.file() << ":" << v.line()
                << " of type id " << std::hex << v.type() << std::dec
                << " at ";

      if(v.has_stack_location()){
        std::cout << "stack offset " << v.stack_location().offset() << std::endl;
      }
      else if(v.has_absolute_location()){
        std::cout << "address " << std::hex << v.absolute_location().address() << std::dec << std::endl;
      }
    }

    std::cout << "SUBPROGRAMS:" << std::endl;
    for(int i=0; i<subprograms.subprogram_size(); i++){
      const Subprogram& s = subprograms.subprogram(i);

      std::cout << "subprogram " << s.name() << " from 0x" << std::hex << s.low_pc() << " to 0x" << s.high_pc() << std::endl;

      std::cout << "loclist:" << std::endl;
      std::cout << "\t" << "compile unit low pc: 0x" << std::hex << s.loclist().cu_low_pc() << std::endl;
      for(int j=0; j<s.loclist().locator_size(); j++){
        const Locator& l = s.loclist().locator(j);
        std::cout << "\t" << "0x" << std::hex << l.low_pc() << " to 0x" << l.high_pc() << ": register " << l.reg() << " + " << std::dec << l.offset() << std::endl;
      }
    }

    std::cout << "LEXICAL SCOPES:" << std::endl;
    std::cout << std::hex;
    for(int i=0; i<scopes.lexical_block_size(); i++){
      const LexicalBlock& l = scopes.lexical_block(i);

      std::cout << "0x" << l.low_pc() << " -> 0x" << l.high_pc() << std::endl;
    }
  }

  bool save(const std::string& filename) const {
    // Open the file for output, and bail if error.
    std::ofstream out(filename.c_str());
    if(!out){
      return false;
    }

    // Write out the type specs.
    {
      const int size = types.ByteSize();
      out.write(reinterpret_cast<const char *>(&size), sizeof(size));
      types.SerializeToOstream(&out);
    }

    // Write out the varaible specs.
    {
      const int size = vars.ByteSize();
      out.write(reinterpret_cast<const char *>(&size), sizeof(size));
      vars.SerializeToOstream(&out);
    }

    // Write out the subprogram specs.
    {
      const int size = subprograms.ByteSize();
      out.write(reinterpret_cast<const char *>(&size), sizeof(size));
      subprograms.SerializeToOstream(&out);
    }

    // Write out the lexical block specs.
    {
      const int size = scopes.ByteSize();
      out.write(reinterpret_cast<const char *>(&size), sizeof(size));
      scopes.SerializeToOstream(&out);
    }

    // Flush the stream, close it, then indicate success.
    out.flush();
    out.close();

    return true;
  }

private:
  void parse_compile_unit(const int indent){
    std::cerr << "Compile unit!" << std::endl;

    uint64_t low_pc = 0x0, high_pc = 0x0;

    // This loop will consume lines until either (1) end-of-file or
    // (2) another root tag is encountered at the same indentation
    // level as the argument to this function (indicating that another
    // element has begun at the same level).
    while(true){
      // Consume a line of input and check for EOF.
      std::string line = buffer.get();
      if(buffer.eof()){
        std::cerr << "File ending" << std::endl;
        return;
      }

      // Skip lines that don't contain tags.
      if(!is_dw_tag(line)){
        continue;
      }

      // Tokenize the input line.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // If it's a root tag, extract the info about it - otherwise,
      // parse the information in the line.
      RootInfo info(tokens);
      if(info.good()){
        // If the indent level matches, we are done - push the last
        // line back onto the input buffer and return.
        if(info.indent() <= indent){
          std::cerr << "Compile unit ending" << std::endl;

          buffer.unget(line);
          return;
        }

        // Otherwise, dispatch the appropriate parsing routine.
        if(is_type_tag(info.tag())){
          // A type tag - dispatch a type parsing routine, and create
          // an entry in a table for the type.
          parse_type_element(info.tag(), info.id(), info.indent());
        }
        else if(info.tag() == "DW_TAG_subprogram"){
          if(low_pc == 0x0){
            std::cerr << "warning (" << buffer.line() << "): compile unit did not specify low_pc, but has subprogram element - skipping" << std::endl;
            skip_subelement(info.indent());
          }
          else{
            // parse_subprogram(info.id(), info.indent());
            parse_subprogram(info.indent(), low_pc);
          }
        }
        else if(info.tag() == "DW_TAG_variable"){
          if(low_pc == 0x0 or high_pc == 0x0){
            std::cerr << "fatal error (" << buffer.line() << "): low_pc or high_pc unspecified, but variable element encountered - skipping" << std::endl;
            skip_subelement(info.indent());
          }
          else{
            parse_variable(info.indent(), info.id(), low_pc, high_pc);
          }
        }
        else if(info.tag() == "DW_TAG_namespace"){
          if(low_pc == 0x0 or high_pc == 0x0){
            std::cerr << "warning (" << buffer.line() << "): low_pc or high_pc unspecified, but namespace element encountered - skipping" << std::endl;
            skip_subelement(info.indent());
          }
          else{
            parse_namespace(info.indent(), low_pc, high_pc);
          }
        }
        else{
          std::cerr << "error: encountered element " << info.tag() << " at line " << buffer.line() << " of input." << std::endl;
          abort();
        }
      }
      else{
        if(tokens[0] == "DW_AT_low_pc"){
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> low_pc;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value" << std::endl;
            exit(1);
          }
        }
        else if(tokens[0] == "DW_AT_high_pc"){
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> high_pc;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value" << std::endl;
            exit(1);
          }
        }
        else{
          std::cerr << "warning: skipping line in compile unit element: " << tokens[0] << ": " << concat(tokens, 1) << std::endl;
        }
      }
    }
  }

  void parse_namespace(const int indent, uint64_t low_pc, uint64_t high_pc, const std::string& prefix_base = ""){
    std::string prefix = "";

    while(true){
      const std::string line = buffer.get();
      if(buffer.eof()){
        std::cerr << "[parse_namespace] warning (" << buffer.line() << "): encountered EOF" << std::endl;
        return;
      }

      // Skip blank lines, etc.
      if(!is_dw_tag(line)){
        continue;
      }

      // Tokenize.
      const std::vector<std::string> tokens = MTV::tokenize(line);

      // Check for a subelement.
      RootInfo info(tokens);
      if(info.good()){
        if(info.indent() <= indent){
          buffer.unget(line);
          return;
        }

        if(info.tag() == "DW_TAG_namespace"){
          // Nested namespace declaration.
          if(prefix == ""){
            std::cerr << "fatal error (" << buffer.line() << "): nested namespace tag in a nameless namespace element" << std::endl;
            abort();
          }

          parse_namespace(info.indent(), low_pc, high_pc, prefix);
        }
        else if(is_type_tag(info.tag())){
          parse_type_element(info.tag(), info.id(), info.indent(), prefix_base);
        }
        else if(info.tag() == "DW_TAG_variable"){
          parse_variable(info.indent(), info.id(), low_pc, high_pc);
        }
        else if(info.tag() == "DW_TAG_subprogram"){
          parse_subprogram(info.indent(), low_pc, prefix_base);
        }
        else if(info.tag() == "DW_TAG_imported_declaration" or
                info.tag() == "DW_TAG_imported_module"){
          // NOTE(choudhury): All skippable tags should be tested for
          // here.
          skip_subelement(info.indent());
        }
        else{
          std::cerr << "fatal error (" << buffer.line() << "): unhandled subelement '" << info.tag() << "' in namespace element." << std::endl;
          abort();
        }
      }
      else{
        if(tokens[0] == "DW_AT_name"){
          prefix = prefix_base + strip_enclosing(tokens[1]) + "::";
        }
        else if(tokens[0] == "DW_AT_decl_file" or
                tokens[0] == "DW_AT_decl_line" or
                tokens[0] == "DW_AT_sibling"){
          continue;
        }
        else{
          std::cerr << "fatal error (" << buffer.line() << "): unknown attribute '" << tokens[0] << "' in namespace element." << std::endl;
          abort();
        }
      }
    }
  }

  void parse_subprogram(const int indent, const uint64_t cu_low_pc, const std::string& prefix = ""){
    // Declare a pair of bounding instruction addresses.
    uint64_t low_pc = 0x0, high_pc = 0x0;

    // Watch for actual source code.
    bool has_source = true;

    // A loclist object created by parsing the "DW_AT_frame_base"
    // attribute.
    Loclist locs;

    // The name of the subprogram.
    std::string name = "";

    while(true){
      // Read a line of input.
      std::string line = buffer.get();
      if(buffer.eof()){
        std::cerr << "[parse_subprogram] warning (" << buffer.line() << "): encountered EOF" << std::endl;
        return;
      }

      // Create token stream.
      std::vector<std::string> tokens = MTV::tokenize(line);
      if(!is_dw_tag(line)){
        // Skip the line if it doesn't contain a DW tag (this includes
        // blank lines).
        continue;
      }

      // Check to see if it's the root of a sub-element.
      RootInfo info(tokens);
      if(info.good()){
        // First check to see if the element is at the same
        // indentation level as the element being processed - if so,
        // create an entry for the subprogram and then return after
        // pushing the line back in the buffer.
        if(info.indent() <= indent){
          // Check for fully specified attributes.
          if(name == "" or low_pc == 0x0 or high_pc == 0x0){
            std::cerr << "fatal error (" << buffer.line() << "): all subprogram attributes not specified." << std::endl;
            abort();
          }

          // Record the name and pc range.
          Subprogram *s = subprograms.add_subprogram();
          s->set_name(name);
          s->set_low_pc(low_pc);
          s->set_high_pc(high_pc);

          // Record the loclist.
          Loclist *l = s->mutable_loclist();
          l->set_cu_low_pc(cu_low_pc);
          for(int i=0; i<locs.locator_size(); i++){
            Locator *loc = l->add_locator();
            loc->set_low_pc(locs.locator(i).low_pc());
            loc->set_high_pc(locs.locator(i).high_pc());
            loc->set_reg(locs.locator(i).reg());
            loc->set_offset(locs.locator(i).offset());
          }

          // // NOTE(choudhury): try just recording the third loclist
          // // entry, representing the "bulk" entry that oversees the
          // // function body itself.
          // Locator *loc = l->add_locator();
          // loc->set_low_pc(locs.locator(2).low_pc());
          // loc->set_high_pc(locs.locator(2).high_pc());
          // loc->set_reg(locs.locator(2).reg());
          // loc->set_offset(locs.locator(2).offset());

          buffer.unget(line);
          return;
        }

        // The subprogram element should specify its PC range before
        // deferring to any subelements.
        if(low_pc == 0x0 or high_pc == 0x0){
          std::cerr << "fatal error (" << buffer.line() << "): subprogram spec did not include both a low_pc and a high_pc" << std::endl;
          abort();
        }

        // Dispatch the proper handler - so far, it's just "lexical
        // block" elements that we expect.
        if(info.tag() == "DW_TAG_lexical_block"){
          if(has_source){
            parse_lexical_block(info.indent());
          }
          else{
            std::cerr << "fatal error (" << buffer.line() << "): declaration-oly subprogram element reports a lexical block" << std::endl;
            abort();
          }
        }
        else if(info.tag() == "DW_TAG_formal_parameter"){
          // Don't try to process the formal parameter list if there
          // is no source code.
          if(has_source){
            parse_variable(info.indent(), info.id(), low_pc, high_pc);
          }
          else{
            skip_subelement(info.indent());
          }
        }
        else if(is_type_tag(info.tag())){
          parse_type_element(info.tag(), info.id(), info.indent());
        }
        else{
          std::cerr << "error (" << buffer.line() << "): unhandled tag '" << info.tag() << "'" << std::endl;
          abort();
        }
      }
      else{
        // Check for low and high pc tags - ignore the rest.
        if(tokens[0] == "DW_AT_low_pc"){
          // TODO(choudhury): figure out what to do with this. Most
          // likely, place in a table of function entries, so aeon.so
          // can compute the stack frame base address, etc., upon
          // entry.
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> low_pc;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
        }
        else if(tokens[0] == "DW_AT_high_pc"){
          // TODO(choudhury): not sure if useful (see DW_AT_low_pc
          // note above).
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> high_pc;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
        }
        else if(tokens[0] == "DW_AT_name"){
          name = strip_enclosing(tokens[1]);
        }
        else if(tokens[0] == "DW_AT_declaration" or tokens[0] == "DW_AT_artificial"){
          has_source = false;

          // Skip the rest of this element, then return.
          skip_subelement(indent);
          return;
        }
        else if(tokens[0] == "DW_AT_frame_base"){
          parse_loclist(locs);
        }
        else{
          std::cerr << "warning (" << buffer.line() << "): ignoring field '" << tokens[0] << "' of subprogram element" << std::endl;
        }
      }
    }
  }

  void parse_loclist(Loclist& locs){
    while(true){
      // Grab a line of input, removing all whitespace.
      const std::string input_line = buffer.get();
      // std::string line = concat(MTV::tokenize(input_line), false);
      std::string line = remove_spaces(MTV::tokenize(input_line));

      // Check to see whether this contains either a DW_TAG_* or
      // DW_AT_* specifier - if so, push it back into the buffer and
      // return.
      //
      // if(is_dw_tag(line) or is_at_tag(line)){
      if(line.find("DW_AT") != std::string::npos or line.find("DW_TAG") != std::string::npos){
        buffer.unget(input_line);
        return;
      }

      // Attempt to parse out the location descriptor from the line.
      // It will look like: [a]<lowpc=0xb><highpc=0xc>regname+offset.
      //
      // Extract the identifier.
      {
        const size_t left = line.find("[");
        const size_t right = line.find("]");
        if(left >= right){
          goto danger;
        }

        // If the identifier is not "2" (the "bulk" loclist entry
        // covering most of the function body), then skip it.
        const std::string id_text = strip_enclosing(line.substr(left, (right+1) - left));
        std::cerr << "id_text: --" << id_text << "--" << std::endl;
        if(id_text != "2"){
          continue;
        }
      }

      // Add a locator object to the loclist.
      Locator *loc = locs.add_locator();

      // Extract the low pc.
      {
        const size_t left = line.find("<");
        const size_t right = line.find(">");
        if(left >= right){
          std::cerr << "1" << std::endl;
          goto danger;
        }

        const std::string low_pc_text = strip_enclosing(line.substr(left, (right + 1) - left));
        const std::string identifier = "lowpc=";
        if(low_pc_text.substr(0, identifier.length()) != identifier){
          std::cerr << "2: " << low_pc_text.substr(0, identifier.length()) << std::endl;
          goto danger;
        }

        // Extract the hex number.
        const std::string target = low_pc_text.substr(identifier.length());
        std::stringstream ss(target);
        uint64_t low_pc;
        ss >> std::hex >> low_pc;
        if(!ss){
          std::cerr << "3" << std::endl;
          goto danger;
        }

        // Save the low pc in the locator object.
        loc->set_low_pc(low_pc);

        // Cut out the processed part of the line.
        line = line.substr(right+1);

        std::cerr << "leftover: " << line << std::endl;
      }

      // Extract the high pc.
      {
        const size_t left = line.find("<");
        const size_t right = line.find(">");
        const std::string high_pc_text = strip_enclosing(line.substr(left, (right + 1) - left));
        std::cerr << "high_pc_text: " << high_pc_text << std::endl;
        const std::string identifier = "highpc=";
        if(high_pc_text.substr(0, identifier.length()) != identifier){
          std::cerr << "4" << std::endl;
          goto danger;
        }

        // Extract the hex number.
        const std::string target = high_pc_text.substr(identifier.length());
        std::cerr << "target: " << target << std::endl;
        std::stringstream ss(target);
        uint64_t high_pc;
        ss >> std::hex >> high_pc;
        if(!ss){
          std::cerr << "5" << std::endl;
          goto danger;
        }

        // Save the high pc in the locator object.
        loc->set_high_pc(high_pc);

        // Cut out the processed part of the line.
        line = line.substr(right+1);

        std::cerr << "leftover: " << line << std::endl;
      }

      // Extract the register/offset at which to find the frame base
      // pointer.
      {
        // Find the plus sign.
        const size_t plus = line.find("+");
        if(plus == std::string::npos){
          std::cerr << "6" << std::endl;
          goto danger;
        }

        // Extract the name of the register to use.
        const std::string reg = line.substr(0, plus);

        // Extract the offset.
        const std::string offset_text = line.substr(plus+1);
        std::stringstream ss(offset_text);
        int offset;
        ss >> offset;
        if(!ss){
          std::cerr << "7" << std::endl;
          goto danger;
        }

        // Save the register and offset in the locator object.
        loc->set_reg(reg);
        loc->set_offset(offset);
      }
    }

    return;

  danger:
    // A bailout condition label for gotos in the above code to jump
    // to.  I don't want to hear about it, Dijkstra.
    std::cerr << "fatal error (" << buffer.line() << "): could not parse loclist entry." << std::endl;
    abort();
  }

  // void parse_lexical_block(const int indent, const uint64_t low_pc, const uint64_t high_pc){
  void parse_lexical_block(const int indent){
    // Low and high pc range.
    uint64_t low_pc = 0x0, high_pc = 0x0;

    while(true){
      // Read a line of input.
      const std::string line = buffer.get();
      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): unexpected EOF in lexical block element" << std::endl;
        abort();
      }

      // Skip if blank (or not a DW_ tag).
      if(!is_dw_tag(line)){
        continue;
      }

      // Tokenize.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check for subelement.
      RootInfo info(tokens);
      if(info.good()){
        // Check for previous PC specifications.
        if(low_pc == 0x0 or high_pc == 0x0){
          std::cerr << "fatal error (" << buffer.line() << "): lexical block spec did not include both a low_pc and a high_pc" << std::endl;
          abort();
        }

        // The element is ending - record the block and return.
        if(info.indent() <= indent){
          LexicalBlock *l = scopes.add_lexical_block();
          l->set_low_pc(low_pc);
          l->set_high_pc(high_pc);

          buffer.unget(line);
          return;
        }

        // Dispatch the correct handler.  We are only expecting
        // "variable" elements and nested "lexical block" elements.
        if(info.tag() == "DW_TAG_variable"){
          parse_variable(info.indent(), info.id(), low_pc, high_pc);
        }
        else if(info.tag() == "DW_TAG_lexical_block"){
          parse_lexical_block(info.indent());
        }
        else if(is_type_tag(info.tag())){
          parse_type_element(info.tag(), info.id(), info.indent());
        }
        else{
          std::cerr << "error (" << buffer.line() << "): unhandled tag '" << info.tag() << "'" << std::endl;
          abort();
        }
      }
      else{
        if(tokens[0] == "DW_AT_low_pc"){
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> low_pc;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): expected hex number, got '" << tokens[1] << "'" << std::endl;
            abort();
          }
        }
        else if(tokens[0] == "DW_AT_high_pc"){
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> high_pc;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): expected hex number, got '" << tokens[1] << "'" << std::endl;
            abort();
          }
        }
        else{
          std::cerr << "warning (" << buffer.line() << "): ignoring field '" << tokens[0] << "' of subprogram element" << std::endl;
        }
      }
    }
  }

  void parse_variable(const int indent, const uint64_t id, const uint64_t low_pc, const uint64_t high_pc){
    std::string name = "", file = "", loc_protocol = "";
    uint64_t line_no = -1, type = -1, loc_addr = -1;
    int32_t loc_offset = -1;
    bool loc_offset_set = false;

    while(true){
      // Grab a line of input.
      const std::string line = buffer.get();
      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): unexpected EOF in variable element" << std::endl;
        abort();
      }

      // Skip blank lines (and other non-DW lines).
      if(!is_dw_tag(line)){
        continue;
      }

      // Tokenize.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check for a subelement (there shouldn't be any).
      RootInfo info(tokens);
      if(info.good()){
        if(info.indent() > indent){
          std::cerr << "fatal error (" << buffer.line() << "): unexpected subelement '" << info.tag() << "' of variable element" << std::endl;
          abort();
        }
        else{
          // Check to see whether the variable has a location - if
          // not, just return immediately.
          if(loc_protocol == ""){
            // Sometimes a variable is only declared but not defined,
            // in which case it exists, but not in a knowable place -
            // when that happens, don't worry about it and just move
            // on.
            std::cerr << "warning (" << buffer.line() << "): variable tag had no location information." << std::endl;
          }
          else if(name == "" or file == "" or line_no == static_cast<uint64_t>(-1) or type == static_cast<uint64_t>(-1) or
                  (loc_protocol == "DW_OP_addr" and loc_addr == static_cast<uint64_t>(-1)) or
                  (loc_protocol == "DW_OP_fbreg" and !loc_offset_set)){
            std::cerr << "warning (" << buffer.line() << "): some variable attributes were not set in the variable element." << std::endl;
          }
          else{
            // Create the protobuf message and place it in the
            // variable list.
            Variable *v = vars.add_var();
            v->set_id(id);
            v->set_name(name);
            v->set_type(type);
            v->set_file(file);
            v->set_line(line_no);
            v->set_low_pc(low_pc);
            v->set_high_pc(high_pc);

            if(loc_protocol == "DW_OP_fbreg"){
              v->mutable_stack_location()->set_offset(loc_offset);
            }
            else if(loc_protocol == "DW_OP_addr"){
              v->mutable_absolute_location()->set_address(loc_addr);
            }
            else{
              std::cerr << "fatal error (" << buffer.line() << "): unknown location protocol in variable tag." << std::endl;
              exit(1);
            }
          }

          buffer.unget(line);
          return;
        }
      }

      // Set the appropriate attribute.
      if(tokens[0] == "DW_AT_name"){
        name = strip_enclosing(tokens[1]);
      }
      else if(tokens[0] == "DW_AT_decl_file"){
        file = tokens[2];
      }
      else if(tokens[0] == "DW_AT_decl_line"){
        std::stringstream ss(tokens[1]);
        ss >> std::hex >> line_no;
        if(!ss){
          std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
          exit(1);
        }
      }
      else if(tokens[0] == "DW_AT_type"){
        // std::stringstream ss(strip_enclosing(tokens[1]));
        std::stringstream ss(strip_enclosing(concat(tokens, 1)));
        ss >> std::hex >> type;
        if(!ss){
          std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
          exit(1);
        }
      }
      else if(tokens[0] == "DW_AT_location"){
        loc_protocol = tokens[1];
        if(loc_protocol == "DW_OP_fbreg"){
          std::stringstream ss(tokens[2]);
          ss >> loc_offset;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
          loc_offset_set = true;
        }
        else if(loc_protocol == "DW_OP_addr"){
          std::stringstream ss(tokens[2]);
          ss >> std::hex >> loc_addr;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
        }
      }
      else{
        std::cerr << "warning (" << buffer.line() << "): ignoring attribute '" << tokens[0] << "' in variable element." << std::endl;
      }
    }
  }

  void parse_type_element(const std::string& tag, const uint64_t id, const int indent, const std::string& prefix = ""){
    if(tag == "DW_TAG_base_type"){
      std::cerr << "parsing base type" << std::endl;
      parse_base_type(id, indent);
      std::cerr << "done parsing base type" << std::endl;
    }
    else if(tag == "DW_TAG_class_type" or tag == "DW_TAG_structure_type"){
      std::cerr << "parsing class type" << std::endl;
      parse_class_type(id, indent, prefix);
      std::cerr << "done parsing class type" << std::endl;
    }
    else if(tag == "DW_TAG_typedef"){
      std::cerr << "parsing typedef type" << std::endl;
      parse_typedef_type(id, indent, prefix);
      std::cerr << "done parsing typedef type" << std::endl;
    }
    else if(tag == "DW_TAG_subroutine_type"){
      std::cerr << "parsing subroutine type" << std::endl;
      parse_typedef_type(id, indent, prefix, false);
      std::cerr << "done parsing subroutine type" << std::endl;
    }
    else if(tag == "DW_TAG_const_type"){
      std::cerr << "parsing const type" << std::endl;
      parse_const_type(id, indent);
      std::cerr << "done parsing const type" << std::endl;
    }
    else if(tag == "DW_TAG_pointer_type"){
      std::cerr << "parsing pointer type" << std::endl;
      parse_pointer_type(id, indent);
      std::cerr << "done parsing pointer type" << std::endl;
    }
    else if(tag == "DW_TAG_reference_type"){
      std::cerr << "parsing reference type" << std::endl;
      parse_reference_type(id, indent);
      std::cerr << "done reference pointer type" << std::endl;
    }
    else if(tag == "DW_TAG_array_type"){
      std::cerr << "parsing array type" << std::endl;
      parse_array_type(id, indent);
      std::cerr << "done parsing array type" << std::endl;
    }
    else{
      std::cerr << "tag '" << tag << "' is not a type tag." << std::endl;
      abort();
    }
  }

  void parse_base_type(const uint64_t id, const int indent){
    std::string name = "123illegal name";
    uint64_t size = -1;

    std::cerr << "PARSE_BASE_TYPE: " << std::hex << id << std::dec << std::endl;

    while(true){
      // Read input line, skip if not a DW_* tag.
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): eof reached in DW_TAG_base_type element." << std::endl;
        abort();
      }

      // Tokenize the input.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the element has ended.
      RootInfo info(tokens);
      if(info.good()){
        // The base-type element is pretty simple.
        if(info.indent() > indent){
          std::cerr << "fatal error (" << buffer.line() << "): found subelement within DW_TAG_base_type element." << std::endl;
          abort();
        }

        // Put the input line back on the buffer.
        buffer.unget(line);

        // Create a protobuffer for the base type and insert it in the
        // type table.
        Type *t = types.add_type();
        t->set_name(name);
        t->set_id(id);

        Type::BaseType *b = t->mutable_base_type();
        b->set_size(size);

        return;
      }
      else{
        if(tokens[0] == "DW_AT_byte_size"){
          // Read out the byte size of the type (which is written out
          // in hex).
          std::stringstream ss(tokens[1]);
          ss >> std::hex >> size;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
        }
        else if(tokens[0] == "DW_AT_name"){
          // Strip off the leading and trailing quotation marks.
          //
          // name = tokens[1].substr(1, tokens[1].length() - 2);
          name = strip_enclosing(tokens[1]);
        }
      }
    }
  }

  void parse_class_type(const uint64_t id, const int indent, const std::string& prefix){
    std::string name = "";
    uint64_t size = -1;

    while(true){
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): eof reached in DW_TAG_class_type element." << std::endl;
        abort();
      }

      // Tokenize the input.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the element has ended.
      RootInfo info(tokens);
      if(info.good()){
        // Either put back the input line and return, or skip the
        // subelement encountered.
        if(info.indent() <= indent){
          // Push this line of input back.
          buffer.unget(line);

          // Create a class type object for the type table, then return.
          Type *t = types.add_type();
          t->set_name(prefix + name);
          t->set_id(id);

          Type::ClassType *c = t->mutable_class_type();
          c->set_size(size);

          return;
        }
        else{
          if(info.tag() == "DW_TAG_class_type"){
            // If there are nested class types, we want to capture
            // them.  The prefix of such a type will be the supplied
            // prefix, joined to the name of the enclosing class with
            // the scope operator ("::").
            if(name == ""){
              std::cerr << "fatal error (" << buffer.line() << "): class tag supplies no name, but has a nested class type." << std::endl;
              abort();
            }
            parse_class_type(info.id(), info.indent(), prefix + name + "::");
          }
          else{
            // Skip all other tag types.
            //
            // For more nuanced analysis, replace this function with a
            // class-element analysis method.
            skip_subelement(info.indent());
          }
        }
      }

      // Process the encountered line.
      if(tokens[0] == "DW_AT_name"){
        // Strip off leading/trailing quotation marks.
        name = strip_enclosing(tokens[1]);
      }
      else if(tokens[0] == "DW_AT_byte_size"){
        // Read out size of type (in hex).
        std::stringstream ss(tokens[1]);
        ss >> std::hex >> size;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
      }
      else{
        // This line is intentionally blank.
        continue;
      }
    }
  }

  void parse_typedef_type(const uint64_t id, const int indent, const std::string& prefix, bool name_needed = true){
    uint64_t type_id = -1;
    std::string name = "";

    while(true){
      // Read input line, skip if not a DW_* tag.
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): eof reached in DW_TAG_typedef element." << std::endl;
        abort();
      }

      // Tokenize the input.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the element has ended.
      RootInfo info(tokens);
      if(info.good()){
        if(info.indent() > indent){
          if(name_needed){
            std::cerr << "fatal error (" << buffer.line() << "): found subelement within DW_TAG_typedef element." << std::endl;
            abort();
          }
          else{
            skip_subelement(info.indent());
          }
        }
        else{
          // Put the input line back on the buffer.
          buffer.unget(line);
        }

        // Check that all attributes were set.
        if((name_needed and (name == "")) or (type_id == static_cast<uint64_t>(-1))){
          std::cerr << "warning (" << buffer.line() << "): typedef attributes (name, type) not all specified - continuing" << std::endl;
          return;
        }

        // Create a protobuffer for the base type and insert it in the
        // type table.
        Type *t = types.add_type();
        t->set_name(prefix + (name_needed ? name : "unknown_name"));
        t->set_id(id);

        Type::TypedefType *r = t->mutable_typedef_type();
        r->set_ref(type_id);

        return;
      }
      else{
        if(tokens[0] == "DW_AT_type"){
          // Read out the type id of the referred type (which is
          // written out in hex).
          std::stringstream ss(strip_enclosing(tokens[1]));
          ss >> std::hex >> type_id;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
        }
        else if(tokens[0] == "DW_AT_name"){
          name = strip_enclosing(concat(tokens,1));
        }
        else if(tokens[0] == "DW_AT_decl_file" or tokens[0] == "DW_AT_decl_line"){
          // This block is intentionally blank.
        }
        else if(tokens[0] == "DW_AT_sibling"){
          // This block is intentionally blank.
        }
        else{
          std::cerr << "fatal error (" << buffer.line() << "): unknown DW tag '" << tokens[0] << "' in element DW_TAG_typedef" << std::endl;
          abort();
        }
      }
    }
  }

  void parse_const_type(const uint64_t id, const int indent){
    uint64_t type_id = -1;

    while(true){
      // Read input line, skip if not a DW_* tag.
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): eof reached in DW_TAG_const_type element." << std::endl;
        abort();
      }

      // Tokenize the input.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the element has ended.
      RootInfo info(tokens);
      if(info.good()){
        if(info.indent() > indent){
          std::cerr << "fatal error (" << buffer.line() << "): found subelement within DW_TAG_const_type element." << std::endl;
          abort();
        }

        // Put the input line back on the buffer.
        buffer.unget(line);

        // Create a protobuffer for the base type and insert it in the
        // type table.
        Type *t = types.add_type();
        t->set_name("");
        t->set_id(id);

        Type::ConstType *c = t->mutable_const_type();
        c->set_ref(type_id);

        return;
      }
      else{
        if(tokens[0] == "DW_AT_type"){
          // Read out the type id of the referred type (which is
          // written out in hex).
          std::stringstream ss(strip_enclosing(tokens[1]));
          ss >> std::hex >> type_id;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
        }
        else{
          std::cerr << "fatal error (" << buffer.line() << "): unknown DW tag '" << tokens[0] << "' in element DW_TAG_const_type" << std::endl;
          abort();
        }
      }
    }
  }

  void parse_pointer_type(const uint64_t id, const int indent){
    uint64_t type_id = -1;
    uint64_t size = -1;

    while(true){
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): eof reached in DW_TAG_pointer_type element." << std::endl;
        abort();
      }

      // Tokenize the input.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the element has ended.
      RootInfo info(tokens);
      if(info.good()){
        // Either put back the input line and return, or skip the
        // subelement encountered.
        if(info.indent() <= indent){
          // Push this line of input back.
          buffer.unget(line);

          // Create a class type object for the type table, then return.
          Type *t = types.add_type();
          t->set_name("");
          t->set_id(id);

          Type::PointerType *p = t->mutable_pointer_type();
          p->set_size(size);
          p->set_ref(type_id);

          return;
        }
        else{
          // For more nuanced analysis, replace this function with a
          // class-element analysis method.
          skip_subelement(info.indent());
        }
      }

      // Process the encountered line.
      if(tokens[0] == "DW_AT_type"){
        // Read out the referred type id (in hex).
        std::stringstream ss(strip_enclosing(tokens[1]));
        ss >> std::hex >> type_id;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
      }
      else if(tokens[0] == "DW_AT_byte_size"){
        // Read out size of type (in hex).
        std::stringstream ss(tokens[1]);
        ss >> std::hex >> size;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
      }
      else{
        // This line is intentionally blank.
      }
    }
  }

  void parse_reference_type(const uint64_t id, const int indent){
    uint64_t type_id = -1;
    uint64_t size = -1;

    while(true){
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): eof reached in DW_TAG_reference_type element." << std::endl;
        abort();
      }

      // Tokenize the input.
      std::vector<std::string> tokens = MTV::tokenize(line);

      // Check to see if the element has ended.
      RootInfo info(tokens);
      if(info.good()){
        // Either put back the input line and return, or skip the
        // subelement encountered.
        if(info.indent() <= indent){
          // Push this line of input back.
          buffer.unget(line);

          // Create a class type object for the type table, then return.
          Type *t = types.add_type();
          t->set_name("");
          t->set_id(id);

          Type::ReferenceType *r = t->mutable_reference_type();
          r->set_size(size);
          r->set_ref(type_id);

          return;
        }
        else{
          // For more nuanced analysis, replace this function with a
          // class-element analysis method.
          skip_subelement(info.indent());
        }
      }

      // Process the encountered line.
      if(tokens[0] == "DW_AT_type"){
        // Read out the referred type id (in hex).
        std::stringstream ss(strip_enclosing(tokens[1]));
        ss >> std::hex >> type_id;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
      }
      else if(tokens[0] == "DW_AT_byte_size"){
        // Read out size of type (in hex).
        std::stringstream ss(tokens[1]);
        ss >> std::hex >> size;
          if(!ss){
            std::cerr << "fatal error (" << buffer.line() << "): could not parse value." << std::endl;
            exit(1);
          }
      }
      else{
        continue;
      }
    }
  }

  void parse_array_type(const uint64_t id, const int indent){
    // TODO(choudhury): make this function work for real.
    //
    // Right now, it just skips lines until the element ends (i.e.,
    // ignores array types).
    while(true){
      std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      std::vector<std::string> tokens = MTV::tokenize(line);

      RootInfo info(tokens);
      if(info.good() and info.indent() <= indent){
        buffer.unget(line);
        return;
      }
    }
  }

  void skip_subelement(const int indent){
    while(true){
      const std::string line = buffer.get();
      if(!is_dw_tag(line)){
        continue;
      }

      if(buffer.eof()){
        std::cerr << "fatal error (" << buffer.line() << "): encountered EOF while skipping a subelement." << std::endl;
        abort();
      }

      std::vector<std::string> tokens = MTV::tokenize(line);
      RootInfo info(tokens);
      if(info.good() and info.indent() <= indent){
        buffer.unget(line);
        return;
      }
    }
  }

  static bool is_dw_tag(const std::string& line){
    // Checks to see whether "DW_" is a substring of the line.
    return line.find("DW_") != std::string::npos;
  }

  static bool is_at_tag(const std::string& line){
    // Checks to see whether "AT_" is a substring of the line.
    return line.find("AT_") != std::string::npos;
  }

  static bool is_type_tag(const std::string& tag){
    return (tag == "DW_TAG_base_type" or
            tag == "DW_TAG_class_type" or tag == "DW_TAG_structure_type" or
            tag == "DW_TAG_typedef" or tag == "DW_TAG_subroutine_type" or
            tag == "DW_TAG_const_type" or
            tag == "DW_TAG_pointer_type" or
            tag == "DW_TAG_reference_type" or
            tag == "DW_TAG_array_type");
  }

  static std::string remove_spaces(const std::vector<std::string>& tokens){
    std::string result = "";

    for(unsigned i=0; i<tokens.size(); i++){
      result += tokens[i];
    }

    return result;
  }

  static std::string concat(const std::vector<std::string>& tokens, unsigned start = 0, unsigned end = std::numeric_limits<unsigned>::max()){
    std::string result = tokens[start];

    if(end > tokens.size()){
      end = tokens.size();
    }

    for(unsigned i=start+1; i<end; i++){
      result += " ";
      result += tokens[i];
    }
    return result;
  }

  static std::string strip_enclosing(const std::string& s){
    return s.substr(1, s.length() - 2);
  }

private:
  LineBuffer buffer;
  std::istream& in;

  TypeList types;
  VariableList vars;
  SubprogramList subprograms;
  LexicalBlockList scopes;
};

int main(int argc, char *argv[]){
  std::string inputfile, outputfile;
  bool print;

  try{
    TCLAP::CmdLine cmd("Parses a dwarf dump file for use in memory analysis.");

    TCLAP::ValueArg<std::string> inputfileArg("i",
                                              "input-file",
                                              "File containing dwarf dump.",
                                              false,
                                              "",
                                              "filename",
                                              cmd);

    TCLAP::ValueArg<std::string> outputfileArg("o",
                                               "output-file",
                                               "File to write protocol buffer to.",
                                               false,
                                               "",
                                               "filename",
                                               cmd);

    TCLAP::SwitchArg printArg("p",
                              "print",
                              "Whether to print the results of parsing",
                              cmd,
                              false);

    cmd.parse(argc, argv);

    inputfile = inputfileArg.getValue();
    outputfile = outputfileArg.getValue();
    print = printArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open the input file.
  if(inputfile == ""){
    inputfile = "/dev/stdin";
  }
  std::ifstream in(inputfile.c_str());
  if(!in){
    std::cerr << "error: could not open file '" << inputfile << "' for reading." << std::endl;
    exit(1);
  }

  // Open the output file.
  std::ofstream out;
  if(outputfile == ""){
    std::cerr << "warning: no output file specified." << std::endl;
  }

  DwarfParser parser(in);
  parser.parse();

  if(print){
    parser.print();
  }

  if(outputfile != ""){
    if(!parser.save(outputfile)){
      std::cerr << "error: could not save protocol buffer to file '" << outputfile << "'." << std::endl;
      exit(1);
    }
  }

  return 0;
}
