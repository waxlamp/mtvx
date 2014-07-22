// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// MemoryRecordFilter.h - Looks at incoming trace records and only
// passes along those that are "M-type" (representing a reference to
// memory).

#ifndef MEMORY_RECORD_FILTER_H
#define MEMORY_RECORD_FILTER_H

// MTV headers.
#include <Core/Dataflow/Filter.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// TODO(choudhury): Create subclasses of MTR::Record that differ only
// in name (they add no data or methods) and static_cast to the right
// type (i.e. MTR::LineRecord, MTR::MemoryRecord, etc.) in the consume
// method before produce()ing.  Then downstream filters can accept the
// correct static type for a good compile-time check, without having
// to change the way those filters access the data in the record.
// This TODO applies as well to LineRecordFilter.h.

namespace MTV{
  class MemoryRecordFilter : public Filter<MTR::Record> {
  public:
    BoostPointers(MemoryRecordFilter);

  public:
    void consume(const MTR::Record& rec){
      if(MTR::type(rec) == MTR::Record::MType){
        this->produce(rec);
      }
    }
  };
}

#endif
