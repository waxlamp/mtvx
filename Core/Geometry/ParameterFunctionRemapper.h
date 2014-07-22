// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ParameterFunctionRemapper.h - Retrieves point locations from a
// Parametrized object, after transforming the range [0,1] to
// f([0,1]), where f is some function.

#ifndef PARAMETER_FUNCTION_REMAPPER_H
#define PARAMETER_FUNCTION_REMAPPER_H

// MTV headers.
#include <Core/Geometry/Parametrized.h>

namespace MTV{
  template<typename Arg, typename Result>
  class ParameterFunctionRemapper : public Parametrized {
  public:
    BoostPointers(ParameterFunctionRemapper);

  public:
    ParameterFunctionRemapper(Parametrized::ptr shape, Result(*f)(Arg))
      : shape(shape),
        f(f)
    {}

    Point position(float p){
      return shape->position(f(p));
    }

  private:
    Parametrized::ptr shape;
    Result(*f)(Arg);
  };
}

#endif
