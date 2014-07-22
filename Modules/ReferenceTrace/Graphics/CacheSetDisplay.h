// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheSetDisplay.h - Widgets for easy display of the levels of a
// cache set.  Analogous to the stuff in CacheDisplay, but for the
// unified cache level shared between the caches in a CacheSet.

#ifndef CACHE_SET_DISPLAY_H
#define CACHE_SET_DISPLAY_H

// MTV headers.
#include <Core/Geometry/Point.h>
#include <Core/Graphics/RowContainer.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>
#include <Modules/ReferenceTrace/Graphics/CacheLevelGlyph.h>
#include <Tools/CacheSimulator/Cache.h>

// System headers.
#include <stdexcept>

namespace MTV{
  using Daly::CacheLevel;

  // class CacheLevelSetDisplay : public Widget {
  // public:
  //   BoostPointers(CacheLevelSetDisplay);

  // public:
  //   CacheLevelSetDisplay(const Point& location, const std::vector<CacheLevel::ptr>& levels, const std::string& label_text);

  //   bool contains(const Point& p);
  //   void draw() const;

  //   void setShellColor(CacheLevel::ptr which, const Color& c){
  //     backplate->setColor(c);
  //   }

  //   const Color& getShellColor(CacheLevel::ptr which) const {
  //     return backplate->getColor();
  //   }

  //   void setBlockColor(CacheLevel::ptr which, unsigned i, const Color& c){
  //     blocks->setStripeColor(i, c);
  //   }

  //   const Color& getBlockColor(CacheLevel::ptr which, unsigned i) const {
  //     return blocks->getStripeColor(i);
  //   }

  //   void setResultColor(CacheLevel::ptr which, unsigned i, const Color& c){
  //     results->setStripeColor(i, c);
  //   }

  //   const Color& getResultColor(CacheLevel::ptr which, unsigned i) const {
  //     return results->getStripeColor(i);
  //   }
  // };

  class CacheSetDisplay : public Widget {
  public:
    BoostPointers(CacheSetDisplay);

  public:
    CacheSetDisplay(const Point& location, const std::vector<std::vector<CacheLevel::ptr> >& levels, const std::vector<std::string>& labels = std::vector<std::string>());

    bool contains(const Point& p){
      return this->childrenContain(p);
    }

    void draw() const {
      this->drawChildren();
    }

    void setShellColor(CacheLevel::ptr which, const Color& c){
      CacheLevelGlyph::ptr glyph = this->getGlyph(which);
      glyph->setShellColor(c);
    }

    const Color& getShellColor(CacheLevel::ptr which) const {
      CacheLevelGlyph::ptr glyph = this->getGlyph(which);
      return glyph->getShellColor();
    }

    void setBlockColor(CacheLevel::ptr which, unsigned i, const Color& c){
      CacheLevelGlyph::ptr glyph = this->getGlyph(which);
      glyph->setBlockColor(i, c);
    }

    const Color& getBlockColor(CacheLevel::ptr which, unsigned i) const {
      CacheLevelGlyph::ptr glyph = this->getGlyph(which);
      return glyph->getBlockColor(i);
    }

    void setResultColor(CacheLevel::ptr which, unsigned i, const Color& c){
      CacheLevelGlyph::ptr glyph = this->getGlyph(which);
      glyph->setResultColor(i, c);
    }

    const Color& getResultColor(CacheLevel::ptr which, unsigned i) const {
      CacheLevelGlyph::ptr glyph = this->getGlyph(which);
      return glyph->getResultColor(i);
    }

  private:
    CacheLevelGlyph::ptr getGlyph(CacheLevel::ptr which) const throw(std::domain_error);

    static std::string makeLabel(const std::vector<std::string>& labels, unsigned rank);

  private:
    std::map<CacheLevel::ptr, CacheLevelGlyph::ptr> glyphs;
    std::vector<RowContainer::ptr> rows;
  };
}

#endif
