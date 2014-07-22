// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheAccessRecord.h - A record showing the changes effected in a
// cache from the access of a given address.

#ifndef CACHE_ACCESS_RECORD_H
#define CACHE_ACCESS_RECORD_H

// MTV includes.
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  struct CacheAccessRecord{
    CacheAccessRecord(const std::vector<Daly::CacheHitRecord>& hits,
                      const std::vector<Daly::CacheEvictionRecord>& evictions,
                      const std::vector<Daly::CacheEntranceRecord>& entrances,
                      MTR::addr_t addr)
      : hits(hits),
        evictions(evictions),
        entrances(entrances),
        addr(addr)
    {}

    std::vector<Daly::CacheHitRecord> hits;
    std::vector<Daly::CacheEvictionRecord> evictions;
    std::vector<Daly::CacheEntranceRecord> entrances;
    MTR::addr_t addr;
  };
}

#endif
