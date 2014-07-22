// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheEventRenderDirector.cpp

// MTV includes.
#include <Modules/ReferenceTrace/Dataflow/CacheEventRenderDirector.h>
using MTV::CacheAccessRecord;
using MTV::CacheEventRenderDirector;

CacheEventRenderDirector::CacheEventRenderDirector(const Color& color)
  : color(color)
{}

void CacheEventRenderDirector::consume(const CacheAccessRecord& rec){
  this->produce(CacheEventRenderCommand(rec.hits, color));
}
