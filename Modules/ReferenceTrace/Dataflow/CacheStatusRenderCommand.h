// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheStatusRenderCommand.h - A description of which cell in a
// region renderer to paint which color, to depict a cache status
// triggered by a memory access.

#ifndef CACHE_STATUS_RENDER_COMMAND_H
#define CACHE_STATUS_RENDER_COMMAND_H

// MTV includes.
#include <Core/Color/Color.h>

namespace MTV{
  struct CacheStatusRenderCommand{
    CacheStatusRenderCommand()
      : cell(static_cast<unsigned>(-1)),
        color(Color::black)
    {}

    CacheStatusRenderCommand(unsigned cell, const Color& color)
      : cell(cell),
        color(color)
    {}

    unsigned cell;
    Color color;
  };
}

#endif
