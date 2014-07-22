// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheLevelGlyph.cpp

// MTV headers.
#include <Core/Color/ColorProfile.h>
#include <Modules/ReferenceTrace/Graphics/CacheLevelGlyph.h>
using MTV::CacheLevelGlyph;

CacheLevelGlyph::CacheLevelGlyph(const Point& location, unsigned numBlocks, float height)
  : Widget(location)
{
  // Set the proportions of the main widget height that each subwidget
  // will take.
  const float blockHeight = 0.8*height;
  const float resultHeight = 0.2*height;
  const float stripeWidth = 10.0f;
  const float gapWidth = 3.0f;

  // Create a StripedQuad widget for the block display.
  //
  // NOTE(choudhury): use the region cold-color just to set off a cold
  // block from a cold result (below).
  blocks = StripedQuad::ptr(new StripedQuad(Point::zero,
                                            numBlocks,
                                            stripeWidth, blockHeight, // width, height of stripes
                                            gapWidth, // width of inter-stripe gap
                                            MTV::Colors::Region::cold));
  
  // Create a StripedQuad widget for the result display.
  results = StripedQuad::ptr(new StripedQuad(Point::zero,
                                             numBlocks,
                                             stripeWidth, resultHeight,
                                             gapWidth,
                                             MTV::Colors::Cache::cold));

  // Background shell widget.
  const float widgetWidth = (stripeWidth + gapWidth) * numBlocks;
  const float shellThickness = 10.0;
  backplate = SolidRectangle::ptr(new SolidRectangle(Point(-shellThickness, -shellThickness),
                                                     Vector(2.0*shellThickness + widgetWidth, 2.0*shellThickness + height),
                                                     Color::black,
                                                     true));

  // Claim the subwidgets as children.
  this->addChild(backplate, Vector(-shellThickness, -shellThickness));
  this->addChild(results, Vector(0.0, 0.0));
  this->addChild(blocks, Vector(0.0, resultHeight));
}
