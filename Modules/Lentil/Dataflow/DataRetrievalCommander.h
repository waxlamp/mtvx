// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// DataRetrievalCommander.h - An object that monitors incoming trace
// signals, and generates a command to read in the next frame of data
// when a specific one comes through.

#ifndef DATA_RETRIEVAL_COMMANDER_H
#define DATA_RETRIEVAL_COMMANDER_H

// MTV includes.
#include <Core/Dataflow/Filter.h>
#include <Core/Dataflow/TraceSignal.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  // Empty class that serves as a command in itself.
  class RetrieveData {};

  class DataRetrievalCommander : public Filter<TraceSignal, RetrieveData> {
  public:
    BoostPointers(DataRetrievalCommander);

  protected:
    DataRetrievalCommander(int signal)
      : signal(signal)
    {}

  public:
    static DataRetrievalCommander::ptr New(int signal){
      return DataRetrievalCommander::ptr(new DataRetrievalCommander(signal));
    }

    void consume(const TraceSignal& s){
      if(s.signal() == signal){
        this->produce(RetrieveData());
      }
    }

  private:
    int signal;
  };
}

#endif
