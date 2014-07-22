// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// Parametrized.h - Abstract object that accepts a parameter and
// returns a position along a curve.

#ifndef PARAMETRIZED_H
#define PARAMETRIZED_H

// MTV headers.
#include <Core/Geometry/Point.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class Parametrized{
  public:
    BoostPointers(Parametrized);

  public:
    virtual ~Parametrized() {}

    virtual Point position(float p) = 0;
  };
}

#endif
