// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheStatusRenderDirector.h - A filter that turns a
// CacheStatusReport into a CacheStatusRenderCommand.

#ifndef CACHE_STATUS_RENDER_DIRECTOR_H
#define CACHE_STATUS_RENDER_DIRECTOR_H

// MTV includes.
#include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Dataflow/Filter.h>
#include <Modules/ReferenceTrace/Dataflow/CacheStatusRenderCommand.h>

namespace MTV{
  class CacheStatusRenderDirector : public Filter<CacheStatusReport, CacheStatusRenderCommand> {
  public:
    CacheStatusRenderDirector(MTR::addr_t base, MTR::addr_t limit, MTR::size_t type)
      : base(base),
        limit(limit),
        type(type)
    {}

    void consume(const CacheStatusReport& report){
      this->produce(CacheStatusRenderCommand((report.addr - base) / type, report.color));
    }

  private:
    MTR::addr_t base, limit;
    MTR::size_t type;
  };
}

#endif
