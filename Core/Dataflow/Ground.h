// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// Ground.h - A special-purpose consumer that does nothing to its
// input.  An instance of Ground<T> can "ground" an implied pathway
// that shouldn't be used, e.g., one or more of the "entry points" in
// the Module interface that has no use in a particular Module's
// implementation.

#ifndef GROUND_H
#define GROUND_H

// MTV includes.
#include <Core/Dataflow/Consumer.h>

namespace MTV{
  template<typename T>
  class Ground : public Consumer<T> {
  public:
    BoostPointers1(Ground, T);

  public:
    void consume(const T& t){
      // Do nothing.
    }

  public:
    static typename Ground<T>::ptr instance(){
      static typename Ground<T>::ptr gnd(new Ground<T>);
      return gnd;
    }

  private:
    // The constructor is protected to prevent direct use -
    // instantiate by using the static singleton method instead.
    Ground(){}
  };
}

#endif
