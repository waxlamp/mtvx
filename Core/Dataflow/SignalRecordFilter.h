// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// SignalRecordFilter.h - Transforms accesses to "barrier" or "signal"
// addresses into TraceSignal objects.

#ifndef SIGNAL_RECORD_FILTER_H
#define SIGNAL_RECORD_FILTER_H

// MTV includes.
#include <Core/Dataflow/Filter.h>
#include <Core/Dataflow/TraceSignal.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// System includes.
#include <iostream>

namespace MTV{
  class SignalRecordFilter : public Filter<MTR::Record, TraceSignal> {
  public:
    BoostPointers(SignalRecordFilter);

  protected:
    SignalRecordFilter(MTR::addr_t base)
      : base(base)
    {}

  public:
    static SignalRecordFilter::ptr New(MTR::addr_t base){
      return SignalRecordFilter::ptr(new SignalRecordFilter(base));
    }

  public:
    void consume(const MTR::Record& rec){
      std::cout << "Signal: " << (rec.addr - base) << std::endl;

      this->produce(TraceSignal(rec.addr - base));
    }

  private:
    MTR::addr_t base;
  };
}

#endif
