// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// PointToPointAnimator.h - Animator that moves a widget from its
// current position to a specified position within a certain
// timeframe.

#ifndef POINT_TO_POINT_ANIMATOR_H
#define POINT_TO_POINT_ANIMATOR_H

// MTV headers.
#include <Core/Animation/Animator.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Util/Timing.h>

// System headers.
#include <iostream>

namespace MTV{
  template<typename Interpolator>
  class PointToPointAnimator : public Animator {
  public:
    BoostPointers(PointToPointAnimator);

  public:
    PointToPointAnimator(Widget::ptr w, Animator::Flags f, float startTime, float duration, const Point& origin, const Point& dest)
      : Animator(w, f),
        origin(origin)
    {
      this->construct(startTime, duration, dest - origin);
    }

    // PointToPointAnimator(Widget::ptr w, Animator::Flags f, float duration, const Point& origin, const Point& dest)
    //   : Animator(w, f),
    //     origin(origin)
    // {
    //   this->construct(MTV::now(), duration, dest - origin);
    // }

    PointToPointAnimator(Widget::ptr w, Animator::Flags f, float startTime, float duration, const Point& origin, const Vector& _travel)
      : Animator(w, f),
        origin(origin)
    {
      this->construct(startTime, duration, _travel);
    }

    // PointToPointAnimator(Widget::ptr w, Animator::Flags f, float duration, const Point& origin, const Vector& _travel)
    //   : Animator(w, f),
    //     origin(origin)
    // {
    //   this->construct(MTV::now(), duration, _travel);
    // }

    bool update(float t){
      // std::cout << w << " -> (" << startTime << ", " << duration << ", " << t << ")" << std::endl;

      if(t < startTime){
        return true;
      }

      // std::cout << "PointToPointAnimator<Interpolator>::timeUpdate(" << t << ")" << std::endl;
      bool retval = true;

      // Compute the fraction of the duration that has elapsed.  If the
      // fraction is greater than or equal to 1.0, then the animation has
      // concluded.
      float frac = (t - startTime) / duration;
      if(frac >= 1.0){
        frac = 1.0;
        retval = false;
      }

      // Compute a new position for the widget by scaling the travel
      // vector by the timepoint fraction, and move the widget there.
      //
      // w->setLocation(origin + frac*travel);
      w->setLocation(origin + Interpolator::interpolate(0.0, 1.0, frac)*travel);
      return retval;
    }

  private:
    void construct(float _startTime, float _duration, const Vector& _travel){
      startTime = _startTime;
      duration = _duration;
      travel = _travel;
    }

  private:
    float startTime, duration;
    Point origin;
    Vector travel;
  };
}

#endif
