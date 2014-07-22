// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheAccessRecord.h - A record showing the changes effected in a
// cache from the access of a given address.

#ifndef CACHE_STATUS_REPORT_H
#define CACHE_STATUS_REPORT_H

// MTV includes.
#include <Core/Color/Color.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  struct CacheStatusReport{
    CacheStatusReport(MTR::addr_t addr, const Color& color)
      : addr(addr),
        color(color)
    {}

    MTR::addr_t addr;
    Color color;
  };
}

#endif
