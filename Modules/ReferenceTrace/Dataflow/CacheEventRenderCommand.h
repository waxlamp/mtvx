// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheEventRenderCommand.h - A description of which lines in which
// levels to activate by painting them a certain color.

#ifndef CACHE_EVENT_RENDER_COMMAND_H
#define CACHE_EVENT_RENDER_COMMAND_H

// MTV includes.
#include <Core/Color/Color.h>
#include <Tools/CacheSimulator/Cache.h>

// System includes.
#include <vector>

namespace MTV{
  struct CacheEventRenderCommand{
    CacheEventRenderCommand(const std::vector<Daly::CacheHitRecord>& hits, const Color& color)
      : hits(hits),
        color(color)
    {}

    std::vector<Daly::CacheHitRecord> hits;
    Color color;
  };
}

#endif
