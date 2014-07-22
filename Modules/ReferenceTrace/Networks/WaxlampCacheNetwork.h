// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampCacheNetwork.h - Data processing network for waxlamp,
// involving actual cache simulation (cf. WaxlampFifoNetwork).

#ifndef WAXLAMP_CACHE_NETWORK_H
#define WAXLAMP_CACHE_NETWORK_H

// MTV headers.
#include <Core/Color/ColorGenerator.h>
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/CachePerformanceCounter.h>
#include <Core/Dataflow/CacheSimulator.h>
#include <Core/Dataflow/HitLevelCounter.h>
#include <Core/Dataflow/LineCacheMissCounter.h>
#include <Core/Dataflow/LineRecordFilter.h>
#include <Core/Dataflow/LineVisitCounter.h>
#include <Core/Dataflow/MemoryRecordFilter.h>
#include <Core/Dataflow/SetUtilizationCounter.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>
#include <Modules/ReferenceTrace/Animation/CacheGrouper.h>
#include <Modules/ReferenceTrace/Networks/WaxlampNetwork.h>
#include <Modules/ReferenceTrace/Renderers/CacheTemperatureRenderer.h>
#include <Tools/CacheSimulator/Cache.h>

// System headers.
#include <fstream>
#include <iostream>

namespace MTV{
  class WaxlampCacheNetwork : public WaxlampNetwork {
  public:
    BoostPointers(WaxlampCacheNetwork);

  public:
    WaxlampCacheNetwork(Cache::ptr c, bool animating, float duration, Clock::ptr clock, ColorGenerator::ptr colorgen);

    Repeater<MTR::Record>::ptr getTraceRepeater() { return MRecordRepeater; }

    // std::vector<Widget::ptr> addRegion(BaseAddressLocator::ptr base, MTR::size_t size, MTR::size_t type);
    std::vector<Widget::ptr> addRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type);
    void removeRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type);

    std::vector<Grouper::ptr> getGroupers(){
      std::vector<Grouper::ptr> groupers = cacheGrouper->getGroupers();
      groupers.push_back(cacheGrouper);
      return groupers;
    }

    CacheGrouper::ptr getCacheGrouper(){
      return cacheGrouper;
    }

    const std::vector<float>& getTemperatures() const {
      return temperature->getRates();
    }

    Widget::ptr getTemperatureRenderer(){
      return temperature->getWidget();
    }

    // void dumpPoints(const std::string& filename) const {
    //   cacheGrouper->dumpPoints(filename, temperature->getRates());
    // }

    void dumpPoints(std::ofstream& out) const {
      cacheGrouper->dumpPoints(out, temperature->getRates());
    }

    LineDumper::ptr lineDumper(WidgetPanel::ptr panel, const std::string& filename){
      linedumper = boost::make_shared<LineDumper>(panel, filename);
      lfilter->addConsumer(linedumper);

      return linedumper;
    }

    LineDumper::ptr lineDumper(WidgetPanel::ptr panel){
      linedumper = boost::make_shared<LineDumper>(panel);
      lfilter->addConsumer(linedumper);

      return linedumper;
    }

    LineCacheMissCounter::ptr missCounter(WidgetPanel::ptr panel, const std::string& filename){
      misscounter = boost::make_shared<LineCacheMissCounter>(panel);
      misscounter->open(filename);

      lfilter->addConsumer(misscounter);
      simulator->Producer<CacheAccessRecord>::addConsumer(misscounter);

      return misscounter;
    }

    LineCacheMissCounter::ptr missCounter(WidgetPanel::ptr panel){
      misscounter = boost::make_shared<LineCacheMissCounter>(panel);

      lfilter->addConsumer(misscounter);
      simulator->Producer<CacheAccessRecord>::addConsumer(misscounter);

      return misscounter;
    }

    LineVisitCounter::ptr visitCounter(WidgetPanel::ptr panel, const std::string& filename){
      visitcounter = boost::make_shared<LineVisitCounter>(panel);
      visitcounter->open(filename);
      
      lfilter->addConsumer(visitcounter);

      return visitcounter;
    }

    LineVisitCounter::ptr visitCounter(WidgetPanel::ptr panel){
      visitcounter = boost::make_shared<LineVisitCounter>(panel);

      lfilter->addConsumer(visitcounter);

      return visitcounter;
    }

    SetUtilizationCounter::ptr setUtilizationCounter(WidgetPanel::ptr panel, Cache::const_ptr c, unsigned window, unsigned period, const std::string& filename){
      setutilcounter = boost::make_shared<SetUtilizationCounter>(panel, c, window, period);
      setutilcounter->open(filename);

      simulator->Producer<CacheAccessRecord>::addConsumer(setutilcounter);

      return setutilcounter;
    }

    SetUtilizationCounter::ptr setUtilizationCounter(WidgetPanel::ptr panel, Cache::const_ptr c, unsigned window, unsigned period){
      setutilcounter = boost::make_shared<SetUtilizationCounter>(panel, c, window, period);

      simulator->Producer<CacheAccessRecord>::addConsumer(setutilcounter);

      return setutilcounter;
    }

    void hitLevelCounter(WidgetPanel::ptr panel, const std::string& filename){
      hitlevelcounter = boost::make_shared<HitLevelCounter>(panel);
      hitlevelcounter->open(filename);

      simulator->Producer<CacheAccessRecord>::addConsumer(hitlevelcounter);
    }

  private:
    MemoryRecordFilter::ptr mfilter;
    LineRecordFilter::ptr lfilter;

    LineDumper::ptr linedumper;

    Repeater<MTR::Record>::ptr MRecordRepeater;
    CacheSimulator::ptr simulator;
    LineVisitCounter::ptr visitcounter;
    LineCacheMissCounter::ptr misscounter;
    SetUtilizationCounter::ptr setutilcounter;
    HitLevelCounter::ptr hitlevelcounter;
    CacheGrouper::ptr cacheGrouper;
    float duration;

    // typedef ColorbrewerQualitative9_Set1_Mod1 Colormap;
    // // typedef ColorbrewerQualitative11_Paired Colormap;
    // Colormap::ptr colorgen;

    ColorGenerator::ptr colorgen;

    CachePerformanceCounter::ptr perf;
    CacheTemperatureRenderer::ptr temperature;

    boost::unordered_map<MTR::addr_t, AddressRangePass::ptr> range_passes;
  };
}

#endif
