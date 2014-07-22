// Copyright 2011 A.N.M. Imroz Choudhury
//
// credit.cpp - Cache simulator.  Outputs average cache hit/miss
// statistics at regular intervals, with specified windows for the
// moving averages.

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
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/CacheSimulator/CacheSet.h>
using Daly::Cache;
using Daly::CacheSet;
using MTV::AddressRangePass;
using MTV::CacheAccessRecord;
using MTV::CacheSimulator;
using MTV::CacheHitRates;
using MTV::CacheMissCountPolicy;
using MTV::CachePerformanceCounter;
using MTV::CacheTemperaturePolicy;
using MTV::HitHistoryManager;
using MTV::HitLevelCounter;
using MTV::LevelToLevelBandwidthPolicy;
using MTV::MemoryRecordFilter;
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
  std::vector<std::string> rangestrings, cachespecfile, dump;

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
    TCLAP::MultiArg<std::string> cachespecfileArg("c",
                                                  "cache-config-file",
                                                  "XML file(s) describing the cache (or cache set) to simulate.",
                                                  true,
                                                  "filenames",
                                                  cmd);

    // Address ranges corresponding to the caches in the cache set.
    TCLAP::MultiArg<std::string> rangestringsArg("a",
                                                 "address-range",
                                                 "Address ranges specifying which simulation and cache handles them (e.g. \"0,1,0x00000001,512\", for \"zeroth simulation, first cache\").",
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
  //
  // std::vector<WhichCache> ranges;
  boost::unordered_map<unsigned, std::vector<WhichCache> > ranges;
  for(std::vector<std::string>::iterator i = rangestrings.begin(); i != rangestrings.end(); i++){
    // Count commas and replace them with spaces.
    unsigned comma_count = 0;
    for(std::string::iterator j = i->begin(); j != i->end(); j++){
      if(*j == ','){
        *j = ' ';
        ++comma_count;
      }
    }

    // Error out if the number of commas is not exactly three - the
    // first field is the simulation identifier, the second is the
    // cache identifier, and the thirdand fourth are the base address
    // and size of the range.
    if(comma_count != 3){
      std::cerr << "error: malformed address range specification \"" << *i << "\" does not have exactly three commas." << std::endl;
      exit(1);
    }

    // Read out the addresses.
    WhichCache range;
    std::stringstream ss(*i);
    MTR::size_t size;
    unsigned whichsim;

    ss >> whichsim >> range.which >> std::hex >> range.base >> std::dec >> size;

    // Don't allow references to cache simulations that were not
    // specified (i.e., using a 2 here with only two -c arguments
    // specified).
    if(whichsim >= cachespecfile.size()){
      std::cerr << "error: simulation " << whichsim << " out of bounds." << std::endl;
      exit(1);
    }

    range.limit = range.base + size;
    ranges[whichsim].push_back(range);
  }

  // // Check to make sure that there were ranges specified - warn if
  // // not.
  // if(ranges.size() == 0){
  //   std::cerr << "warning: no address ranges were specified." << std::endl;
  // }

  // For each cache set in the list of cache set specs, create one
  // simulation network.
  std::vector<std::vector<CachePerformanceCounter::ptr> > perfs;
  Printer<CacheHitRates>::ptr printer(new Printer<CacheHitRates>(std::cout, false));
  for(unsigned k=0; k<cachespecfile.size(); k++){
    // Create a cache set.
    CacheSet::ptr caches;
    std::string error;
    caches = CacheSet::newFromSpec(cachespecfile[k], bsfile, numstreams, error);
    if(!caches){
      // Try to load a single cache from the spec file.
      Cache::ptr cache = Cache::newFromSpec(cachespecfile[k], trace, bsfile, numstreams);
      if(!cache){
        std::cerr << "error: could not initialize cache set from spec file." << std::endl;
        std::cerr << error << std::endl;
        exit(1);
      }

      // Create a singleton cache set for use in the program run.
      std::vector<Cache::ptr> cachevec;
      cachevec.push_back(cache);
      caches = boost::make_shared<CacheSet>(cachevec);

      if(!caches){
        std::cerr << "error: could not initialize cache set from spec file." << std::endl;
        exit(1);
      }
    }

    // Not needed, trace is supplied to constructor above.
    //
    // // If any of the caches in the set uses OPT-style or PES-style
    // // block replacement, tell them about the trace reader now.
    // for(std::vector<Cache::ptr>::iterator i=caches->getCaches().begin(); i<caches->getCaches().end(); i++){
    //   Daly::ApproxOPT::ptr opt = boost::dynamic_pointer_cast<Daly::ApproxOPT, Daly::EvictionBlockSelector>((*i)->evictionPolicy());
    //   Daly::ApproxPES::ptr pes = boost::dynamic_pointer_cast<Daly::ApproxPES, Daly::EvictionBlockSelector>((*i)->evictionPolicy());
    //   if(opt){
    //     std::cerr << "simulation " << k << ": found an ApproxOPT cache" << std::endl;
    //     opt->setReader(trace);
    //   }
    //   else if(pes){
    //     std::cerr << "simulation " << k << ": found an ApproxPES cache" << std::endl;
    //     pes->setReader(trace);
    //   }
    // }

    // // Dump info about the caches.
    // foreach(Cache::ptr c, caches->getCaches()){
    //   for(unsigned i=0; i<c->numLevels(); i++){
    //     std::cout << c->level(i) << ' ';
    //   }
    //   std::cout << std::endl;
    // }

    // Set up CacheSimulator objects, one per cache in the set.
    std::vector<CacheSimulator::ptr> simulators;
    foreach(Cache::ptr c, caches->getCaches()){
      CacheSimulator::ptr p(new CacheSimulator(c));
      simulators.push_back(p);
    }

    // TraceReader -> MFilter -> Filter{i} -> Cache{j} -> Averager{j} -> File
    MemoryRecordFilter::ptr mfilter(new MemoryRecordFilter);
    trace->addConsumer(mfilter);

    // Set up a path for each address range, so the addresses can be
    // funneled to the proper cache simulator.
    if(ranges.find(k) == ranges.end()){
      std::cerr << "warning: simulation " << k << " has no address ranges specified - passing all addresses." << std::endl;
      AddressRangePass::ptr allpass = AddressRangePass::all();
      mfilter->addConsumer(allpass);
      foreach(CacheSimulator::ptr p, simulators){
        allpass->MTV::Producer<MTR::Record>::addConsumer(p);
      }
    }
    else{
      foreach(const WhichCache& range, ranges[k]){
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
      HitHistoryManager::ptr mgr = boost::make_shared<HitHistoryManager>(windowsize);
      foreach(CacheSimulator::const_ptr s, simulators){
        counters.push_back(boost::make_shared<CacheTemperaturePolicy>(s->getCache(), mgr));
      }
    }
    else if(policy == "bandwidth"){
      foreach(CacheSimulator::const_ptr s, simulators){
        counters.push_back(boost::make_shared<LevelToLevelBandwidthPolicy>(s->getCache()));
      }
    }
    else if(policy == "miss-count"){
      foreach(CacheSimulator::const_ptr s, simulators){
        counters.push_back(boost::make_shared<CacheMissCountPolicy>(s->getCache()));
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

        perfs.push_back(std::vector<CachePerformanceCounter::ptr>());
        perfs.back().push_back(perf);

        // NOTE(choudhury): don't connect them to the printer, run the
        // printer by hand.
        //
        // perf->addConsumer(printer);
      }

      // Additionally, connect each simulator to the requested
      // dumpers.
      foreach(const std::string& s, dump){
        if(s == "hit-level"){
          std::stringstream ss;
          ss << "hit-level.c" << k << ".dat";

          HitLevelCounter::ptr h = boost::make_shared<HitLevelCounter>();
          if(!h->open(ss.str())){
            std::cerr << "error: could not open file '" << ss.str() << "' for writing (for use with HitLevelCounter)." << std::endl;
            exit(1);
          }

          // TODO(choudhury): this dumper just connects to the first
          // cache within a set.  Need a better way to handle this.
          simulators[0]->MTV::Producer<CacheAccessRecord>::addConsumer(h);
        }
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
  try{
    for(unsigned long i=0; i < numrecords; i++){
      // // TEST(choudhury): rebuffer the trace at intervals to see if
      // // the results change.
      // if(i % 256 == 0){
      //   trace->rebuffer();
      // }

      trace->nextRecord();

      if(period > 0 and i % period == 0){
        for(unsigned k=0; k<perfs.size(); k++){
          foreach(CachePerformanceCounter::ptr p, perfs[k]){
            printer->consume(p->rates());
            std::cout << " ";
          }
          if(k < perfs.size() - 1){
            std::cout << "| ";
          }
        }
        std::cout << std::endl;
      }

      // Update the progress bar.
      if(numrefs != -1 and i % one_percent == 0){
        const static int numcells = 40;
        const float percent = static_cast<float>(i) / static_cast<float>(numrefs);
        const int filled = numcells * percent;

        // Draw the progress bar.
        std::cerr << "\r[";
        for(int j=0; j<filled; j++){
          std::cerr << "=";
        }
        for(int j=0; j<numcells-filled; j++){
          std::cerr << "-";
        }
        std::cerr << "] ";

        // Print out numbers reflecting the progress.
        std::cerr << "(" << i << "/" << numrefs << ") ";
        std::cerr << percent*100 << "%";

        // Flush the buffer to force printing.
        std::cerr << std::flush;
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
      for(unsigned k=0; k<perfs.size(); k++){
        foreach(CachePerformanceCounter::ptr p, perfs[k]){
          printer->consume(p->rates());
          std::cout << " ";
        }
        if(k < perfs.size() - 1){
          std::cout << "| ";
        }
      }
      std::cout << std::endl;
    }
  }

  return 0;
}
