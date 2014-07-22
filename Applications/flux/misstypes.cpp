// Copyright 2011 A.N.M. Imroz Choudhury
//
// misstypes.cpp - Reads in a family of hit-level output files,
// determining the miss type of each cache miss.

// MTV headers.
#include <Core/Dataflow/TraceReader.h>
using MTV::TraceReader;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <cstdlib>
#include <string>
#include <vector>

struct MissCounts{
  MissCounts()
    : compulsory(0),
      capacity(0),
      mapping(0),
      replacement(0)
  {}

  uint64_t compulsory;
  uint64_t capacity;
  uint64_t mapping;
  uint64_t replacement;
};

int main(int argc, char *argv[]){
  std::string tracefile;
  std::vector<std::string> hitlevelfiles;
  unsigned blocksize, misslevel;

  try{
    TCLAP::CmdLine cmd("Determine miss types from a family of hit-level data files.");

    // Reference trace file.
    TCLAP::ValueArg<std::string> tracefileArg("t",
                                              "trace-file",
                                              "Reference trace file - needed for addresses.",
                                              true,
                                              "",
                                              "filename",
                                              cmd);

    // Hit-level data files.
    TCLAP::MultiArg<std::string> hitlevelfilesArg("l",
                                                  "hit-level-file",
                                                  "Hit level file - needed for determining miss types.",
                                                  true,
                                                  "filenames",
                                                  cmd);

    // Block size.
    TCLAP::ValueArg<unsigned> blocksizeArg("b",
                                           "block-size",
                                           "Block size of the cache, in bytes",
                                           true,
                                           0,
                                           "positive integer",
                                           cmd);

    // Miss level - the level of cache hit that represents an actual
    // miss (a "hit" to main memory).
    TCLAP::ValueArg<unsigned> misslevelArg("m",
                                           "miss-level",
                                           "The number in the hit-level data that corresponds to a cache miss.",
                                           0,
                                           true,
                                           "positive integer",
                                           cmd);

    cmd.parse(argc, argv);

    tracefile = tracefileArg.getValue();
    hitlevelfiles = hitlevelfilesArg.getValue();
    blocksize = blocksizeArg.getValue();
    misslevel = misslevelArg.getValue();
  }  
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Check for three hit-level files.
  if(hitlevelfiles.size() != 3){
    std::cerr << "error: expected 3 hit-level data files, got " << hitlevelfiles.size() << std::endl;
    exit(1);
  }

  // Open the hit level files.
  std::ifstream capacity(hitlevelfiles[0].c_str());
  std::ifstream associative(hitlevelfiles[1].c_str());
  std::ifstream real(hitlevelfiles[2].c_str());
  if(!capacity or !associative or !real){
    std::cerr << "error: could not open hit-level files for reading." << std::endl;
    exit(1);
  }

  // Open the trace file.
  TraceReader reader;
  if(!reader.open(tracefile)){
    std::cerr << "error: cannot open trace file '" << tracefile << "' for reading." << std::endl;
    exit(1);
  }

  // Instantiate a set object, used to watch for previously unseen
  // block addresses.
  boost::unordered_set<uint64_t> blocks;

  // Use this to count the miss types.
  MissCounts counts;

  // Begin reading addresses from the trace.
  uint64_t trace_size;
  try{
    for(trace_size = 0; ; trace_size++){
      // Grab a record and compute its block address.
      const uint64_t block_addr = reader.nextRecord().addr / blocksize;

      // Read out one record from each of the hit level files - this
      // needs to be done even if the miss is compulsory, so just do
      // it here.
      unsigned cap_hit, assoc_hit, real_hit;
      capacity.read(reinterpret_cast<char *>(&cap_hit), sizeof(cap_hit));
      associative.read(reinterpret_cast<char *>(&assoc_hit), sizeof(cap_hit));
      real.read(reinterpret_cast<char *>(&real_hit), sizeof(cap_hit));

      // Check whether this is the first time this block has been
      // accessed - if so, it is a compulsory miss.
      boost::unordered_set<uint64_t>::const_iterator i = blocks.find(block_addr);
      if(i == blocks.end()){
        counts.compulsory++;
        blocks.insert(block_addr);
      }
      else{
        // If the access is not a hit, then this filter of if-else
        // statements will determine what the type is.
        if(cap_hit == misslevel){
          // Capacity miss.
          counts.capacity++;
        }
        else if(assoc_hit == misslevel){
          // Mapping miss (i.e. due to associativity).
          counts.mapping++;
        }
        else if(real_hit == misslevel){
          // Replacement miss (i.e. due to replacement policy).
          counts.replacement++;
        }
        else{
          // Cache hit.
          //
          // NOTE(choudhury): this block is intentionally blank, as it
          // could be used to create a report of a hit at that point
          // in the trace, etc.
        }
      }
    }
  }
  catch(TraceReader::End){}

  // The total number of accesses in the trace minus
  // the sum total of all misses gives the total number of hits.
  const uint64_t num_hits = trace_size - (counts.compulsory + counts.capacity + counts.mapping + counts.replacement);

  // Write a report on stdout.
  std::cout << "Compulsory: " << counts.compulsory << std::endl
            << "Capacity: " << counts.capacity << std::endl
            << "Mapping: " << counts.mapping << std::endl
            << "Replacement: " << counts.replacement << std::endl
            << "Hits: " << num_hits << std::endl;

  return 0;
}
