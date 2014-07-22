// Copyright 2010 A.N.M. Imroz Choudhury
//
// aeon.cpp - A pintool (http://pintool.org) for capturing memory
// reference traces.

// Pin headers.
#include "pin.H"

// MTV headers.
#include <Core/Dataflow/TraceWriter.h>
#include <Core/Util/BoostForeach.h>
#include <Tools/ReferenceTrace/mtrtools.h>
#include <Tools/ReferenceTrace/StackInfo.h>
using MTV::StackInfo;
using MTV::TraceWriter;

// Boost headers.
#include <boost/unordered_map.hpp>

// System headers.
#include <iostream>
#include <fstream>
#include <stdexcept>

// Global trace writer object to be used by instrumentation routines.
TraceWriter::ptr writer;

// Command line switches.
KNOB<string> outfile(KNOB_MODE_WRITEONCE, "pintool", "o", "reftrace", "specify output file name");
KNOB<string> linemap(KNOB_MODE_WRITEONCE, "pintool", "l", "", "specify lineno map file");
KNOB<string> encoding(KNOB_MODE_WRITEONCE, "pintool", "e", "raw", "specify encoding (raw or gzip)");
KNOB<string> stackinfo(KNOB_MODE_WRITEONCE, "pintool", "s", "", "specify stack info file (output from mithril)");

// Maps for holding data during instrumentation.
typedef boost::unordered_map<MTR::addr_t, std::pair<unsigned, unsigned> > Linetable;
typedef boost::unordered_map<std::string, unsigned> Filetable;
Linetable lineno;
Filetable filetable;
StackInfo stack_info;

VOID record_memory_read(VOID *ea){
  MTR::Record rec;
  rec.code = MTR::Record::Read;
  rec.addr = reinterpret_cast<MTR::addr_t>(ea);

  // writer->addRecord(rec);
  writer->consume(rec);
}

VOID record_memory_write(VOID *ea){
  MTR::Record rec;
  rec.code = MTR::Record::Write;
  rec.addr = reinterpret_cast<MTR::addr_t>(ea);

  // writer->addRecord(rec);
  writer->consume(rec);
}

VOID record_lineno_info(UINT32 file, UINT32 line){
  MTR::Record rec;
  rec.code = MTR::Record::LineNumber;
  rec.file = file;
  rec.line = line;

  // std::cerr << "file: " << file << ", line: " << line << std::endl;

  // writer->addRecord(rec);
  writer->consume(rec);
}

VOID scope_entry(UINT64 addr, unsigned num){
  std::cerr << "entering scope at 0x" << std::hex << addr << std::dec << std::endl;

  for(unsigned i=0; i<num; i++){
    stack_info.scopes.top().push(addr);
  }

  MTR::Record rec;
  rec.code = MTR::Record::ScopeEntry;
  rec.addr = addr;
  writer->consume(rec);
}

VOID scope_exit(UINT64 requested_addr, unsigned num){
  if(stack_info.scopes.size() == 0 or
     stack_info.scopes.top().size() == 0){
    std::cout << "empty stack on scope exit (requested address 0x" << std::hex << requested_addr << std::dec << "): not a true scope exit" << std::endl;
    return;
  }

  const uint64_t addr = stack_info.scopes.top().top();
  std::cerr << "exiting " << num << " scope(s) at 0x" << std::hex << addr << " (requested address 0x" << requested_addr << ")" << std::dec << std::endl;
  for(unsigned i=0; i<num and stack_info.scopes.top().size() > 0; i++){
    stack_info.scopes.top().pop();
  }

  MTR::Record rec;
  rec.code = MTR::Record::ScopeExit;
  rec.addr = addr;
  writer->consume(rec);
}

VOID function_entry(const std::string *name, UINT64 addr){
  std::cerr << "entering function " << *name << std::endl;

  stack_info.frame_base.push(0);
  stack_info.scopes.push(std::stack<uint64_t>());

  MTR::Record rec;
  rec.code = MTR::Record::FunctionEntry;
  rec.addr = addr;
  writer->consume(rec);

  std::cerr << "frame stack size: " << stack_info.frame_base.size() << std::endl;
  std::cerr << "scope stack size: " << stack_info.scopes.size() << std::endl;
}

VOID function_exit(const std::string *name, UINT64 addr){
  std::cerr << "exiting function " << *name << std::endl;

  // // Exit each current scope within the function (while recording the
  // // exits in the trace file).
  // while(stack_info.scopes.top().size() > 0){
  //   scope_exit(0, 1);
  // }

  // Remove the mini-stack from the stack.
  stack_info.scopes.pop();

  // Pop the frame base information.
  stack_info.frame_base.pop();

  // if(stack_info.frame_base.size() > 0){
  //   MTR::Record rec;
  //   rec.code = MTR::Record::FramePointer;
  //   rec.addr = stack_info.frame_base.top();

  //   writer->consume(rec);
  // }

  MTR::Record rec;
  rec.code = MTR::Record::FunctionExit;
  rec.addr = addr;
  writer->consume(rec);
}

// VOID frame_entry_bare(){
//   std::cerr << "entering frame" << std::endl;
// }

VOID frame_entry(CONTEXT *ctx, ADDRINT addr, const StackInfo::FrameBaseLocator *fbl){
  std::cerr << "entering frame at 0x" << std::hex << addr << std::dec << std::endl;
  std::cerr << "fbl: " << std::hex << fbl << std::dec << std::endl;
  std::cerr << "fbl->reg: " << fbl->reg << std::endl;

  // // DEBUG: print out all the relevant register values.
  // static const REG regs[] = {
  //   REG_RAX,
  //   REG_RBX,
  //   REG_RCX,
  //   REG_RDX,
  //   REG_RSI,
  //   REG_RDI,
  //   REG_RBP,
  //   REG_RSP
  // };

  // std::cerr << std::hex;
  // for(unsigned i=0; i<sizeof(regs)/sizeof(regs[0]); i++){
  //   const uint64_t val = PIN_GetContextReg(ctx, regs[i]);
  //   std::cerr << "reg " << i << ": 0x" << (val + fbl->offset) << std::endl;
  // }
  // std::cerr << std::dec;

  // Determine which symbolic register to use.
  REG reg;
  if(fbl->reg == "DW_OP_breg7"){
    reg = REG_RSP;
  }
  else if(fbl->reg == "DW_OP_breg6"){
    reg = REG_RBP;
  }
  else{
    std::cerr << "fatal error: unhandled register argument in locator object." << std::endl;
    abort();
  }

  std::cerr << "REG: " << reg << std::endl;

  // Retrieve the register value, add the offset, and store the result
  // in the top of the stack.
  const uint64_t frame_base = PIN_GetContextReg(ctx, reg) + fbl->offset;
  std::cerr << "frame_base: " << std::hex << frame_base << std::endl;
  if(stack_info.frame_base.size() == 0){
    std::cerr << "uh oh" << std::endl;
    return;
  }
  stack_info.frame_base.top() = frame_base;

  // Report the frame pointer.
  MTR::Record rec;
  rec.code = MTR::Record::FramePointer;
  rec.addr = frame_base;

  writer->consume(rec);
}

// VOID emit_pc(UINT64 addr){
//   MTR::Record rec;
//   rec.code = MTR::Record::ProgramCounter;
//   rec.addr = addr;

//   writer->consume(rec);
// }

int level = 0;

VOID up_level(){
  ++level;
}

VOID down_level(){
  --level;
}

VOID routine(RTN rtn, VOID *){
  RTN_Open(rtn);

  // INS ins = RTN_InsHead(rtn);
  // std::cout << "routine address: 0x" << std::hex << RTN_Address(rtn) << std::endl
  //           << "first address: 0x" << INS_Address(ins) << std::dec << std::endl;

  const INS ins = RTN_InsHead(rtn);
  const ADDRINT addr = INS_Address(ins);
  const std::string *func_name = &RTN_Name(rtn);
  if(stack_info.function_entry.find(addr) != stack_info.function_entry.end()){
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)up_level, IARG_END);
    RTN_InsertCall(rtn, IPOINT_BEFORE, (AFUNPTR)function_entry, IARG_PTR, func_name, IARG_ADDRINT, addr, IARG_END);

    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)down_level, IARG_END);
    RTN_InsertCall(rtn, IPOINT_AFTER, (AFUNPTR)function_exit, IARG_PTR, func_name, IARG_ADDRINT, addr, IARG_END);
  }

  RTN_Close(rtn);
}

VOID Fini(INT32, VOID *){
  std::cout << "final level: " << level << std::endl;
}

VOID instruction(INS ins, VOID *){
  // Some instructions may both read AND write memory, so we may
  // instrument instructions twice (no "else if").
  //
  // Some instructions are predicated - InsertPredicatedCall takes
  // this into account and only runs the analysis code if the
  // predicate exists and is true for the instruction being
  // instrumented.

  // Instrument read instructions.
  if(INS_IsMemoryRead(ins)){
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)record_memory_read,
                             IARG_MEMORYREAD_EA,
                             IARG_END);
  }

  // Instrument write instructions.
  if(INS_IsMemoryWrite(ins)){
    INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)record_memory_write,
                             IARG_MEMORYWRITE_EA,
                             IARG_END);
  }

  // Instrument all "line number" instructions.
  const uint64_t addr = INS_Address(ins);

  {
    // Linetable::const_iterator i = lineno.find(INS_Address(ins));
    Linetable::const_iterator i = lineno.find(addr);

    // if(INS_Address(ins) == 0x413e07){
    //   std::cerr << "INS_address: " << INS_Address(ins) << std::endl;
    //   if(i == lineno.end()){
    //     std::cerr << "NOOOOOO" << std::endl;
    //   }
    //   else{
    //     std::cerr << i->second.first << " -> " << i->second.second << std::endl;
    //   }
    // }

    if(i != lineno.end()){
      INS_InsertPredicatedCall(ins, IPOINT_BEFORE, (AFUNPTR)record_lineno_info,
                               IARG_UINT32, i->second.first,
                               IARG_UINT32, i->second.second,
                               IARG_END);
    }
  }

  // Instrument all function entry/exit and scope entry/exit
  // instructions. Each PC should be only one or the other (entry or
  // exit), but function entry/exit points may match up with scope
  // entry/exit points.
  //
  // Function entry/exit.
  //
  // boost::unordered_set<uint64_t>::const_iterator j;
  // j = stack_info.function_exit.find(addr);
  // if(j != stack_info.function_exit.end()){
  //   std::cerr << "exiting function at 0x" << std::hex << *j << std::dec << std::endl;

  //   if(stack_info.frame_base.size() == 0){
  //     std::cerr << "fatal error: frame base stack was empty upon encountering a function exit PC (0x" << std::hex << *j << ")" << std::endl;
  //     // abort();
  //   }
  //   // INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)function_exit, IARG_END);
  // }

  // j = stack_info.function_entry.find(addr);
  // if(j != stack_info.function_entry.end()){
  //   std::cerr << "entering function at 0x" << std::hex << *j << std::dec << std::endl;
  //   // INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)function_entry, IARG_END);
  // }
  // else{
  //   std::cout << "COULDN'T FIND ADDRESS 0x" << std::hex << addr << std::dec << std::endl;
  // }

  // TODO(choudhury): careful! this is the old else block that went
  // with the first of the if blocks immediately above.
  //
  // else{
  //   j = stack_info.function_entry.find(addr);
  //   if(j != stack_info.function_entry.end()){
  //     std::cerr << "entering function at 0x" << std::hex << *j << std::dec << std::endl;
  //     // INS_InsertCall(ins, IPOINT_AFTER, (AFUNPTR)function_entry, IARG_END);
  //   }
  //   else{
  //     std::cout << "COULDN'T FIND ADDRESS 0x" << std::hex << addr << std::dec << std::endl;
  //   }
  // }

  // Frame entry (no special action for frame exit).
  boost::unordered_map<uint64_t, StackInfo::FrameBaseLocator>::const_iterator i;
  i = stack_info.frame_entry.find(addr);
  if(i != stack_info.frame_entry.end()){
    const StackInfo::FrameBaseLocator& fbl = i->second;
    std::cerr << "fbl.reg = " << fbl.reg << std::endl
              << "fbl.offset = " << fbl.offset << std::endl;

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)frame_entry,
                   IARG_CONTEXT,
                   IARG_INST_PTR,
                   IARG_PTR, &fbl,
                   IARG_END);

    // INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)frame_entry_bare,
    //                IARG_END);
  }

  // Scope entry/exit.
  //
  // First test for exit (since this pops the mini-stack, and the same
  // address may indicate the NEW scope that "replaces" the exiting
  // one on the mini-stack).
  boost::unordered_map<uint64_t, unsigned>::const_iterator j;
  j = stack_info.scope_exit.find(addr);
  if(j != stack_info.scope_exit.end()){
    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)scope_exit, 
                   IARG_ADDRINT, j->first,
                   IARG_UINT32, j->second,
                   IARG_END);
  }

  // Next check to see if the address begins a new scope.
  j = stack_info.scope_entry.find(addr);
  if(j != stack_info.scope_entry.end()){
    // // INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)emit_pc, IARG_INST_PTR, IARG_END);

    INS_InsertCall(ins, IPOINT_BEFORE, (AFUNPTR)scope_entry,
                   IARG_INST_PTR,
                   IARG_UINT32, j->second,
                   IARG_END);
  }
}

int main(int argc, char *argv[]){
  // Initialize pin.
  if(PIN_Init(argc, argv)){
    std::cerr << "error: PIN_Init() failed!" << std::endl;
    return 1;
  }

  PIN_InitSymbols();

  // Grab the command line argument names.
  std::string tracefile = outfile.Value();

  // If a line number map file is specified, read it in.
  if(linemap.Value() != ""){
    std::ifstream in(linemap.Value().c_str());
    unsigned filecount = 0;

    while(true){
      // Read in two items (from a single line).
      MTR::addr_t addr;
      std::string text;
      in >> std::hex >> addr >> text;

      // std::cerr << "addr: " << addr << std::endl;

      // If the reads failed, then we're done.
      if(!in.good()){
        break;
      }

      // Split the text into two fields, a filename and a line number
      // (the separator is a colon).
      size_t colon = text.find(':');
      if(colon == std::string::npos){
        std::cerr << "error: value in line number map '" << text << "' does not contain a colon." << std::endl;
        exit(1);
      }

      std::string filename = text.substr(0, colon);
      std::string line = text.substr(colon+1);

      // Find an index for the filename, creating one if necessary.
      Filetable::const_iterator i = filetable.find(filename);
      if(i == filetable.end()){
        filetable[filename] = filecount++;
        i = filetable.find(filename);
      }

      // Convert the line number to a numeric type.
      unsigned linenumber = lexical_cast<unsigned>(line);

      // Install the address in the lineno map.
      lineno[addr] = std::make_pair(i->second, linenumber);
    }

    // foreach(Linetable::value_type i, lineno){
    //   std::cout << i.first << " -> (" << i.second.first << ", " << i.second.second << ")" << std::endl;
    // }

    // Dump the file table.
    const std::string filetable_filename = tracefile + ".filetable";
    std::ofstream file(filetable_filename.c_str());
    if(!file){
      std::cerr << "error: could not open file '" << filetable_filename << "' for output." << std::endl;
      exit(1);
    }

    foreach(Filetable::value_type i, filetable){
      file << i.second << " " << i.first << std::endl;
    }
    file.close();
  }

  if(stackinfo.Value() != ""){
    std::cerr << "hello" << std::endl;
    if(!stack_info.populate(stackinfo.Value())){
      std::cerr << "error: could not read mithril information file '" << stackinfo.Value() << "'." << std::endl;
      exit(1);
    }
    std::cerr << "yay!" << std::endl;
    stack_info.print();
  }

  // Allocate buffer.
  const string& enc = encoding.Value();
  TraceWriter::Encoding e;
  if(enc == "raw"){
    e = TraceWriter::Raw;
  }
  else if(enc == "gzip"){
    e = TraceWriter::Gzip;
    tracefile += ".gzip";
  }
  else{
    std::cerr << "error: encoding argument was '" << enc << "', must be one of (raw, gzip)." << std::endl;
    exit(1);
  }
  tracefile += ".mtr";

  writer = TraceWriter::ptr(new TraceWriter(e, 256));
  if(!writer->open(tracefile)){
    std::cerr << "error: could not open file '" << outfile.Value() << "' for output." << std::endl;
    exit(1);
  }

  // Install callbacks.
  //
  // This handles watching for (1) loclist entries (to adjust the
  // frame base value) and (2) scope entry/exit addresses.
  INS_AddInstrumentFunction(instruction, 0);

  // This handles calls to and returns from functions, for purposes of
  // pushing new values onto the stack of FB/scope information, etc.
  RTN_AddInstrumentFunction(routine, 0);

  // This is to report a sanity check of how many function calls
  // vs. function returns there were (they should be equal).
  PIN_AddFiniFunction(Fini, 0);

  // Transfer control to target program.
  PIN_StartProgram();

  // PIN_StartProgram() never returns, so to arrive here indicates
  // some sort of error condition.
  return 1;
}
