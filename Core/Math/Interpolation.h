// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// Interpolation.h - Contains policy classes for performing
// interpolation.

#ifndef INTERPOLATION_H
#define INTERPOLATION_H

namespace MTV{
  // class LinearInterpolator{
  // public:
  //   static float interpolate(float low, float high, float p);
  // };

  template<typename T>
  class LinearInterpolator{
  public:
    static T interpolate(T low, T high, float p){
      return low + p*(high-low);
    }
  };

  class CubicInterpolator{
  public:
    static float interpolate(float low, float high, float p);
  };

  class SigmoidInterpolator{
  public:
    static float interpolate(float low, float high, float p);
  };

  // class KickInterpolator{
  // public:
  //   static float interpolate(float low, float high, float p);
  // };
}

#endif
