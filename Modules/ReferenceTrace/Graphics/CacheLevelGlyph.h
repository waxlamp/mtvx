// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheLevelGlyph.h - A widget for displaying just the blocks of a
// cache. Composed with a TextWidget to form a CacheLevelDisplay
// widget.

#ifndef CACHE_LEVEL_GLYPH_H
#define CACHE_LEVEL_GLYPH_H

// MTV headers.
#include <Core/Graphics/SolidRectangle.h>
#include <Core/Graphics/StripedQuad.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class CacheLevelGlyph : public Widget {
  public:
    BoostPointers(CacheLevelGlyph);

  public:
    CacheLevelGlyph(const Point& location, unsigned numBlocks, float height);

    float width() const {
      return backplate->width();
    }

    float height() const {
      return this->backplateHeight();
    }

    bool contains(const Point& p){
      return this->childrenContain(p);
    }

    void draw() const {
      this->drawChildren();
    }

    void setShellColor(const Color& c){
      backplate->setColor(c);
    }

    const Color& getShellColor() const {
      return backplate->getColor();
    }

    void setBlockColor(unsigned i, const Color& c){
      blocks->setStripeColor(i, c);
    }

    const Color& getBlockColor(unsigned i) const {
      return blocks->getStripeColor(i);
    }

    void setResultColor(unsigned i, const Color& c){
      results->setStripeColor(i, c);
    }

    const Color& getResultColor(unsigned i) const {
      return results->getStripeColor(i);
    }

    float backplateWidth() const {
      return backplate->width();
    }

    float backplateHeight() const {
      return backplate->height();
    }

  private:
    SolidRectangle::ptr backplate;
    StripedQuad::ptr blocks;
    StripedQuad::ptr results;
  };
}

#endif
