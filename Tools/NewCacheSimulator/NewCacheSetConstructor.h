// -*- c++ -*-
//
// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCacheSetConstructor.h - A reader for cache set spec XML files that
// can also construct a vector of Cache objects from the spec.

#ifndef NEW_CACHE_SET_CONSTRUCTOR_H
#define NEW_CACHE_SET_CONSTRUCTOR_H

// MTV headers.
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/ErrorMessage.h>
#include <Tools/NewCacheSimulator/NewCache.h>
#include <Tools/NewCacheSimulator/NewCacheSet.h>

// System headers.
#include <vector>

namespace MTV{
  class NewCacheSetConstructor : public ErrorMessage {
  public:
    // NewCacheSetConstructor(const std::string& filename, const std::string& bsfile, unsigned numstreams);
    NewCacheSetConstructor(const std::string& filename, TraceReader::ptr trace = TraceReader::ptr(), BlockStreamReader::ptr bsreader = BlockStreamReader::ptr());

    NewCacheSet::ptr constructNewCacheSet(){
      NewCacheSet::ptr p(boost::make_shared<NewCacheSet>(cacheset));
      return p;
    }

  private:
    std::vector<NewCache::ptr> cacheset;

    TraceReader::ptr trace;
    BlockStreamReader::ptr bsreader;
  };
}

#endif
