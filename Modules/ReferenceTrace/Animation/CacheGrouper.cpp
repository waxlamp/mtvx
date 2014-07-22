// Copyright 2011 A.N.M. Imroz Choudhury
//
// CacheGrouper.cpp

// MTV headers.
#include <Core/Animation/ColorAnimator.h>
#include <Core/Animation/GlowAnimator.h>
#include <Core/Animation/PointToPointAnimator.h>
#include <Core/Animation/PulseAnimator.h>
#include <Core/Animation/ShapeGrouper.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Geometry/ArchimedeanSpiral.h>
#include <Core/Geometry/ParameterAffineRemapper.h>
#include <Core/Geometry/ParameterFunctionRemapper.h>
#include <Core/Geometry/ParametrizedCircle.h>
#include <Core/Graphics/FadingPoint.h>
#include <Core/Math/Interpolation.h>
#include <Core/Util/Util.h>
#include <Modules/ReferenceTrace/Animation/CacheGrouper.h>
using Daly::CacheEntranceRecord;
using Daly::CacheEvictionRecord;
using Daly::CacheHitRecord;
using MTV::ArchimedeanSpiral;
using MTV::CacheAccessRecord;
using MTV::CacheGrouper;
using MTV::Clock;
using MTV::Color;
using MTV::ColorAnimator;
using MTV::FadingPoint;
using MTV::GlowAnimator;
using MTV::LinearInterpolator;
using MTV::ParameterAffineRemapper;
using MTV::ParameterFunctionRemapper;
using MTV::ParametrizedCircle;
using MTV::Point;
using MTV::PointToPointAnimator;
using MTV::PulseAnimator;
using MTV::Tau;
using MTV::ShapeGrouper;
using MTV::Widget;
using MTV::WidgetPanel;

namespace FD = MTV::FrameDump;

// System headers.
#include <fstream>
#include <signal.h>

const Point CacheGrouper::center(360, 360);
const float CacheGrouper::widgetSize = 20;
const float CacheGrouper::gap = 5;
const float CacheGrouper::circle_radius = 50;
float CacheGrouper::outer_radius = 0;
float CacheGrouper::spiral_radius = 0;
const Color CacheGrouper::ghostColor(0.7*Color::white);

CacheGrouper::CacheGrouper(Cache::const_ptr c, bool animating, float duration, Clock::ptr clock)
  : occupancy(c->num_levels()),
    ghosts(c->num_levels() + 1),
    // circles(c->num_levels() + 1),
    circles(c->num_levels()),
    associativity(c->num_levels()),
    blocksize(c->block_size()),
    duration(duration),
    panel(WidgetPanel::ptr()),
    clock(clock),
    cache(c),
    animating(animating)
{
  // Open the output file, and create a protobuffer instance if we're
  // not animating.
  if(!animating){
    // frameout.open("frameout.pb");
    framedump = boost::make_shared<FD::FrameDump>();
  }

  // Record the cache level associativities.
  for(unsigned i=0; i<associativity.size(); i++){
    associativity[i] = c->level(i)->associativity();
  }

  // Set the cache layers as concentric circles at 50-unit radial
  // intervals.
  //
  // NOTE(choudhury): this center point assumes a 720p display.
  //
  // const Point center(360, 360);
  // const float gap = 5;
  // const float circle_radius = 50;

  // This is computed from the "end" of one of the spirals.
  //
  // float spiral_radius;

  // NOTE(choudhury): this code path assumes a two-level cache, with
  // associativities (2, 4).
  //
  // For L1, two opposing spirals.
  const float spiral_b = 20.0;
  // const float big_spiral_b = 1.5*spiral_b;
  {
    ArchimedeanSpiral::ptr spiral = boost::make_shared<ArchimedeanSpiral>(center + Vector(0, -gap), 0.0, spiral_b);
    ParameterAffineRemapper::ptr remap1 = boost::make_shared<ParameterAffineRemapper>(spiral, 0.0, 0.75);
    ParameterFunctionRemapper<double, double>::ptr remap = boost::make_shared<ParameterFunctionRemapper<double, double> >(remap1, sqrt);

    circles[0].push_back(boost::make_shared<ShapeGrouper>(remap, animating, duration));

    // Compute the radius of the spirals.
    const Point p = spiral->position(1.0);
    spiral_radius = sqrt(sq(p.x - center.x) + sq(p.y - center.y));
  }

  {
    ArchimedeanSpiral::ptr spiral = boost::make_shared<ArchimedeanSpiral>(center + Vector(0, gap), 0.0, -spiral_b);
    ParameterAffineRemapper::ptr remap1 = boost::make_shared<ParameterAffineRemapper>(spiral, 0.0, 0.75);
    ParameterFunctionRemapper<double, double>::ptr remap = boost::make_shared<ParameterFunctionRemapper<double, double> >(remap1, sqrt);

    circles[0].push_back(boost::make_shared<ShapeGrouper>(remap, animating, duration));
  }

  // For L2, four semicircles, "pointing" the same way.
  {
    // "Up".
    //
    // ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(0, spiral_radius + 2*gap + circle_radius),
    //                                                                         circle_radius);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.75, 1.25);

    // ArchimedeanSpiral::ptr L2_spiral = boost::make_shared<ArchimedeanSpiral>(center + Vector(0, spiral_radius), 0.0, big_spiral_b);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(L2_spiral, 1.125, 1.25);

    ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(-0.75*spiral_radius, spiral_radius), 0.75*spiral_radius);
    ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.0, 0.25);

    circles[1].push_back(boost::make_shared<ShapeGrouper>(remap, animating, duration));
  }

  {
    // "Left".
    //
    // ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(-(spiral_radius + 2*gap + circle_radius), 0),
    //                                                                         circle_radius);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.0, 0.5);

    // ArchimedeanSpiral::ptr spiral = boost::make_shared<ArchimedeanSpiral>(center, 0.0 - big_spiral_b*(Tau/4), big_spiral_b);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(spiral, 1.125 + 0.25, 1.25 + 0.25);

    ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(-spiral_radius, -0.75*spiral_radius), 0.75*spiral_radius);
    ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.25, 0.50);

    circles[1].push_back(boost::make_shared<ShapeGrouper>(remap, animating, duration));
  }

  {
    // "Down".
    //
    // ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(0, -(spiral_radius + 2*gap + circle_radius)),
    //                                                                         circle_radius);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.25, 0.75);

    // ArchimedeanSpiral::ptr L2_spiral = boost::make_shared<ArchimedeanSpiral>(center, 0.0 - big_spiral_b*(Tau/2), big_spiral_b);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(L2_spiral, 1.125 + 0.50, 1.25 + 0.50);

    ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(0.75*spiral_radius, -spiral_radius), 0.75*spiral_radius);
    ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.50, 0.75);

    circles[1].push_back(boost::make_shared<ShapeGrouper>(remap, animating, duration));
  }

  {
    // "Right".

    // ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(spiral_radius + 2*gap + circle_radius, 0),
    //                                                                         circle_radius);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.5, 1.0);

    // ArchimedeanSpiral::ptr L2_spiral = boost::make_shared<ArchimedeanSpiral>(center, 0.0 - big_spiral_b*(0.75*Tau), big_spiral_b);
    // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(L2_spiral, 1.125 + 0.75, 1.25 + 0.75);

    ParametrizedCircle::ptr circle = boost::make_shared<ParametrizedCircle>(center + Vector(spiral_radius, 0.75*spiral_radius), 0.75*spiral_radius);
    ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circle, 0.75, 1.0);

    circles[1].push_back(boost::make_shared<ShapeGrouper>(remap, animating, duration));
  }

#if 0
  // for(unsigned i=0; i<c->num_levels(); i++){
  //   CircleGrouper::ptr circle(new CircleGrouper(center, 50*(i+1), duration));
  //   circles[i] = circle;
  // }
#elif 0
  // for(unsigned i=0; i<c->num_levels(); i++){
  //   const unsigned numSets = associativity[i];
  //   circles[i].resize(numSets);

  //   for(unsigned j=0; j<numSets; j++){
  //     // Compute the angle at which to situate this circle grouper.
  //     const float angle = (j+0.5) * 2*M_PI / numSets;
  //     const Point loc = center + 100*(i)*Vector(cos(angle), sin(angle));

  //     // CircleGrouper::ptr circle(new CircleGrouper(loc, 25, duration));
  //     // ShapeGrouper::ptr shape(new ShapeGrouper(spiral, duration));
  //     ShapeGrouper::ptr shape;

  //     if(i == 0){
  //       ArchimedeanSpiral::ptr spiral(new ArchimedeanSpiral(loc, 0.0, j%2 == 0 ? 10.0 : -10.0));
  //       // ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(spiral, 0.0, 1.5*M_PI);
  //       ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(spiral, 0.0, 0.75);
  //       shape = boost::make_shared<ShapeGrouper>(remap, duration);
  //     }
  //     else{
  //       ParametrizedCircle::ptr circ(new ParametrizedCircle(loc, 25));
  //       ParameterAffineRemapper::ptr remap = boost::make_shared<ParameterAffineRemapper>(circ, (7./8), (7./8) + 0.5);
  //       shape = boost::make_shared<ShapeGrouper>(remap, duration);
  //     }
  //     circles[i][j] = shape;
  //   }
  // }
#endif

  // // Set the main memory layer to be 100 units from the largest cahce
  // // layer.
  // // CircleGrouper::ptr circle(new CircleGrouper(center, 50*c->num_levels() + 100, duration));
  // ParametrizedCircle::ptr circ(new ParametrizedCircle(center, 50*c->num_levels() + 100));
  // ShapeGrouper::ptr mm(new ShapeGrouper(circ, duration));
  // circles[c->num_levels()].push_back(mm);

  // Set outer radius.
  //
  outer_radius = 50*c->num_levels() + 200;
  // outer_radius = 2*spiral_radius;
}

void CacheGrouper::consume(const CacheAccessRecord& rec){
  std::cout << "CacheGrouper::consume(0x" << std::hex << rec.addr << std::dec << ")" << std::endl;

  typedef std::pair<MTR::addr_t, Widget::ptr> WidgetTablePair;
  const float now = clock->noww();

  // // Empty the protocol buffer.
  // if(!animating){
  //   // TODO(choudhury): if it gets to be too big, we can delete/renew
  //   // it here - Clear() does not free the allocated memory.
  //   framedump->Clear();
  // }

  // std::cout << "consume-----" << std::endl;

  // Move any "hit" widgets to the front of their groupers, and make
  // them pulse.
  const std::vector<CacheHitRecord>& hits = rec.hits;
  foreach(const CacheHitRecord& h, hits){
    // if(h.L < associativity.size() and widgets.find(h.addr) != widgets.end()){
    MTR::addr_t haddr = h.addr;
    if(h.L < associativity.size()){
      if(widgets.find(haddr) == widgets.end()){
        // std::cout << "turning " << std::hex << haddr << " into ";
        // haddr /= 8;
        // haddr *= 8;
        // std::cout << haddr << std::dec << std::endl;

        continue;
      }

      // while(widgets.find(haddr) == widgets.end()){
      //   haddr--;
      // }

      // std::cout << "hit: " << std::hex << haddr << std::dec << std::endl;
      this->moveToFront(haddr, h.L, now);
    }
  }

  // Move the evicted widgets out of place.
  const std::vector<CacheEvictionRecord>& evictions = rec.evictions;
  // std::cout << evictions.size() << " eviction records" << std::endl;

  foreach(const CacheEvictionRecord& e, evictions){
    // Find each widget lying in the same cache block as the evicted
    // block, and evict them one by one.
    foreach(WidgetTable::value_type p, widgets){
      if(p.first / blocksize == e.blockaddr){
        // std::cout << "eviction from " << e.L << ": " << std::hex << p.first << std::dec << std::endl;
        this->evict(p.first, e.L, e.writeback, e.dirty, now);
      }
    }
  }

  // Move the entering widgets into place.
  const std::vector<CacheEntranceRecord>& entrances = rec.entrances;

  // Begin by tracking the LOWEST level to which each address is being entered.
  typedef boost::unordered_map<MTR::addr_t, unsigned> LowestEntryMap;
  typedef std::pair<MTR::addr_t,unsigned> LowestEntryIterator;
  LowestEntryMap lowest_entry;
  foreach(const CacheEntranceRecord& e, entrances){
    std::cout << "entrance: " << std::hex << e.addr << std::dec << " at level " << e.L << std::endl;

    if(lowest_entry.find(e.addr) == lowest_entry.end()){
      lowest_entry[e.addr] = e.L;
    }
    else{
      if(e.L < lowest_entry[e.addr]){
        lowest_entry[e.addr] = e.L;
      }
    }
  }

  // Enter the widgets into their lowest entry levels.
  foreach(LowestEntryIterator i, lowest_entry){
    // Also enter each address in the widget table that is in the same
    // block as this one.
    const uint64_t blockaddr = i.first / blocksize;
    std::cout << "blocksize = " << blocksize << std::endl;
    std::cout << "matching to address " << reinterpret_cast<void *>(i.first) << " (block address " << reinterpret_cast<void *>(blockaddr) << ")" << std::endl;
    foreach(WidgetTablePair p, widgets){
      std::cout << "checking " << reinterpret_cast<void *>(p.first) << " (block address " << reinterpret_cast<void *>(p.first / blocksize) << ")" << std::endl;
      if(p.first / blocksize == blockaddr){
        std::cout << "also " << i.second << ": " << std::hex << p.first << std::dec << std::endl;
        this->enter(p.first, i.second, now);
      }
    }
  }

  // // Dump out the cache state.
  // Cache::Snapshot snap = cache->state();
  // for(unsigned i=0; i<snap.state.size(); i++){
  //   for(unsigned j=0; j<snap.state[i].size(); j++){
  //     std::cout << "(" << std::hex << snap.state[i][j].addr * cache->blocksize() << ", " << std::dec << cache->getModtime(snap.state[i][j].addr) << ") ";
  //   }
  //   std::cout << std::endl;
  // }

  // The entrance/eviction/moveToFront methods all change the widget
  // sets without marshalling - so do it one time here.
  this->marshal(now);

  if(!animating){
    // Gather up the information in the protocol buffer and dump it to
    // disk.
    this->doFrameDump();

    // Erase all the death row widgets
    for(std::vector<std::pair<unsigned, MTR::addr_t> >::const_iterator i = ghost_death_row.begin(); i != ghost_death_row.end(); i++){
      const unsigned L = i->first;
      const unsigned addr = i->second;

      // Erase the requested ghost.
      ghosts[L].erase(addr);
    }

    // Empty out death row.
    ghost_death_row.resize(0);
  }
}

void CacheGrouper::addWidget(MTR::addr_t addr, Widget::ptr w, unsigned r, bool marshal){
  // Add the address/widget mapping to the global widget table.
  //
  // The widget should not be already present in the table.
  assert(widgets.find(addr) == widgets.end());
  widgets[addr] = w;

  // Add the widget to the "main memory" layer.
  //
  // // ShapeGrouper::ptr mm = circles.back()[r];
  // std::list<ShapeGrouper::ptr>::iterator mm = circles.back().begin();
  // for(int i=0; i<r; i++){
  //   mm++;
  // }

  // (*mm)->addWidget(w, marshal);

  mm_layer.getRegion(r)->shape_grouper->addWidget(w, marshal);

  // Register the widget in the "widget home" table, which states
  // which main memory region the widget belongs to.
  widget_home[w] = r;
}

// void CacheGrouper::addWidget(BaseAddressLocator::ptr addr, Widget::ptr w, unsigned r, bool marshal){
//   // Add the address/widget mapping to the global widget table.
//   //
//   // The widget should not be already present in the table.
//   assert(widgets.find(addr) == widgets.end());
//   widgets[addr] = w;

//   // Add the widget to the "main memory" layer.
//   //
//   // ShapeGrouper::ptr mm = (*(circles.end() - 1))[0];
//   ShapeGrouper::ptr mm = circles.back()[r];
//   mm->addWidget(w, marshal);

//   // Register the widget in the "widget home" table, which states
//   // which main memory region the widget belongs to.
//   widget_home[w] = r;
// }

Widget::ptr CacheGrouper::removeWidget(MTR::addr_t addr){
  std::cout << "removing " << reinterpret_cast<void *>(addr) << std::endl;

  // Remove all traces of the widget.
  //
  // Remove it from the widget table.
  Widget::ptr w;
  {
    WidgetTable::iterator i = widgets.find(addr);
  
    // Delete it from the table, but grab the widget pointer itself (it
    // is the index into the widget home table.
    if(i != widgets.end()){
      w = i->second;
      widgets.erase(i);
    }
    else{
      // If it's not in the widget table, then it's not in the cache
      // grouper at all.
      return Widget::ptr();
    }
  }

  // Remove it from the widget home map.
  widget_home.erase(w);

  // From the occupancy sets.
  for(unsigned i=0; i<occupancy.size(); i++){
    occupancy[i].erase(addr);
  }

  // From the ghost tables.
  for(unsigned i=0; i<ghosts.size(); i++){
    ghosts[i].erase(addr);
  }

  // From the cache level shape groupers.
  for(unsigned i=0; i<circles.size(); i++){
    for(std::vector<ShapeGrouper::ptr>::iterator j=circles[i].begin(); j != circles[i].end(); j++){
      (*j)->removeWidget(w, 0.0);
    }
  }

  return w;
}

void CacheGrouper::marshal(float when){
  // const float now = clock->now();

  std::cout << "marshaling" << std::endl;

  for(unsigned i=0; i<circles.size(); i++){
    foreach(ShapeGrouper::ptr c, circles[i]){
      c->marshal(when);
    }
  }

  mm_layer.marshal(when);
}

// void CacheGrouper::dumpPoints(const std::string& filename, const std::vector<float>& rates) const {
void CacheGrouper::dumpPoints(std::ofstream& out, const std::vector<float>& rates) const {
  // // Open the output file.
  // std::ofstream out(filename.c_str());
  // if(!out){
  //   std::cerr << "error: could not open file " << filename << " for output." << std::endl;
  //   return;
  // }

  // First add up the numbers of points in the widgets table and
  // the ghost tables.
  unsigned total = widgets.size();
  for(unsigned i=0; i<ghosts.size(); i++){
    total += ghosts[i].size();
  }

  // Write the count out to the file.
  //
  out << total << std::endl;
  // out.write(reinterpret_cast<const char *>(&total), sizeof(unsigned));

  // Write the cache hit rates out.
  //
  for(unsigned i=0; i<rates.size(); i++){
    out << rates[i] << " ";
  }
  out << std::endl;
  // out.write(reinterpret_cast<const char *>(&rates[0]), rates.size()*sizeof(rates[0]));

  // Create a vector of WidgetTable pointers (to allow reuse of the
  // loop below).
  std::vector<const WidgetTable *> tables;
  tables.push_back(&widgets);
  for(unsigned i=0; i<ghosts.size(); i++){
    tables.push_back(&ghosts[i]);
  }

  // Now dump the widget information.
  foreach(const WidgetTable *table, tables){
    for(WidgetTable::const_iterator i = table->begin(); i != table->end(); i++){
      // Cast to concrete type so we can extract the color.
      FadingPoint::ptr fp = boost::static_pointer_cast<FadingPoint, Widget>(i->second);

      // Extract the info.
      const Color& color = fp->getColor();
      const Point& location = fp->getLocation();
      const float size = fp->getSize();

      // Dump.
      //
      out << i->first << " "
          << color.r << " " << color.g << " " << color.b << " " << color.a << " "
          << location.x << " " << location.y << " "
          << size << std::endl;
      // out.write(reinterpret_cast<const char *>(&(i->first)), sizeof(MTR::addr_t));
      // out.write(reinterpret_cast<const char *>(&color.r), 4*sizeof(color.r));
      // out.write(reinterpret_cast<const char *>(&location.x), 2*sizeof(location.x));
      // out.write(reinterpret_cast<const char *>(&size), sizeof(size));
    }
  }

  out.flush();
}

void CacheGrouper::enter(MTR::addr_t addr, unsigned L, float when){
  std::cout << "enter(0x" << std::hex << addr << ", " << std::dec << L << ", " << when << ")" << std::endl;
  // raise(SIGSTOP);

  // Find the actual widget.  If this function is called, the widget
  // should exist.
  WidgetTable::iterator w = widgets.find(addr);
  assert(w != widgets.end());

  // The requested item should not already be in this level.
  assert(occupancy[L].find(addr) == occupancy[L].end());
  // if(occupancy[L].find(addr) != occupancy[L].end()){
  //   std::cout << "already in this level!!!!!" << std::endl;
  //   return;
  // }

  // Place the widget in the requested occupancy table.
  occupancy[L].insert(addr);

  unsigned cur;
  // // Remove ghosts of this address from all lower levels (and the
  // // current level).
  // for(cur=0; cur<=L; cur++){
  //   this->removeGhost(addr, cur, when);
  // }

  this->removeGhost(addr, L, when);

  // Find the lowest level that currently contains the widget, adding
  // the address to the occupancy tables as we go.
  for(cur=L+1; cur<occupancy.size(); cur++){
    this->addGhost(addr, cur, when);

    if(occupancy[cur].find(addr) != occupancy[cur].end()){
      break;
    }
    occupancy[cur].insert(addr);
  }

  // Add a ghost to the top level, if the widget is not entering
  // there.
  if(L != occupancy.size() - 1){
    this->addGhost(addr, occupancy.size(), when);
  }

  // std::cout << "moving widget from " << cur << " to " << L << std::endl;

  const unsigned cur_set = cur == associativity.size() ? widget_home[w->second] : (addr / blocksize) % associativity[cur];
  const unsigned L_set = L == associativity.size() ? widget_home[w->second] : (addr / blocksize) % associativity[L];

  // std::cout << "circles.size(): " << circles.size() << std::endl;
  // for(unsigned i=0; i<circles.size(); i++){
  //   std::cout << "circles[" << i << "].size(): " << circles[i].size() << std::endl;
  //   for(unsigned j=0; j<circles[i].size(); j++){
  //     std::cout << circles[i][j] << std::endl;
  //   }
  // }

  // Move the widget from its current location to the new level.
  const bool bad = !(cur == associativity.size() ? mm_layer.getRegion(cur_set)->shape_grouper->removeWidget(w->second, when, false) : circles[cur][cur_set]->removeWidget(w->second, when, false));
  // if(bad){
  //   std::cout << "expected address " << addr << " in level " << L << ", set " << L_set << std::endl;
  //   for(unsigned i=0; i<circles.size(); i++){
  //     for(unsigned j=0; j<circles[i].size(); j++){
  //       if(circles[i][j]->hasWidget(w->second)){
  //         std::cout << "found in level " << i << ", set " << j << std::endl;
  //       }
  //     }
  //   }
  // }

  // std::cout << "entering " << std::hex << addr << std::dec << " to level " << L << ", set " << L_set << std::endl;
  if(bad){
    abort();
  }

  if(L == associativity.size()){
    // const unsigned i = (addr - arrays[L_set].addr) / arrays[L_set].type;
    // // const unsigned i = (addr - arrays[L_set].addr->value()) / arrays[L_set].type;
    // // std::cout << "inserting at known position " << i << std::endl;
    // circles[L][L_set]->addWidget(w->second, i, when, false);

    const MainMemoryLayer::Data *d = mm_layer.getRegion(L_set);
    const unsigned i = (addr - d->base_addr) / d->type;
    d->shape_grouper->addWidget(w->second, i, when, false);
  }
  else{
    circles[L][L_set]->addWidget(w->second, when, false);
  }

  // Make the widget glow.
  if(animating){
    FadingPoint::ptr fp = boost::static_pointer_cast<FadingPoint, Widget>(w->second);
    GlowAnimator<FadingPoint, LinearInterpolator<float> >::ptr glow(new GlowAnimator<FadingPoint,LinearInterpolator<float> >(w->second, Animator::Preemptible, fp->getColor(), Color::red, when, 5*duration));
    animators.push_back(glow);
  }
  else{
    // Create an acitivity object.
    FD::FrameDump::Activity *a = framedump->add_activity();
    a->set_type(FD::FrameDump::Activity::ColorPulseActivity);
    a->set_id(w->first);
    a->set_ghost(0);

    FD::FrameDump::Activity::ColorPulse *cp = a->mutable_color_pulse();
    cp->set_color(FD::FrameDump::Activity::CacheMissColor);
    // cp->set_color(2);
    cp->set_apex(0.5);
  }
}

void CacheGrouper::evict(MTR::addr_t addr, unsigned L, bool writeback, bool dirty, const float when){
  // std::cout << "evict!" << std::endl;

  OccupancySet::iterator i = occupancy[L].find(addr);
  if(i == occupancy[L].end()){
    std::cerr << "[evict] at level " << L << ", error with address 0x" << std::hex << addr << std::endl;

    return;
  }
  // assert(i != occupancy[L].end());

  // Find the widget.  If this function is called, it should exist.
  WidgetTable::iterator w = widgets.find(addr);
  assert(w != widgets.end());

  // Remove the widget from its eviction level.
  occupancy[L].erase(i);

  // std::cout << "evicting widget from " << L << " to " << (L+1) << std::endl;

  const unsigned L_set = L == associativity.size() ? widget_home[w->second] : (addr / blocksize) % associativity[L];
  const unsigned L1_set = (L+1) == associativity.size() ? widget_home[w->second] : (addr / blocksize) % associativity[L+1];

  // if(L+1 == associativity.size()){
  //   std::cout << "widget home: " << widget_home[w->second] << std::endl;
  // }

  // Move the widget one level up.
  if(!(L == associativity.size() ? mm_layer.getRegion(L_set)->shape_grouper->removeWidget(w->second, when, false) : circles[L][L_set]->removeWidget(w->second, when, false) ) ){
    // std::cout << "expected address " << addr << " in level " << L << ", set " << L_set << std::endl;
    // for(unsigned i=0; i<circles.size(); i++){
    //   for(unsigned j=0; j<circles[i].size(); j++){
    //     if(circles[i][j]->hasWidget(w->second)){
    //       std::cout << "found in level " << i << ", set " << j << std::endl;
    //     }
    //   }
    // }

    // Cache::Snapshot snap = cache->state();
    // for(unsigned i=0; i<snap.state.size(); i++){
    //   for(unsigned j=0; j<snap.state[i].size(); j++){
    //     std::cout << "(" << std::hex << snap.state[i][j].addr * cache->blocksize() << ", " << std::dec << cache->getModtime(snap.state[i][j].addr) << ") ";
    //   }
    //   std::cout << std::endl;
    // }

    abort();
  }

  if(L+1 == associativity.size()){
    // const unsigned i = (addr - arrays[L1_set].addr) / arrays[L1_set].type;
    // // const unsigned i = (addr - arrays[L1_set].addr->value()) / arrays[L1_set].type;
    // // std::cout << "inserting at known position " << i << std::endl;
    // circles[L+1][L1_set]->addWidget(w->second, i, when, false);

    const MainMemoryLayer::Data *d = mm_layer.getRegion(L1_set);
    const unsigned i = (addr - d->base_addr) / d->type;
    d->shape_grouper->addWidget(w->second, i, when, false);
  }
  else{
    circles[L+1][L1_set]->addWidget(w->second, when, false);
  }

  if(writeback){
    if(animating){
      // NOTE(choudhury): The commented out nondirty color is a nice
      // cloudy blue.
      //
      // const Color flashcolor = dirty ? Color::red : Color(0.82,0.93,0.99);
      const Color flashcolor = dirty ? Color::red : Color(0.8,0.8,0.8);

      FadingPoint::ptr fp = boost::static_pointer_cast<FadingPoint, Widget>(w->second);

      // Change color very quickly to the signal color.
      ColorAnimator<FadingPoint, LinearInterpolator<float> >::ptr to_red(boost::make_shared<ColorAnimator<FadingPoint,LinearInterpolator<float> > >(w->second,
                                                                                                                                                    Animator::NoFlags,
                                                                                                                                                    fp->getBaseColor(),
                                                                                                                                                    flashcolor,
                                                                                                                                                    when,
                                                                                                                                                    0.1*duration));

      // Fade back slowly to the base color.
      ColorAnimator<FadingPoint, LinearInterpolator<float> >::ptr cooldown(boost::make_shared<ColorAnimator<FadingPoint,LinearInterpolator<float> > >(w->second,
                                                                                                                                                      Animator::Preemptible,
                                                                                                                                                      flashcolor,
                                                                                                                                                      fp->getBaseColor(),
                                                                                                                                                      when + 0.1*duration,
                                                                                                                                                      10*duration));

      animators.push_back(to_red);
      animators.push_back(cooldown);
    }
    else{
      // Select the proper color for this write back event.
      const FD::FrameDump::Activity::SpecialColor color = dirty ? FD::FrameDump::Activity::WriteBackDirtyColor : FD::FrameDump::Activity::WriteBackCleanColor;

      // Create an activity entry in the frame dump protocol buffer.
      FD::FrameDump::Activity *a = framedump->add_activity();
      a->set_type(FD::FrameDump::Activity::ColorPulseActivity);
      a->set_id(w->first);
      a->set_ghost(0);

      // TODO(choudhury): the duration is supposed to last longer than
      // a single frame - see about how to accomplish this in the
      // current format.
      FD::FrameDump::Activity::ColorPulse *cp = a->mutable_color_pulse();
      cp->set_color(color);
      cp->set_apex(0.1);
    }
  }

  this->removeGhost(addr, L+1, when);
}

void CacheGrouper::moveToFront(MTR::addr_t addr, unsigned L, float when){
  // if(i == occupancy[L].end()){
  //   for(unsigned j=L+1; j<occupancy.size(); j++){
  //     std::cout << "dying: " << (occupancy[j].find(addr) != occupancy[j].end()) << std::endl;
  //   }
  //   // return;
  // }

  if(occupancy[L].find(addr) == occupancy[L].end()){
    std::cerr << "[moveToFront] at level " << L << ", error with address 0x" << std::hex << addr << std::endl;
    // std::cerr << "dumping occupancy table:" << std::endl;
    // for(OccupancySet::const_iterator i = occupancy[L].begin(); i != occupancy[L].end(); i++){
    //   std::cerr << "0x" << *i << std::endl;
    // }

    // assert(occupancy[L].find(addr) != occupancy[L].end());

    // Enter the address in the appropriate level before continuing.
    this->enter(addr, L, when);
  }

  // Find the widget.
  WidgetTable::iterator w = widgets.find(addr);
  assert(w != widgets.end());

  // Find the proper set.
  const unsigned L_set = (addr / blocksize) % associativity[L];

  // Shift the widget to the front of its grouper.
  circles[L][L_set]->shiftWidget(w->second, when, false);

  // Make it pulse.
  if(animating){
    typedef PulseAnimator<FadingPoint, LinearInterpolator<float> > Pulser;
    Pulser::ptr pulse = boost::make_shared<Pulser>(w->second, Animator::Preemptible, CacheGrouper::widgetSize, 2*CacheGrouper::widgetSize, when, duration);
    animators.push_back(pulse);
  }
  else{
    // Create an acitivity object.
    FD::FrameDump::Activity *a = framedump->add_activity();
    a->set_type(FD::FrameDump::Activity::SizePulseActivity);
    a->set_id(w->first);
    a->set_ghost(0);

    FD::FrameDump::Activity::SizePulse *sp = a->mutable_size_pulse();
    sp->set_size(2*CacheGrouper::widgetSize);
    sp->set_apex(0.5);
  }

  // Check for ghosts - if found, move them to the front of their sets
  // as well (don't move the ghosts in the main memory layer, but make
  // them pulse).
  for(unsigned i=L+1; i < ghosts.size(); i++){
    WidgetTable::iterator g = ghosts[i].find(addr);
    if(g != ghosts[i].end()){
      if(i < ghosts.size() - 1){
        const unsigned L_set = (addr / blocksize) % associativity[i];
        circles[i][L_set]->shiftWidget(g->second, when, false);
      }

      if(animating){
        typedef PulseAnimator<FadingPoint, LinearInterpolator<float> > Pulser;
        Pulser::ptr pulse = boost::make_shared<Pulser>(g->second, Animator::Preemptible, CacheGrouper::widgetSize, 2*CacheGrouper::widgetSize, when, duration);
        animators.push_back(pulse);
      }
      else{
        // Create an acitivity object.
        FD::FrameDump::Activity *a = framedump->add_activity();
        a->set_type(FD::FrameDump::Activity::SizePulseActivity);
        a->set_id(g->first);
        a->set_ghost(i+1);

        FD::FrameDump::Activity::SizePulse *sp = a->mutable_size_pulse();
        sp->set_size(2*CacheGrouper::widgetSize);
        sp->set_apex(0.5);
      }
    }
  }
}

void CacheGrouper::addGhost(MTR::addr_t addr, unsigned L, float when){
  if(ghosts[L].find(addr) == ghosts[L].end()){
    // std::cout << "addGhost at level " << L << std::endl;

    // Find the "real" widget associated to the address.
    WidgetTable::iterator w = widgets.find(addr);
    assert(w != widgets.end());

    // Create a ghost widget.
    Color widgetColor = boost::static_pointer_cast<FadingPoint, Widget>(w->second)->getColor();
    FadingPoint::ptr ghost = boost::make_shared<FadingPoint>(w->second->getLocation(), widgetColor, CacheGrouper::widgetSize);

    // Add it to the widget panel.
    if(panel){
      panel->add(ghost);
    }

    // The ghost widget starts out the same color as its living
    // counterpart - fade it to be ghostly.
    Color ghostColor = widgetColor;
    ghostColor.a = 0.2;

    if(animating){
      ColorAnimator<FadingPoint, LinearInterpolator<float> >::ptr fade =
        boost::make_shared<ColorAnimator<FadingPoint, LinearInterpolator<float> > >(ghost, Animator::NoFlags, widgetColor, ghostColor, when, duration);
      animators.push_back(fade);
    }
    else{
      // This block intentionally left blank.
      //
      // No need to set up protocol buffer stuff here - we'll just
      // assume the glyph starts out ghost-colored in this instance.
    }

    // Compute the set the ghost belongs to.
    const unsigned L_set = L == associativity.size() ? widget_home[w->second] : (addr / blocksize) % associativity[L];

    // Add the ghost to the appropriate level.
    if(L == associativity.size()){
      // Compute the position in the memory array where the ghost
      // ought to go.
      //
      // const unsigned i = (addr - arrays[L_set].addr) / arrays[L_set].type;
      // // const unsigned i = (addr - arrays[L_set].addr->value()) / arrays[L_set].type;
      // // std::cout << "inserting at known position " << i << std::endl;
      // circles[L][L_set]->addWidget(ghost, i, when, false);

      const MainMemoryLayer::Data *d = mm_layer.getRegion(L_set);
      const unsigned i = (addr - d->base_addr) / d->type;
      d->shape_grouper->addWidget(ghost, i, when, false);
    }
    else{
      circles[L][L_set]->addWidget(ghost, when, false);
    }

    // Record the ghost in the ghost table.
    ghosts[L][addr] = ghost;
  }
  // else{
  //   std::cout << "ghost was already there :(" << std::endl;
  // }
}

void CacheGrouper::removeGhost(MTR::addr_t addr, unsigned L, float when){
  WidgetTable::iterator ghost = ghosts[L].find(addr);
  if(ghost != ghosts[L].end()){
    // std::cout << "removeGhost at level " << L << std::endl;

    // Find the "real" widget associated to the address.
    WidgetTable::iterator w = widgets.find(addr);
    assert(w != widgets.end());

    // Compute the set the ghost belongs to.
    const unsigned L_set = L == associativity.size() ? widget_home[w->second] : (addr / blocksize) % associativity[L];

    // Remove the ghost widget.
    if(L == associativity.size()){
      mm_layer.getRegion(L_set)->shape_grouper->removeWidget(ghost->second, when, false);
    }
    else{
      circles[L][L_set]->removeWidget(ghost->second, when, false);
    }

    // // Fade the ghost widget away.
    if(animating){
      ColorAnimator<FadingPoint, LinearInterpolator<float> >::ptr fade =
        boost::make_shared<ColorAnimator<FadingPoint, LinearInterpolator<float> > >(ghost->second, Animator::WidgetKilling, boost::static_pointer_cast<FadingPoint, Widget>(ghost->second)->getColor(), Color(1,1,1,0), when, duration);
      animators.push_back(fade);

      // Remove the ghost from the ghosts table.
      ghosts[L].erase(ghost);
    }
    else{
      // Create an activity entry in the frame dump protocol buffer.
      FD::FrameDump::Activity *a = framedump->add_activity();
      a->set_type(FD::FrameDump::Activity::ColorChangeActivity);
      a->set_id(ghost->first);
      a->set_ghost(L+1);

      FD::FrameDump::Activity::ColorChange *cc = a->mutable_color_change();
      cc->set_color(FD::FrameDump::Activity::BlankColor);

      // Put the widget on death row (to be deleted en masse at the
      // end of the consume() function).
      ghost_death_row.push_back(std::make_pair(L, addr));
    }

  }
  // else{
  //   std::cout << "ghost not found :(" << std::endl;
  // }
}

void CacheGrouper::doFrameDump(){
  std::cout << "doFrameDump!" << std::endl;

  // Move through the list of widgets and place their information in
  // the protocol buffer.
  for(WidgetTable::const_iterator i = widgets.begin(); i != widgets.end(); i++){
    const MTR::addr_t& addr = i->first;
    const Widget::ptr p = i->second;

    std::cout << "widget 0x" << std::hex << addr << std::dec << std::endl;

    FD::FrameDump::Glyph *g = framedump->add_glyph();

    // Set the id of the glyph - equal to the identifying address of
    // the widget (i.e. the address that maps to the widget in the
    // widget table, NOT the address of the widget itself).
    g->set_id(addr);

    // The ghost level for "regular" widgets is zero.
    g->set_ghost(0);

    // Set the widget coordinates.
    const Point& loc = p->getLocation();
    g->set_x(loc.x);
    g->set_y(loc.y);

    g->set_color(widget_home[p]);

    if(p->extra() == "polar"){
      FD::FrameDump::Activity *a = framedump->add_activity();
      a->set_type(FD::FrameDump::Activity::PolarInterpolationActivity);
      a->set_id(addr);
      a->set_ghost(0);
    }
    else if(p->extra() == "linear" or p->extra() == "linear (polar)"){
      // This block intentionally left blank.
    }
    else{
      std::cerr << "unknown extra field: " << p->extra() << std::endl;
      abort();
    }

    // p->extra() = "";
  }

  // Now do the same for the ghost widgets.
  for(unsigned i=0; i<ghosts.size(); i++){
    // The level of the cache is one more than the index. (0 contains
    // L1, etc.).
    const unsigned L = i+1;

    for(WidgetTable::const_iterator j = ghosts[i].begin(); j != ghosts[i].end(); j++){
      const MTR::addr_t& addr = j->first;
      const Widget::ptr p = j->second;

      FD::FrameDump::Glyph *g = framedump->add_glyph();

      g->set_id(addr);
      g->set_ghost(L);

      const Point& loc = p->getLocation();
      g->set_x(loc.x);
      g->set_y(loc.y);

      g->set_color(widget_home[widgets[addr]]);
    }
  }

  // // Record the byte size of the protocol buffer in the output.
  // const int size = framedump->ByteSize();
  // std::cerr << "CacheGrouper::doFrameDump(): size = " << size << std::endl;
  // frameout.write(reinterpret_cast<const char *>(&size), sizeof(size));

  // // Record the protocol buffer itself.
  // framedump->SerializeToOstream(&frameout);
}
