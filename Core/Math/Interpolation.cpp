// Copyright 2011 A.N.M. Imroz Choudhury
//
// Interpolation.cpp

// MTV headers.
#include <Core/Math/Interpolation.h>
using MTV::CubicInterpolator;
using MTV::LinearInterpolator;
using MTV::SigmoidInterpolator;

// float LinearInterpolator::interpolate(float low, float high, float p){
//   return low + p*(high-low);
// }

float CubicInterpolator::interpolate(float low, float high, float p){
  return low + p*p*p*(high-low);
}

float SigmoidInterpolator::interpolate(float low, float high, float p){
  float val = 2.0*p*p;
  if(p > 0.5){
    val = -val + 4.0*p - 1.0;
  }

  return low + val*(high-low);
}

// float KickInterpolator::interpolate(float low, float high, float p){
//   return -1;
// }
