// Copyright 2011 A.N.M. Imroz Choudhury
//
// HitLevelCounter.cpp

// MTV headers.
#include <Core/Dataflow/HitLevelCounter.h>
using MTV::HitLevelCounter;

HitLevelCounter::~HitLevelCounter(){
  out.flush();
  out.close();
}

bool HitLevelCounter::open(const std::string& filename){
  out.open(filename.c_str());
  return out.good();
}

void HitLevelCounter::consume(const CacheAccessRecord& rec){
  // Compute the level of cache in which the reference's data block
  // was found.
  unsigned L = 0;
  for(unsigned i=0; i<rec.hits.size(); i++){
    if(L < rec.hits[i].L){
      L = rec.hits[i].L;
    }
  }

  if(panel){
    out << panel->getFrame() << ' ';
  }
  // out << rec.addr << ' ' << L << std::endl;
  out.write(reinterpret_cast<const char *>(&L), sizeof(L));
  out.flush();
}
