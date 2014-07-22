// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// CacheRenderer.h - Renders a cache with all its level and lines,
// taking input from a cache simulator output.

#ifndef CACHE_RENDERER_H
#define CACHE_RENDERER_H

// MTV includes.
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/UpdateNotifier.h>
#include <Core/Util/BoostPointers.h>
#include <Modules/ReferenceTrace/Dataflow/CacheEventRenderCommand.h>
#include <Modules/ReferenceTrace/Graphics/CacheDisplay.h>
#include <Tools/CacheSimulator/Cache.h>
using Daly::Cache;

// System includes.
#include <vector>

namespace MTV{
  class CacheRenderer : public Consumer<CacheEventRenderCommand>,
                        public UpdateNotifier {
  public:
    BoostPointers(CacheRenderer);

  public:
    CacheRenderer(Cache::const_ptr c, unsigned historySize = 5);

    CacheDisplay::ptr getWidget() const {
      return cacheWidget;
    }

    void consume(const CacheEventRenderCommand& data);

  private:
    CacheDisplay::ptr cacheWidget;
    std::vector<CacheLevelDisplay::ptr> levels;

    std::vector<std::vector<bool> > hitHistory;
    std::vector<unsigned> hitHistoryPtr;
  };
}

#endif
