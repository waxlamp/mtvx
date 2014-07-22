// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampFifoNetwork.cpp

// MTV headers.
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/ArrayIndexer.h>
#include <Core/Dataflow/ItemSelector.h>
#include <Core/Graphics/FadingPoint.h>
#include <Core/Graphics/GLPoint.h>
#include <Modules/ReferenceTrace/Networks/WaxlampFifoNetwork.h>
using MTV::AddressRangePass;
using MTV::ArrayIndexer;
using MTV::Clock;
using MTV::FadingPoint;
using MTV::GLPoint;
using MTV::ItemSelector;
using MTV::WaxlampFifoNetwork;
using MTV::Widget;

WaxlampFifoNetwork::WaxlampFifoNetwork(float duration, bool animating, Clock::ptr clock)
  : MRecordRepeater(new Repeater<MTR::Record>),
    cacheGrouper(new AccessGrouper(Point(360, 360), animating, 100, 20, 300, duration, duration, clock, true)),
    clock(clock)
{}

std::vector<Widget::ptr> WaxlampFifoNetwork::addRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type){
  // Create a region filter, feed the result to an item selector, and
  // then hook that to a cache grouper.

  // Get the current time.
  const float now = clock->noww();

  // Create a region filter, and connect the trace source to it.
  const MTR::addr_t limit = base + size;
  AddressRangePass::ptr pass(new AddressRangePass(base, limit));
  MRecordRepeater->addConsumer(pass);

  // Create an indexer, and hook the address filter to it.
  ArrayIndexer::ptr indexer(new ArrayIndexer(base, type));
  pass->Filter<MTR::Record>::addConsumer(indexer);

  // Create a widget selector, and populate it with several widgets.
  std::vector<Widget::ptr> widgets;
  ItemSelector<Widget::ptr>::ptr selector(new ItemSelector<Widget::ptr>);
  for(unsigned i=0; i<(limit - base) / type; i++){
    // GLPoint::ptr w(new GLPoint(10));
    FadingPoint::ptr w(new FadingPoint(20));
    widgets.push_back(w);
    selector->addItem(w);

    // TODO(choudhury): testing.
    cacheGrouper->getAllGrouper()->addWidget(w, now);
  }

  cacheGrouper->getAllGrouper()->marshal(now);

  // Hook the indexer to the selector.
  indexer->addConsumer(selector);

  // Hook the selector up to the cache grouper.
  selector->addConsumer(cacheGrouper);

  // Return the list of widgets created for this region.
  return widgets;
}
