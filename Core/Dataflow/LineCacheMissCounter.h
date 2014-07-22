// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LineCacheMissCounter.h - Accepts trace records and cache simulation
// results, and records how many cache misses occur in each line.

#ifndef LINE_CACHE_MISS_COUNTER_H
#define LINE_CACHE_MISS_COUNTER_H

// MTV headers.
#include <Core/FrameDump.pb.h>
#include <Core/Dataflow/CacheAccessRecord.h>
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

  class LineCacheMissCounter : public Consumer<MTR::Record>,
                               public Consumer<CacheAccessRecord> {
  public:
    BoostPointers(LineCacheMissCounter);

  public:
    struct CacheMissCount{
      unsigned misses;
      unsigned total;
    };

  public:
    LineCacheMissCounter(WidgetPanel::ptr panel)
      : panel(panel),
        go(false),
        textfile(false),
        misses(0),
        total(0)
    {}

    ~LineCacheMissCounter(){
      this->consume(curlinerec);

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
        out << "### framenum filenum linenum cache-miss-count total-count" << std::endl;
      }

      return good;
    }

    void consume(const MTR::Record& rec);
    void consume(const CacheAccessRecord& rec);

    FD::FrameDump::LineMissCount& getLineMissCountPB(){
      return linemisscount;
    }

  private:
    WidgetPanel::ptr panel;

    bool go, textfile;
    // unsigned curfile, curline;
    MTR::Record curlinerec;
    unsigned misses, total;

    std::ofstream out;

    boost::unordered_map<std::pair<uint32_t, uint32_t>, CacheMissCount> misscount;

    FD::FrameDump::LineMissCount linemisscount;
  };
}

#endif
