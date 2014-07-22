// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ParticleSelector.h - Examines a particle data reference, and
// determines which particle (by index) the reference represents.
// Also bundles this information with a cache status report.

#ifndef PARTICLE_SELECTOR_H
#define PARTICLE_SELECTOR_H

// MTV includes.
#include <Modules/Lentil/Dataflow/ParticleStatusRenderCommand.h>

namespace MTV{
  class ParticleSelector : public Consumer<MTR::Record>,
                           public Filter<CacheStatusReport, ParticleStatusRenderCommand> {
  public:
    BoostPointers(ParticleSelector);

  protected:
    ParticleSelector(MTR::addr_t base, MTR::size_t type)
      : base(base),
        type(type)
    {}

  public:
    static ParticleSelector::ptr New(MTR::addr_t base, MTR::size_t type){
      return ParticleSelector::ptr(new ParticleSelector(base, type));
    }
    
    // This method ONLY sets some information.
    void consume(const MTR::Record& rec){
      pIndex = (rec.addr - base) / type;
    }

    // This method generates a rendering command based on the index
    // computed by the call to consume(const MTR::Record&), and the
    // cache status report being consumed now.
    void consume(const CacheStatusReport& report){
      this->produce(ParticleStatusRenderCommand(pIndex, report.color));
    }

  private:
    MTR::addr_t base;
    MTR::size_t type;

    unsigned pIndex;
  };
}

#endif
