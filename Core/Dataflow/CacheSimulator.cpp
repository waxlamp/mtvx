// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheSimulator.cpp

// MTV includes.
// #include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/CacheSimulator.h>
// #include <Core/Dataflow/CacheStatusReport.h>
#include <Core/Util/LevelComparator.h>
using MTV::CacheAccessRecord;
using MTV::CacheSimulator;
using MTV::CacheStatusReport;
using MTV::Producer;

CacheSimulator::CacheSimulator(Cache::ptr c)
  : c(c)
{}

void CacheSimulator::consume(const MTR::Record& rec){
  // Perform a step of cache simulation.
  switch(rec.code){
  case MTR::Record::Read:
    c->load(rec.addr);
    break;

  case MTR::Record::Write:
    c->store(rec.addr);
    break;

  default:
    // TODO(choudhury): MTR::Record::LineNumber should never show up
    // here.  Cause the appropriate error condition.
    break;
  }

  std::cout << "INFO REPORT" << std::endl;
  c->infoReport();
  std::cout << "-----" << std::endl;

  // Broadcast the hit info.
  this->Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport>::produce(CacheAccessRecord(c->hitInfo(),
                                                                                              c->evictionInfo(),
                                                                                              c->entranceInfo(), rec.addr));

  // Compute the proper cache status color (ranging from blue for a
  // hit in the lowest level up to red for a miss to main memory).
  //
  // Start by computing the highest level hit.
  const float highest = std::max_element(c->hitInfo().begin(), c->hitInfo().end(), LevelComparator<Daly::CacheHitRecord>())->L;
  // const float red = highest / c->numLevels();

  // TODO(choudhury): put these colors in a profile.
  Color out = Color::black;
  if(static_cast<int>(highest) == 0){
    out = Color(0.0, 143./255., 1.0);
  }
  else if(static_cast<int>(highest) == 1){
    out = Color(0.5, 0.0, 0.5);
  }
  else{
    out = Color(1.0, 0.0, 0.0);
  }

  // Construct a CacheStatusReport and broadcast it.
  // this->Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport>::produce(CacheStatusReport(rec.addr, Color(red, 0.0, 1.0 - red)));
  this->Filter2<MTR::Record, CacheAccessRecord, CacheStatusReport>::produce(CacheStatusReport(rec.addr, out));
}
