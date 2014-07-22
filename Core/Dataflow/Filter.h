// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Filter.h - This is, simply put, both a Consumer and a Producer.  It
// sits in the "middle" stages of a data pipeline, transforming data
// and sending it downstream.

#ifndef FILTER_H
#define FILTER_H

// MTV includes.
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/Producer.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  template<typename In, typename Out = In>
  class Filter : public Consumer<In>,
                 public Producer<Out> {
  public:
    BoostPointers2(Filter,In,Out);

  public:
    virtual ~Filter() {}

    // NOTE(choudhury): this function is fixed with a "null"
    // production action (simply pushing the data out); this way, the
    // produce() method can be used explicitly in the implementer's
    // consume() method.  Since the consume() method MUST be specified
    // in the implementer, any and all logic related to production
    // (e.g., whether to produce at all) can be placed there, rather
    // than in the produce() method, which is simply fixed here for
    // simplicity.
    void produce(const Out& out){
      this->broadcast(out);
    }

    virtual void print() const {}
  };

  // A two-output-type filter.
  template<typename In, typename Out1, typename Out2>
  class Filter2 : public Consumer<In>,
                  public Producer<Out1>,
                  public Producer<Out2> {
  public:
    BoostPointers3(Filter2,In,Out1,Out2);

  public:
    virtual ~Filter2() {}

    // NOTE(choudhury): see note above.
    void produce(const Out1& out){
      this->Producer<Out1>::broadcast(out);
    }

    void produce(const Out2& out){
      this->Producer<Out2>::broadcast(out);
    }
  };
}

#endif
