// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// FadingPoint.h - A widget consisting of an alpha-modulated circular
// region.

#ifndef FADING_POINT_H
#define FADING_POINT_H

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/Graphics/GaussianAlphaGlyph.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class FadingPoint : public Widget {
  public:
    BoostPointers(FadingPoint);

  public:
    FadingPoint(const Point& p, const Color& c = Color::white, int size = 1)
      : Widget(p),
        basecolor(c),
        color(c),
        size(size)
    {}

    FadingPoint(const Color& c = Color::white, int size = 1)
      : Widget(Point::zero),
        color(c),
        size(size)
    {}

    FadingPoint(const Point& p, int size)
      : Widget(p),
        color(Color::white),
        size(size)
    {}

    FadingPoint(int size)
      : Widget(Point::zero),
        color(Color::white),
        size(size)
    {}

    const Color& getColor() const { return color; }

    const Color& getBaseColor() const { return basecolor; }

    void setColor(const Color& c){
      color = c;
    }

    void setSize(int sz){
      size = sz;
    }

    int getSize() const {
      return size;
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
    static GaussianAlphaGlyph glyph;

  private:
    const Color basecolor;
    Color color;
    int size;
  };
}

#endif
