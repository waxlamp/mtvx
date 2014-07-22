// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// RegionDisplay.cpp

// MTV headers.
#include <Core/Graphics/Tick.h>
#include <Modules/ReferenceTrace/Graphics/RegionDisplay.h>
using MTV::Color;
using MTV::RegionDisplay;
using MTV::SolidRectangle;
using MTV::StripedQuad;
using MTV::Tick;

RegionDisplay::RegionDisplay(const Point& location,
                             MTR::addr_t base, MTR::addr_t limit, MTR::size_t type,
                             const Color& shellColor,
                             float height,
                             const std::string& title)
  : Widget(location),
    shell(shellColor)
{
  // Compute the number of stripes needed; this will be the same for
  // both the data quad and the cache quad.
  const size_t size = limit - base;
  const unsigned numStripes = size / type;

  // Set the proportions of the main widget height each sub-widget
  // will take.
  const float dataHeight = 0.8*height;
  const float cacheHeight = 0.2*height;
  const float stripeWidth = 10.0f;
  const float gapWidth = 3.0f;

  // TODO(choudhury): make sure it's ok to just assign a
  // StripedQuad::ptr to data (boost docs say to ALWAYS declare a new
  // smart pointer variable and then use it by name).
  //
  // Data display subwidget.
  data = StripedQuad::ptr(new StripedQuad(Point::zero,
                                          numStripes,
                                          stripeWidth, dataHeight, // width, height of stripes
                                          gapWidth, // width of inter-stripe gap
                                          MTV::Colors::Region::cold));
  //Color(0.3, 0.3, 0.3)));

  // Cache display subwidget.
  cache = StripedQuad::ptr(new StripedQuad(Point::zero,
                                           numStripes,
                                           stripeWidth, cacheHeight,
                                           gapWidth,
                                           Colors::Cache::cold));

  // Background "shell" subwidget of a certain color.
  const float widgetWidth = (stripeWidth + gapWidth) * numStripes;
  const float shellThickness = 10.0;
  backplate = SolidRectangle::ptr(new SolidRectangle(Point(-shellThickness, -shellThickness),
                                                     Vector(2.0*shellThickness + widgetWidth, 2.0*shellThickness + height),
                                                     shellColor,
                                                     true));

  // Title placard subwidget.
  TextWidget::ptr placard(new TextWidget(Point::zero, title, WidgetPanel::computer));

  // Tick subwidgets.
  //
  // TODO(choudhury): set the spacing according to how long the label
  // text is (not set arbitrarily to 5).
  for(unsigned i=0; i<numStripes; i+=5){
    std::stringstream ss;
    ss << "0x" << std::hex << (base + i*type);

    const float extra = i/5 % 2 == 0 ? 0.0 : QFontMetrics(WidgetPanel::computer).height();
    Tick::ptr tick(new Tick(Point::zero, ss.str().c_str(), extra));

    this->addChild(tick, Vector(i*(stripeWidth+gapWidth) + 0.5*stripeWidth, -shellThickness - 0.5*tick->tickHeight()));
  }

  // Claim the subwidgets as children - they will be deleted
  // automatically at the right time.
  this->addChild(backplate, Vector(-shellThickness, -shellThickness));
  this->addChild(cache, Vector(0.0, 0.0));
  this->addChild(data, Vector(0.0, cacheHeight));
  this->addChild(placard, Vector(0.5*widgetWidth - 0.5*placard->width(), shellThickness + height + 0.5*placard->height()));
}

void RegionDisplay::setDataColor(unsigned i, const Color& color){
  data->setStripeColor(i, color);
}

const Color& RegionDisplay::getDataColor(unsigned i) const {
  return data->getStripeColor(i);
}

void RegionDisplay::setCacheResultColor(unsigned i, const Color& color){
  cache->setStripeColor(i, color);
}

const Color& RegionDisplay::getCacheResultColor(unsigned i) const {
  return cache->getStripeColor(i);
}

void RegionDisplay::setShellColor(const Color& color){
  shell = color;
}

const Color& RegionDisplay::getShellColor() const {
  return shell;
}

bool RegionDisplay::contains(const Point& p){
  return childrenContain(p);
}

void RegionDisplay::draw() const {
  this->drawChildren();
}
