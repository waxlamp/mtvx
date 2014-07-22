// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// PulseAnimator.h - Causes a widget to change its size temporarily.

#ifndef PULSE_ANIMATOR_H
#define PULSE_ANIMATOR_H

// MTV headers.
#include <Core/Animation/Animator.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>

/* // System headers. */
/* #include <iostream> */

namespace MTV{
  template<typename T, typename Interpolator>
  class PulseAnimator : public Animator {
  public:
    BoostPointers(PulseAnimator);

  public:
    // PulseAnimator(Widget::ptr p, Animator::Flags f, float original, float pulse, float duration)
    //   : Animator(p,f),
    //     original(original),
    //     pulse(pulse),
    //     start(MTV::now()),
    //     duration(duration)
    // {}

    PulseAnimator(Widget::ptr p, Animator::Flags f, float original, float pulse, float start, float duration)
      : Animator(p,f),
        original(original),
        pulse(pulse),
        start(start),
        duration(duration)
    {}

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

      // Compute a new size.
      //
      // First convert the frac value to an interpolation value,
      // depending on how far it is from 0.5.
      if(frac > 0.5)
        frac = 1.0 - frac;
      frac *= 2.0;

      // Interpolate between the original and glow colors.
      const float size = Interpolator::interpolate(original, pulse, frac);
      boost::static_pointer_cast<T, Widget>(w)->setSize(size);

      return retval;
    }

  private:
    float original, pulse;
    float start, duration;
  };
}

#endif
