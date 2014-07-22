// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury, all rights reserved
//
// CacheTemperatureDisplay.h - A widget consisting of concentric
// circles, each of which has an independent color.  These are meant
// to show the "cache temperature" of each level of a cache.

#ifndef CACHE_TEMPERATURE_DISPLAY_H
#define CACHE_TEMPERATURE_DISPLAY_H

// MTV headers.
#include <Core/Geometry/Point.h>
#include <Core/Graphics/SolidRectangle.h>
#include <Core/Graphics/FadingPoint.h>
#include <Core/Graphics/FadingRing.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class CacheTemperatureDisplay : public Widget {
  public:
    BoostPointers(CacheTemperatureDisplay);

  public:
    CacheTemperatureDisplay(const Point& location,
                            unsigned numLevels,
                            float radius);

    void setColor(unsigned i, const Color& color){
      i == 0 ? center->setColor(color) : levels[i-1]->setColor(color);

      // levels[i]->setColor(color);
    }

    const Color& getColor(unsigned i) const {
      return i == 0 ? center->getColor() : levels[i-1]->getColor();

      // return levels[i]->getColor();
    }

    unsigned numLevels() const {
      return levels.size() + 1;

      // return levels.size();
    }

    // From the Widget interface.
    bool contains(const Point& p);
    void draw() const;

  private:
    FadingPoint::ptr center;
    // std::vector<SolidRectangle::ptr> levels;

    std::vector<FadingRing::ptr> levels;
  };
}

#endif
