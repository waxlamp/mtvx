// MTV headers.
#include <Core/Type.pb.h>
#include <Core/Variable.pb.h>
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Dataflow/TraceWriter.h>
#include <Core/Util/Util.h>
#include <Tools/ReferenceTrace/StackInfo.h>
using MTV::AddressRangePass;
using MTV::StackInfo;
using MTV::TraceReader;
using MTV::TraceWriter;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <string>

int main(int argc, char *argv[]){
  std::string inputfile, outputfile, regfile, mithrilfile;
  bool crop_prelude, crop_epilogue;
  try{
    // Command line parser.
    TCLAP::CmdLine cmd("Filter away non-relelvant addresses in a reference trace.");

    // Input file.
    TCLAP::ValueArg<std::string> inputfile_arg("i",
                                              "input",
                                              "input reference trace file",
                                              true,
                                              "",
                                              "filename",
                                              cmd);

    // Output file.
    TCLAP::ValueArg<std::string> outputfile_arg("o",
                                               "output",
                                               "output reference trace file",
                                               true,
                                               "",
                                               "filename",
                                               cmd);

    // Registration file for filtering statically known addresses.
    TCLAP::ValueArg<std::string> regfile_arg("r",
                                            "registration-file",
                                            "Registration file by which to filter.",
                                            false,
                                            "",
                                            "filename",
                                            cmd);

    // Mithril stack-info file for filtering runtime stack addresses.
    TCLAP::ValueArg<std::string> mithrilfile_arg("m",
                                                "mithril-file",
                                                "Mithril file by which to filter.",
                                                false,
                                                "",
                                                "filename",
                                                cmd);

    // Switch for filtering away the prelude.
    TCLAP::SwitchArg crop_prelude_arg("p",
                                      "crop-prelude",
                                      "Skips the prelude - the addresses before the first stack manipulation or line number record.",
                                      cmd,
                                      false);

    // Switch for filtering away the epilogue.
    TCLAP::SwitchArg crop_epilogue_arg("e",
                                       "crop-epilogue",
                                       "Skips the epilogue - the addresses after the matching FunctionExit to the first FunctionEntry",
                                       cmd,
                                       false);

    // Parse the command line.
    cmd.parse(argc, argv);

    // Grab the values.
    inputfile = inputfile_arg.getValue();
    outputfile = outputfile_arg.getValue();
    regfile = regfile_arg.getValue();
    mithrilfile = mithrilfile_arg.getValue();
    crop_prelude = crop_prelude_arg.getValue();
    crop_epilogue = crop_epilogue_arg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open the trace file.
  TraceReader::ptr reader = boost::make_shared<TraceReader>();
  if(!reader->open(inputfile)){
    std::cerr << "error: could not open input trace file '" << inputfile << "'." << std::endl;
    exit(1);
  }

  // Open a trace writer.
  TraceWriter::ptr writer = boost::make_shared<TraceWriter>(TraceWriter::Raw);
  if(!writer->open(outputfile)){
    std::cerr << "error: could not open output file '" << outputfile << "'." << std::endl;
    exit(1);
  }

  // Open the registration file (if any), and create a range pass
  // object.
  AddressRangePass regions;
  if(regfile != ""){
    // Parse out the registration information.
    MTR::RegionRegistration reg;
    reg.read(regfile.c_str());
    if(reg.hasError()){
      std::cerr << "error: " << reg.error() << std::endl;
      exit(1);
    }

    for(unsigned i=0; i<reg.arrayRegions.size(); i++){
      regions.addRange(reg.arrayRegions[i].base, reg.arrayRegions[i].base + reg.arrayRegions[i].size);
    }
  }
  else{
    std::cerr << "warning: no registration file specified." << std::endl;
  }

  // If there is a mithril file, open it and process its contents.
  //
  // TODO(choudhury): this function should be abstracted into a header
  // file so it can be called from waxlamp.cpp.
  boost::unordered_map<uint64_t, MTV::Mithril::Type> type_table;
  boost::unordered_map<uint64_t, std::vector<MTV::Mithril::Variable> > var_entry, var_exit;
  if(mithrilfile != ""){
    std::ifstream mithril(mithrilfile.c_str());
    if(!mithril){
      std::cerr << "error: could not open file '" << mithrilfile << "' for reading." << std::endl;
      exit(1);
    }

    // Read out the type information.
    MTV::Mithril::TypeList types;
    MTV::readProtocolBuffer(mithril, types);
    
    // Read out the variable information.
    MTV::Mithril::VariableList vars;
    MTV::readProtocolBuffer(mithril, vars);

    // // Read out the subprogram information.
    // MTV::Mithril::SubprogramList subprogs;
    // MTV::readProtocolBuffer(mithril, subprogs);

    // // Read out the lexical scope information.
    // MTV::Mithril::LexicalBlockList scopes;
    // MTV::readProtocolBuffer(mithril, scopes);
    
    if(!mithril){
      std::cerr << "error: could not read out data from file '" << mithrilfile << "'" << std::endl;
      exit(1);
    }

    // Make a table of the types.
    for(int i=0; i<types.type_size(); i++){
      const MTV::Mithril::Type& t = types.type(i);
      type_table[t.id()] = t;
    }

    // Create two tables mapping from address to variable - one for
    // ENTRY addresses and one for EXIT.  Each table maps a lexical
    // block start/end address to a list of variables that
    // started/ended their influence at that address.  When such
    // variables are introduced into the visualization, their address
    // will be computed from the current value of the frame base
    // pointer (which also is reported in the trace).
    for(int i=0; i<vars.var_size(); i++){
      const MTV::Mithril::Variable& v = vars.var(i);
      var_entry[v.low_pc()].push_back(v);
      var_exit[v.high_pc()].push_back(v);
    }
  }
  else{
    std::cerr << "warning: no mithril file specified." << std::endl;
  }

  // If prelude is being cropped, advance the trace file until the
  // first non-memory record.
  //
  // NOTE(choudhury): initialize the rec variable with the first
  // record from the trace in case we are NOT cropping the prelude -
  // it is needed in the bulk-processing loop below.
  MTR::Record rec = reader->nextRecord();
  if(crop_prelude){
    std::cerr << "info: cropping prelude." << std::endl;
    while(MTR::type(rec) == MTR::Record::MType){
      rec = reader->nextRecord();
    }
  }

  // Process the bulk of the trace.  Exit the loop when either (1)
  // end-of-trace or (2) a balancing FunctionExit to the initial
  // FunctionEntry comes.
  uint64_t init_func = 0x0;
  std::stack<uint64_t> frame_base;
  std::vector<boost::unordered_map<uint64_t, AddressRangePass> > vars;

  std::cerr << "info: processing bulk." << std::endl;

  try{
    while(true){
      std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec.code << std::endl;

      // The next record to process is in the "rec" variable already, so
      // process it.
      if(rec.code == MTR::Record::LineNumber){
        // If it is a line number record, copy it to the output
        // immediately.
        writer->consume(rec);
        std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;
      }
      else if(MTR::type(rec) == MTR::Record::MType){
        // If it is a memory record, it must pass one of the filters in
        // order to be copied to the output.
        bool copy = false;
        if(regions.test(rec)){
          // First check the statically known addresses.
          copy = true;
        }
        else{
          // If the address fails the statically known ranges, then test
          // it against the current stack addresses.
          //
          // NOTE(choudhury): go in reverse order, because typical
          // programs refer mostly to variables in the current stack
          // frame.
          if(vars.size() > 0){
            for(int i=static_cast<int>(vars.size()) - 1; i>=0; i--){
              for(boost::unordered_map<uint64_t, AddressRangePass>::iterator j = vars[i].begin();
                  j != vars[i].end();
                  j++){
                if(j->second.test(rec)){
                  copy = true;
                  goto breakout;
                }
              }
            }
          }
        }

      breakout:
        if(copy){
          writer->consume(rec);
          std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;
        }
      }
      else if(rec.code == MTR::Record::FunctionEntry){
        // Push a zero on the frame base stack - it will be replaced
        // with a correct value in a FramePointer event.
        frame_base.push(0);
        vars.push_back(boost::unordered_map<uint64_t, AddressRangePass>());

        // Copy the entry to the output.
        writer->consume(rec);
        std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;

        // Special case: if the initial function address is 0x0, then
        // this represents the initial function entry - record the
        // address.
        if(init_func == 0x0){
          init_func = rec.addr;
        }
      }
      else if(rec.code == MTR::Record::FunctionExit){
        // Pop both the frame base stack and the vars stack, since the
        // function is ending.
        frame_base.pop();
        vars.pop_back();

        // Copy the record to the output.
        writer->consume(rec);
        std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;

        // Special case: if the address matches the initial function
        // address, then we've reached the end of the bulk section.
        if(rec.addr == init_func){
          break;
        }
      }
      else if(rec.code == MTR::Record::ScopeEntry){
        // A new lexical scope - create an address filter for each new
        // local variable and place them in the set at the top of the
        // "vars" stack (which represents all variables in the current
        // function).
        //
        // Get an iterator to the vector containing the vars list for
        // this address.
        boost::unordered_map<uint64_t, std::vector<MTV::Mithril::Variable> >::const_iterator i = var_entry.find(rec.addr);
        if(i != var_entry.end()){
          const std::vector<MTV::Mithril::Variable>& varlist = i->second;
          for(unsigned j=0; j<varlist.size(); j++){
            // Compute the size of the variable.
            unsigned num;
            const uint32_t type = StackInfo::get_type_size(type_table, varlist[j].type(), num);

            // Compute its base address.
            uint64_t base;
            if(varlist[j].has_stack_location()){
              base = frame_base.top() + varlist[j].stack_location().offset();
            }
            else if(varlist[j].has_absolute_location()){
              base = varlist[j].absolute_location().address();
            }
            else{
              std::cerr << "fatal error: variable has no address locator method" << std::endl;
              abort();
            }

            // Grab its id.
            const uint64_t id = varlist[j].id();

            // Create an address range filter to represent the
            // variable's addresses.
            AddressRangePass pass(base, base + num*type);

            // Place this filter in the table on top of the vars stack.
            if(vars.back().find(id) != vars.back().end()){
              std::cerr << "fatal error: variable (id " << id << ") already present in vars table." << std::endl;
              abort();
            }
            vars.back()[id] = pass;
          }

          // Copy the record to the output.
          writer->consume(rec);
          std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;
        }
      }
      else if(rec.code == MTR::Record::ScopeExit){
        // Get the set of variables ending on this address and remove
        // them from the table at the top of the vars stack.
        boost::unordered_map<uint64_t, std::vector<MTV::Mithril::Variable> >::const_iterator i = var_exit.find(rec.addr);      
        if(i != var_exit.end()){
          const std::vector<MTV::Mithril::Variable>& varlist = i->second;
          for(unsigned j=0; j<vars.size(); j++){
            // Remove the variable from the top table of the vars stack.
            const uint64_t id = varlist[j].id();
            vars.back().erase(id);
          }
        }

        // Copy the record to the output.
        writer->consume(rec);
        std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;
      }
      else if(rec.code == MTR::Record::FramePointer){
        // Set the frame base top value, after checking for illegal
        // state.
        if(frame_base.top() != 0){
          std::cerr << "fatal error: top of frame base stack was non-zero when FramePointer event occurred." << std::endl;
          abort();
        }
        frame_base.top() = rec.addr;

        // Copy the record to the output.
        writer->consume(rec);
        std::cerr << reader->getTracePoint() << ", " << __LINE__ << ": " << rec << std::endl;
      }

      // Read out the next record.
      rec = reader->nextRecord();
    }
  }
  catch(TraceReader::End){
    std::cerr << "info: end-of-trace occurred while processing bulk of trace." << std::endl;
    writer->flush();
    exit(0);
  }

  // If we are not skipping the epilogue, copy it to the output.
  if(not crop_epilogue){
    std::cerr << "info: copying epilogue to output." << std::endl;
    try{
      rec = reader->nextRecord();
      writer->consume(rec);
    }
    catch(TraceReader::End){}
  }
  else{
    std::cerr << "info: cropping epilogue." << std::endl;
  }

  // Clean up and exit.
  writer->flush();
  return 0;
}

