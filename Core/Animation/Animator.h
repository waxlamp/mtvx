// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// Animator.h - A class that can receive timing updates and
// appropriately change the state of an attached Widget, for purposes
// of animation.

#ifndef ANIMATOR_H
#define ANIMATOR_H

// MTV headers.
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class Animator{
  public:
    BoostPointers(Animator);

  public:
    enum Flags{
      NoFlags       = 0,
      Preemptible   = 1 << 0,
      WidgetKilling = 1 << 1
    };

  public:
    // Construct an animator with the widget it is to act on.
    Animator(Widget::ptr w, Flags f)
      : w(w),
        f(f)
    {}

    // Receive a time t, and perform an update on the widget state,
    // etc.
    virtual bool update(float t) = 0;

    Widget::ptr getWidget() const {
      return w;
    }

    unsigned getFlags() const {
      return f;
    }

  protected:
    Widget::ptr w;
    unsigned f;
  };
}

#endif
