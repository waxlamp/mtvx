// Copyright 2012 A.N.M. Imroz Choudhury
//
// debit.cpp - Cache simulator using "new" cache simulator.

// MTV headers.
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CachePerformanceCounter.h>
#include <Core/Dataflow/CacheSimulator.h>
#include <Core/Dataflow/HitLevelCounter.h>
#include <Core/Dataflow/MemoryRecordFilter.h>
#include <Core/Dataflow/Printer.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/BoostForeach.h>
#include <Tools/NewCacheSimulator/CacheLevel.h>
#include <Tools/NewCacheSimulator/NewCache.h>
#include <Tools/NewCacheSimulator/NewCacheSet.h>
using MTV::AddressRangePass;
using MTV::CacheLevel;
using MTV::CacheAccessRecord;
using MTV::NewCacheSimulator;
using MTV::CacheHitRates;
using MTV::CachePerformanceCounter;
using MTV::HitLevelCounter;
using MTV::MemoryRecordFilter;
using MTV::NewCache;
using MTV::NewCacheLevelToLevelBandwidthPolicy;
using MTV::NewCacheMissCountPolicy;
using MTV::NewCacheSet;
using MTV::NewCacheTemperaturePolicy;
using MTV::NewHitHistoryManager;
using MTV::PerformanceCounterPolicy;
using MTV::Printer;
using MTV::TraceReader;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <iostream>
#include <signal.h>
#include <string>
#include <vector>

void sighandler(int sig){
  if(sig == SIGINT){
    std::cerr << "interrupt" << std::endl;;
    exit(0);
  }
}

// A simple record type to record associations between address ranges
// and caches.
struct WhichCache {
  unsigned which;
  MTR::addr_t base, limit;
};

int main(int argc, char *argv[]){
  signal(SIGINT, sighandler);

  // Handle command line arguments.
  std::string tracefile, policy;
  // std::string outfile;
  unsigned long numrecords, period;
  long windowsize;
  long numrefs;
  std::string bsfile;
  unsigned numstreams;
  std::string cachespecfile;
  std::vector<std::string> rangestrings, dump;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("Run a trace through a cache simulator and dump cache hit/miss statistics at regular intervals.");

    // Reference trace file.
    TCLAP::ValueArg<std::string> tracefileArg("t",
                                             "trace-file",
                                             "Reference trace file to use as data source.",
                                             true,
                                             "",
                                             "filename",
                                             cmd);

    // Cache configuration.
    TCLAP::ValueArg<std::string> cachespecfileArg("c",
                                                  "cache-config-file",
                                                  "XML file describing the cache (or cache set) to simulate.",
                                                  true,
                                                  "",
                                                  "filename",
                                                  cmd);

    // Address ranges corresponding to the caches in the cache set.
    TCLAP::MultiArg<std::string> rangestringsArg("a",
                                                 "address-range",
                                                 "Address ranges specifying which cache (of a cache set) handles them (e.g. \"1,0x00000001,512\", for \"second cache of set\").",
                                                 false,
                                                 "strings",
                                                 cmd);

    // Number of records to read.
    TCLAP::ValueArg<unsigned long> numrecordsArg("n",
                                                 "num-records",
                                                 "Number of records to process from the trace file (-1 for no limit).",
                                                 false,
                                                 static_cast<unsigned long>(-1),
                                                 "number",
                                                 cmd);

    // Performance policy.
    TCLAP::ValueArg<std::string> policyArg("m",
                                           "peformance-metric",
                                           "Which performance metric to use (options: temperature, bandwidth, miss-count).",
                                           false,
                                           "",
                                           "policy",
                                           cmd);

    // Extra information to dump.
    TCLAP::MultiArg<std::string> dumpArg("d",
                                         "dump",
                                         "extra information to dump (options: hit-level).",
                                         false,
                                         "dumper",
                                         cmd);

    // Window size for moving average computation.
    TCLAP::ValueArg<long> windowsizeArg("w",
                                        "window-size",
                                        "Number of latest records over which to compute moving averages.",
                                        false,
                                        100,
                                        "number",
                                        cmd);

    // Output period.
    TCLAP::ValueArg<unsigned long> periodArg("p",
                                             "output-period",
                                             "Number of records to process before outputting cache statistics (use 0 for single report at end of trace).",
                                             false,
                                             50,
                                             "number",
                                             cmd);

    // Draw a progress meter.
    TCLAP::ValueArg<long> numrefsArg("b",
                                     "progress-bar",
                                     "Number of records in the trace - will be used to draw a progress bar",
                                     false,
                                     -1,
                                     "number",
                                     cmd);

    // Blockstream file.
    TCLAP::ValueArg<std::string> bsfileArg("",
                                           "blockstream-file",
                                           "File containing blockstreams for trace",
                                           false,
                                           "",
                                           "filename",
                                           cmd);

    // Number of streams to use (0 for max possible).
    TCLAP::ValueArg<int> numstreamsArg("",
                                       "num-streams",
                                       "Number of streams to use for blockstream reader (0 for maximum)",
                                       false,
                                       0,
                                       "non-negative number",
                                       cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Collect the arguments.
    tracefile = tracefileArg.getValue();
    cachespecfile = cachespecfileArg.getValue();
    rangestrings = rangestringsArg.getValue();
    numrecords = numrecordsArg.getValue();
    policy = policyArg.getValue();
    dump = dumpArg.getValue();
    windowsize = windowsizeArg.getValue();
    period = periodArg.getValue();
    numrefs = numrefsArg.getValue();
    bsfile = bsfileArg.getValue();
    numstreams = numstreamsArg.getValue();
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

  // Create a blockstream reader (if the file was supplied).  Leave an
  // empty pointer if there is no block stream file to use.
  BlockStreamReader::ptr bsreader;
  if(bsfile != ""){
    bsreader = boost::make_shared<BlockStreamReader>();
    const bool success = bsreader->open(bsfile, numstreams);
    if(not success){
      std::cerr << "fatal error: could not open block stream data file '" << bsfile << "'" << std::endl;
      exit(1);
    }
  }

  // Validate the policy string.
  if(policy != "" and
     policy != "temperature" and
     policy != "bandwidth" and
     policy != "miss-count"){
    std::cerr << "error: invalid policy '" << policy << "'" << std::endl;
    exit(1);
  }

  // Validate the dump string.
  foreach(const std::string& s, dump){
    if(s != "hit-level"){
      std::cerr << "error: invalid dumper '" << s << "'" << std::endl;
      exit(1);
    }
  }

  // Parse the address ranges.
  std::vector<WhichCache> ranges;
  for(std::vector<std::string>::iterator i = rangestrings.begin(); i != rangestrings.end(); i++){
    // Count commas and replace them with spaces.
    unsigned comma_count = 0;
    for(std::string::iterator j = i->begin(); j != i->end(); j++){
      if(*j == ','){
        *j = ' ';
        ++comma_count;
      }
    }

    // Error out if the number of commas is not exactly two - the
    // first field is the cache identifier, and the second and third
    // are the base address and size of the range.
    if(comma_count != 2){
      std::cerr << "error: malformed address range specification \"" << *i << "\" does not have exactly two commas." << std::endl;
      exit(1);
    }

    // Read out the addresses.
    WhichCache range;
    std::stringstream ss(*i);
    MTR::size_t size;

    ss >> range.which >> std::hex >> range.base >> std::dec >> size;

    range.limit = range.base + size;
    ranges.push_back(range);
  }

  // // Check to make sure that there were ranges specified - warn if
  // // not.
  // if(ranges.size() == 0){
  //   std::cerr << "warning: no address ranges were specified." << std::endl;
  // }

  // For each cache set in the list of cache set specs, create one
  // simulation network.
  std::vector<CachePerformanceCounter::ptr> perfs;
  Printer<CacheHitRates>::ptr printer(new Printer<CacheHitRates>(std::cout, false));
  // Create a cache set.
  NewCacheSet::ptr caches;
  std::string error;
  caches = NewCacheSet::newFromSpec(cachespecfile, trace, bsreader, error);
  if(!caches){
    // Try to load a single cache from the spec file.
    NewCache::ptr cache = NewCache::newFromSpec(cachespecfile, trace, bsreader, error);
    if(!cache){
      // std::cerr << "error: could not initialize cache set from spec file." << std::endl;
      std::cerr << error << std::endl;
      exit(1);
    }

    // Create a singleton cache set for use in the program run.
    std::vector<NewCache::ptr> cachevec;
    cachevec.push_back(cache);
    caches = boost::make_shared<NewCacheSet>(cachevec);

    if(!caches){
      std::cerr << "error: could not initialize cache set from spec file." << std::endl;
      exit(1);
    }
  }

  // // Dump info about the caches.
  // foreach(NewCache::ptr c, caches->getCaches()){
  //   for(unsigned i=0; i<c->numLevels(); i++){
  //     std::cout << c->level(i) << ' ';
  //   }
  //   std::cout << std::endl;
  // }

  // Set up CacheSimulator objects, one per cache in the set.
  std::vector<NewCacheSimulator::ptr> simulators;
  foreach(NewCache::ptr c, caches->getCaches()){
    NewCacheSimulator::ptr p(new NewCacheSimulator(c));
    simulators.push_back(p);
  }

  // TraceReader -> MFilter -> Filter{i} -> NewCache{j} -> Averager{j} -> File
  MemoryRecordFilter::ptr mfilter(new MemoryRecordFilter);
  trace->addConsumer(mfilter);

  // Set up a path for each address range, so the addresses can be
  // funneled to the proper cache simulator.
  if(ranges.size() == 0){
    std::cerr << "warning: no address ranges specified - passing all addresses." << std::endl;
    AddressRangePass::ptr allpass = AddressRangePass::all();
    mfilter->addConsumer(allpass);
    foreach(NewCacheSimulator::ptr p, simulators){
      allpass->MTV::Producer<MTR::Record>::addConsumer(p);
    }
  }
  else{
    foreach(const WhichCache& range, ranges){
      // Connect the trace reader to an address filter.
      AddressRangePass::ptr pass(new AddressRangePass(range.base, range.limit));
      mfilter->addConsumer(pass);

      // Connect the address filter to the appropriate cache simulator.
      pass->MTV::Producer<MTR::Record>::addConsumer(simulators[range.which]);
    }
  }

  // Connect each cache simulator to a performance counter, each of
  // which connects to a Printer object.
  std::vector<PerformanceCounterPolicy::ptr> counters;
  if(policy == "temperature"){
    NewHitHistoryManager::ptr mgr = boost::make_shared<NewHitHistoryManager>(windowsize);
    foreach(NewCacheSimulator::const_ptr s, simulators){
      counters.push_back(boost::make_shared<NewCacheTemperaturePolicy>(s->getCache(), mgr));
    }
  }
  else if(policy == "bandwidth"){
    foreach(NewCacheSimulator::const_ptr s, simulators){
      counters.push_back(boost::make_shared<NewCacheLevelToLevelBandwidthPolicy>(s->getCache()));
    }
  }
  else if(policy == "miss-count"){
    foreach(NewCacheSimulator::const_ptr s, simulators){
      counters.push_back(boost::make_shared<NewCacheMissCountPolicy>(s->getCache()));
    }
  }
  else if(policy == ""){
    // This block is intentionally blank.
  }
  else{
    abort();
  }

  if(counters.size() == 0){
    std::cerr << "warning: no performance metric specified." << std::endl;
  }

  // Create the counter objects and connect the cache simulators to
  // them.
  for(unsigned i=0; i<simulators.size(); i++){
    if(counters.size() > 0){
      CachePerformanceCounter::ptr perf = boost::make_shared<CachePerformanceCounter>(period);
      perf->setPolicy(counters[i]);

      simulators[i]->MTV::Producer<CacheAccessRecord>::addConsumer(perf);

      perfs.push_back(perf);

      // NOTE(choudhury): don't connect them to the printer, run the
      // printer by hand.
      //
      // perf->addConsumer(printer);
    }

    // Additionally, connect each simulator to the requested
    // dumpers.
    foreach(const std::string& s, dump){
      if(s == "hit-level"){
        HitLevelCounter::ptr h = boost::make_shared<HitLevelCounter>();
        if(!h->open("hit-level.dat")){
          std::cerr << "error: could not open file 'hit-level.dat' for writing (for use with HitLevelCounter)." << std::endl;
          exit(1);
        }

        // TODO(choudhury): this dumper just connects to the first
        // cache within a set.  Need a better way to handle this.
        simulators[0]->MTV::Producer<CacheAccessRecord>::addConsumer(h);
      }
    }
  }

  // The network is now complete.  Each record that comes from the
  // trace will flow through the filters; if at least one filter
  // passes the record, it will engage the simulators, which in turn
  // send their outputs to performance counters, which aggregate hit
  // rates and print them to stdout at regular intervals.
  //
  // Run the trace.
  const long one_percent = std::max(static_cast<long>(numrefs * 0.01), static_cast<long>(1));
  std::cerr.precision(1);
  std::cerr << std::fixed;
  try{
    for(unsigned long i=0; i < numrecords; i++){
      trace->nextRecord();

      if(period > 0 and i % period == 0){
        foreach(CachePerformanceCounter::ptr p, perfs){
          printer->consume(p->rates());
          std::cout << " ";
        }
        std::cout << std::endl;
      }

      // Update the progress bar.
      if(numrefs != -1 and i % one_percent == 0){
#if 0
        // const float percent = static_cast<float>(i) / static_cast<float>(numrefs);
        // const static int numcells = 40;
        // const int filled = numcells * percent;

        // // Draw the progress bar.
        // std::cerr << "\r[";
        // for(int j=0; j<filled; j++){
        //   std::cerr << "=";
        // }
        // for(int j=0; j<numcells-filled; j++){
        //   std::cerr << "-";
        // }
        // std::cerr << "] ";

        // // Print out numbers reflecting the progress.
        // std::cerr << "(" << i << "/" << numrefs << ") ";
        // std::cerr << percent*100 << "%";

        // // Flush the buffer to force printing.
        // std::cerr << std::flush;
#endif

        // Print out numbers reflecting the progress.
        const float percent = static_cast<float>(i) / static_cast<float>(numrefs);
        std::cerr << "(" << i << "/" << numrefs << ") " << percent*100 << "%" << std::endl;
      }
    }
  }
  catch(TraceReader::End){
    if(numrefs != -1){
      std::cerr << std::endl;
    }

    // If the period argument is 0, then do a single print action at
    // the end of the trace.
    if(period == 0 and perfs.size() > 0){
      foreach(CachePerformanceCounter::ptr p, perfs){
        printer->consume(p->rates());
        std::cout << " ";
      }
      std::cout << std::endl;
    }
  }

  return 0;
}
