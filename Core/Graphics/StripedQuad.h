// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// StripedQuad.h - Draws a "light panel" of vertical bars that can be
// individually controlled to appear as a certain color.

#ifndef STRIPED_QUAD_H
#define STRIPED_QUAD_H

// MTV headers.
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <vector>

namespace MTV{
  class StripedQuad : public Widget {
  public:
    BoostPointers(StripedQuad);

  public:
    StripedQuad(Point location, int numStripes, float stripeWidth, float stripeHeight, float spacing, Color blank);

    unsigned numStripes() const { return stripes.size(); }

    const Color& getBlankColor() const { return blank; }

    const Color& getStripeColor(int i) const { return stripes[i]; }

    void setStripeColor(int i, const Color& c){
      stripes[i] = c;
    }

    bool contains(const Point& p);
    void draw() const;

  private:
    // The "blank" (i.e. "off" color).
    Color blank;
    std::vector<Color> stripes;

    float stripeWidth;
    float spacing;

    float height;
  };
}

#endif
