// Copyright 2011 A.N.M. Imroz Choudhury
//
// blockstreamrecon.cpp - A program that shows the "next appearance"
// information for each block in a block stream, once for each trace
// point in the trace.

// MTV headers.
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
using MTV::BlockStreamReader;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <iostream>
#include <string>

int main(int argc, char *argv[]){
  std::string infile;
  unsigned numstreams;

  try{
    TCLAP::CmdLine cmd("Reconstruct the sequence of block accesses from a block stream file.");
    
    // Input file.
    TCLAP::ValueArg<std::string> infileArg("i",
                                           "input",
                                           "Input filename",
                                           true,
                                           "",
                                           "filename",
                                           cmd);

    TCLAP::ValueArg<unsigned> numstreamsArg("s",
                                            "num-filestreams",
                                            "Number of input filestreams to use (0 for max possible)",
                                            false,
                                            0,
                                            "number",
                                            cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract values.
    infile = infileArg.getValue();
    numstreams = numstreamsArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Create a block stream reader for the file.
  BlockStreamReader reader;
  if(!reader.open(infile, numstreams)){
    std::cerr << "error: could not instantiate BlockStreamReader." << std::endl;
    exit(1);
  }

  // Start querying the block steram reader for the next occurrence of
  // each block for sequential simulated trace points, until all
  // blocks report the "never" value.
  for(uint64_t i=0; ; i++){
    // This bool tracks whether any of the reported values at this
    // point are not the "never" value - if all of them are, then it's
    // time to quit.
    bool finished = true;

    std::cout << "Trace point " << i << std::endl;
    for(BlockStreamReader::Table::const_iterator s = reader.begin(); s != reader.end(); s++){
      const uint64_t next = reader.next(s->first, i);
      if(next != BlockStreamReader::never){
        finished = false;
      }

      std::cout << "  " << s->first << " -> " << next << std::endl;
    }

    if(finished){
      break;
    }
  }

  return 0;
}
