// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheEventRenderDirector.h - A filter that turns a
// CacheAccessRecord into a CacheEventRenderCommand.h.

#ifndef CACHE_EVENT_RENDER_DIRECTOR_H
#define CACHE_EVENT_RENDER_DIRECTOR_H

// MTV includes.
#include <Core/Color/Color.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/Filter.h>
#include <Modules/ReferenceTrace/Dataflow/CacheEventRenderCommand.h>
#include <Modules/ReferenceTrace/Renderers/RegionRenderer.h>
#include <Tools/CacheSimulator/Cache.h>

namespace MTV{
  class CacheEventRenderDirector : public Filter<CacheAccessRecord, CacheEventRenderCommand> {
  public:
    CacheEventRenderDirector(const Color& color);

    template<bool ComputeDelta>
    CacheEventRenderDirector(typename RegionRenderer$Template<ComputeDelta>::const_ptr rr)
      : color(rr->getWidget()->getShellColor())
    {}

    void consume(const CacheAccessRecord& rec);

  private:
    Color color;
  };
}


#endif
