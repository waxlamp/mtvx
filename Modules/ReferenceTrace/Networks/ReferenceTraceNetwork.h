// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ReferenceTraceNetwork.h - A consumer of reference trace records
// that also manages an internal networked structure of MTV modules.

#ifndef REFERENCE_TRACE_NETWORK_H
#define REFERENCE_TRACE_NETWORK_H

// MTV includes.
#include <Core/Color/ColorGenerator.h>
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/LineRecordFilter.h>
#include <Core/Dataflow/MemoryRecordFilter.h>
#include <Core/Dataflow/Repeater.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>
//#include <Core/Util/Util.h>
#include <Modules/ReferenceTrace/Dataflow/CacheEventRenderDirector.h>
#include <Modules/ReferenceTrace/Dataflow/CacheStatusRenderDirector.h>
#include <Modules/ReferenceTrace/Dataflow/RecordRenderDirector.h>
#include <Modules/ReferenceTrace/Renderers/CacheRenderer.h>
#include <Modules/ReferenceTrace/Renderers/RegionRenderer.h>

namespace MTV{
  template<bool ComputeDelta>
  class ReferenceTraceNetwork$Template{
  public:
    BoostPointers(ReferenceTraceNetwork$Template);

  public:
    // TODO(choudhury): allow color generator to vary at runtime as a menu
    // item (or something).
    ReferenceTraceNetwork$Template()
      : colorgen(new RGBCorners),
        MRecordRepeater(new Repeater<MTR::Record>),
        cacheStatusRepeater(new Repeater<CacheStatusReport>),
        cacheAccessRepeater(new Repeater<CacheAccessRecord>),
        catchallCacheEventRenderDirector(new CacheEventRenderDirector(colorgen->color(0))),
        catchall(new AddressRangeStop)
        // , mfilter(new MemoryRecordFilter),
        //   lfilter(new LineRecordFilter)
    {
      // Connect the trace record source and cache event source to the
      // catchall object.
      MRecordRepeater->addConsumer(catchall);
      cacheAccessRepeater->addConsumer(catchall);

      // Connect the catchall cache event render director to the
      // address-stop filter.
      catchall->Producer<CacheAccessRecord>::addConsumer(catchallCacheEventRenderDirector);
    }

    Repeater<MTR::Record>::ptr getTraceRepeater() { return MRecordRepeater; }
    Repeater<CacheStatusReport>::ptr getCacheStatusRepeater() { return cacheStatusRepeater; }
    Repeater<CacheAccessRecord>::ptr getCacheAccessRepeater() { return cacheAccessRepeater; }

    // TODO(choudhury): add this method to the Module network.  Figure
    // out a better uniform methodology for calling it.
    std::vector<Memorable::ptr> getMemorables() {
      std::vector<Memorable::ptr> mems;
      foreach(Memorable::ptr p, regionRenderers){
        mems.push_back(p);
      }

      return mems;
    }

    // If the rows or cols arguments are passed in as zero, then the
    // region added will be rendered as a linear array; otherwise, the
    // dimensions are used to create a matrix view.
    //
    // TODO(choudhury): handle the matrix view case.
    typename RegionRenderer$Template<ComputeDelta>::ptr addRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type, const std::string& title="",
                                                                  int rows=0, int cols=0){
      const MTR::addr_t limit = base + size;

      // Generate a default region title if none was supplied.
      std::string truetitle;
      if(title == ""){
        std::stringstream ss;
        ss << "Region " << regionFilters.size();

        truetitle = ss.str();
      }
      else{
        truetitle = title;
      }

      // Create an AddressRangePass for the new region.
      AddressRangePass::ptr rangePass(new AddressRangePass(base, limit));
      regionFilters.push_back(rangePass);

      // Update the AddressRangeStop object with the same range.
      catchall->addRange(base, base+size);

      // Create a region director, cache status director, and a region renderer.
      //
      // NOTE(choudhury): the cache uses the 0th color in colorgen to
      // represent "other" regions; hence the regions being added use
      // colors from the generator AFTER the 0th.
      RecordRenderDirector::ptr director(new RecordRenderDirector(base, limit, type));
      typename RegionRenderer$Template<ComputeDelta>::ptr renderer(new RegionRenderer$Template<ComputeDelta>(Point(0.0, 0.0), base, limit, type, colorgen->color(regionRenderers.size()+1), truetitle));
      CacheEventRenderDirector::ptr eventDirector(new CacheEventRenderDirector(renderer->getShellColor()));
      CacheStatusRenderDirector::ptr statusDirector(new CacheStatusRenderDirector(base, limit, type));

      // Connect the memory record filter to the new components.
      //
      // First, the trace source and the cache result source connect to
      // the range filter.
      MRecordRepeater->addConsumer(rangePass);
      cacheStatusRepeater->addConsumer(rangePass);

      // The range filter connects to the render directors.
      rangePass->Producer<MTR::Record>::addConsumer(director);
      rangePass->Producer<CacheStatusReport>::addConsumer(statusDirector);

      // Finally, the directors connect to the region renderer.
      director->addConsumer(renderer);
      statusDirector->addConsumer(renderer);

      // Now, the cache event source connects to the range filter.
      cacheAccessRepeater->addConsumer(rangePass);
      rangePass->Producer<CacheAccessRecord>::addConsumer(eventDirector);
      if(cacheRenderer){
        eventDirector->addConsumer(cacheRenderer);
      }

      // Save a pointer to the new components.
      //
      // TODO(choudhury): may not need this step.
      regionDirectors.push_back(director);
      regionRenderers.push_back(renderer);
      cacheEventRenderDirectors.push_back(eventDirector);

      // Return the renderer pointer, as it will be useful to display
      // classes.
      return renderer;
    }

    CacheRenderer::ptr renderCache(Cache::const_ptr cache){
      // Create a new cache renderer.
      cacheRenderer = CacheRenderer::ptr(new CacheRenderer(cache, 100));

      // Connect all the cache event render directors to the new renderer.
      foreach(CacheEventRenderDirector::ptr d, cacheEventRenderDirectors){
        d->addConsumer(cacheRenderer);
      }

      // Connect the catchall render director to the cache renderer as
      // well.
      catchallCacheEventRenderDirector->addConsumer(cacheRenderer);

      return cacheRenderer;
    }

  public:
    static typename ReferenceTraceNetwork$Template::ptr newFromSpec(const std::string& regfile){
      // Attempt to read in the registration file - if we cannot, then
      // bail with a null pointer return value.
      MTR::RegionRegistration reg;

      if(!reg.read(regfile.c_str())){
        return typename ReferenceTraceNetwork$Template<ComputeDelta>::ptr();
      }

      // Create a reference trace network object, then populate it with
      // the regions appearing in the registration.
      typename ReferenceTraceNetwork$Template<ComputeDelta>::ptr net(new ReferenceTraceNetwork$Template<ComputeDelta>);
      foreach(MTR::ArrayRegion& r, reg.arrayRegions){
        net->addRegion(r.base, r.size, r.type, r.title);
      }

      return net;
    }

  private:
    // A color generator for the region renderer shells.
    ColorGenerator::ptr colorgen;

    // This will forward trace records to each point they need to go
    // (the regionFilters and the catchall below).
    Repeater<MTR::Record>::ptr MRecordRepeater;

    // These forward cache information to the appropriate places.
    Repeater<CacheStatusReport>::ptr cacheStatusRepeater;
    Repeater<CacheAccessRecord>::ptr cacheAccessRepeater;

    // Collection of paths to handle different memory regions.  Each
    // consists of an AddressRangePass coupled to a
    // RecordRenderDirector, coupled to a RegionRenderer.
    std::vector<AddressRangePass::ptr> regionFilters;
    std::vector<RecordRenderDirector::ptr> regionDirectors;
    std::vector<typename RegionRenderer$Template<ComputeDelta>::ptr> regionRenderers;

    // Each region also has a cache event render director (so that the
    // cache renderer knows about cache block occupancy, ID'd by
    // region color).
    std::vector<CacheEventRenderDirector::ptr> cacheEventRenderDirectors;

    // There is also a cache event render director associated with the
    // "catchall" address-stop filter - this is to show cache
    // occupancy from anywhere in memory not represented by a region.
    CacheEventRenderDirector::ptr catchallCacheEventRenderDirector;

    // A cache renderer.
    CacheRenderer::ptr cacheRenderer;

    // There is also an AddressRangeStop, which represents the
    // complement of the union of the region filters; this one is used
    // to send along reference records to the through the cache
    // simulator with their own identifying color (for displaying
    // cache events in the cache renderer).
    AddressRangeStop::ptr catchall;
  };

  typedef ReferenceTraceNetwork$Template<false> ReferenceTraceNetwork;
  typedef ReferenceTraceNetwork$Template<true> ReferenceTraceNetwork$ComputeDelta;
}

#endif
