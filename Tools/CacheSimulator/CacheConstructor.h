// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheConstructor.h - A reader for cache spec XML files that can
// also construct a Cache object from the spec.

#ifndef CACHE_CONSTRUCTOR_H
#define CACHE_CONSTRUCTOR_H

// MTV includes.
#include <Tools/CacheSimulator/Cache.h>

// System includes.
#include <map>
#include <string>
#include <vector>

namespace Daly{
  class CacheConstructor{
    class CacheLevelConstructor{
    public:
      CacheLevelConstructor(unsigned numBlocks, unsigned associativity, Daly::WritePolicy writePolicy)
        : numBlocks(numBlocks),
          associativity(associativity),
          writePolicy(writePolicy),
          use_level_pointer(false)
      {}

      CacheLevelConstructor(CacheLevel::ptr p)
        : level(p),
          use_level_pointer(true)
      {}

      bool useLevelPointer() const { return use_level_pointer; }

    private:
      unsigned numBlocks, associativity;
      Daly::WritePolicy writePolicy;

      CacheLevel::ptr level;

      bool use_level_pointer;

      friend class CacheConstructor;
    };

  public:
    CacheConstructor(const std::string& filename, const std::map<std::string, CacheLevel::ptr>& shared = (std::map<std::string, CacheLevel::ptr>()));

    const std::string& error() const { return error_message; }

    void addLevelConstructor(unsigned numBlocks, unsigned associativity, Daly::WritePolicy writePolicy){
       levels.push_back(CacheLevelConstructor(numBlocks, associativity, writePolicy));
    }

    void addLevelConstructor(CacheLevel::ptr p){
      levels.push_back(CacheLevelConstructor(p));
    }

    Cache::ptr constructCache(TraceReader::ptr reader, ModtimeTable::ptr modtime = boost::make_shared<ModtimeTable>()) const;
    Cache::ptr constructCache(TraceReader::ptr reader, const std::string& bsfile, unsigned numstreams) const;

  private:
    Cache::ptr constructCacheHelper(TraceReader::ptr reader, ModtimeTable::ptr modtime, const std::string& bsfile, unsigned numstreams) const;

  private:
    unsigned blocksize;
    Daly::WriteMissPolicy writeMissPolicy;
    std::vector<CacheLevelConstructor> levels;
    std::string evictionPolicy;

    std::string error_message;
  };
}

#endif
