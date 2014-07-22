// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// SetUtilizationCounter.h - Consumes trace records and computes set
// utilization rates, saving them to disk at regular intervals.

#ifndef SET_UTILIZATION_COUNTER_H
#define SET_UTILIZATION_COUNTER_H

// MTV headers.
#include <Core/FrameDump.pb.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CachePerformanceCounter.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/CacheSimulator/Cache.h>
// #include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  using Daly::Cache;
  namespace FD = MTV::FrameDump;

  class SetUtilizationCounter : public Consumer<CacheAccessRecord> {
  public:
    BoostPointers(SetUtilizationCounter);

  public:
    SetUtilizationCounter(WidgetPanel::ptr panel, Cache::const_ptr c, unsigned window, unsigned period)
      : panel(panel),
        counter(boost::make_shared<CacheSetUtilizationPolicy>(c, window)),
        period(period),
        textfile(false)
    {}

    bool open(const std::string& outfile){
      out.open(outfile.c_str());

      // Write out a header.
      const bool good = out.good();
      if(good){
        textfile = true;
        out << "# framenum <L1-set1> ... <L1-setN> <L2-set1> ... <L2-setM> ..." << std::endl;
      }

      return good;
    }

    void consume(const CacheAccessRecord& rec);

    FD::FrameDump::CacheSetUtilization& getCacheSetUtilizationPB(){
      return setutilPB;
    }

  private:
    WidgetPanel::ptr panel;

    std::ofstream out;

    CacheSetUtilizationPolicy::ptr counter;
    unsigned period;

    bool textfile;

    FD::FrameDump::CacheSetUtilization setutilPB;
  };
}

#endif
