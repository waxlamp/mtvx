// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// HitLevelCounter.h - Consumes cache access records and writes out
// the address accessed along with the level of cache in which it was
// found.

#ifndef HIT_LEVEL_COUNTER_H
#define HIT_LEVEL_COUNTER_H

// MTV headers.
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/BoostPointers.h>

// Sytem headers.
#include <fstream>
#include <string>

namespace MTV{
  class HitLevelCounter : public Consumer<CacheAccessRecord> {
  public:
    BoostPointers(HitLevelCounter);

  public:
    HitLevelCounter(WidgetPanel::const_ptr panel = WidgetPanel::const_ptr())
      : panel(panel)
    {}

    ~HitLevelCounter();

    bool open(const std::string& filename);

    void consume(const CacheAccessRecord& rec);

  private:
    std::ofstream out;
    WidgetPanel::const_ptr panel;
  };
}

#endif
