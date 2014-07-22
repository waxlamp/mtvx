// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ColorAnimator.h - Causes a widget to change its color.

#ifndef COLOR_ANIMATOR_H
#define COLOR_ANIMATOR_H

// MTV headers.
#include <Core/Animation/Animator.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>

namespace MTV{
  template<typename T, typename Interpolator>
  class ColorAnimator : public Animator {
  public:
    BoostPointers2(ColorAnimator, T, Interpolator);

  public:
    // ColorAnimator(Widget::ptr p, Animator::Flags f, const Color& original, const Color& color, float duration)
    //   : Animator(p,f),
    //     original(original),
    //     start(MTV::now()),
    //     duration(duration)
    // {
    //   color_offset[0] = color.r - original.r;
    //   color_offset[1] = color.g - original.g;
    //   color_offset[2] = color.b - original.b;
    //   color_offset[3] = color.a - original.a;
    // }

    ColorAnimator(Widget::ptr p, Animator::Flags f, const Color& original, const Color& color, float start, float duration)
      : Animator(p,f),
        original(original),
        start(start),
        duration(duration)
    {
      color_offset[0] = color.r - original.r;
      color_offset[1] = color.g - original.g;
      color_offset[2] = color.b - original.b;
      color_offset[3] = color.a - original.a;
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

      // Interpolate between the original and destination colors.
      const float val = Interpolator::interpolate(0.0, 1.0, frac);
      Color c(original.r + val*color_offset[0],
              original.g + val*color_offset[1],
              original.b + val*color_offset[2],
              original.a + val*color_offset[3]);

      boost::static_pointer_cast<T, Widget>(w)->setColor(c);

      return retval;
    }

  private:
    const Color original;
    float color_offset[4];
    const float start, duration;
  };
}

#endif
