// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// PolarPointAnimator.h - Animator that moves a widget from point a to
// b around some specified center point.

#ifndef POLAR_POINT_ANIMATOR_H
#define POLAR_POINT_ANIMATOR_H

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
  class PolarPointAnimator : public Animator {
  public:
    BoostPointers(PolarPointAnimator);

  public:
    PolarPointAnimator(Widget::ptr w, Animator::Flags f, float startTime, float duration, const Point& center, const Point& origin, const Point& dest)
      : Animator(w, f),
        center(center)
    {
      this->construct(startTime, duration, origin, dest);
    }

    // PolarPointAnimator(Widget::ptr w, Animator::Flags f, float duration, const Point& origin, const Point& dest)
    //   : Animator(w, f),
    //     origin(origin)
    // {
    //   this->construct(MTV::now(), duration, dest - origin);
    // }

    // PolarPointAnimator(Widget::ptr w, Animator::Flags f, float startTime, float duration, const Point& origin, const Vector& _travel)
    //   : Animator(w, f),
    //     origin(origin)
    // {
    //   this->construct(startTime, duration, _travel);
    // }

    // PolarPointAnimator(Widget::ptr w, Animator::Flags f, float duration, const Point& origin, const Vector& _travel)
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

      // std::cout << "PolarPointAnimator<Interpolator>::timeUpdate(" << t << ")" << std::endl;
      bool retval = true;

      // Compute the fraction of the duration that has elapsed.  If the
      // fraction is greater than or equal to 1.0, then the animation has
      // concluded.
      float frac = (t - startTime) / duration;
      if(frac >= 1.0){
        frac = 1.0;
        retval = false;
      }

      // Compute a new polar coordinate for the current position by
      // interpolating between both the radius and angle of the start
      // and end points.
      const float r = Interpolator::interpolate(o_radius, d_radius, frac);
      const float th = Interpolator::interpolate(o_angle, d_angle, frac);
      w->setLocation(center + r*Vector(cos(th), sin(th)));

      // std::cout << (center + r*Vector(cos(th), sin(th))) << std::endl;

      return retval;
    }

  private:
    void construct(float _startTime, float _duration, const Point& origin, const Point& dest){
      startTime = _startTime;
      duration = _duration;
      
      // Remove the origin.
      const Vector o = origin - center;
      const Vector d = dest - center;

      // std::cout << "o = " << o << std::endl;
      // std::cout << "d = " << d << std::endl;

      // Convert to polar coordinates.
      o_radius = sqrt(o.x*o.x + o.y*o.y);
      o_angle = atan2(o.y, o.x);
      if(o_angle < 0.0)
        o_angle += 2*M_PI;

      // std::cout << o_radius << " cis " << (o_angle / (2*M_PI)) << "Tau" << std::endl;

      d_radius = sqrt(d.x*d.x + d.y*d.y);
      d_angle = atan2(d.y, d.x);
      if(d_angle < 0.0)
        d_angle += 2*M_PI;

      // std::cout << d_radius << " cis " << (d_angle / (2*M_PI)) << "Tau" << std::endl;
    }

  private:
    float startTime, duration;
    Point center;
    float o_radius, o_angle;
    float d_radius, d_angle;
  };
}

#endif
