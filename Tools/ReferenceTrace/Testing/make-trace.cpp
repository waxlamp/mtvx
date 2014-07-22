// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// make-trace.cpp - An executable to dump out an example reference
// trace.

// MTV headers.
#include <Tools/ReferenceTrace/mtrtools.h>

// System headers.
#include <cstdlib>
#include <iostream>
#include <fstream>

int main(){
  // Open a file for writing; bail out if it can't be created.
  std::ofstream out("trace.mrt");
  if(!out){
    std::cerr << "error: could not create file 'trace.mrt' for writing." << std::endl;
    exit(1);
  }

  MTR::Record trace[64];

  // Place a line number record in the first position.
  trace[0].code = MTR::Record::LineNumber;
  trace[0].file = 0;
  trace[0].line = 47;

  // Fill the next 31 records with ascending references (as to an
  // array).
  const MTR::addr_t base1 = 0x300000;
  for(int i=1; i<32; i++){
    trace[i].code = i % 2 == 0 ? MTR::Record::Read : MTR::Record::Write;
    trace[i].addr = base1 + 4*(i-1);
  }

  // Repeat the pattern, but for different numbers. Start with a
  // different line number record.
  trace[32].code = MTR::Record::LineNumber;
  trace[32].file = 1;
  trace[32].line = 724;

  // Fill the remainder with descending references.
  const MTR::addr_t base2 = 0x6000000;
  for(int i=33; i<64; i++){
    trace[i].code = i % 2 == 0 ? MTR::Record::Read : MTR::Record::Write;
    trace[i].addr = base2 - 8*(i-1);
  }

  // Write the records out to disk.
  out.write(reinterpret_cast<const char *>(&trace[0]), sizeof(trace[0])*64);

  // Close the file and exit.
  out.close();
  return 0;
}
