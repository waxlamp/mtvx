// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilNetwork.h - A module for handling data from the Lentil MPM
// program.

#ifndef LENTIL_NETWORK_H
#define LENTIL_NETWORK_H

// MTV includes.
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/Ground.h>
#include <Core/Dataflow/Repeater.h>
#include <Core/Dataflow/Module.h>
#include <Modules/Lentil/Dataflow/DataRetrievalCommander.h>
// #include <Modules/Lentil/Dataflow/GridNodeSelector.h>
#include <Modules/Lentil/Dataflow/ParticleSelector.h>
#include <Modules/Lentil/Renderers/LentilDataRenderer.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  class LentilNetwork : public Module {
  public:
    BoostPointers(LentilNetwork);

  public:
    LentilNetwork(const std::string& dir);

    bool good() const { return ok; }

    LentilDataDisplay::ptr getWidget() { return renderer->getWidget(); }

    LentilDataRenderer::ptr getRenderer() { return renderer; }

    // Module interface - simply patch through to the network object.
    Consumer<MTR::Record>::ptr memoryRecordEntryPoint() { return MRepeater; }
    Consumer<MTR::Record>::ptr lineRecordEntryPoint() { return LGround; }
    Consumer<CacheAccessRecord>::ptr cacheAccessRecordEntryPoint() { return CAGround; }
    Consumer<CacheStatusReport>::ptr cacheStatusReportEntryPoint() { return CSRepeater; }
    Consumer<TraceSignal>::ptr signalEntryPoint() { return dataUpdateCommander; }

  private:
    // Data sources.
    //
    // NOTE(choudhury): The trace signals flow into an object that
    // triggers a data update when the appropriate signal is picked up
    // - that's the only receiver for trace signals, so there is no
    // need for a separate repeater object.
    Repeater<MTR::Record>::ptr MRepeater;
    Ground<MTR::Record>::ptr LGround;
    Repeater<CacheStatusReport>::ptr CSRepeater;
    Ground<CacheAccessRecord>::ptr CAGround;
    DataRetrievalCommander::ptr dataUpdateCommander;

    // The M-records and cache statuses both need to be filtered
    // through the address ranges for particles and grid nodes.
    std::vector<AddressRangePass::ptr> pRange;
    // std::vector<AddressRangePass::ptr> gridNodeRange;

    // The particle and grid node addresses feed into "selectors" that
    // identify the index of the particular piece of data being
    // accessed.
    std::vector<ParticleSelector::ptr> pSelector;
    // std::vector<GridNodeSelector::ptr> gnSelector;

    // All of these informational filters flow into the renderer.
    LentilDataRenderer::ptr renderer;

    bool ok;
  };
}

#endif
