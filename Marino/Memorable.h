// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Memorable.h - An interface expressing the idea that an object can
// save/restore its state to an MTV::Marino memento object.

#ifndef MEMORABLE_H
#define MEMORABLE_H

// MTV includes.
#include <Core/Dataflow/Producer.h>
#include <Core/Util/BoostPointers.h>
#include <Marino/Memento.pb.h>

namespace MTV{
  using Marino::DeltaMemento;
  using Marino::WarmMemento;

  class Memorable : public Producer<DeltaMemento> {
  public:
    BoostPointers(Memorable);

  public:
    // Memorable(bool computeDelta)
    //   : computeDelta(computeDelta)
    // {}

    // The produce() action is simply to forward the object.
    void produce(const DeltaMemento& t){
      this->broadcast(t);
    }

    void setId(int _id){
      id = _id;
    }

    int getId() const {
      return id;
    }

    // These methods work with mementos designed to be applied during
    // a runtime session (when the objects in question are "warm").
    virtual void loadWarm(const WarmMemento& state) = 0;
    virtual void saveWarm(WarmMemento& state) const = 0;

    // TODO(choudhury): {load|save}Cold().  These are designed for
    // constructing a complete object from disk (from "cold storage")
    // or creating such a snapshot to save to disk.

    // These methods deal with a change in the runtime state of an
    // object.  The change spec should also store the previous state
    // of the object, so that they can be "unapplied" as well.
    virtual void applyDelta(const DeltaMemento& delta) = 0;
    virtual void unapplyDelta(const DeltaMemento& delta) = 0;

  protected:
    int id;
  };
}

#endif
