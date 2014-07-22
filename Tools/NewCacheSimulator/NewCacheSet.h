// -*- c++ -*-
//
// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCacheSet.h - A collection of caches, bundled together to make
// computer certain properties of the collection easier (e.g.,
// visualization elements for shared cache levels, etc.).

#ifndef NEW_CACHE_SET_H
#define NEW_CACHE_SET_H

// MTV headers.
#include <Core/Util/BoostPointers.h>
#include <Tools/NewCacheSimulator/NewCache.h>

// System headers.
#include <string>
#include <vector>

namespace MTV{
  class NewCacheSet{
  public:
    BoostPointers(NewCacheSet);

  public:
    NewCacheSet(const std::vector<NewCache::ptr>& caches)
      : cacheset(caches)
    {}

    const std::vector<NewCache::ptr>& getCaches() const {
      return cacheset;
    }

    std::vector<std::vector<CacheLevel::ptr> > getUnifiedCacheStructure() const;

    static NewCacheSet::ptr newFromSpec(const std::string& specfile, TraceReader::ptr trace, BlockStreamReader::ptr bsreader, std::string& error);

  private:
    std::vector<NewCache::ptr> cacheset;
  };
}

#endif
