// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampFifoNetwork.h - Data processing network for waxlamp (organic
// vis take on reference trace visualization) using a simple "FIFO"
// cache.

#ifndef WAXLAMP_FIFO_NETWORK_H
#define WAXLAMP_FIFO_NETWORK_H

// MTV headers.
#include <Core/Dataflow/Repeater.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>
#include <Modules/ReferenceTrace/Animation/AccessGrouper.h>
#include <Modules/ReferenceTrace/Networks/WaxlampNetwork.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  class WaxlampFifoNetwork : public WaxlampNetwork {
  public:
    BoostPointers(WaxlampNetwork);

  public:
    WaxlampFifoNetwork(bool animating, float duration, Clock::ptr clock);

    Repeater<MTR::Record>::ptr getTraceRepeater() { return MRecordRepeater; }

    AccessGrouper::ptr getAccessGrouper() const {
      return cacheGrouper;
    }

    std::vector<Widget::ptr> addRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type);

    std::vector<Grouper::ptr> getGroupers() {
      std::vector<Grouper::ptr> groupers;
      groupers.push_back(cacheGrouper);
      groupers.push_back(cacheGrouper->getAllGrouper());
      groupers.push_back(cacheGrouper->getAccessGrouper());

      return groupers;
    }

    Widget::ptr getTemperatureRenderer(){
      return Widget::ptr();
    }

    void dumpPoints(std::ofstream& filename) const {
      // Blank function body.
    }

    void lineDumper(WidgetPanel::ptr, const std::string& filename){
      // Blank function body.
    }

    void visitCounter(WidgetPanel::ptr, const std::string&){
      // Blank function body.
    }

    void missCounter(WidgetPanel::ptr, const std::string&){
      // Blank function body.
    }

    void setUtilizationCounter(WidgetPanel::ptr panel, Cache::const_ptr c, unsigned window, unsigned period, const std::string& filename){
      // Blank function body.
    }

    void hitLevelCounter(WidgetPanel::ptr panel, const std::string& filename){
      // Blank function body.
    }

  private:
    Repeater<MTR::Record>::ptr MRecordRepeater;
    AccessGrouper::ptr cacheGrouper;

    Clock::ptr clock;
  };
}

#endif
