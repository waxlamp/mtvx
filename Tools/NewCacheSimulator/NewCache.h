// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// NewCache.h

#ifndef NEW_CACHE_H
#define NEW_CACHE_H

// #define USE_STRING_INFO

// MTV headers.
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/NewCacheSimulator/CacheLevel.h>
using Daly::CacheEntranceRecord;
using Daly::CacheEvictionRecord;
using Daly::CacheHitRecord;

// Boost headers.
#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_map.hpp>

// System headers.
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <sstream>
#include <vector>

namespace MTV{
  class AllocateExisting : public std::logic_error {
  public:
    AllocateExisting()
      : std::logic_error("Attempt to allocate a block already present in the cache.")
    {}
  };

  class UninitializedBlockstreamReader : public std::logic_error {
  public:
    UninitializedBlockstreamReader()
      : std::logic_error("OPT or PES policy requested with uninitialized blockstream reader.")
    {}
  };

  class UninitializedTraceReader : public std::logic_error {
  public:
    UninitializedTraceReader()
      : std::logic_error("OPT or PES policy requested with uninitialized trace reader.")
    {}
  };

  class NewCache : public boost::enable_shared_from_this<NewCache> {
    friend class CacheLevel;

  public:
    BoostPointers(NewCache);

  public:
    class UnevenSets {};

  public:
    enum WriteMissPolicy{
      WriteAllocate,
      WriteNoAllocate
    };

  public:
    static NewCache::ptr create(uint64_t blocksize, WriteMissPolicy write_miss_pol){
      NewCache::ptr p(new NewCache(blocksize, write_miss_pol));
      return p;
    }

    static NewCache::ptr newFromSpec(const std::string& specfile, TraceReader::ptr trace, BlockStreamReader::ptr bsreader, std::string& error);

    static NewCache::ptr dummy(TraceReader::ptr trace, BlockStreamReader::ptr bsreader){
      NewCache::ptr p(new NewCache(0, NewCache::WriteAllocate));
      p->set_trace_reader(trace);
      p->set_blockstream_reader(bsreader);

      return p;
    }

  public:
    WriteMissPolicy write_miss_policy() const {
      return write_miss_pol;
    }

    uint64_t block_size() const {
      return blocksize;
    }

    unsigned num_levels() const {
      return levels.size();
    }

    CacheLevel::const_ptr level(unsigned i) const {
      return levels[i];
    }

    void set_trace_reader(TraceReader::const_ptr _trace){
      trace = _trace;
    }

    void set_blockstream_reader(BlockStreamReader::ptr _bs_reader){
      bs_reader = _bs_reader;
    }

    CacheLevel::ptr add_level(CacheLevel::ptr level){
      levels.push_back(level);
      return level;
    }

    CacheLevel::ptr add_level(unsigned num_blocks, unsigned num_sets, CacheLevel::WritePolicy write_policy, CacheLevel::ReplacementPolicy repl_policy);

    const std::vector<CacheHitRecord>& hitInfo() const {
      return hit_info;
    }

    const std::vector<CacheEvictionRecord>& evictionInfo() const {
      return eviction_info;
    }

    const std::vector<CacheEntranceRecord>& entranceInfo() const {
      return entrance_info;
    }

    void load(const uint64_t addr);

    void store(const uint64_t addr);

    void print(std::ostream& out) const {
      for(unsigned L=0; L<levels.size(); L++){
        out << "L" << (L+1) << ":" << std::endl;
        levels[L]->print(out, "\t");
      }
    }

#ifdef USE_STRING_INFO
    const std::vector<std::string>& hits_string() const {
      return hit_info_string;
    }

    const std::vector<std::string>& evictions_string() const {
      return eviction_info_string;
    }

    const std::vector<std::string>& entrances_string() const {
      return entrance_info_string;
    }
#endif

    const std::vector<Daly::CacheHitRecord>& hits() const {
      return hit_info;
    }

    const std::vector<Daly::CacheEvictionRecord>& evictions() const {
      return eviction_info;
    }

    const std::vector<Daly::CacheEntranceRecord>& entrances() const {
      return entrance_info;
    }

  private:
    NewCache(uint64_t blocksize, WriteMissPolicy write_miss_pol)
      : blocksize(blocksize),
        write_miss_pol(write_miss_pol)
    {}

  private:
    void write_at_level(const unsigned level, const uint64_t addr);

  private:
    std::vector<CacheLevel::ptr> levels;
    const unsigned blocksize;
    const WriteMissPolicy write_miss_pol;

    BlockStreamReader::ptr bs_reader;
    TraceReader::const_ptr trace;

    std::vector<Daly::CacheHitRecord> hit_info;
    std::vector<Daly::CacheEvictionRecord> eviction_info;
    std::vector<Daly::CacheEntranceRecord> entrance_info;

#ifdef USE_STRING_INFO
    // String versions of the info records (to be obsoleted).
    std::vector<std::string> hit_info_string;
    std::vector<std::string> eviction_info_string;
    std::vector<std::string> entrance_info_string;
#endif
  };
}

#endif
