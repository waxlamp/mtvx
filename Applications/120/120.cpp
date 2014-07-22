// Copyright 2011 A.N.M. Imroz Choudhury
//
// 120.cpp - Application that runs a trace file through a cache
// simulator and dumps the cache state at regular intervals.

// MTV headers.
#include <Core/Dataflow/CacheSimulator.h>
#include <Core/Dataflow/TraceReader.h>
#include <Tools/CacheSimulator/Cache.h>
using Daly::BlockRecord;
using Daly::Cache;
using MTV::CacheSimulator;
using MTV::TraceReader;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <iostream>
#include <string>

// Predicate for block ordering.
bool block_less(const BlockRecord& a, const BlockRecord& b){
  return a.addr < b.addr;
}

// Utility method for dumping the blocks from a cache state.
void dump(const std::vector<std::vector<BlockRecord> >& levels, const std::string& prefix, std::ofstream& out){
  for(unsigned i=0; i<levels.size(); i++){
    for(unsigned j=0; j<levels[i].size(); j++){
      out << prefix << levels[i][j].addr << ' ';
    }
  }
  out << std::endl;
}

int main(int argc, char *argv[]){
  // Handle command line arguments.
  std::string tracefile, cacheconfigfile, outfile;
  unsigned long numRecords, period;
  bool hex;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("Run a trace through a cache simulator and dump the cache state at regular intervals.");

    // Reference trace file.
    TCLAP::ValueArg<std::string> reftraceArg("t",
                                             "trace-file",
                                             "Reference trace file to use as data source.",
                                             true,
                                             "",
                                             "filename",
                                             cmd);

    // Cache configuration.
    TCLAP::ValueArg<std::string> cacheConfigArg("c",
                                                "cache-config-file",
                                                "XML file describing the cache to simulate.",
                                                false,
                                                "",
                                                "filename",
                                                cmd);

    // Number of records to read.
    TCLAP::ValueArg<unsigned long> numRecordsArg("n",
                                                 "num-records",
                                                 "Number of records to process from the trace file (-1 for no limit).",
                                                 false,
                                                 static_cast<unsigned long>(-1),
                                                 "number",
                                                 cmd);

    // Frequency of recording.
    TCLAP::ValueArg<unsigned long> periodArg("p",
                                             "recording-period",
                                             "Number of records to process before recording the cache state.",
                                             false,
                                             1,
                                             "positive number",
                                             cmd);

    // Formatting for addresses.
    TCLAP::SwitchArg hexArg("x",
                            "display-hexadecimal",
                            "Display block addresses in hexadecimal format.",
                            cmd,
                            false);

    // Output file.
    TCLAP::ValueArg<std::string> outfileArg("o",
                                            "output-file",
                                            "File to write records to.",
                                            false,
                                            "/dev/stdout",
                                            "filename",
                                            cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract the arguments.
    tracefile = reftraceArg.getValue();
    cacheconfigfile = cacheConfigArg.getValue();
    numRecords = numRecordsArg.getValue();
    period = periodArg.getValue();
    hex = hexArg.getValue();
    outfile = outfileArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open the trace file.
  TraceReader::ptr trace(new TraceReader);
  if(!trace->open(tracefile)){
    std::cerr << "error: cannot open trace file '" << tracefile << "' for reading." << std::endl;
    exit(1);
  }

  // Open the output file.
  std::ofstream out(outfile.c_str());
  if(!out){
    std::cerr << "error: cannot open file '" << outfile << "' for output." << std::endl;
    exit(1);
  }

  // Set hexadecimal output if requested.
  std::string prefix = "";
  if(hex){
    out << std::hex;
    prefix = "0x";
  }

  // Create the cache.
  Cache::ptr cache;
  if(cacheconfigfile.length() > 0){
    std::cerr << "error: cache specification path unimplemented." << std::endl;
    exit(1);
  }
  else{
    cache = Daly::defaultCache();
  }

  // Create a cache simulator object.
  CacheSimulator::ptr sim(new CacheSimulator(cache));

  // Hook the trace reader to the cache simulator.
  trace->addConsumer(sim);

  // Dump the initial state.
  dump(cache->state().state, prefix, out);

  // Begin reading out trace records.
  try{
    for(unsigned i=0; i<numRecords/period; i++){
      // Read out records for one recording period.
      for(unsigned j=0; j<period; j++){
        trace->nextRecord();
      }

      // Record and dump the state of the cache.
      //
      // First grab a snapshot of the current cache state.
      //
      // Cache::Snapshot snap = cache->state();
      std::vector<std::vector<BlockRecord> > snap = cache->state().state;

      // Now move through the levels and sort by block address.
      for(unsigned i=0; i<snap.size(); i++){
        std::sort(snap[i].begin(), snap[i].end(), block_less);
      }

      // Dump to output.
      dump(snap, prefix, out);
    }
  }
  catch(TraceReader::End){
    std::cerr << "warning: trace file ran out of records." << std::endl;
  }

  // Clean up and exit.
  out.close();
  return 0;
}
