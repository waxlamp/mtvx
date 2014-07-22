// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// FadingRing.h - A widget consisting of an alpha-modulated annular
// region.

#ifndef FADING_RING_H
#define FADING_RING_H

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/Graphics/SineAlphaGlyph.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class FadingRing : public Widget {
  public:
    BoostPointers(FadingRing);

  public:
    FadingRing(const Point& p, const Color& c = Color::white, int size = 1)
      : Widget(p),
        color(c),
        size(size)
    {}

    FadingRing(const Color& c = Color::white, int size = 1)
      : Widget(Point::zero),
        color(c),
        size(size)
    {}

    FadingRing(const Point& p, int size)
      : Widget(p),
        color(Color::white),
        size(size)
    {}

    FadingRing(int size)
      : Widget(Point::zero),
        color(Color::white),
        size(size)
    {}

    const Color& getColor() const { return color; }

    void setColor(const Color& c){
      color = c;
    }

    // TODO(choudhury): fix this.
    bool contains(const Point& p) { return false; }

    void draw() const {
      glyph.EnableBlending();
      color.glSet();
      glyph.Draw(location.x, location.y, size);
      glyph.DisableBlending();
    };

  private:
    static SineAlphaGlyph glyph;

  private:
    Color color;
    int size;
  };
}

#endif
