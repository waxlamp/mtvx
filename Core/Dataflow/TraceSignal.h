// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// TraceSignal.h - Encapsulates the idea of the trace having accessed
// one of the "barrier" addresses.  These are better termed "signals"
// since they are used to signal some condition at runtime.

#ifndef TRACE_SIGNAL_H
#define TRACE_SIGNAL_H

// MTV includes.
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class TraceSignal{
  public:
    BoostPointers(TraceSignal);

  public:
    TraceSignal(int sig)
      : sig(sig)
    {}

    int signal() const { return sig; }

  private:
    int sig;
  };
}

#endif
