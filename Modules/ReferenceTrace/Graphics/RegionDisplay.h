// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// RegionDisplay.h - A widget composed of two StripedQuads (one for
// addresses, and one for cache status) and a border, used to display
// a block of several contiguous addresses.

#ifndef REGION_DISPLAY_H
#define REGION_DISPLAY_H

// MTV headers.
#include <Core/Graphics/SolidRectangle.h>
#include <Core/Graphics/StripedQuad.h>
#include <Core/Graphics/TextWidget.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>

namespace MTV{
  class RegionDisplay : public Widget {
  public:
    BoostPointers(RegionDisplay);

  public:
    RegionDisplay(const Point& location,
                  MTR::addr_t base, MTR::addr_t limit, MTR::size_t type,
                  const Color& shellcolor,
                  float height,
                  const std::string& title);

    unsigned numStripes() const { return data->numStripes(); }

    void setDataColor(unsigned i, const Color& color);
    const Color& getDataColor(unsigned i) const;

    void setCacheResultColor(unsigned i, const Color& color);
    const Color& getCacheResultColor(unsigned i) const;

    void setShellColor(const Color& color);
    const Color& getShellColor() const;

    // From the Widget interface.
    bool contains(const Point& p);
    void draw() const;

  private:
    Color shell;

    // TextWidget::ptr title;
    SolidRectangle::ptr backplate;
    StripedQuad::ptr data;
    StripedQuad::ptr cache;
  };
}

#endif
