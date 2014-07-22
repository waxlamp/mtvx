// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// Repeater.h - Utility filter that simply produces its input (on
// possibly several output channels, as for any filter)--it is the
// "identity filter", but is useful passing along an input unchanged,
// as over the border between different class instances, or else
// simply to replicate an input for several outputs.

#ifndef REPEATER_H
#define REPEATER_H

// MTV includes.
#include <Core/Dataflow/Filter.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  template<typename T>
  class Repeater : public Filter<T> {
  public:
    BoostPointers(Repeater);

  public:
    void consume(const T& t){
      this->produce(t);
    }
  };
}

#endif
