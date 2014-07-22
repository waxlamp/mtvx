// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ArrayIndexer.h - Converts address into array indices.

#ifndef ARRAY_INDEXER_H
#define ARRAY_INDEXER_H

// MTV headers.
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  class ArrayIndexer : public Filter<MTR::Record, unsigned> {
  public:
    BoostPointers(ArrayIndexer);

  public:
    ArrayIndexer(MTR::addr_t base, MTR::size_t type)
      : base(base),
        type(type)
    {}

    void consume(const MTR::Record& rec){
      this->produce(static_cast<unsigned>((rec.addr - base) / type));
    }

  private:
    MTR::addr_t base;
    MTR::size_t type;
  };
}

#endif
