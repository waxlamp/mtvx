// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury, all rights reserved
//
// CacheTemperatureRenderer.h - Consumes HitRecords objects and
// converts them into a color to display in a cache temperature
// display.

#ifndef CACHE_TEMPERATURE_RENDERER_H
#define CACHE_TEMPERATURE_RENDERER_H

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Dataflow/CachePerformanceCounter.h>
#include <Core/Dataflow/UpdateNotifier.h>
#include <Core/Geometry/Point.h>
#include <Core/Util/BoostPointers.h>
#include <Modules/ReferenceTrace/Graphics/CacheTemperatureDisplay.h>

namespace MTV{
  class CacheTemperatureRenderer : public Consumer<CacheHitRates>,
                                   public UpdateNotifier {
  public:
    BoostPointers(CacheTemperatureRenderer);

  public:
    CacheTemperatureRenderer(const Point& location,
                             unsigned numLevels,
                             float radius,
                             const Color& cold,
                             const Color& neutral,
                             const Color& hot);

    void consume(const CacheHitRates& rates);

    CacheTemperatureDisplay::ptr getWidget(){
      return display;
    }

    const std::vector<float>& getRates() const {
      return rates;
    }

  private:
    Color cold, neutral, hot;
    CacheTemperatureDisplay::ptr display;

    std::vector<float> rates;
  };
}

#endif
