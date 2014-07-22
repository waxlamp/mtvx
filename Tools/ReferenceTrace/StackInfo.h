// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// StackInfo.h - Defines containers for dealing with stack info data
// (as output by the "mithril" program).

#ifndef STACK_INFO_H
#define STACK_INFO_H

// MTV headers.
#include <Core/LexicalBlock.pb.h>
#include <Core/Subprogram.pb.h>
#include <Core/Type.pb.h>
#include <Core/Variable.pb.h>
#include <Core/Util/Boost.h>
#include <Core/Util/Util.h>

// System headers.
#include <fstream>
#include <stack>
#include <string>
#include <vector>

namespace MTV{
  struct StackInfo{
    struct FrameBaseLocator{
      std::string reg;
      int offset;
    };

    // Set of addresses at which functions begin - use this to push a
    // new value on the frame base value stack.
    boost::unordered_set<uint64_t> function_entry;

    // The set of addresses at which functions are exiting.
    boost::unordered_set<uint64_t> function_exit;

    // Maps from "frame entry PCs" to a frame locator method.
    boost::unordered_map<uint64_t, FrameBaseLocator> frame_entry;

    // Tracks addresses marking scope entry.  Maps to the number of
    // scopes starting on that address.
    boost::unordered_map<uint64_t, unsigned> scope_entry;

    // Tracks addresses marking scope exit.  Maps to the number of
    // scopes ending on that address.
    boost::unordered_map<uint64_t, unsigned> scope_exit;

    // Keeps track of the frame base pointers as functions are called
    // and then exit.
    std::stack<uint64_t> frame_base;

    // Keeps track of the changing scopes from both function calls and
    // from nested scopes within a function.
    std::stack<std::stack<uint64_t> > scopes;

    static uint32_t get_type_size(const boost::unordered_map<uint64_t, MTV::Mithril::Type>& types, uint64_t id, unsigned& howmany){
      boost::unordered_map<uint64_t, MTV::Mithril::Type>::const_iterator i = types.find(id);
      if(i == types.end()){
        return 0;
      }

      // All the non-array types report a single "item".
      howmany = 1;

      const MTV::Mithril::Type& t = i->second;
      if(t.has_base_type()){
        return t.base_type().size();
      }
      else if(t.has_class_type()){
        return t.class_type().size();
      }
      else if(t.has_typedef_type()){
        return get_type_size(types, t.typedef_type().ref(), howmany);
      }
      else if(t.has_const_type()){
        return get_type_size(types, t.const_type().ref(), howmany);
      }
      else if(t.has_pointer_type()){
        return t.pointer_type().size();
      }
      else if(t.has_reference_type()){
        return t.reference_type().size();
      }
      else if(t.has_array_type()){
        // return t.array_type().size() * get_type_size(types, t.array_type().ref());

        // For array types, the size contained in the size field is
        // actually the length of the array.
        howmany = t.array_type().size();
        unsigned dummy;
        return get_type_size(types, t.array_type().ref(), dummy);
      }
      else{
        abort();
      }
    }

    // Populate the struct from a mithril dump file.
    bool populate(const std::string& filename){
      // Open the file.
      std::ifstream in(filename.c_str());
      if(!in){
        return false;
      }

      // int size;

      // Read out the type information.
      MTV::Mithril::TypeList types;
      MTV::readProtocolBuffer(in, types);
    
      // Read out the variable information.
      MTV::Mithril::VariableList vars;
      MTV::readProtocolBuffer(in, vars);

      // Read out the subprogram information.
      MTV::Mithril::SubprogramList subprogs;
      MTV::readProtocolBuffer(in, subprogs);

      // Read out the lexical scope information.
      MTV::Mithril::LexicalBlockList scopes;
      MTV::readProtocolBuffer(in, scopes);

      if(!in){
        return false;
      }

      // Close the file.
      in.close();

      // Make a table of type sizes.
      //
      // boost::unordered_map<uint64_t, MTV::Mithril::Type> typetable;
      // for(int i=0; i<types.type_size(); i++){
      //   const MTV::Mithril::Type& t = types.type(i);
      //   typetable[t.id()] = t;
      // }

      // Populate the function entry/exit structures.
      for(int i=0; i<subprogs.subprogram_size(); i++){
        const MTV::Mithril::Subprogram& s = subprogs.subprogram(i);

        std::cerr << "subprogram " << s.name() << std::endl;

        // std::cerr << "checking for duplicates..." << std::endl;

        // Check for duplicates.
        if(function_entry.find(s.low_pc()) != function_entry.end()){
          std::cerr << "fatal error: function entry address 0x" << std::hex << s.low_pc() << " already in entry table." << std::endl;
          abort();
        }
        function_entry.insert(s.low_pc());

        if(function_exit.find(s.high_pc()) != function_exit.end()){
          std::cerr << "fatal error: function exit address 0x" << std::hex << s.low_pc() << " already in exit table." << std::endl;
          abort();
        }
        function_exit.insert(s.high_pc());

        // std::cerr << "ok" << std::endl;
      }

      // Populate the frame entry/exit tables.
      for(int i=0; i<subprogs.subprogram_size(); i++){
        const MTV::Mithril::Subprogram& s = subprogs.subprogram(i);

        // const uint64_t subprog_low_pc = s.low_pc();
        // const uint64_t subprog_high_pc = s.high_pc();

        // The locator addresses are globally contiguous, so keep the
        // base address handy for calculating proper offsets from the
        // function's low PC address.
        if(s.loclist().locator_size() > 0){
          // const uint64_t base_addr = s.loclist().locator(0).low_pc();
          const uint64_t base_addr = s.loclist().cu_low_pc();

          for(int j=0; j<s.loclist().locator_size(); j++){
            const MTV::Mithril::Locator& loc = s.loclist().locator(j);

            // const uint64_t frame_low_pc = subprog_low_pc + (loc.low_pc() - base_addr);
            const uint64_t frame_low_pc = base_addr + loc.low_pc();

            // Emplace the low pc value, mapping it to the method for
            // computing the frame base address.
            //
            // Check to make sure the specified addresses are not
            // already in the tables.
            if(frame_entry.find(frame_low_pc) != frame_entry.end()){
              std::cerr << "fatal error: function " << s.name() << " introducing duplicate frame low_pc: 0x" << std::hex << loc.low_pc() << std::endl;
              abort();
            }

            // Create a frame base locator and populate it.
            FrameBaseLocator& fbl = frame_entry[frame_low_pc];
            fbl.reg = loc.reg();
            fbl.offset = loc.offset();

            // // Similar process for high pc.
            // //
            // // Check for duplicates.
            // if(frame_exit.find(frame_high_pc) != frame_exit.end()){
            //   std::cerr << "fatal error: function " << s.name() << " introducing duplicate frame low_pc: 0x" << std::hex << loc.high_pc() << std::endl;
            //   abort();
            // }

            // // Place the exit address into the set.
            // frame_exit.insert(frame_high_pc);
          }
        }
      }

      // Place the lexical scopes into the scope tables.
      for(int i=0; i<scopes.lexical_block_size(); i++){
        const MTV::Mithril::LexicalBlock& l = scopes.lexical_block(i);

        // Increment the counter for both the low and high PC.  These
        // counts will be used later to determine how many times to
        // push/pop the stack of scopes during trace generation.
        scope_entry[l.low_pc()]++;
        scope_exit[l.high_pc()]++;
      }

      // // Place the variables into the scope tables.
      // for(int i=0; i<vars.var_size(); i++){
      //   const MTV::Mithril::Variable& v = vars.var(i);

      //   // Get the size of the type.
      //   //
      //   // TODO(choudhury): the sizing stuff will be useful for
      //   // waxlamp when it generates motion data.
      //   //
      //   // const uint32_t size = get_type_size(typetable, v.type());

      //   // Grab the low and high PCs for the variable.
      //   const uint64_t low_pc = v.low_pc();
      //   const uint64_t high_pc = v.high_pc();

      //   // Add the variable to the list of "activations" for the
      //   // appropriate scope.
      //   //
      //   // scope_entry[low_pc].push_back(Variable(v.name(), size, file, line, v.has_stack_location() ? StackLocator(v.stack_location().offset()) : AbsoluteLocator(v.absolute_location().address())));
      //   scope_entry.insert(low_pc);

      //   // Also check for consistency in the scope exit table.
      //   //
      //   // If there is as yet no entry, insert one based on this
      //   // variable; otherwise, make sure the expected value matches
      //   // what is already there.
      //   if(scope_exit.find(high_pc) == scope_exit.end()){
      //     scope_exit.insert(high_pc);
      //   }
      //   else{
      //     std::cerr << "fatal error: scope exit PC 0x" << std::hex << high_pc << " duplicated." << std::endl;
      //     abort();
      //   }
      // }

      return true;
    }

    void print(){
      std::cout << "Function entry PCs" << std::endl;
      std::cout << std::hex;
      for(boost::unordered_set<uint64_t>::const_iterator i = function_entry.begin(); i != function_entry.end(); i++){
        std::cout << "0x" << *i << std::endl;
      }
    }
  };

}

#endif
