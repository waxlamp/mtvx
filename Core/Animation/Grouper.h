// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// Grouper.h - An interface for classes that manage widgets, emitting
// "grouping" Animator instances for them when they need to "regroup".
// A simple example is the class CircleGrouper, which accepts widgets
// and directs them to lie along the edge of a specified circle.

#ifndef GROUPER_H
#define GROUPER_H

// MTV headers.
#include <Core/Animation/Animator.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <vector>

namespace MTV{
  class Grouper{
  public:
    BoostPointers(Grouper);

  public:
    virtual ~Grouper() {}

    virtual const std::vector<Animator::ptr>& getGrouping() const {
      return animators;
    }

    virtual void clearGrouping(){
      animators.clear();
    }

  protected:
    std::vector<Animator::ptr> animators;
  };
}

#endif
