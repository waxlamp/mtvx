// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheDisplay.cpp

// MTV includes.
#include <Core/Color/ColorProfile.h>
#include <Modules/ReferenceTrace/Graphics/CacheDisplay.h>
using MTV::CacheDisplay;
using MTV::CacheLevelDisplay;
using MTV::TextWidget;
using MTV::Vector;

CacheLevelDisplay::CacheLevelDisplay(const Point& location, unsigned numBlocks, float height, const std::string& labelText)
  : Widget(location),
    glyph(new CacheLevelGlyph(Point::zero, numBlocks, height)),
    placard(new TextWidget(Point::zero, labelText, WidgetPanel::computer))
{
  // Claim the subwidgets as children.
  this->addChild(glyph, Vector(0.0, 0.0));
  this->addChild(placard, Vector(-10 - placard->width(), 0.5*height - 0.5*placard->height()));
}

void CacheDisplay::addLevel(CacheLevelDisplay::ptr level, const Vector& offset){
  this->addChild(level, offset);
}
