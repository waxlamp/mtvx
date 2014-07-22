// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheSimulator.h - A filter that takes in consumes records by
// chugging them through a cache simulator, then producing information
// records about that step of simulation.

#ifndef CACHE_SIMULATOR_H
#define CACHE_SIMULATOR_H

// MTV includes.
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/Filter.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/LevelComparator.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/NewCacheSimulator/NewCache.h>
#include <Tools/ReferenceTrace/mtrtools.h>
// using Daly::Cache;
using Daly::CacheHitRecord;

namespace MTV{
  template<typename CacheType>
  class CacheSimulatorT : public Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport> {
  public:
    BoostPointers1(CacheSimulatorT, CacheType);

  public:
    CacheSimulatorT(typename CacheType::ptr c)
      : c(c)
    {}

    typename CacheType::const_ptr getCache() const { return c; }
    
    // Consumer interface.
    //
    // Simulate the effects of a single trace record.
    void consume(const MTR::Record& rec){
      // Perform a step of cache simulation.
      switch(rec.code){
      case MTR::Record::Read:
        c->load(rec.addr);
        break;

      case MTR::Record::Write:
        c->store(rec.addr);
        break;

      default:
        // TODO(choudhury): MTR::Record::LineNumber should never show up
        // here.  Cause the appropriate error condition.
        break;
      }

      // Broadcast the hit info.
      this->Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport>::produce(CacheAccessRecord(c->hitInfo(),
                                                                                                  c->evictionInfo(),
                                                                                                  c->entranceInfo(), rec.addr));

      // Compute the proper cache status color (ranging from blue for a
      // hit in the lowest level up to red for a miss to main memory).
      //
      // Start by computing the highest level hit.
      const float highest = std::max_element(c->hitInfo().begin(), c->hitInfo().end(), LevelComparator<Daly::CacheHitRecord>())->L;
      // const float red = highest / c->numLevels();

      // TODO(choudhury): put these colors in a profile.
      Color out = Color::black;
      if(static_cast<int>(highest) == 0){
        out = Color(0.0, 143./255., 1.0);
      }
      else if(static_cast<int>(highest) == 1){
        out = Color(0.5, 0.0, 0.5);
      }
      else{
        out = Color(1.0, 0.0, 0.0);
      }

      // Construct a CacheStatusReport and broadcast it.
      // this->Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport>::produce(CacheStatusReport(rec.addr, Color(red, 0.0, 1.0 - red)));
      this->Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport>::produce(CacheStatusReport(rec.addr, out));
    }

  private:
    typename CacheType::ptr c;
  };

  typedef CacheSimulatorT<Daly::Cache> CacheSimulator;
  typedef CacheSimulatorT<MTV::NewCache> NewCacheSimulator;
}

#endif
