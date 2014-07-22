// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ParametrizedCircle.h - Circular path.

#ifndef PARAMETRIZED_CIRCLE_H
#define PARAMETRIZED_CIRCLE_H

// MTV headers.
#include <Core/Geometry/Parametrized.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <cmath>

namespace MTV{
  class ParametrizedCircle : public Parametrized {
  public:
    BoostPointers(ParametrizedCircle);

  public:
    ParametrizedCircle(const Point& c, float r)
      : c(c),
        r(r)
    {}

    Point position(float p){
      // p is in [0,1], so turn it into an angle.
      const float theta = p * (2*M_PI);

      return c + r*Vector(cos(theta), sin(theta));
    }

  private:
    Point c;
    float r;
  };
}

#endif
