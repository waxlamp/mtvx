// Copyright 2010 A.N.M. Imroz Choudhury
//
// altnat.cpp - Given a specification for a data network, a cache, and
// a reference trace, convertes the trace into a sequence of
// higher-level events as affects the network.

// MTV includes.
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/CacheSimulator.h>
#include <Core/Dataflow/Producer.h>
#include <Core/Dataflow/TraceReader.h>
#include <Marino/DeltaMementoRecorder.h>
#include <Marino/Memorable.h>
#include <Modules/ReferenceTrace/Networks/ReferenceTraceNetwork.h>
using MTV::CacheAccessRecord;
using MTV::CacheStatusReport;
using MTV::CacheSimulator;
using MTV::DeltaMementoRecorder;
using MTV::ImmediateDeltaMementoRecorder;
using MTV::Memorable;
using MTV::Producer;
using MTV::ReferenceTraceNetwork$ComputeDelta;
using MTV::TraceReader;

// TCLAP includes.
#include <tclap/CmdLine.h>

// System includes.
#include <iostream>
#include <string>

int main(int argc, char *argv[]){
  // This program doesn't do any display, but relies on Qt libraries
  // nonetheless.
  QApplication app(argc, argv);

  // Process command line arguments.
  std::string tracefile, registrationfile, cachespecfile;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("Convert a reference trace into a sequence of visualization update events.");

    // Reference trace file.
    TCLAP::ValueArg<std::string> reftraceArg("t",
                                             "trace-file",
                                             "Reference trace file to use as data source.",
                                             true,
                                             "",
                                             "filename",
                                             cmd);

    // Region registration file.
    TCLAP::ValueArg<std::string> registrationArg("r",
                                                 "region-registration",
                                                 "Region registration describing different memory regions.",
                                                 true,
                                                 "",
                                                 "filename",
                                                 cmd);

    // Cache specification file.
    TCLAP::ValueArg<std::string> cacheSpecArg("c",
                                              "cache-spec",
                                              "Cache specification.",
                                              false,
                                              "",
                                              "filename",
                                              cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract the arguments.
    tracefile = reftraceArg.getValue();
    registrationfile = registrationArg.getValue();
    cachespecfile = cacheSpecArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // TODO(choudhury): open region registration and construct network.
  ReferenceTraceNetwork$ComputeDelta::ptr net = ReferenceTraceNetwork$ComputeDelta::newFromSpec(registrationfile);
  if(!net){
    std::cerr << "error: could not create a reference trace network from registration file '" << registrationfile << "'." << std::endl;
    exit(1);
  }

  // Create a trace reader object.
  //
  // TODO(choudhury): need to get the signal range information from
  // the network object.
  TraceReader::ptr trace(new TraceReader(128*1024));
  trace->open(tracefile);

  // Construct a cache.
  Daly::Cache::ptr cache;
  if(cachespecfile == ""){
    // No cache spec given - use the default cache.
    cache = Daly::defaultCache();
  }
  else{
    // Construct a cache from the description.
    cache = Cache::newFromSpec(cachespecfile, trace);
  }

  // Make a cache simulator filter out of the cache.
  CacheSimulator::ptr cachesim(new CacheSimulator(cache));

  // Instruct the reference trace network to make a cache renderer.
  net->renderCache(cache);

  // Attach trace reader to the network.
  trace->addConsumer(net->getTraceRepeater());

  // Attach trace reader to cache simulation filter.
  trace->addConsumer(cachesim);

  // Attach output of cache simulator to network.
  cachesim->Producer<CacheAccessRecord>::addConsumer(net->getCacheAccessRepeater());
  cachesim->Producer<CacheStatusReport>::addConsumer(net->getCacheStatusRepeater());

  // TODO(choudhury): get list of Memorables and attach
  // DeltaMementoRecorders to them.
  DeltaMementoRecorder::ptr deltaSink(new ImmediateDeltaMementoRecorder(tracefile + ".event"));
  std::vector<Memorable::ptr> mem = net->getMemorables();
  for(unsigned i=0; i<mem.size(); i++){
    Memorable::ptr p = mem[i];

    p->setId(i);
    p->addConsumer(deltaSink);
  }

  // TODO(choudhury): run the trace from start to finish.
  try{
    while(true){
      trace->nextRecord();
    }
  }
  catch(TraceReader::End){}

  return 0;
}
