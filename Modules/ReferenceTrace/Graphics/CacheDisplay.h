// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheDisplay.h - Widgets for easy display of the levels of a cache,
// with labels, etc.

#ifndef CACHE_DISPLAY_H
#define CACHE_DISPLAY_H

// MTV includes.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Graphics/Widget.h>
#include <Core/Graphics/SolidRectangle.h>
#include <Core/Graphics/StripedQuad.h>
#include <Core/Graphics/TextWidget.h>
#include <Core/Util/BoostPointers.h>
#include <Modules/ReferenceTrace/Graphics/CacheLevelGlyph.h>

namespace MTV{
  class CacheLevelDisplay : public Widget {
  public:
    BoostPointers(CacheLevelDisplay);

  public:
    CacheLevelDisplay(const Point& location, unsigned numBlocks, float height, const std::string& labelText);

    bool contains(const Point& p){
      return this->childrenContain(p);
    }

    void draw() const {
      this->drawChildren();
    }

    void setShellColor(const Color& c){
      glyph->setShellColor(c);
    }

    const Color& getShellColor() const {
      return glyph->getShellColor();
    }

    void setBlockColor(unsigned i, const Color& c){
      glyph->setBlockColor(i, c);
    }

    const Color& getBlockColor(unsigned i) const {
      return glyph->getBlockColor(i);
    }

    void setResultColor(unsigned i, const Color& c){
      glyph->setResultColor(i, c);
    }

    const Color& getResultColor(unsigned i) const {
      return glyph->getResultColor(i);
    }

    float width() const {
      return glyph->width();
    }

    float height() const {
      return glyph->height();
    }

    float backplateWidth() const {
      return glyph->backplateWidth();
    }

    float backplateHeight() const {
      return glyph->backplateHeight();
    }

  private:
    CacheLevelGlyph::ptr glyph;
    TextWidget::ptr placard;
  };

  class CacheDisplay : public Widget {
  public:
    BoostPointers(CacheDisplay);

  public:
    CacheDisplay(const Point& location)
      : Widget(location)
    {}

    void addLevel(CacheLevelDisplay::ptr level, const Vector& offset = Vector::zero);

    bool contains(const Point& p){
      return this->childrenContain(p);
    }

    void draw() const {
      return this->drawChildren();
    }

    void setShellColor(unsigned L, const Color& c){
      levels[L]->setShellColor(c);
    }

    const Color& getShellColor(unsigned L) const {
      return levels[L]->getShellColor();
    }

    void setBlockColor(unsigned L, unsigned i, const Color& c){
      levels[L]->setBlockColor(i, c);
    }

    const Color& getBlockColor(unsigned L, unsigned i) const {
      return levels[L]->getBlockColor(i);
    }

    void setResultColor(unsigned L, unsigned i, const Color& c){
      levels[L]->setResultColor(i, c);
    }

    const Color& getResultColor(unsigned L, unsigned i) const {
      return levels[L]->getResultColor(i);
    }

  private:
    std::vector<CacheLevelDisplay::ptr> levels;
  };
}

#endif
