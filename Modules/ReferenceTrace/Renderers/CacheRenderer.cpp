// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheRenderer.cpp

// MTV includes.
#include <Core/Util/Boost.h>
#include <Core/Util/LevelComparator.h>
#include <Modules/ReferenceTrace/Renderers/CacheRenderer.h>
using MTV::CacheRenderer;

CacheRenderer::CacheRenderer(Cache::const_ptr c, unsigned historySize)
  : cacheWidget(new CacheDisplay(Point::zero)),
    levels(c->num_levels()),
    hitHistory(c->num_levels(), std::vector<bool>(historySize, false)),
    hitHistoryPtr(hitHistory.size(), 0)
{
  for(unsigned i=0; i<levels.size(); i++){
    // Create a level of the cache.
    const std::string label = "L" + boost::lexical_cast<std::string>(i+1);
    CacheLevelDisplay::ptr level(new CacheLevelDisplay(Point::zero, c->level(i)->numBlocks(), 100, label));

    std::cout << "L" << i << ": backplate height = " << level->backplateHeight() << std::endl;

    // Add it to the display widget and the vector of levels.
    cacheWidget->addLevel(level, Vector(-0.5*level->backplateWidth(), -static_cast<float>(level->backplateHeight())*i));
    levels[i] = level;
  }
}

void CacheRenderer::consume(const CacheEventRenderCommand& data){
  // Find the highest level of cache that was involved.
  unsigned highest = std::max_element(data.hits.begin(), data.hits.end(),LevelComparator<Daly::CacheHitRecord>())->L;

  for(std::vector<Daly::CacheHitRecord>::const_iterator i = data.hits.begin(); i != data.hits.end(); i++){
#ifndef NDEBUG
    // // This test simply asserts that a properly constructed
    // // CacheHitRecord should only refer to cache levels when the
    // // record does not represent a cache miss.
    // assert( (i->L == levels.size() and i->p) or
    //         (i->L < levels.size() and !i->p) );
#endif

    // If the hit record references a level of cache (and not main
    // memory), change the appropriate stripe color in the appropriate
    // cache level display.
    if(i->L < levels.size()){
      levels[i->L]->setBlockColor(i->cell, data.color);
    }
  }

  // Set all the hit cache blocks to have a cache status color
  // indicating the highest level of the cache that was reached during
  // this memory operation (e.g., if the access went all the way to
  // main memory, even the block that was read into L1 should indicate
  // a cache miss).
  const Color& cacheStatusColor = highest == 0 ? MTV::Colors::Cache::L1 : (highest == 1 ? MTV::Colors::Cache::L2 : MTV::Colors::Cache::miss);
  for(std::vector<Daly::CacheHitRecord>::const_iterator i=data.hits.begin(); i != data.hits.end(); i++){
    if(i->L < levels.size()){
      levels[i->L]->setResultColor(i->cell, cacheStatusColor);
    }
  }

  // Place hit counts in the histories.
  //
  // TODO(choudhury): is the hitHistoryPtr *array* necessary?  Don't
  // we update all the hitHistory vectors in lockstep, so that we can
  // just use a single hitHistoryPtr value?
  //
  // Every cache level below "highest" was hit.
  for(unsigned i=0; i<highest; i++){
    hitHistory[i][hitHistoryPtr[i]] = true;
    hitHistoryPtr[i] = (hitHistoryPtr[i] + 1) % hitHistory[i].size();
  }

  // If "highest" doesn't indicate a full cache miss, then the level
  // represented by "highest" was hit too.
  if(highest < levels.size()){
    hitHistory[highest][hitHistoryPtr[highest]] = true;
    hitHistoryPtr[highest] = (hitHistoryPtr[highest] + 1) % hitHistory[highest].size();
  }

  // All levels above "highest" did NOT hit.
  for(unsigned i=highest+1; i<levels.size(); i++){
    hitHistory[i][hitHistoryPtr[i]] = false;
    hitHistoryPtr[i] = (hitHistoryPtr[i] + 1) % hitHistory[i].size();
  }

  // Compute cache temperatures for each level and set the shell color
  // appropriately.
  //
  // TODO(choudhury): this is incorrect.  We should in any case use a
  // CachePerformanceCounter object to feed into this class.
  for(unsigned i=0; i<hitHistory.size(); i++){
    float value = 0.0;
    for(unsigned j=0; j<hitHistory[i].size(); j++)
      if(hitHistory[i][j])
        value += 1.0;

    // TODO(choudhury): replace divide with an appropriate multiply.
    // Also: better to store the history size in the class instead of
    // calling std::vector::size() all the time?
    value /= hitHistory[i].size();

    // Use "value" to interpolate between "cold" and "hot" colors
    // (black = cold, white = hot).
    //
    // TODO(choudhury): use a true blackbody radiation colormap.
    //
    // levels[i]->setShellColor(Color(value, value, value, 0.2));
    levels[i]->setShellColor(Color(value, value, value));
  }

  // emit updated();
}
