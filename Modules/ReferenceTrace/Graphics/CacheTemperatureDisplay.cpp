// Copyright 2011 A.N.M. Imroz Choudhury, all rights reserved
//
// CacheTemperatureDisplay.cpp

// MTV headers.
#include <Core/Util/BoostForeach.h>
#include <Modules/ReferenceTrace/Graphics/CacheTemperatureDisplay.h>
using MTV::CacheTemperatureDisplay;
using MTV::FadingPoint;
using MTV::FadingRing;
using MTV::Point;
using MTV::SolidRectangle;
using MTV::Widget;

CacheTemperatureDisplay::CacheTemperatureDisplay(const Point& location, unsigned numLevels, float radius)
  : Widget(location),
    center(boost::make_shared<FadingPoint>(location, Color::white, 4*radius)),

    levels(numLevels-1)
    // levels(numLevels)
{
  const Vector off(radius, radius);

  // Create the levels in a sane order.
  for(unsigned i=0; i<levels.size(); i++){
    // levels[i] = boost::make_shared<SolidRectangle>(location - (i+1)*off, location + (i+1)*off, Color::white, true);

    levels[i] = boost::make_shared<FadingRing>(location, Color::white, (i+2.5)*radius);
  }

  // Add them as child widgets in reverse order (so they are drawn
  // correctly).
  foreach_reverse(Widget::ptr w, levels){
    this->addChild(w);
  }

  // Add the central widget too.
  this->addChild(center);
}

bool CacheTemperatureDisplay::contains(const Point& p){
  return levels.back()->contains(p);
}

void CacheTemperatureDisplay::draw() const {
  this->drawChildren();
}
