// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// AccessGrouper.h - Manages two CircleGrouper instances, and ferries
// accesses from the outside circle to the inside circle as they
// occur.

#ifndef ACCESS_GROUPER_H
#define ACCESS_GROUPER_H

// MTV headers.
// #include <Core/Animation/CircleGrouper.h>
#include <Core/Animation/GlowAnimator.h>
#include <Core/Animation/Grouper.h>
#include <Core/Animation/ShapeGrouper.h>
#include <Core/Geometry/ParametrizedCircle.h>
#include <Core/Math/Interpolation.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>

namespace MTV{
  class AccessGrouper : public Grouper,
                        public Consumer<Widget::ptr> {
  public:
    BoostPointers(AccessGrouper);

  public:
    AccessGrouper(const Point& center, bool animating, float i_radius, int size, float o_radius, float glow_duration, float move_duration, Clock::ptr clock, bool fifo = false)
      : access_group(boost::make_shared<ShapeGrouper>(boost::make_shared<ParametrizedCircle>(center, i_radius), animating, move_duration)),
        all_group(boost::make_shared<ShapeGrouper>(boost::make_shared<ParametrizedCircle>(center, o_radius), animating, move_duration)),
        glow_duration(glow_duration),
        size(size),
        p(0),
        clock(clock),
        fifo(fifo)
    {}

    // TODO(choudhury): watch out that taking the Widget::ptr argument
    // as a reference won't screw anything up...
    void consume(const Widget::ptr& w){
      // const float now = MTV::now();
      const float now = clock->noww();

      // Check to see if the widget is in the inner ring.
      if(!access_group->hasWidget(w)){
        // Move the widget from the outer ring to the inner ring.
        //
        // First remove it from the outer ring.
        all_group->removeWidget(w, now);

        // Check to see if the inner ring is full; if it is, remove
        // the oldest widget and place it back on the outer ring.
        if(access_group->size() == size){
          Widget::ptr old = access_group->removeLastWidget(now);
          all_group->addWidget(old, now);
        }

        // Add the new widget.
        access_group->addWidget(w, now);
      }
      else if(fifo){
        // Move the accessed widget to the "front".
        access_group->shiftWidget(w, now);
      }

      // Make the widget glow.
      if(animating){
        GLPoint::ptr glp = boost::static_pointer_cast<GLPoint, Widget>(w);
        GlowAnimator<GLPoint, LinearInterpolator<float> >::ptr glow(new GlowAnimator<GLPoint, LinearInterpolator<float> >(w, Animator::Preemptible, glp->getColor(), Color::red, now, glow_duration));
        animators.push_back(glow);
      }
      else{
        abort();
      }
    }

    ShapeGrouper::ptr getAccessGrouper() const {
      return access_group;
    }

    ShapeGrouper::ptr getAllGrouper() const {
      return all_group;
    }

  private:
    ShapeGrouper::ptr access_group, all_group;
    float glow_duration;
    unsigned size, p;
    Clock::ptr clock;

    bool fifo;
    bool animating;
  };
}

#endif
