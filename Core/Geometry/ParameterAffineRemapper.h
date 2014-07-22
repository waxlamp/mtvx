// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ParameterAffineRemapper.h - Retrieves point locations from a
// Parametrized object, after transforming the range [0,1] to [a,b].

#ifndef PARAMETER_AFFINE_REMAPPER_H
#define PARAMETER_AFFINE_REMAPPER_H

// MTV headers.
#include <Core/Geometry/Parametrized.h>

namespace MTV{
  class ParameterAffineRemapper : public Parametrized {
  public:
    BoostPointers(ParameterAffineRemapper);

  public:
    ParameterAffineRemapper(Parametrized::ptr shape, float a, float b)
      : shape(shape),
        a(a),
        b(b)
    {}

    Point position(float p){
      return shape->position((b - a)*p + a);
    }

  private:
    Parametrized::ptr shape;
    float a, b;
  };
}

#endif
