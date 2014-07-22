// Copyright 2011 A.N.M. Imroz Choudhury
//
// SetUtilizationCounter.cpp

// MTV headers.
#include <Core/Dataflow/SetUtilizationCounter.h>
using MTV::CacheAccessRecord;
using MTV::SetUtilizationCounter;

void SetUtilizationCounter::consume(const CacheAccessRecord& rec){
  static unsigned call_count = 0;

  counter->consume(rec);
  if(++call_count == period){
    CacheHitRates rates = counter->rates();

    if(textfile){
      out << panel->getFrame() << ' ';
      for(unsigned i=0; i<rates.getRates().size(); i++){
        out << rates[i] << ' ';
      }
      out << std::endl;
    }
    else{
      for(unsigned i=0; i<rates.getRates().size(); i++){
        setutilPB.add_utilization(rates[i]);
      }
    }

    call_count = 0;
  }
}
