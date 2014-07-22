// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheSet.h - A collection of caches, bundled together to make
// computing certain properties of the collection easier (e.g.,
// visualization elements for possible shared cache levels, etc.).

#ifndef CACHE_SET_H
#define CACHE_SET_H

// MTV headers.
#include <Tools/CacheSimulator/Cache.h>

// System headers.
#include <string>
#include <vector>

namespace Daly{
  class CacheSet{
  public:
    BoostPointers(CacheSet);

  public:
    CacheSet(const std::vector<Cache::ptr>& caches)
      : cacheset(caches)
    {}

    const std::vector<Cache::ptr>& getCaches() const { return cacheset; }
    std::vector<Cache::ptr>& getCaches() { return cacheset; }

    std::vector<std::vector<CacheLevel::ptr> > getUnifiedCacheStructure() const;

    // static CacheSet::ptr newFromSpec(const std::string& specfile);
    static CacheSet::ptr newFromSpec(const std::string& specfile, const std::string& bsfile, unsigned numstreams, std::string& error);

  private:
    std::vector<Cache::ptr> cacheset;
  };
}

#endif
