// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampCacheNetwork.cpp

// MTV headers.
#include <Core/Dataflow/Printer.h>
#include <Core/Geometry/ParameterAffineRemapper.h>
#include <Core/Geometry/RandomAnnulus.h>
#include <Core/Geometry/RandomDisc.h>
#include <Core/Graphics/FadingPoint.h>
#include <Core/Graphics/GLPoint.h>
#include <Modules/ReferenceTrace/Networks/WaxlampCacheNetwork.h>
using Daly::Cache;
using MTV::AddressRangePass;
using MTV::CacheAccessRecord;
using MTV::CachePerformanceCounter;
using MTV::CacheSimulator;
using MTV::Clock;
using MTV::ColorbrewerQualitative9_Set1;
using MTV::ColorbrewerQualitative11_Paired;
using MTV::FadingPoint;
using MTV::GLPoint;
using MTV::LineCacheMissCounter;
using MTV::LineRecordFilter;
using MTV::LineVisitCounter;
using MTV::MemoryRecordFilter;
using MTV::ParameterAffineRemapper;
using MTV::Printer;
using MTV::RandomAnnulus;
using MTV::RandomDisc;
using MTV::WaxlampCacheNetwork;
using MTV::Widget;

WaxlampCacheNetwork::WaxlampCacheNetwork(Cache::ptr c, bool animating, float duration, Clock::ptr clock, ColorGenerator::ptr colorgen)
  : mfilter(boost::make_shared<MemoryRecordFilter>()),
    lfilter(boost::make_shared<LineRecordFilter>()),
    MRecordRepeater(new Repeater<MTR::Record>),
    simulator(new CacheSimulator(c)),
    // visitcounter(boost::make_shared<LineVisitCounter>()),
    // misscounter(boost::make_shared<LineCacheMissCounter>()),
    cacheGrouper(new CacheGrouper(c, animating, duration, clock)),
    duration(duration),
    // colorgen(boost::make_shared<Colormap>()),
    colorgen(colorgen),
    temperature(boost::make_shared<CacheTemperatureRenderer>(CacheGrouper::center, c->num_levels(), 150,
                                                             // Color(0.5,0,0,0.05), Color::white, Color(0,0,0.5,0.05)))
                                                             Color(0.0,0,0.5), Color::white, Color(0.5,0,0.0)))
{
  // Set up a path for line records to be processed.
  MRecordRepeater->addConsumer(lfilter);

  // // Open a file for the line visit counter, and connect the line
  // // record filter to it.
  // if(!visitcounter->open("visit-count.dat")){
  //   std::cerr << "error: can't open 'visit-count.dat' for writing." << std::endl;

  //   // TODO(choudhury): throw exception instead.
  //   exit(1);
  // }
  // lfilter->addConsumer(visitcounter);

  // // Open a file for the cache miss counter, and connect the line
  // // record filter and the simulator to it.
  // if(!misscounter->open("miss-count.dat")){
  //   std::cerr << "error: can't open 'miss-count.dat' for writing." << std::endl;

  //   // TODO(choudhury): throw exception instead.
  //   exit(1);
  // }
  // lfilter->addConsumer(misscounter);
  // simulator->Producer<CacheAccessRecord>::addConsumer(misscounter);

  // Connect the trace source to the cache simulator.
  MRecordRepeater->addConsumer(mfilter);
  mfilter->addConsumer(simulator);

  // Create a performance counter for the cache simulator.
  //
  perf = boost::make_shared<CachePerformanceCounter>(1);
  CacheTemperaturePolicy::ptr tempPolicy = boost::make_shared<CacheTemperaturePolicy>(c, 100);
  perf->setPolicy(tempPolicy);
  // MTV::HitHistoryManager mgr(100);
  // Cache::const_ptr cc = c;
  // perf = boost::make_shared<CachePerformanceCounter>(cc, mgr, 1);

  simulator->Producer<CacheAccessRecord>::addConsumer(perf);

  // Attach the counter to the temperature renderer.
  perf->addConsumer(temperature);
}

std::vector<Widget::ptr> WaxlampCacheNetwork::addRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type){
  std::cout << "addRegion 0x" << std::hex << base << ", " << std::dec << size << ", " << type << std::endl;

  // Check to see if the region already exists.
  boost::unordered_map<MTR::addr_t, AddressRangePass::ptr>::const_iterator i = range_passes.find(base);  
  if(i != range_passes.end()){
    // Do a sanity check - are the base AND limit the same as what's
    // now being requested?
    AddressRangePass::const_ptr pass = i->second;
    if(base != pass->firstBase() or (base + size != pass->firstLimit())){
      std::cerr << "WARNING: reuse of region with different size." << std::endl;
      abort();
    }

    return std::vector<Widget::ptr>();
  }

  // Create a region filter, connect the cache simulator to it, and
  // connect it to the cache grouper.
  const MTR::addr_t limit = base + size;
  // AddressRangePass::ptr pass(new AddressRangePass(base, limit));
  AddressRangePass::ptr pass(new AddressRangePass(base, limit));
  simulator->Producer<CacheAccessRecord>::addConsumer(pass);
  pass->Producer<CacheAccessRecord>::addConsumer(cacheGrouper);

  std::cout << "adding range pass:" << std::endl;
  // pass->print();

  // Save the address range pass in a table for easy retrieval later
  // (to allow for removal from the network).
  range_passes[base] = pass;

  // Create a new region area in the cache grouper.
  unsigned r;

  std::cout << "numRegions: " << cacheGrouper->numRegions() << std::endl;

  if(cacheGrouper->numRegions() == 0){
    // Giant circle.
    r = cacheGrouper->addRegion(CacheGrouper::center, CacheGrouper::outer_radius, base, type);

    // std::cout << "First region! Circle is (" << CacheGrouper::center << ", " << CacheGrouper::outer_radius << ")" << std::endl;
  }

  else{
    // Reset circles.
    //
    // Divide one circle-turn by the number of regions (including the
    // new one).
    const float angle_interval = 2*M_PI / (cacheGrouper->numRegions() + 1);

    // Move through the current regions and reset their circles.
    unsigned i;
    for(i=0; i<cacheGrouper->numRegions(); i++){
      const float theta = (i + 0.5)*angle_interval;
      // const Vector v = (CacheGrouper::outer_radius + 0.5*CacheGrouper::circle_radius)*Vector(cos(theta), sin(theta));

#if 1
      ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(CacheGrouper::center, CacheGrouper::outer_radius);
      ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle,
                                                                                       (theta - 0.5*angle_interval) / (2*M_PI),
                                                                                       (theta + 0.5*angle_interval) / (2*M_PI));
      cacheGrouper->resetRegionShape(i, remap);
#elif 0
      cacheGrouper->resetRegionShape(i, boost::make_shared<RandomDisc>(CacheGrouper::center + v, 0.5*CacheGrouper::circle_radius, 50));
#elif 0
      cacheGrouper->resetRegionShape(i, boost::make_shared<RandomAnnulus>(CacheGrouper::center,
                                                                          CacheGrouper::outer_radius,
                                                                          CacheGrouper::outer_radius + CacheGrouper::circle_radius,
                                                                          theta - 0.5*angle_interval,
                                                                          theta + 0.5*angle_interval,
                                                                          50));
#endif
    }

    const float theta = (i + 0.5)*angle_interval;
    const Vector v = (CacheGrouper::outer_radius + 0.5*CacheGrouper::circle_radius)*Vector(cos(theta), sin(theta));
    r = cacheGrouper->addRegion(CacheGrouper::center + v, 0.5*CacheGrouper::circle_radius, base, type);
#if 1
    ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(CacheGrouper::center, CacheGrouper::outer_radius);
    ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle,
                                                                                     (theta - 0.5*angle_interval) / (2*M_PI),
                                                                                     (theta + 0.5*angle_interval) / (2*M_PI));
    cacheGrouper->resetRegionShape(r, remap);
#elif 0
    cacheGrouper->resetRegionShape(r, boost::make_shared<RandomDisc>(CacheGrouper::center + v, 0.5*CacheGrouper::circle_radius, 50));
#elif 0
    cacheGrouper->resetRegionShape(i, boost::make_shared<RandomAnnulus>(CacheGrouper::center,
                                                                        CacheGrouper::outer_radius, 
                                                                        CacheGrouper::outer_radius + CacheGrouper::circle_radius,
                                                                        theta - 0.5*angle_interval,
                                                                        theta + 0.5*angle_interval,
                                                                        50));
#endif
  }

  // Create widgets and add them to the cache grouper.
  std::vector<Widget::ptr> widgets;
  for(unsigned i=0; i<(limit - base) / type; i++){
    // Select a color.
    //
    // const Color c = cacheGrouper->numRegions() == 1 ? Color::black : colorgen->color(cacheGrouper->numRegions() - 2);
    const Color c = colorgen->color(cacheGrouper->numRegions() - 1);

    FadingPoint::ptr w(new FadingPoint(Point::zero, c, CacheGrouper::widgetSize));
    widgets.push_back(w);

    // Add the widget to the cache grouper, but do not marshal
    // (marshal all widgets at once below).
    cacheGrouper->addWidget(base + i*type, w, r, false);
  }

  // // Make sure the new widgets fall into place.
  // cacheGrouper->marshal();

  // Return the list of widgets created for this region.
  return widgets;
}

void WaxlampCacheNetwork::removeRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type){
  std::cerr << "Removing region! 0x" << std::hex << base << ", " << std::dec << size << ", " << type << std::endl;

  // Remove the region from the cache grouper.
  cacheGrouper->removeRegion(base, size, type);

  // Find the address range filter associated to this region.
  boost::unordered_map<MTR::addr_t, AddressRangePass::ptr>::iterator i = range_passes.find(base);
  if(i == range_passes.end()){
    abort();
  }

  // Delete the dataflow path associated to the region.
  simulator->Producer<CacheAccessRecord>::removeConsumer(i->second);

  // Delete the record of the filter from the lookup table.
  range_passes.erase(i);
}


// std::vector<Widget::ptr> WaxlampCacheNetwork::addRegion(BaseAddressLocator::ptr base, MTR::size_t size, MTR::size_t type){
//   // Create a region filter, connect the cache simulator to it, and
//   // connect it to the cache grouper.
//   //
//   // const MTR::addr_t limit = base + size;
//   BaseAddressLocator::ptr limit = *base + size;
//   AddressRangePass::ptr pass(new AddressRangePass(base, limit));
//   simulator->Producer<CacheAccessRecord>::addConsumer(pass);
//   pass->Producer<CacheAccessRecord>::addConsumer(cacheGrouper);

//   // Create a new region area in the cache grouper.
//   unsigned r;

//   std::cout << "numRegions: " << cacheGrouper->numRegions() << std::endl;

//   if(cacheGrouper->numRegions() == 0){
//     // Giant circle.
//     r = cacheGrouper->addRegion(CacheGrouper::center, CacheGrouper::outer_radius, base, type);

//     // std::cout << "First region! Circle is (" << CacheGrouper::center << ", " << CacheGrouper::outer_radius << ")" << std::endl;
//   }

//   else{
//     // Reset circles.
//     //
//     // Divide one circle-turn by the number of regions (including the
//     // new one).
//     const float angle_interval = 2*M_PI / (cacheGrouper->numRegions() + 1);

//     // Move through the current regions and reset their circles.
//     unsigned i;
//     for(i=0; i<cacheGrouper->numRegions(); i++){
//       const float theta = (i + 0.5)*angle_interval;
//       // const Vector v = (CacheGrouper::outer_radius + 0.5*CacheGrouper::circle_radius)*Vector(cos(theta), sin(theta));

// #if 1
//       ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(CacheGrouper::center, CacheGrouper::outer_radius);
//       ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle,
//                                                                                        (theta - 0.5*angle_interval) / (2*M_PI),
//                                                                                        (theta + 0.5*angle_interval) / (2*M_PI));
//       cacheGrouper->resetRegionShape(i, remap);
// #elif 0
//       cacheGrouper->resetRegionShape(i, boost::make_shared<RandomDisc>(CacheGrouper::center + v, 0.5*CacheGrouper::circle_radius, 50));
// #elif 0
//       cacheGrouper->resetRegionShape(i, boost::make_shared<RandomAnnulus>(CacheGrouper::center,
//                                                                           CacheGrouper::outer_radius,
//                                                                           CacheGrouper::outer_radius + CacheGrouper::circle_radius,
//                                                                           theta - 0.5*angle_interval,
//                                                                           theta + 0.5*angle_interval,
//                                                                           50));
// #endif
//     }

//     const float theta = (i + 0.5)*angle_interval;
//     const Vector v = (CacheGrouper::outer_radius + 0.5*CacheGrouper::circle_radius)*Vector(cos(theta), sin(theta));
//     r = cacheGrouper->addRegion(CacheGrouper::center + v, 0.5*CacheGrouper::circle_radius, base, type);
// #if 1
//     ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(CacheGrouper::center, CacheGrouper::outer_radius);
//     ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle,
//                                                                                      (theta - 0.5*angle_interval) / (2*M_PI),
//                                                                                      (theta + 0.5*angle_interval) / (2*M_PI));
//     cacheGrouper->resetRegionShape(r, remap);
// #elif 0
//     cacheGrouper->resetRegionShape(r, boost::make_shared<RandomDisc>(CacheGrouper::center + v, 0.5*CacheGrouper::circle_radius, 50));
// #elif 0
//     cacheGrouper->resetRegionShape(i, boost::make_shared<RandomAnnulus>(CacheGrouper::center,
//                                                                         CacheGrouper::outer_radius, 
//                                                                         CacheGrouper::outer_radius + CacheGrouper::circle_radius,
//                                                                         theta - 0.5*angle_interval,
//                                                                         theta + 0.5*angle_interval,
//                                                                         50));
// #endif
//   }

//   // Create widgets and add them to the cache grouper.
//   std::vector<Widget::ptr> widgets;
//   for(unsigned i=0; i < size / type; i++){
//     // Select a color.
//     //
//     // const Color c = cacheGrouper->numRegions() == 1 ? Color::black : colorgen->color(cacheGrouper->numRegions() - 2);
//     const Color c = colorgen->color(cacheGrouper->numRegions() - 1);

//     FadingPoint::ptr w(new FadingPoint(Point::zero, c, CacheGrouper::widgetSize));
//     widgets.push_back(w);

//     // Add the widget to the cache grouper, but do not marshal
//     // (marshal all widgets at once below).
//     cacheGrouper->addWidget(*base + i*type, w, r, false);
//   }

//   // // Make sure the new widgets fall into place.
//   // cacheGrouper->marshal();

//   // Return the list of widgets created for this region.
//   return widgets;
// }

// std::vector<Widget::ptr> WaxlampCacheNetwork::addRegion(MTR::addr_t base, MTR::size_t size, MTR::size_t type){
//   return addRegion(boost::make_shared<ConstantBaseAddress>(base), size, type);
// }
