// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Consumer.h - Interface for dataflow objects that receive data of
// some type.  It may, for instance, transform that data and pass it
// along to a different consumer (i.e., a filter), or it may perform
// some external action with it, such as updating an on-screen
// visualization.

#ifndef CONSUMER_H
#define CONSUMER_H

// MTV headers.
#include <Core/Util/BoostPointers.h>

namespace MTV{
  template<typename In>
  class Consumer{
  public:
    BoostPointers(Consumer<In>);

  public:
    virtual ~Consumer() {}
    virtual void consume(const In& in) = 0;

    // In case something wants to print itself out.
    virtual void print() const {}
  };

  template<typename In1, typename In2>
  class Consumer2 : public Consumer<In1>,
                    public Consumer<In2> {
  public:
    BoostPointers2(Consumer2, In1, In2);

  public:
    Consumer2()
      : has_in1(false),
        has_in2(false)
    {}

    virtual ~Consumer2(){}

    // The single-argument consume() methods take a value of the
    // appropriate type, then check to see whether the other method
    // has already been called; if so, it dispatches the two-argument
    // consume(), which must be supplied by the implementing class.
    //
    void consume(const In1& in){
      in1 = in;
      has_in1 = true;

      if(has_in2){
        this->consume2(in1, in2);
        has_in1 = false;
        has_in2 = false;
      }
    }

    void consume(const In2& in){
      in2 = in;
      has_in2 = true;

      if(has_in1){
        this->consume2(in1, in2);
        has_in1 = false;
        has_in2 = false;
      }
    }

    // This is where the "work" for this class gets done.
    virtual void consume2(const In1& in1, const In2& in2) = 0;

  protected:
    // const In1 *in1;
    // const In2 *in2;

    // Can't use pointers here; the pointers would become invalid by
    // the time the second of two consume() methods is called.
    In1 in1;
    In2 in2;
    bool has_in1;
    bool has_in2;
  };
}

#endif
