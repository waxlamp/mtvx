// Copyright 2011 A.N.M. Imroz Choudhury, all rights reserved
//
// CacheTemperatureRenderer.cpp

// MTV headers.
#include <Core/Math/Interpolation.h>
#include <Modules/ReferenceTrace/Renderers/CacheTemperatureRenderer.h>
using MTV::CacheTemperatureRenderer;
using MTV::Color;
using MTV::LinearInterpolator;

CacheTemperatureRenderer::CacheTemperatureRenderer(const Point& location,
                                                   unsigned numLevels,
                                                   float radius,
                                                   const Color& cold,
                                                   const Color& neutral,
                                                   const Color& hot)
  : cold(cold),
    neutral(neutral),
    hot(hot),
    display(boost::make_shared<CacheTemperatureDisplay>(location, numLevels, radius))
{}

void CacheTemperatureRenderer::consume(const CacheHitRates& r){
  // Grab the vector of hit rates.
  // const std::vector<float>& rates = r.getRates();
  rates = r.getRates();

  // for(unsigned i=0; i<rates.size(); i++){
  //   std::cout << rates[i] << std::endl;
  // }

  // Interpolate each color from cold to hot, and set accordingly.
  for(unsigned i=0; i<display->numLevels(); i++){
    // Select a color and use the magnitude of the value to interpolate.
    const Color& color = rates[i] < 0 ? cold : hot;
    const float val = std::abs(rates[i]);

    const Color c(LinearInterpolator<float>::interpolate(neutral.r, color.r, val),
                  LinearInterpolator<float>::interpolate(neutral.g, color.g, val),
                  LinearInterpolator<float>::interpolate(neutral.b, color.b, val) );

    display->setColor(i, c);
  }

  // Emit update signal.
  emit updated();
}
