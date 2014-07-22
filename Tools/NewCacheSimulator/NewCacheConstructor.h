// -*- c++ -*-
//
// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCacheConstructor.h - A reader for cache spec XML files that can
// also construct a Cache object from the spec.

#ifndef NEW_CACHE_CONSTRUCTOR_H
#define NEW_CACHE_CONSTRUCTOR_H

// MTV includes.
#include <Tools/NewCacheSimulator/CacheLevel.h>
#include <Tools/NewCacheSimulator/NewCache.h>

// System headers.
#include <string>
#include <vector>

namespace MTV{
  struct CacheLevelParams{
    CacheLevelParams()
      : num_blocks(0),
        num_sets(0),
        write_policy(CacheLevel::WriteThrough),
        repl_policy(CacheLevel::LRU)
    {}

    CacheLevelParams(unsigned num_blocks, unsigned num_sets, CacheLevel::WritePolicy write_policy, CacheLevel::ReplacementPolicy repl_policy)
      : num_blocks(num_blocks),
        num_sets(num_sets),
        write_policy(write_policy),
        repl_policy(repl_policy)
    {}

    unsigned num_blocks;
    unsigned num_sets;
    CacheLevel::WritePolicy write_policy;
    CacheLevel::ReplacementPolicy repl_policy;
  };

  class NewCacheConstructor{
  public:
    typedef boost::unordered_map<std::string, CacheLevel::ptr> SharedLevelTable;

  private:
    class NewCacheLevelConstructor{
    public:
      // NewCacheLevelConstructor(unsigned numBlocks, unsigned associativity, CacheLevel::WritePolicy writePolicy)
      //   : numBlocks(numBlocks),
      //     associativity(associativity),
      //     writePolicy(writePolicy),
      //     use_level_pointer(false)
      // {}

      NewCacheLevelConstructor(CacheLevelParams p)
        : params(p)
      {}

      NewCacheLevelConstructor(CacheLevel::ptr p)
        : level(p) //,
          // use_level_pointer(true)
      {}

      // bool useLevelPointer() const { return use_level_pointer; }

      CacheLevel::ptr constructLevel(NewCache::ptr c){
        if(level){
          return c->add_level(level);
        }
        else{
          return c->add_level(params.num_blocks, params.num_sets, params.write_policy, params.repl_policy);
        }
      }

    private:
      // unsigned numBlocks, associativity;
      // CacheLevel::WritePolicy writePolicy;
      CacheLevelParams params;

      CacheLevel::ptr level;

      bool use_level_pointer;

      friend class NewCacheConstructor;
    };

  public:
    NewCacheConstructor(const std::string& filename, const SharedLevelTable& shared = SharedLevelTable());

    const std::string& error() const { return error_message; }

    void addLevelConstructor(unsigned num_blocks, unsigned num_sets, CacheLevel::WritePolicy write_policy, CacheLevel::ReplacementPolicy repl_policy){
      levels.push_back(NewCacheLevelConstructor(CacheLevelParams(num_blocks, num_sets, write_policy, repl_policy)));
    }

    void addLevelConstructor(CacheLevel::ptr p){
      levels.push_back(NewCacheLevelConstructor(p));
    }

    NewCache::ptr constructCache(TraceReader::ptr trace, BlockStreamReader::ptr bsreader) const;

  private:
    // NewCache::ptr constructCacheHelper(TraceReader::ptr reader, ModtimeTable::ptr modtime, const std::string& bsfile, unsigned numstreams) const;

  private:
    unsigned blocksize;
    NewCache::WriteMissPolicy writeMissPolicy;
    std::vector<NewCacheLevelConstructor> levels;
    std::string evictionPolicy;

    std::string error_message;
  };
}

#endif
