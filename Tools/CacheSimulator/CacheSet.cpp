// Copyright 2011 A.N.M. Imroz Choudhury
//
// CacheSet.cpp

// MTV headers.
#include <Tools/CacheSimulator/CacheSetConstructor.h>
#include <Core/Util/BoostForeach.h>
#include <Core/Util/Util.h>
using Daly::CacheLevel;
using Daly::CacheSet;
using Daly::CacheSetConstructor;

// System headers.
#include <algorithm>
#include <iostream>

std::vector<std::vector<CacheLevel::ptr> > CacheSet::getUnifiedCacheStructure() const {
  // Create an empty result vector.
  std::vector<std::vector<CacheLevel::ptr> > result;

  // Move through the levels of all the caches, stopping only when
  // none of the caches has a level L.
  for(unsigned L=0; ; L++){
    // Collect all the cache levels at level L, tagging them by order
    // added.
    std::vector<std::pair<CacheLevel::ptr, unsigned> > levels;
    foreach(Cache::ptr p, cacheset){
      if(p->hasLevel(L)){
        unsigned pos = levels.size();
        levels.push_back(std::make_pair(p->level(L), pos));
      }
    }

    // Break out if there are no cache levels L in any of the caches.
    if(levels.size() == 0){
      break;
    }

    typedef std::pair<CacheLevel::ptr,unsigned> pair;
    foreach(pair p, levels){
      std::cout << "(" << p.first << ", " << p.second << ") ";
    }
    std::cout << std::endl;

    // Uniquify the list of cache levels.
    //
    // Begin by sorting the pointers by their pointer value.
    std::sort(levels.begin(), levels.end(), MTV::lessthanFirst<CacheLevel::ptr, unsigned>);

    // Throw out consecutive non-unique values (grabbing the iterator
    // at which the uniquified sequence now ends).
    std::vector<std::pair<CacheLevel::ptr, unsigned> >::iterator unique_end = std::unique(levels.begin(), levels.end(), MTV::equalFirst<CacheLevel::ptr, unsigned>);

    // Sort the new sequence back into its original order.
    std::sort(levels.begin(), unique_end, MTV::lessthanSecond<CacheLevel::ptr, unsigned>);

    // Finally, store the uniquified pointers in the results vector.
    result.push_back(std::vector<CacheLevel::ptr>());
    for(std::vector<std::pair<CacheLevel::ptr, unsigned> >::const_iterator i=levels.begin(); i != unique_end; i++){
      (result.end() - 1)->push_back(i->first);
    }
  }

  return result;
}

CacheSet::ptr CacheSet::newFromSpec(const std::string& specfile, const std::string& bsfile, unsigned numstreams, std::string& error){
  CacheSetConstructor c(specfile, bsfile, numstreams);
  if(c.hasError()){
    error = c.error();
    return CacheSet::ptr();
  }

  return c.constructCacheSet();
}
