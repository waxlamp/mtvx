// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// DeltaMementoRecorder.h - An object that consumes DeltaMemento
// objects and serializes them to disk.

#ifndef DELTA_MEMENTO_RECORDER_H
#define DELTA_MEMENTO_RECORDER_H

// MTV includes.
#include <Core/Dataflow/Consumer.h>
#include <Core/Util/BoostPointers.h>
#include <Marino/Memento.pb.h>
using MTV::Marino::DeltaMemento;

// System includes.
#include <fstream>

namespace MTV{
  class DeltaMementoRecorder : public Consumer<DeltaMemento> {
  public:
    BoostPointers(DeltaMementoRecorder);

  public:
    // Codes to identify file type.
    enum FileType{
      None = 0,
      Immediate,
      Chunked
    };

  public:
    virtual ~DeltaMementoRecorder() {};

    // TODO(choudhury): see if it's worthwhile to implement this:
    // http://www.artima.com/cppsource/safebool.html
    virtual operator bool() const = 0;

    virtual void consume(const DeltaMemento& delta) = 0;
  };

  class ImmediateDeltaMementoRecorder : public DeltaMementoRecorder {
  public:
    BoostPointers(ImmediateDeltaMementoRecorder);

  public:
    ImmediateDeltaMementoRecorder(const std::string& filename);
    ~ImmediateDeltaMementoRecorder();

    operator bool() const {
      return static_cast<bool>(out);
    }

    void consume(const DeltaMemento& delta);

  protected:
    std::ofstream out;
  };
}

#endif
