// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// RegionRenderer.h - An object that receives different kinds of
// rendering commands and posts them to a coherent display.

#ifndef REGION_RENDERER_H
#define REGION_RENDERER_H

// MTV headers.
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/UpdateNotifier.h>
#include <Core/Util/BoostPointers.h>
#include <Marino/Memento.pb.h>
#include <Marino/Memorable.h>
#include <Marino/RegionRenderer.pb.h>
#include <Modules/ReferenceTrace/Dataflow/CacheStatusRenderCommand.h>
#include <Modules/ReferenceTrace/Dataflow/RecordRenderCommand.h>
#include <Modules/ReferenceTrace/Graphics/RegionDisplay.h>

namespace MTV{
  using Marino::ColorCold;
  using Marino::DeltaMemento;
  using Marino::RegionRendererDelta;
  using Marino::RegionRendererWarm;

  template<bool ComputeDelta>
  class RegionRenderer$Template : public Consumer2<RecordRenderCommand, CacheStatusRenderCommand>,
                                  public Memorable,
                                  public UpdateNotifier {
  public:
    BoostPointers(RegionRenderer$Template<ComputeDelta>);

  public:
    // TODO(choudhury): see what mtv2 does with the "ratio" parameter;
    // possibly use a "magnify" parameter to control overly long
    // region widgets.
    RegionRenderer$Template(Point location, MTR::addr_t base, MTR::addr_t limit, MTR::size_t type, const Color& shellcolor, const std::string& title)
      : region(new RegionDisplay(location, base, limit, type, shellcolor, 100, title))
    {}

    RegionDisplay::ptr getWidget() const {
      return region;
    }

    const Color& getShellColor() const { return region->getShellColor(); }

    // Consumer2 interface.
    void consume2(const RecordRenderCommand& rec_cmd, const CacheStatusRenderCommand& cache_cmd){
      static const float decrement = 0.01;

      // // debug code
      // std::vector<Color> origdatacolors;
      // std::vector<Color> origcachecolors;
      // if(ComputeDelta){
      //   for(unsigned i=0; i<region->numStripes(); i++){
      //     origdatacolors.push_back(region->getDataColor(i));
      //     origcachecolors.push_back(region->getCacheResultColor(i));
      //   }
      // }

      // Set the newest access to maximum brightness.
      //
      // NOTE(choudhury): set to one quantum above 1.0 so that in the code
      // below, this is "faded" down to 1.0 before the color is set.
      history[rec_cmd.cell] = 1 + decrement;

      Color original_datacolor;
      Color original_resultcolor;

      DeltaMemento delta;
      RegionRendererDelta *drr;

      if(ComputeDelta){
        // Initialize.
        delta.set_type(DeltaMemento::REGION_RENDERER);
        delta.set_id(this->getId());
        drr = delta.mutable_region_renderer();

        // Save the original colors for the changing stripe, for use later.
        original_datacolor = region->getDataColor(rec_cmd.cell);
        original_resultcolor = region->getCacheResultColor(rec_cmd.cell);
      }

      // TODO(choudhury): these colors need to come from a color profile
      // of some kind.
      region->setDataColor(rec_cmd.cell, rec_cmd.code == MTR::Record::Read ? Colors::Region::read : Colors::Region::write);
      region->setCacheResultColor(cache_cmd.cell, cache_cmd.color);

      // Keep a list of entries to erase from the history map.
      std::vector<std::map<unsigned, float>::iterator> deathrow;

      // Move through the history map, fading the colors appropriately.
      for(std::map<unsigned, float>::iterator i = history.begin(); i != history.end(); i++){
        const unsigned& cell = i->first;
        float& value = i->second;

        // Advance the history of the cell.  Keep the value at 0.0 (it
        // will be removed from the map later for this condition).
        if((value -= decrement) <= 0.0){
          value = 0.0;
        }

        const Color& datacolor = region->getDataColor(cell);
        const Color& resultcolor = region->getCacheResultColor(cell);

        RegionRendererDelta::Delta *d;
        if(ComputeDelta){
          // Store the pre-colors (if the cell is not the one that
          // received a new color - that color was already recorded as a
          // special case above).
          d = drr->add_delta();
          d->set_index(cell);

          // Select which color to use (don't use the CURRENT color
          // for the cell that is changing - it changes twice in this
          // function, once to set the new color, and then again to
          // "fade" it (even though it won't actually fade -- it has a
          // value of 1.0).
          const Color& use_datacolor = cell == rec_cmd.cell ? original_datacolor : datacolor;
          const Color& use_resultcolor = cell == rec_cmd.cell ? original_resultcolor : resultcolor;

          ColorCold *c = d->mutable_data_before();
          c->set_r(use_datacolor.r);
          c->set_g(use_datacolor.g);
          c->set_b(use_datacolor.b);
          c->set_a(use_datacolor.a);

          c = d->mutable_cache_before();
          c->set_r(use_resultcolor.r);
          c->set_g(use_resultcolor.g);
          c->set_b(use_resultcolor.b);
          c->set_a(use_resultcolor.a);
        }

        // Fade the color itself.
        const Color faded_datacolor = value*datacolor + (1.0 - value)*Colors::Region::cold;
        const Color faded_resultcolor = value*resultcolor + (1.0 - value)*Colors::Cache::cold;

        region->setDataColor(cell, faded_datacolor);
        region->setCacheResultColor(cell, faded_resultcolor);

        if(ComputeDelta){
          // Store the post-colors.
          ColorCold *c = d->mutable_data_after();
          c->set_r(faded_datacolor.r);
          c->set_g(faded_datacolor.g);
          c->set_b(faded_datacolor.b);
          c->set_a(faded_datacolor.a);

          // std::cout << "cell " << cell << std::endl;
          // std::cout << faded_datacolor << std::endl;
          // std::cout << "(" << c->r() << ", " << c->g() << ", " << c->b() << ")" << std::endl;
          // std::cout << region->getDataColor(cell);

          c = d->mutable_cache_after();
          c->set_r(faded_resultcolor.r);
          c->set_g(faded_resultcolor.g);
          c->set_b(faded_resultcolor.b);
          c->set_a(faded_resultcolor.a);

          // std::cout << faded_resultcolor << std::endl;
          // std::cout << "(" << c->r() << ", " << c->g() << ", " << c->b() << ")" << std::endl;
          // std::cout << region->getCacheResultColor(cell);
        }


        // Save the iterator for deletion after the processing loop is
        // through.
        if(value == 0.0){
          deathrow.push_back(i);
        }
      }

      // Delete the zero-entries.
      typedef std::map<unsigned, float>::iterator MapIterator;
      foreach(MapIterator i, deathrow){
        history.erase(i);
      }

      if(ComputeDelta){
        // Send out the delta memento.
        this->produce(delta);
      }

      // // debug code
      // if(ComputeDelta){
      //   std::cout << "checking (RecordRender(" << rec_cmd.cell << ", " << (rec_cmd.code == MTR::Record::Read ? "read" : "write")
      //             << "), CacheStatusRender(" << cache_cmd.cell << ", " << cache_cmd.color << ")" << std::endl;

      //   std::cout << "Object " << this->getId() << ", " << drr->delta_size() << " deltas" << std::endl;

      //   for(int i=0; i<drr->delta_size(); i++){
      //     const RegionRendererDelta::Delta& d = drr->delta(i);

      //     std::cout << "\tdelta " << i << std::endl;

      //     assert(checkColors(Color(d.data_before()), origdatacolors[d.index()], "data before"));
      //     assert(checkColors(Color(d.cache_before()), origcachecolors[d.index()], "cache before"));

      //     assert(checkColors(Color(d.data_after()), region->getDataColor(d.index()), "data after", d.index()));
      //     assert(checkColors(Color(d.cache_after()), region->getCacheResultColor(d.index()), "cache after"));
      //   }

      //   std::cout << "----------" << std::endl;
      // }

      // Ask to be re-rendered.
      emit updated();
    }

    // Memorable interface.
    void loadWarm(const WarmMemento& state){
      // Check runtime type.
      assert(state.type() == WarmMemento::REGION_RENDERER and state.has_region_renderer());

      // Extract the memento for a region renderer.
      const Marino::RegionRendererWarm& warm = state.region_renderer();

      // Check sizes.
      assert(region->numStripes() == static_cast<unsigned>(warm.stripe_size()));

      // Apply the colors.
      //
      // First the data stripes.
      for(int i=0; i<warm.stripe_size(); i++){
        const ColorCold& c = warm.stripe(i).data();
        // region->setDataColor(i, Color(c.r(), c.g(), c.b()));
        region->setDataColor(i, Color(c));
      }

      // Now the cache colors.
      for(int i=0; i<warm.stripe_size(); i++){
        const ColorCold& c = warm.stripe(i).cache();
        // region->setCacheResultColor(i, Color(c.r(), c.g(), c.b()));
        region->setCacheResultColor(i, Color(c));
      }

      emit updated();
    }


    void saveWarm(WarmMemento& state) const {
      // Set the type field.
      state.set_type(WarmMemento::REGION_RENDERER);

      // Create a warm memento and fill it with the appropriate data and
      // cache colors.
      RegionRendererWarm *warm = state.mutable_region_renderer();
      for(unsigned i=0; i<region->numStripes(); i++){
        // Add a stripe, then set its component colors.
        RegionRendererWarm::Stripe *stripe = warm->add_stripe();

        stripe->mutable_data()->set_r(region->getDataColor(i).r);
        stripe->mutable_data()->set_g(region->getDataColor(i).g);
        stripe->mutable_data()->set_b(region->getDataColor(i).b);

        stripe->mutable_cache()->set_r(region->getCacheResultColor(i).r);
        stripe->mutable_cache()->set_g(region->getCacheResultColor(i).g);
        stripe->mutable_cache()->set_b(region->getCacheResultColor(i).b);
      }
    }

    // TODO(choudhury): this function and the next should make a call to a
    // helper function, with the "before" and "after" arguments specified.
    void applyDelta(const DeltaMemento& state){
      // std::cout << "applyDelta, element " << state.id() << std::endl;

      // Check runtime type.
      assert(state.type() == DeltaMemento::REGION_RENDERER);

      // Extract the RegionRenderer delta.
      const RegionRendererDelta& delta = state.region_renderer();

      // One by one apply the changes.
      for(int i=0; i<delta.delta_size(); i++){
        const RegionRendererDelta::Delta& d = delta.delta(i);

        // Runtime debug check - make sure the object's precondition
        // matches what is encoded in the delta.
        //
        // assert(region->getDataColor(d.index()) == Color(d.data_before()));
        // assert(region->getCacheResultColor(d.index()) == Color(d.cache_before()));
        assert(checkColors(region->getDataColor(d.index()), Color(d.data_before())));
        assert(checkColors(region->getCacheResultColor(d.index()), Color(d.cache_before())));

        // std::cout << "component " << i << ":" << std::endl;
        // std::cout << "\tData before: " << region->getDataColor(d.index()) << std::endl;
        // std::cout << "\tCache before: " << region->getCacheResultColor(d.index()) << std::endl;

        region->setDataColor(d.index(), Color(d.data_after()));
        region->setCacheResultColor(d.index(), Color(d.cache_after()));

        // std::cout << "\tData after: " << region->getDataColor(d.index()) << std::endl;
        // std::cout << "\tCache after: " << region->getCacheResultColor(d.index()) << std::endl;
      }

      emit updated();
    }

    // TODO(choudhury): see todo for applyDelta().
    void unapplyDelta(const DeltaMemento& state){
      // Check runtime type.
      assert(state.type() == DeltaMemento::REGION_RENDERER);

      // Extract the RegionRenderer delta.
      const RegionRendererDelta& delta = state.region_renderer();

      // One by one apply the changes.
      for(int i=0; i<delta.delta_size(); i++){
        const RegionRendererDelta::Delta& d = delta.delta(i);

        // Runtime debug check - make sure the object's precondition
        // matches what is encoded in the delta.
        //
        // assert(region->getDataColor(d.index()) == Color(d.data_after()));
        // assert(region->getCacheResultColor(d.index()) == Color(d.cache_after()));
        assert(checkColors(region->getDataColor(d.index()), Color(d.data_after())));
        assert(checkColors(region->getCacheResultColor(d.index()), Color(d.cache_after())));

        region->setDataColor(d.index(), Color(d.data_before()));
        region->setCacheResultColor(d.index(), Color(d.cache_before()));
      }

      emit updated();
    }

  private:
    static bool checkColors(const Color& c1, const Color& c2, const std::string& type = "", int idx=-1){
      std::string message;
      if(type != ""){
        std::stringstream ss;
        ss << type << ", cell " << idx << ":\n";
        message = ss.str();
      }

      if(c1 != c2){
        std::cout << message;
        std::cout << "Color 1: " << c1 << std::endl;
        std::cout << "Color 2: " << c2 << std::endl;
        std::cout << "diff color: (" << (c1.r - c2.r) << ", " << (c1.g - c2.g) << ", " << (c1.r - c2.r) << ")" << std::endl;
        return false;
      }

      return true;
    }

  private:
    // The widget representing the region.
    RegionDisplay::ptr region;

    // A map of the last several accesses to this region, used to fade
    // out older accesses gradually.
    std::map<unsigned, float> history;
  };

  typedef RegionRenderer$Template<false> RegionRenderer;
  typedef RegionRenderer$Template<true> RegionRenderer$ComputeDelta;
}

#endif
