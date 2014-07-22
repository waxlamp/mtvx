// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// Module.h - Interface class describing MTV's "modules" - this simply
// means such modules have a way to report the entry point for trace
// records.

#ifndef MODULE_H
#define MODULE_H

// MTV includes.
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/TraceSignal.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  class Module{
  public:
    virtual Consumer<MTR::Record>::ptr memoryRecordEntryPoint() = 0;
    virtual Consumer<MTR::Record>::ptr lineRecordEntryPoint() = 0;
    virtual Consumer<CacheAccessRecord>::ptr cacheAccessRecordEntryPoint() = 0;
    virtual Consumer<CacheStatusReport>::ptr cacheStatusReportEntryPoint() = 0;
    virtual Consumer<TraceSignal>::ptr signalEntryPoint() = 0;
  };
}

#endif
