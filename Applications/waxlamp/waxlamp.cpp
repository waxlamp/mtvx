// Copyright 2011 A.N.M. Imroz Choudhury
//
// waxlamp.cpp - Application testbed for organic vis ideas.

// MTV headers.
#include <Core/FrameDump.pb.h>
#include <Core/Type.pb.h>
#include <Core/Variable.pb.h>
#include <Core/Color/Color.h>
#include <Core/Color/ColorGenerator.h>
#include <Core/Dataflow/LineCacheMissCounter.h>
#include <Core/Dataflow/LineVisitCounter.h>
#include <Core/Dataflow/SetUtilizationCounter.h>
#include <Core/Dataflow/Printer.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/Timing.h>
#include <Core/Util/Util.h>
#include <Modules/ReferenceTrace/Networks/WaxlampNetwork.h>
#include <Modules/ReferenceTrace/Networks/WaxlampCacheNetwork.h>
// #include <Modules/ReferenceTrace/Networks/WaxlampFifoNetwork.h>
#include <Modules/ReferenceTrace/UI/WaxlampPanel.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/ReferenceTrace/StackInfo.h>
using Daly::Cache;
using MTV::Clock;
using MTV::ClockedTraceReader;
using MTV::Color;
using MTV::ColorGenerator;
using MTV::LineDumper;
using MTV::LineCacheMissCounter;
using MTV::LineVisitCounter;
using MTV::Printer;
using MTV::SetUtilizationCounter;
using MTV::TickingClock;
using MTV::WallClock;
using MTV::WaxlampNetwork;
using MTV::WaxlampCacheNetwork;
// using MTV::WaxlampFifoNetwork;
using MTV::WaxlampPanel;
using MTV::StackInfo;
using MTV::TimedTraceReader;
using MTV::TraceReader;
namespace FD = MTV::FrameDump;

// Boost headers.
#include <boost/algorithm/string.hpp>

// Qt headers.
#include <QtGui>

// TCLAP headers.
#include <tclap/CmdLine.h>

std::string getFileText(const std::string& filename){
  // Open the file - bail if error.
  std::ifstream in(filename.c_str());
  if(!in){
    return "";
  }

  // Compute the size of the file.
  in.seekg(0, std::ios_base::end);
  const std::streampos size = in.tellg();
  in.seekg(0, std::ios_base::beg);

  // Allocate a string object and read out the entire file.
  std::string text(static_cast<size_t>(size), '\0');
  in.read(&text[0], size);
  in.close();

  return text;
}

bool readFileTable(const std::string& path, std::map<unsigned, std::string>& table){
  // Open the file.
  std::ifstream in(path.c_str());
  if(!in){
    return false;
  }

  // Read out a line at a time, populating the map with the
  // information.
  while(true){
    // Read in two values and break out if there's nothing left to
    // read.
    std::string indextext, filepath;
    in >> indextext >> filepath;
    if(in.eof()){
      break;
    }

    // Convert the index text into an integer.
    unsigned index;
    std::stringstream ss(indextext);
    ss >> index;

    // Insert a new entry in the map.
    table[index] = filepath;
  }

  return true;
}

struct VarSize{
  const MTV::Mithril::Variable *var;
  uint64_t base;
  uint64_t size;
  uint64_t type;
};

int main(int argc, char *argv[]){
  // Create controller application.
  QApplication app(argc, argv);

  // Get command line arguments.
  std::string tracefile, registrationfile, cachespec, colorgenspec;
  bool print, linedumping, framedumping, visitcountdumping, misscountdumping, setutildumping, hitleveldumping, fast, invisible;
  std::vector<std::string> dumping;
  bool keyframes;
  std::string outputfile, filetable, mithrilfile;

  // Default is not to dump any data.
  linedumping = framedumping = visitcountdumping = misscountdumping = setutildumping = false;

  // Make these command line arguments.
  //
  float motion_duration = 0.3;
  float trace_duration = 300;

  try{
    // Create command line parser.
    TCLAP::CmdLine cmd("Run a reference trace with an organic visualization.");

    // Reerence trace file.
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

    // Whether to print out the trace records as they come.
    TCLAP::SwitchArg printArg("p",
                              "print-records",
                              "Print records as they are produced from the trace file.",
                              cmd);

    // Whether to include cache simulation.
    TCLAP::ValueArg<std::string> cachespecArg("c",
                                              "cache-spec",
                                              "Cache specification file.",
                                              false,
                                              "",
                                              "filename",
                                              cmd);

    // What data to dump.
    TCLAP::MultiArg<std::string> dumpingArg("d",
                                            "dump",
                                            "Dump data for each frame (options are \"frames\", \"source-info\", \"visit-counts\", \"miss-counts\", \"set-utilization\", \"hit-level\", \"all\")",
                                            false,
                                            "data specs",
                                            cmd);

    // // Whether to dump line numbers.
    // TCLAP::SwitchArg linedumpingArg("l",
    //                                 "line-dump",
    //                                 "Dump the source code information.",
    //                                 cmd);

    // Faster speed.
    TCLAP::SwitchArg fastArg("f",
                             "fast",
                             "Faster playback speed",
                             cmd);

    // Invisible window.
    TCLAP::SwitchArg invisibleArg("i",
                                  "invisible",
                                  "Do not show the rendering window",
                                  cmd);

    // Color generator.
    TCLAP::ValueArg<std::string> colorgenspecArg("g",
                                                 "colorgen",
                                                 "Color generator selector",
                                                 false,
                                                 "q9s1m1",
                                                 "name",
                                                 cmd);

    // Whether to animate, or only dump keyframes.
    TCLAP::SwitchArg keyframesArg("k",
                                  "keyframes",
                                  "Only dumps keyframes, at the completion of each cache event.",
                                  cmd);

    // Output file.
    TCLAP::ValueArg<std::string> outputfileArg("o",
                                               "output-file",
                                               "Output file for protocol buffer.",
                                               true,
                                               "",
                                               "filename",
                                               cmd);

    // Source file table.
    TCLAP::ValueArg<std::string> filetableArg("",
                                              "file-table",
                                              "File table for source code correlation.",
                                              false,
                                              "",
                                              "filename",
                                              cmd);

    TCLAP::ValueArg<std::string> mithrilfileArg("m",
                                                "mithril",
                                                "Mithril file (containing distilled dwarf debug info).",
                                                false,
                                                "",
                                                "filename",
                                                cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract the arguments.
    tracefile = reftraceArg.getValue();
    registrationfile = registrationArg.getValue();
    print = printArg.getValue();
    cachespec = cachespecArg.getValue();
    dumping = dumpingArg.getValue();
    // linedumping = linedumpingArg.getValue();
    fast = fastArg.getValue();
    invisible = invisibleArg.getValue();
    colorgenspec = colorgenspecArg.getValue();
    keyframes = keyframesArg.getValue();
    outputfile = outputfileArg.getValue();
    filetable = filetableArg.getValue();
    mithrilfile = mithrilfileArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  if(fast){
    motion_duration = 0.1;
    trace_duration = 100;
  }

  // Look through the requested dumping parameters, and set the flags
  // accordingly.
  for(std::vector<std::string>::const_iterator i = dumping.begin(); i != dumping.end(); i++){
    if(*i == "frames"){
      framedumping = true;
    }
    else if(*i == "source-info"){
      linedumping = true;
    }
    else if(*i == "visit-counts"){
      visitcountdumping = true;
    }
    else if(*i == "miss-counts"){
      misscountdumping = true;
    }
    else if(*i == "set-utilization"){
      setutildumping = true;
    }
    else if(*i == "hit-level"){
      hitleveldumping = true;
    }
    else if(*i == "all"){
        linedumping = framedumping = visitcountdumping = misscountdumping = setutildumping = hitleveldumping = true;
        break;
    }
    else{
      std::cerr << "error: invalid dumping parameter '" << *i << "'" << std::endl;
      exit(1);
    }
  }

  // Warn if the user asked for line dumping, but didn't supply a file
  // table.
  if(linedumping and filetable == ""){
    std::cerr << "WARNING: source-info dump requested, but no file table specified (--file-table)" << std::endl;
  }

  // Create a clock for timing purposes.  Use an artificially ticking
  // clock if we're dumping frames, and a "time-travel" clock (one
  // where we can set the time manually) for keyframing.
  Clock::ptr clock;
  if(framedumping){
    clock = boost::make_shared<TickingClock>(1.0 / 30.0);
    // clock = boost::make_shared<TickingClock>(0.01);
  }
  else if(!keyframes){
    // clock = boost::make_shared<WallClock>();
    clock = boost::make_shared<WallClock>();
  }
  else{
    // Make a clock that ticks to each keyframe, which corresponds to
    // the "motion duration" - the time it takes for a single cache
    // event to move the appropriate glyphs from start to finish.
    clock = boost::make_shared<TickingClock>(motion_duration);
  }

  // Create a color generator.
  ColorGenerator::ptr colorgen;
  if(colorgenspec == "q9s1m1"){
    colorgen = boost::make_shared<MTV::ColorbrewerQualitative9_Set1_Mod1>();
  }
  else if(colorgenspec == "q9s1m2"){
    colorgen = boost::make_shared<MTV::ColorbrewerQualitative9_Set1_Mod2>();
  }

  // Open the trace file.
  ClockedTraceReader::ptr trace = boost::make_shared<ClockedTraceReader>(clock);
  trace->setInterval(trace_duration);
  trace->open(tracefile);

  // Instantiate a waxlamp network.
  //
  // WaxlampNetwork::ptr net;
  WaxlampCacheNetwork::ptr net;

  Cache::ptr c;
  // NOTE(choudhury): this block was commented out to remove the
  // reference to fifo networks.
  //
  // if(cachespec != ""){
  //   c = Cache::newFromSpec(cachespec, trace);
  //   if(!c){
  //     std::cerr << "error: failed to load cache from file '" << cachespec << "'." << std::endl;
  //     exit(1);
  //   }

  //   // net = WaxlampNetwork::ptr(new WaxlampCacheNetwork(c, motion_duration, clock));
  //   net = boost::make_shared<WaxlampCacheNetwork>(c, motion_duration, clock, colorgen);
  // }
  // else{
  //   // net = WaxlampNetwork::ptr(new WaxlampFifoNetwork(motion_duration, clock));
  //   net = boost::make_shared<WaxlampFifoNetwork>(motion_duration, clock);
  // }
  c = Cache::newFromSpec(cachespec, trace);
  if(!c){
    std::cerr << "error: failed to load cache from file '" << cachespec << "'." << std::endl;
    exit(1);
  }
  // NOTE(choudhury): "!keyframes" being true means we expect to be
  // animating between the keyframes.
  net = boost::make_shared<WaxlampCacheNetwork>(c, !keyframes, motion_duration, clock, colorgen);

  // Instantiate a waxlamp panel.
  WaxlampPanel::ptr panel(new WaxlampPanel(clock, net));
  panel->setBackgroundColor(Color::white);
  panel->setDumping(framedumping);

  // Connect the reader to it.
  trace->addConsumer(panel->memoryRecordEntryPoint());

  // Capture source code/frame information.
  LineDumper::ptr linedumper;
  if(linedumping){
    std::cout << "adding line dumper" << std::endl;
    // linedumper = net->lineDumper(panel, "sourcelines.txt");
    linedumper = net->lineDumper(panel);
  }

  // Capture per-line visit count information.
  LineVisitCounter::ptr visitcounter;
  if(visitcountdumping){
    std::cout << "adding visit count dumper" << std::endl;
    // net->visitCounter(panel, "visit-count.txt");
    visitcounter = net->visitCounter(panel);
  }

  // Capture per-line miss count information.
  LineCacheMissCounter::ptr misscounter;
  if(misscountdumping){
    std::cout << "adding miss count dumper" << std::endl;
    // net->missCounter(panel, "miss-count.txt");
    misscounter = net->missCounter(panel);
  }

  SetUtilizationCounter::ptr setutilcounter;
  if(setutildumping){
    std::cout << "adding set utilization dumper" << std::endl;
    // net->setUtilizationCounter(panel, c, 100, 1, "set-utilization.txt");
    setutilcounter = net->setUtilizationCounter(panel, c, 100, 1);
  }

  if(hitleveldumping){
    std::cout << "adding hit level dumper" << std::endl;
    net->hitLevelCounter(panel, "hit-level.txt");
  }

  // Set the network's cache grouper to know about the panel (a hack
  // to allow for adding new widgets at vis time).
  //
  // WaxlampCacheNetwork::ptr cast = boost::dynamic_pointer_cast<WaxlampCacheNetwork, WaxlampNetwork>(net);
  // if(cast){
  //   cast->getCacheGrouper()->setWidgetPanel(panel);
  // }
  net->getCacheGrouper()->setWidgetPanel(panel);

  // HD size.
  // panel->resize(720, 720);
  panel->resize(1280, 720);

  // Open the specification file.
  MTR::RegionRegistration reg;
  reg.read(registrationfile.c_str());
  if(reg.hasError()){
    std::cerr << "error: " << reg.error() << std::endl;
    exit(1);
  }

  // Add the regions to the panel.
  for(unsigned i=0; i<reg.arrayRegions.size(); i++){
    std::cout << "Adding region: (" << reg.arrayRegions[i].base << ", " << reg.arrayRegions[i].size << ", " << reg.arrayRegions[i].type << ")" << std::endl;
    panel->addRegion(reg.arrayRegions[i].base, reg.arrayRegions[i].size, reg.arrayRegions[i].type);
  }

  panel->installGroupers();

  // If requested, connect the trace file to a console printer as
  // well.
  if(print){
    Printer<MTR::Record>::ptr printer(new Printer<MTR::Record>);
    trace->addConsumer(printer);
  }

  // If there is a mithril file, open it and process its contents.
  //
  // TODO(choudhury): this function should be abstracted into a header
  // file so it can be called from mtrstackfilter.cpp.
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

  std::ofstream keyframeout;
  if(framedumping){
    if(keyframes){
      // Open the output file.
      keyframeout.open(outputfile.c_str());
      if(!keyframeout){
        std::cerr << "error: could not open file '" << outputfile << "' for output." << std::endl;
        exit(1);
      }

      // Create a header and write it out to the file.
      FD::FrameDumpHeader hdr;

      // First set the creation time.
      time_t now;
      time(&now);
      std::string date(ctime(&now));
      boost::algorithm::trim(date);
      hdr.set_date(date);

      // Set the path to the trace file.
      hdr.set_trace_path(boost::filesystem::absolute(tracefile).native());

      // Set the path to the registration file.
      //
      // hdr.set_registration_path(boost::filesystem::absolute(registrationfile).native());
      hdr.mutable_registration()->set_path(boost::filesystem::absolute(registrationfile).native());
      hdr.mutable_registration()->set_text(getFileText(registrationfile));

      // Set the path to the cache specification file.
      //
      // hdr.set_cache_spec_path(boost::filesystem::absolute(cachespec).native());
      hdr.mutable_cache_spec()->set_path(boost::filesystem::absolute(cachespec).native());
      hdr.mutable_cache_spec()->set_text(getFileText(cachespec));

      // Create a file table object from the file table spec file.
      std::map<unsigned, std::string> source_files;
      if(!readFileTable(filetable, source_files)){
        std::cerr << "error: could not open file '" << filetable << "' for reading." << std::endl;
        exit(1);
      }

      // Walk through the entries of the map, adding entries to the
      // header as we go.
      for(std::map<unsigned, std::string>::const_iterator i = source_files.begin(); i != source_files.end(); i++){
        // Create and populate a SourceFile object in the header.
        FD::FrameDumpHeader::SourceFile *sf = hdr.add_source_file();
        sf->set_index(i->first);
        sf->set_path(i->second);

        // Read out the file and stuff the contents in the header.
        sf->set_text(getFileText(i->second));
      }

      // Set the flags specifying what kinds of data appear in the file.
      //
      // hdr.set_source_code(linedumping);
      hdr.set_visit_count(visitcountdumping);
      hdr.set_miss_count(misscountdumping);
      hdr.set_cache_set_utilization(setutildumping);

      // Write the header out to the output file.
      //
      // First write out the size...
      const int size = hdr.ByteSize();
      keyframeout.write(reinterpret_cast<const char *>(&size), sizeof(size));

      // ...and then the header itself.
      hdr.SerializeToOstream(&keyframeout);
    }

    // A stack of frame base address values.
    std::stack<uint64_t> frame_base;
    std::stack<boost::unordered_map<uint64_t, VarSize> > active_vars;

    // // A set of scopes 
    // boost::unordered_set<> active_scopes;

    while(true){
      // This record object will keep track of whether a scope
      // entry/exit action occurred in the trace (to be processed
      // later in this while loop).
      //
      // MTR::Record stack_action;
      // stack_action.code = MTR::Record::NoCode;

      // Check the time and produce a record if appropriate.
      if(keyframes){
        // First clear out any protocol buffer components that need to
        // be fresh before seeing the reference trace records.
        if(visitcountdumping){
          visitcounter->getLineVisitCountPB().Clear();
        }

        if(misscountdumping){
          misscounter->getLineMissCountPB().Clear();
        }

        if(setutildumping){
          setutilcounter->getCacheSetUtilizationPB().Clear();
        }

        // NOTE(choudhury): Don't call the timeout() function
        // directly, as it relies on a finer-ticking clock to work
        // correctly (i.e. it will be called multiple times in this
        // loop, without always firing trace records off).
        try{
          while(true){
            std::cout << "trace record!!" << std::endl;
            const MTR::Record& rec = trace->nextRecord();
            std::cout << "was " << rec.code << std::endl;

            if(MTR::type(rec) == MTR::Record::OtherType){
              switch(rec.code){
              case MTR::Record::FunctionEntry:
                // New function, so new frame base address.  We don't
                // know what the address is until a FramePointer
                // event.
                frame_base.push(0);
                active_vars.push(boost::unordered_map<uint64_t, VarSize>());

                break;

              case MTR::Record::FunctionExit:
                {
                  // Leaving a function, so drop back to the calling
                  // fucntion's frame base address.
                  frame_base.pop();

                  // Remove all remaining variables in the active_vars
                  // set for the function.
                  boost::unordered_map<uint64_t, VarSize>& dying_vars = active_vars.top();
                  FD::FrameDump& framedump = net->getCacheGrouper()->getFrameDumpPB();
                  for(boost::unordered_map<uint64_t, VarSize>::iterator i = dying_vars.begin(); i != dying_vars.end(); i++){
                    // net->removeRegion(i->second.base, i->second.size, i->second.type);

                    for(unsigned j=0; j<i->second.size / i->second.type; j++){
                      FD::FrameDump::Activity *a = framedump.add_activity();
                      a->set_type(FD::FrameDump::Activity::DeathActivity);
                      a->set_id(i->second.base + j*i->second.type);
                      a->set_ghost(0);
                    }
                  }

                  // Pop the top map off the active vars stack as
                  // well.
                  active_vars.pop();
                }
                break;

              case MTR::Record::FramePointer:
                // Modify the top value on the stack - this is the
                // event that dictates a new function's frame base
                // address.  It is an error if the top value is NOT
                // zero at the time this event occurs.
                if(frame_base.top() != 0){
                  std::cerr << "fatal error: frame base address stack had a 0 on top when a FramePointer event was encountered." << std::endl;
                  abort();
                }
                frame_base.top() = rec.addr;
                break;

              case MTR::Record::ScopeEntry:
                {
                  // New lexical scope - look up the variables that
                  // begin in this frame and add them to the pool of
                  // active variables, and to the dataflow network.
                  boost::unordered_map<uint64_t, std::vector<MTV::Mithril::Variable> >::const_iterator i = var_entry.find(rec.addr);
                  if(i != var_entry.end()){
                    const std::vector<MTV::Mithril::Variable>& vars = i->second;
                    FD::FrameDump& framedump = net->getCacheGrouper()->getFrameDumpPB();
                    for(unsigned j=0; j<vars.size(); j++){
                      unsigned num;
                      const uint32_t type = StackInfo::get_type_size(type_table, vars[j].type(), num);

                      uint64_t base;
                      if(vars[j].has_stack_location()){
                        base = frame_base.top() + vars[j].stack_location().offset();
                      }
                      else if(vars[j].has_absolute_location()){
                        base = vars[j].absolute_location().address();
                      }
                      else{
                        std::cerr << "fatal error: variable has no address locator method" << std::endl;
                        abort();
                      }

                      VarSize vs;
                      vs.var = &vars[j];
                      vs.base = base;
                      vs.size = num*type;
                      vs.type = type;

                      if(active_vars.top().find(vars[j].id()) != active_vars.top().end()){
                        std::cerr << "fatal error("<< trace->getTracePoint() << "): attempting to insert duplicate variable into active vars map" << std::endl;
                        abort();
                      }
                      active_vars.top()[vars[j].id()] = vs;

                      panel->addRegion(vs.base, vs.size, vs.type);

                      // Mark the glyphs as having been born.
                      for(unsigned i=0; i<vs.size / vs.type; i++){
                        FD::FrameDump::Activity *a = framedump.add_activity();
                        a->set_type(FD::FrameDump::Activity::BirthActivity);
                        a->set_id(base + i*vs.type);
                        a->set_ghost(0);
                      }
                    }
                  }
                }
                break;

              case MTR::Record::ScopeExit:
                {
                  // Leaving a lexical scope, so remove the variables
                  // from the dataflow network that lived in the
                  // scope.
                  boost::unordered_map<uint64_t, std::vector<MTV::Mithril::Variable> >::const_iterator i = var_exit.find(rec.addr);
                  if(i != var_exit.end()){
                    const std::vector<MTV::Mithril::Variable>& vars = i->second;
                    FD::FrameDump& framedump = net->getCacheGrouper()->getFrameDumpPB();
                    for(unsigned j=0; j<vars.size(); j++){
                      boost::unordered_map<uint64_t, VarSize>::iterator k = active_vars.top().find(vars[j].id());
                      if(k != active_vars.top().end()){
                        // NOTE(choudhury): actually, don't remove
                        // them from the system - they are
                        // semantically erased, but physically still
                        // present.
                        //
                        // // Remove the variable from the dataflow network.
                        // const VarSize& vs = k->second;
                        // net->removeRegion(vs.base, vs.size, vs.type);

                        // Mark the glyphs as having died.
                        const VarSize& vs = k->second;
                        for(unsigned i=0; i<vs.size / vs.type; i++){
                          FD::FrameDump::Activity *a = framedump.add_activity();
                          a->set_type(FD::FrameDump::Activity::DeathActivity);
                          a->set_id(vs.base + i*vs.type);
                          a->set_ghost(0);
                        }

                        // Remove it from the table of active variables.
                        active_vars.top().erase(k);
                      }
                    }
                  }
                }
                break;

              default:
                std::cerr << "fatal error: record code of type OtherType was " << rec.code << std::endl;
                abort();
                break;
              }

              // This will force a frame dump whenever the glyph set
              // changes.
              break;
            }
            else if(MTR::type(rec) == MTR::Record::MType){
              // This will force a frame dump whenever there is memory
              // activity.
              break;
            }
          }
        }
        catch(TraceReader::End){
          keyframeout.flush();
          keyframeout.close();
          exit(0);
        }
      }
      else{
        trace->timeout();
      }

      // Perform an animation update.
      if(!keyframes){
        panel->animationUpdate();
      }

      if(keyframes){
        // Extract the frame dump protocol buffer.
        FD::FrameDump& framedump = net->getCacheGrouper()->getFrameDumpPB();

        // Make the dump only if there are already glyphs in the
        // framedump object.
        if(framedump.glyph_size() > 0){
          // Emplace the temperatures.
          const std::vector<float>& temps = net->getTemperatures();
          framedump.clear_temperature();
          for(unsigned i=0; i<temps.size(); i++){
            framedump.add_temperature(temps[i]);
          }

          // Grab source code information.
          if(linedumping){
            // Grab the accumulated source code information and pack it
            // into the frame dump.
            const std::vector<FD::FrameDump::SourceCode>& scvec = linedumper->getSourceCodePB();
            framedump.clear_source_code();
            for(unsigned i=0; i<scvec.size(); i++){
              FD::FrameDump::SourceCode *sc = framedump.add_source_code();
              *sc = scvec[i];
            }

            // Clear the accumulated source code info from the line
            // dumper object.
            linedumper->clearSourceCodePB();
          }

          // Grab line visit count information.
          if(visitcountdumping){
            FD::FrameDump::LineVisitCount& lvc = visitcounter->getLineVisitCountPB();
            if(lvc.count_size() > 0){
              *(framedump.mutable_visit_count()) = lvc;
            }
          }

          // Grab line miss count information.
          if(misscountdumping){
            FD::FrameDump::LineMissCount& lmc = misscounter->getLineMissCountPB();
            if(lmc.count_size() > 0){
              *(framedump.mutable_miss_count()) = lmc;
            }
          }

          // Grab cache set utilization rates.
          if(setutildumping){
            FD::FrameDump::CacheSetUtilization& csu = setutilcounter->getCacheSetUtilizationPB();
            if(csu.utilization_size() > 0){
              *(framedump.mutable_cache_set_utilization()) = csu;
            }
          }

          // Record to disk.
          //
          // First record the size of the buffer.
          const int size = framedump.ByteSize();
          keyframeout.write(reinterpret_cast<const char *>(&size), sizeof(size));

          // Next serialize the buffer itself.
          framedump.SerializeToOstream(&keyframeout);

          // Clear out the protocol buffer.
          framedump.Clear();
        }
      }

      // Tick the clock.
      clock->tick();
    }
  }
  else{
    // Start the trace.
    trace->start();

    // Make the panel visible and launch the application.
    if(!invisible){
      panel->show();
    }

    app.exec();
  }
}
