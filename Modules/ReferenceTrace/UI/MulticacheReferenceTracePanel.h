// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// MultiReferenceTracePanel.h - A concrete derived class of
// WidgetPanel, for displaying regions associated to multiple caches.

#ifndef MULTICACHE_REFERENCE_TRACE_PANEL_H
#define MULTICACHE_REFERENCE_TRACE_PANEL_H

// MTV headers.
#include <Core/Dataflow/Ground.h>
#include <Core/Dataflow/Module.h>
#include <Modules/ReferenceTrace/Networks/MulticacheReferenceTraceNetwork.h>

// TODO(choudhury): the "panel" classes always contain a "network"
// class.  They also patch through many of the network object's public
// methods.  It might be better to simply inherit from the network
// class, and avoid this code patch-through.  This makes better design
// sense anyway - a panel class is really just the network class with
// support for display within the Qt system.

namespace MTV{
  class MulticacheReferenceTracePanel : public WidgetPanel,
                                        public Module {
    // NOTE(choudhury): This class doesn't get a BoostPointer typedef,
    // because it is managed by the Qt system.

  public:
    MulticacheReferenceTracePanel();

    MulticacheReferenceTraceNetwork::ptr getNetwork() { return net; }

    // CacheRenderer::ptr addCache(Cache::const_ptr cache) { return net->addCache(cache); }

    // typename RegionRenderer$Template<ComputeDelta>::ptr addRegion(unsigned which,
    //                                                               MTR::addr_t base, MTR::size_t size, MTR::size_t type, const std::string& title="",
    //                                                               int rows=0, int cols=0){
    //   return net->addRegion(which, base, size, type, title, rows, cols);
    // }

    // Module interface.
    Consumer<MTR::Record>::ptr memoryRecordEntryPoint(){
      return net->getTraceRepeater();
    }

    Consumer<MTR::Record>::ptr lineRecordEntryPoint(){
      // TODO(choudhury): The module class should provide default
      // functions that return a ground object.
      return Ground<MTR::Record>::instance();
    }

    Consumer<CacheAccessRecord>::ptr cacheAccessRecordEntryPoint(){
      return Ground<CacheAccessRecord>::instance();
    }

    Consumer<CacheStatusReport>::ptr cacheStatusReportEntryPoint(){
      return Ground<CacheStatusReport>::instance();
    }

    Consumer<TraceSignal>::ptr signalEntryPoint(){
      return Ground<TraceSignal>::instance();
    }

    static MulticacheReferenceTracePanel *newFromSpec(const std::string& regfile);

  private:
    MulticacheReferenceTraceNetwork::ptr net;
  };
}

#endif
