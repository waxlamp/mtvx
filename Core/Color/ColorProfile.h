// -*- c++ -*-
//
// ColorProfile.h - Interface class (and some example implementors)
// for a color profile, handling all color aspects of rendering.

#ifndef COLOR_PROFILE_H
#define COLOR_PROFILE_H

// MTV includes.
#include <Core/Color/Color.h>

namespace MTV{
  class ColorProfile{
  public:
    virtual Color background() const = 0;
    virtual Color text() const = 0;
  };

  class PrintColorProfile : public ColorProfile {
    Color background() const { return Color(0.9, 0.9, 0.9); }
    Color text() const { return Color::black; }
  };

  class ScreenColorProfile : public ColorProfile {
    Color background() { return Color::black; }
    Color text() { return Color::white; }
  };

  namespace Colors{
    namespace Region{
      const Color read(0.0, 200.0/255, 142.0/255);
      const Color write(239.0/255, 137.0/255, 0.0);

      const Color cold(0.3, 0.3, 0.3);
    }
 
    namespace Cache{
      const Color L1(0.0, 0.0, 214./255);
      const Color L2(101./255, 0.0, 202./255);
      const Color miss(197./255, 0.0, 0.0);

      const Color cold(0.2, 0.2, 0.2);
    }
  }
}

#endif
