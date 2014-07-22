// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ArchimedeanSpiral.h - Parametrized path for an Archimedean spiral.

// MTV headers.
#include <Core/Geometry/Parametrized.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <cmath>

namespace MTV{
  class ArchimedeanSpiral : public Parametrized {
  public:
    BoostPointers(ArchimedeanSpiral);

  public:
    ArchimedeanSpiral(const Point& c, float a, float b)
      : c(c),
        a(a),
        b(b)
    {}

    Point position(float p){
      // p is in [0,1], so turn it into an angle.
      const float theta = p * (2*M_PI);
      const float r = a + b*theta;

      return c + r*Vector(cos(theta), sin(theta));
    }

  private:
    Point c;
    float a, b;
  };
}
