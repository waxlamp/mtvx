// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// CacheGrouper.h - Manages several CircleGrouper instances
// representing different levels of cache, and consumes
// CacheAccessRecords in order to move items between the circles.

#ifndef CACHE_GROUPER_H
#define CACHE_GROUPER_H

// MTV headers.
#include <Core/FrameDump.pb.h>
#include <Core/Animation/Grouper.h>
#include <Core/Animation/ShapeGrouper.h>
// #include <Core/Dataflow/BaseAddressLocator.h>
#include <Core/Dataflow/CacheAccessRecord.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/Geometry/ParametrizedCircle.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/Boost.h>
#include <Core/Util/Timing.h>
#include <Tools/CacheSimulator/Cache.h>

namespace MTV{
  using Daly::Cache;
  namespace FD = MTV::FrameDump;

  class MainMemoryLayer{
  public:
    struct Data{
      ShapeGrouper::ptr shape_grouper;
      MTR::addr_t base_addr;
      MTR::addr_t type;
    };

  public:
    MainMemoryLayer()
      : next_region_id(0)
    {}

    unsigned addRegion(MTR::addr_t addr, MTR::addr_t type, ShapeGrouper::ptr grouper){
      // Grab a new region id.
      const unsigned region = next_region_id++;

      // Install the address-to-region entry.
      by_addr[addr] = region;

      // Install the region-to-data entry.
      by_region[region].shape_grouper = grouper;
      by_region[region].base_addr = addr;
      by_region[region].type = type;

      std::cerr << "adding main memory region! addr = 0x" << std::hex << addr << ", type = " << std::dec << type << ", region = " << region << std::endl;

      return region;
    }

    void removeRegion(MTR::addr_t addr){
      // Find the region entry.
      boost::unordered_map<MTR::addr_t, unsigned>::iterator i = by_addr.find(addr);
      if(i == by_addr.end()){
        return;
      }

      // Grab the region id and delete the entry.
      const unsigned region = i->second;
      by_addr.erase(i);

      // Find the data entry.
      boost::unordered_map<unsigned, Data>::iterator j = by_region.find(region);
      if(j == by_region.end()){
        std::cerr << "fatal error: inconsistent MainMemoryLayer" << std::endl;
        abort();
      }

      // TODO(choudhury): place the "freed" region id into a stack for
      // re-use.

      // Delete the entry from the data table.
      by_region.erase(j);
    }

    unsigned numRegions() const {
      return by_region.size();
    }

    const Data *getRegion(unsigned r) const {
      boost::unordered_map<unsigned, Data>::const_iterator i = by_region.find(r);
      if(i == by_region.end()){
        return 0;
      }

      return &(i->second);
    }

    void marshal(float when){
      for(boost::unordered_map<unsigned, Data>::iterator i = by_region.begin(); i != by_region.end(); i++){
        i->second.shape_grouper->marshal(when);
      }
    }

    std::vector<Grouper::ptr> getGroupers() const {
      std::vector<Grouper::ptr> result;
      for(boost::unordered_map<unsigned, Data>::const_iterator i = by_region.begin(); i != by_region.end(); i++){
        result.push_back(i->second.shape_grouper);
      }

      return result;
    }

  private:
    // "addr" maps from a base address to a region number.  "region"
    // maps from a region number to both a shape grouper AND the base
    // address.  This allows for deleting a region either by base
    // address or by region number.
    boost::unordered_map<MTR::addr_t, unsigned> by_addr;
    boost::unordered_map<unsigned, Data> by_region;

    unsigned next_region_id;
  };

  class CacheGrouper : public Grouper,
                       public Consumer<CacheAccessRecord> {
  public:
    BoostPointers(CacheGrouper);

  public:
    typedef boost::unordered_map<MTR::addr_t, Widget::ptr> WidgetTable;
    // typedef boost::unordered_map<BaseAddressLocator::ptr, Widget::ptr> WidgetTable;
    typedef boost::unordered_set<MTR::addr_t> OccupancySet;

  private:
    struct AddrType{
      AddrType(MTR::addr_t addr, MTR::addr_t type)
        : addr(addr),
          type(type)
      {}

      MTR::addr_t addr, type;
    };

    // struct AddrType{
    //   AddrType(BaseAddressLocator::ptr addr, MTR::addr_t type)
    //     : addr(addr),
    //       type(type)
    //   {}

    //   BaseAddressLocator::ptr addr;
    //   MTR::addr_t type;
    // };

  public:
    static const Point center;
    static const float widgetSize;
    static const float gap; // = 5;
    static const float circle_radius; // = 50;
    static float outer_radius;
    static float spiral_radius;
    static const Color ghostColor;

  public:
    CacheGrouper(Cache::const_ptr c, bool animating, float duration, Clock::ptr clock);

    void consume(const CacheAccessRecord& rec);

    unsigned addRegion(const Point& center, float radius, MTR::addr_t base, MTR::addr_t type){
      // Create a new circle grouper at the "main memory" level -
      // don't marshal now, it will happen normally during consume().
      //
      // circles.back().push_back(boost::make_shared<DriftoutShapeGrouper>(boost::make_shared<ParametrizedCircle>(center, radius), animating, duration, 30, true, CacheGrouper::center));
      ShapeGrouper::ptr p = boost::make_shared<DriftoutShapeGrouper>(boost::make_shared<ParametrizedCircle>(center, radius), animating, duration, 30, true, CacheGrouper::center);
      const unsigned region = mm_layer.addRegion(base, type, p);

      // Add the appropriate widgets.
      

      return region;

      // // Record a mapping from the base address to an iterator to the
      // // new element.
      // which_circle[base] = circles.back().end();
      // which_circle[base]--;

      // // Record the base and typesize of the region.
      // arrays.push_back(AddrType(base, type));

      // return circles.back().size() - 1;
    }

    void removeRegion(MTR::addr_t base, MTR::addr_t size, MTR::addr_t type){
      // Remove the widgets.
      for(unsigned i=0; i < size/type; i++){
        this->removeWidget(base + i*type);
      }

      // Remove the region from the main memory layer.
      mm_layer.removeRegion(base);
    }

    unsigned numRegions() const {
      // return circles.back().size();
      return mm_layer.numRegions();
    }

    void resetRegionShape(unsigned i, Parametrized::ptr p){
      // // circles.back()[i]->resetShape(p);

      // std::list<ShapeGrouper::ptr>::iterator it = circles.back().begin();
      // for(unsigned j=0; j<i; j++){
      //   it++;
      // }

      // (*it)->resetShape(p);

      mm_layer.getRegion(i)->shape_grouper->resetShape(p);
    }

    void addWidget(MTR::addr_t addr, Widget::ptr w, unsigned region, bool marshal = true);

    Widget::ptr removeWidget(MTR::addr_t addr);

    void marshal(float when);

    // void dumpPoints(const std::string& filename, const std::vector<float>& rates) const;
    void dumpPoints(std::ofstream& filename, const std::vector<float>& rates) const;

    FD::FrameDump& getFrameDumpPB(){
      return *framedump;
    }

    std::vector<Grouper::ptr> getGroupers(){
      std::vector<Grouper::ptr> groupers;
      for(unsigned i=0; i<circles.size(); i++){
        groupers.insert(groupers.end(), circles[i].begin(), circles[i].end());
      }

      std::vector<Grouper::ptr> mm_groupers = mm_layer.getGroupers();
      groupers.insert(groupers.end(), mm_groupers.begin(), mm_groupers.end());

      return groupers;
    }

    void setWidgetPanel(WidgetPanel::ptr p){
      panel = p;
    }

  private:
    void enter(MTR::addr_t addr, unsigned L, float when);
    void evict(MTR::addr_t addr, unsigned L, bool writeback, bool dirty, const float when);
    void moveToFront(MTR::addr_t addr, unsigned L, float when);
    void addGhost(MTR::addr_t addr, unsigned L, float when);
    void removeGhost(MTR::addr_t addr, unsigned L, float when);

    void doFrameDump();

  private:
    WidgetTable widgets;
    boost::unordered_map<Widget::ptr, unsigned> widget_home;
    boost::unordered_map<MTR::addr_t, std::list<ShapeGrouper::ptr>::iterator> which_circle;
    std::vector<OccupancySet> occupancy;
    std::vector<WidgetTable> ghosts;
    std::vector<std::vector<ShapeGrouper::ptr> > circles;
    // std::vector<std::list<ShapeGrouper::ptr> > circles;
    // std::vector<boost::unordered_map<unsigned, ShapeGrouper::ptr> > circles;
    MainMemoryLayer mm_layer;
    std::vector<unsigned> associativity;
    // std::vector<AddrType> arrays;
    unsigned blocksize;
    float duration;

    WidgetPanel::ptr panel;

    Clock::ptr clock;

    Cache::const_ptr cache;

    bool animating;
    boost::shared_ptr<FD::FrameDump> framedump;
    std::vector<std::pair<unsigned, MTR::addr_t> > ghost_death_row;
    // std::ofstream frameout;
  };
}

#endif
