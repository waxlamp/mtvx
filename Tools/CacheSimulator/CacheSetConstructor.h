// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheConstructor.h - A reader for cache set spec XML files that can
// also construct a vector of Cache objects from the spec.

#ifndef CACHE_SET_CONSTRUCTOR_H
#define CACHE_SET_CONSTRUCTOR_H

// MTV headers.
#include <Core/Util/ErrorMessage.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/CacheSimulator/CacheSet.h>

// System headers.
#include <vector>

namespace Daly{
  using MTV::ErrorMessage;

  class CacheSetConstructor : public ErrorMessage {
  public:
    CacheSetConstructor(const std::string& filename, const std::string& bsfile, unsigned numstreams);

    CacheSet::ptr constructCacheSet(){
      CacheSet::ptr p(new CacheSet(cacheset));
      return p;
    }

  private:
    std::vector<Cache::ptr> cacheset;
  };
      
}
#endif
