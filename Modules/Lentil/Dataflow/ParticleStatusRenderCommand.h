// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ParticleStatusRenderCommand.h - A description of which particle in
// a data set, and what color to paint its "cache status" indicator
// (so as to the effect accessing that particle would have in the
// cache).

#ifndef PARTICLE_STATUS_RENDER_COMMAND_H
#define PARTICLE_STATUS_RENDER_COMMAND_H

// MTV includes.
#include <Core/Color/Color.h>

namespace MTV{
  struct ParticleStatusRenderCommand{
    ParticleStatusRenderCommand(unsigned index, const Color& color)
      : index(index),
        color(color)
    {}

    unsigned index;
    Color color;
  };
}

#endif
