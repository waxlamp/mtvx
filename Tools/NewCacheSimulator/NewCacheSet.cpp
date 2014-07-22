// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCacheSet.cpp

// MTV headers.
#include <Tools/NewCacheSimulator/CacheLevel.h>
#include <Tools/NewCacheSimulator/NewCacheSet.h>
#include <Tools/NewCacheSimulator/NewCacheSetConstructor.h>
using MTV::CacheLevel;
using MTV::NewCacheSet;
using MTV::NewCacheSetConstructor;

// The function is currently unimplemented.
std::vector<std::vector<CacheLevel::ptr> > NewCacheSet::getUnifiedCacheStructure() const {
  abort();
  return std::vector<std::vector<CacheLevel::ptr> >();
}

NewCacheSet::ptr NewCacheSet::newFromSpec(const std::string& specfile, TraceReader::ptr trace, BlockStreamReader::ptr bsreader, std::string& error){
  NewCacheSetConstructor c(specfile, trace, bsreader);
  if(c.hasError()){
    error = c.error();
    return NewCacheSet::ptr();
  }

  return c.constructNewCacheSet();
}
