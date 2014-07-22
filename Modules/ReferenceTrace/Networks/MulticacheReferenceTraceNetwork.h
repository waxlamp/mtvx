// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// MulticacheReferenceTraceNetwork.h - A version of the reference
// trace network that can support multiple caches, with address-based
// decision of which cache to send which references.

#ifndef MULTICACHE_REFERENCE_TRACE_NETWORK_H
#define MULTICACHE_REFERENCE_TRACE_NETWORK_H

// MTV headers.
#include <Core/Dataflow/CacheSimulator.h>
#include <Core/Color/ColorGenerator.h>
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/Repeater.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Util.h>
#include <Modules/ReferenceTrace/Dataflow/CacheEventRenderDirector.h>
#include <Modules/ReferenceTrace/Dataflow/CacheStatusRenderDirector.h>
#include <Modules/ReferenceTrace/Dataflow/RecordRenderDirector.h>
#include <Modules/ReferenceTrace/Renderers/CacheRenderer.h>
#include <Modules/ReferenceTrace/Renderers/RegionRenderer.h>

namespace MTV{
  template<bool ComputeDelta>
  class MulticacheReferenceTraceNetwork$Template{
  public:
    BoostPointers(MulticacheReferenceTraceNetwork$Template);

  public:
    MulticacheReferenceTraceNetwork$Template()
      : colorgen(new RGBCorners),
        MRecordRepeater(new Repeater<MTR::Record>)
      // ,
      //   cacheStatusRepeater(new Ground<CacheStatusReport>),
      //   cacheAccessRepeater(new Ground<CacheAccessRecord>),
    {}

    Repeater<MTR::Record>::ptr getTraceRepeater() { return MRecordRepeater; }

    CacheRenderer::ptr addCache(Cache::ptr cache){
      // Construct a cache simulator and a renderer from the cache.
      CacheSimulator::ptr sim(new CacheSimulator(cache));
      CacheRenderer::ptr rend(new CacheRenderer(cache));

      // Track the objects in-class.
      sims.push_back(sim);
      rends.push_back(rend);

      return rend;
    }

    typename RegionRenderer$Template<ComputeDelta>::ptr addRegion(unsigned which,
                                                                  MTR::addr_t base, MTR::size_t size, MTR::size_t type, const std::string& title="",
                                                                  int rows=0, int cols=0){
      const MTR::addr_t limit = base + size;

      // Generate a default title.
      const std::string truetitle = MTV::makeTitle(title, regionFilters.size());

      // One address pass filter to select addresses coming from the
      // trace within the new region, and another to filter cache
      // status events coming from the simulator for that region (they
      // will be filtering different objects at different parts of the
      // network).
      AddressRangePass::ptr traceRangePass(new AddressRangePass(base, limit));
      AddressRangePass::ptr cacheRangePass(new AddressRangePass(base, limit));

      // Create some data processing objects needed for the new
      // visualization processing paths.
      RecordRenderDirector::ptr director(new RecordRenderDirector(base, limit, type));
      typename RegionRenderer$Template<ComputeDelta>::ptr renderer(new RegionRenderer$Template<ComputeDelta>(Point(0.0,0.0), base, limit, type, colorgen->color(regionRenderers.size()+1), truetitle));
      CacheEventRenderDirector::ptr eventDirector(new CacheEventRenderDirector(renderer->getShellColor()));
      CacheStatusRenderDirector::ptr statusDirector(new CacheStatusRenderDirector(base, limit, type));

      // The trace reader connects to the trace filter.
      MRecordRepeater->addConsumer(traceRangePass);

      // The trace filter sends its records to both the cache
      // simulator and the region render director.
      CacheSimulator::ptr sim = sims[which];
      traceRangePass->Producer<MTR::Record>::addConsumer(sim);
      traceRangePass->Producer<MTR::Record>::addConsumer(director);

      // The cache simulator sends its event feed to the event
      // director, and its status feed to the other address range
      // filter, which in turn feeds statuses to the status director.
      sim->Producer<CacheAccessRecord>::addConsumer(eventDirector);
      sim->Producer<CacheStatusReport>::addConsumer(cacheRangePass);
      cacheRangePass->Producer<CacheStatusReport>::addConsumer(statusDirector);

      // The event director connects to the proper cache renderer.
      CacheRenderer::ptr rend = rends[which];
      eventDirector->addConsumer(rend);

      // The record and status directors feed to the region renderer.
      director->addConsumer(renderer);
      statusDirector->addConsumer(renderer);
    }

  private:
    // For region renderer shells.
    ColorGenerator::ptr colorgen;

    // Forward trace records to the region filters.
    Repeater<MTR::Record>::ptr MRecordRepeater;

    // Collection of paths to handle different memory regions.  Each
    // consists of an AddressRangePass coupled to a
    // RecordRenderDirector, coupled to a RegionRenderer.
    std::vector<AddressRangePass::ptr> regionFilters;
    std::vector<RecordRenderDirector::ptr> regionDirectors;
    std::vector<typename RegionRenderer$Template<ComputeDelta>::ptr> regionRenderers;

    // The regions in this network are bound to specific caches, so
    // these simulators must be run on the incoming records AFTER they
    // are filtered as for the region rendering paths above.
    std::vector<CacheSimulator::ptr> sims;
    std::vector<CacheRenderer::ptr> rends;
  };

  typedef MulticacheReferenceTraceNetwork$Template<false> MulticacheReferenceTraceNetwork;
  typedef MulticacheReferenceTraceNetwork$Template<true> MulticacheReferenceTraceNetwork$ComputeDelta;
}

#endif
