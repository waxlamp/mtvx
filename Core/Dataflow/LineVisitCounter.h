// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LineVisitCounter.h - Accepts trace records and records how many
// visits each source code line receives, recording this information
// to disk.

#ifndef LINE_VISIT_COUNTER_H
#define LINE_VISIT_COUNTER_H

// MTV headers.
#include <Core/FrameDump.pb.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/Boost.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// System headers.
#include <fstream>
#include <string>

namespace MTV{
  namespace FD = MTV::FrameDump;

  class LineVisitCounter : public Consumer<MTR::Record> {
  public:
    BoostPointers(LineVisitCounter);

  public:
    LineVisitCounter(WidgetPanel::ptr panel)
      : panel(panel),
        go(false),
        textfile(false)
    {}

    ~LineVisitCounter(){
      if(textfile){
        out.close();
      }
    }

    bool open(const std::string& outfile){
      out.open(outfile.c_str());

      // Write out a header.
      const bool good = out.good();
      if(good){
        textfile = true;
        out << "# framenum filenum linenum visit-count" << std::endl;
      }

      return good;
    }

    // FD::FrameDump::LineVisitCount& getLineVisitCountPB(){
    FD::FrameDump::LineVisitCount& getLineVisitCountPB(){
      return visitcount;
    }

    void consume(const MTR::Record& rec);

  private:
    WidgetPanel::ptr panel;

    std::ofstream out;

    bool go, textfile;
    MTR::Record cur;
    boost::unordered_map<std::pair<uint32_t, uint32_t>, unsigned> linecount;
    FD::FrameDump::LineVisitCount visitcount;
  };
}

#endif
