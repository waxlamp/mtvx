// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// RecordRenderDirector.h - Converts references into appropriate
// RenderCommand instances.

#ifndef RECORD_RENDER_DIRECTOR_H
#define RECORD_RENDER_DIRECTOR_H

// MTV headers.
#include <Core/Util/BoostPointers.h>
#include <Modules/ReferenceTrace/Dataflow/RecordRenderCommand.h>

namespace MTV{
  // TODO(choudhury): "limit" is not used here, and is not useful
  // aside from range-checking.  Get rid of it?
  class RecordRenderDirector : public Filter<MTR::Record, RecordRenderCommand> {
  public:
    BoostPointers(RecordRenderDirector);

  public:
    RecordRenderDirector(MTR::addr_t base, MTR::addr_t limit, MTR::size_t type)
      : base(base),
        limit(limit),
        type(type)
    {}

    void consume(const MTR::Record& rec){
      RecordRenderCommand command(rec.code, (rec.addr - base) / type);
      this->produce(command);
    }

  private:
    MTR::addr_t base, limit;
    MTR::size_t type;
  };
}

#endif
