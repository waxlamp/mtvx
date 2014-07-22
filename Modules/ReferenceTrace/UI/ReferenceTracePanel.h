// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ReferenceTracePanel.h - A concrete derived class of WidgetPanel,
// for displaying regions and a cache widget.

#ifndef REFERENCE_TRACE_PANEL_H
#define REFERENCE_TRACE_PANEL_H

// MTV includes.
#include <Core/Dataflow/Ground.h>
#include <Core/Dataflow/Module.h>
#include <Core/Dataflow/Repeater.h>
#include <Core/Color/ColorProfile.h>
#include <Core/UI/WidgetManipulationPanel.h>
#include <Modules/ReferenceTrace/Networks/ReferenceTraceNetwork.h>
#include <Modules/ReferenceTrace/Renderers/RegionRenderer.h>

// System includes.
#include <string>

namespace MTV{
  class ReferenceTracePanel : public WidgetManipulationPanel,
                              public Module {
    // NOTE(choudhury): This class doesn't get a BoostPointer typedef,
    // because it is managed by the Qt system.

  public:
    ReferenceTracePanel();

    ReferenceTraceNetwork::ptr getNetwork() { return net; }

    void addRegion(MTR::addr_t base, MTR::addr_t limit, MTR::size_t type, const std::string& title = "");

    void renderCache(Cache::const_ptr cache);

    void setSignalBase(MTR::addr_t base) { signal_base = base; }
    MTR::addr_t getSignalBase() const { return signal_base; }
    void setSignalLimit(MTR::addr_t limit) { signal_limit = limit; }
    MTR::addr_t getSignalLimit() const { return signal_limit; }

    // Module interface.
    Consumer<MTR::Record>::ptr memoryRecordEntryPoint(){
      return net->getTraceRepeater();
    }

    Consumer<MTR::Record>::ptr lineRecordEntryPoint(){
      return Ground<MTR::Record>::instance();
    }

    Consumer<CacheAccessRecord>::ptr cacheAccessRecordEntryPoint(){
      return net->getCacheAccessRepeater();
    }

    Consumer<CacheStatusReport>::ptr cacheStatusReportEntryPoint(){
      return net->getCacheStatusRepeater();
    }

    Consumer<TraceSignal>::ptr signalEntryPoint(){
      // TODO(choudhury): this module ought to show the signal
      // accesses.
      return Ground<TraceSignal>::instance();
    }

  public:
    static ReferenceTracePanel *newFromSpec(const std::string& regfile);

  private:
    ReferenceTraceNetwork::ptr net;
    MTR::addr_t signal_base, signal_limit;
  };
}

#endif
