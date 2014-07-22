// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// GlowAnimator.h - Causes a widget to temporarily change its color.

#ifndef GLOW_ANIMATOR_H
#define GLOW_ANIMATOR_H

// MTV headers.
#include <Core/Animation/Animator.h>
#include <Core/Graphics/GLPoint.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>

// System headers.
#include <iostream>

namespace MTV{
  template<typename T, typename Interpolator>
  class GlowAnimator : public Animator {
  public:
    BoostPointers2(GlowAnimator, T, Interpolator);

  public:
    GlowAnimator(Widget::ptr p, Animator::Flags f, const Color& original, const Color& glow, float start, float duration)
      : Animator(p,f),
        original(original),
        start(start),
        duration(duration)
    {
      glow_offset[0] = glow.r - original.r;
      glow_offset[1] = glow.g - original.g;
      glow_offset[2] = glow.b - original.b;
    }

    bool update(float t){
      bool retval = true;

      if(t < start){
        return true;
      }

      // Compute the fraction of the animation cycle elapsed.
      float frac = (t - start) / duration;
      if(frac >= 1.0){
        frac = 1.0;
        retval = false;
      }

      // Compute a new color.
      //
      // First convert the frac value to an interpolation value,
      // depending on how far it is from 0.5.
      if(frac > 0.5)
        frac = 1.0 - frac;
      frac *= 2.0;

      // Interpolate between the original and glow colors.
      const float val = Interpolator::interpolate(0.0, 1.0, frac);
      Color c(original.r + val*glow_offset[0],
              original.g + val*glow_offset[1],
              original.b + val*glow_offset[2]);

      boost::static_pointer_cast<T, Widget>(w)->setColor(c);

      return retval;
    }

  private:
    Color original;
    float glow_offset[3];
    float start, duration;
  };
}

#endif
