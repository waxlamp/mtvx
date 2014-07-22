// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// LineRecordFilter.h - Looks at incoming trace records and only
// passes along those that are "L-type" (representing information
// about the source code location of a sequence of references).

#ifndef LINE_RECORD_FILTER_H
#define LINE_RECORD_FILTER_H

// MTV headers.
#include <Core/Dataflow/Filter.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// TODO(choudhury): see TODO in MemoryRecordFilter.h.

namespace MTV{
  class LineRecordFilter : public Filter<MTR::Record> {
  public:
    BoostPointers(LineRecordFilter);

  public:
    void consume(const MTR::Record& rec){
      if(MTR::type(rec) == MTR::Record::LType){
        this->produce(rec);
      }
    }
  };
}

#endif
