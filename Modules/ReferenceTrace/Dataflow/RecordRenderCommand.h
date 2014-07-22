// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// RecordRenderCommand.h - A data item that is consumed by a
// RegionRenderer to post a graphical change to it.

#ifndef RECORD_RENDER_COMMAND_H
#define RECORD_RENDER_COMMAND_H

// MTV headers.
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  class RecordRenderCommand{
  public:
    RecordRenderCommand()
      : code(MTR::Record::NoCode),
        cell(static_cast<unsigned>(-1))
    {}

    RecordRenderCommand(MTR::Record::Code code, unsigned cell)
      : code(code),
        cell(cell)
    {}

    MTR::Record::Code code;
    unsigned cell;
  };
}

#endif
