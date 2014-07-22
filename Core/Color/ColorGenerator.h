// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ColorGenerator.h - A way to produce different colors on demand.

#ifndef COLOR_GENERATOR_H
#define COLOR_GENERATOR_H

// MTV includes.
#include <Core/Color/Color.h>
#include <Core/Util/BoostPointers.h>

// System includes.
#include <vector>

namespace MTV{
  class ColorGenerator{
  public:
    BoostPointers(ColorGenerator);

  public:
    virtual const Color& color(unsigned i) = 0;
  };

  class RGBCorners : public ColorGenerator {
  public:
    BoostPointers(RGBCorners);

  public:
    RGBCorners();

    const Color& color(unsigned i);

  private:
    std::vector<Color> colors;
  };

  class ColorbrewerQualitative11_Paired : public ColorGenerator {
  public:
    BoostPointers(ColorbrewerQualitative11_Paired);

  public:
    const Color& color(unsigned i){
      return colors[i % numColors];
    }

  private:
    static const Color colors[];
    static const unsigned numColors;
  };

  class ColorbrewerQualitative9_Set1 : public ColorGenerator {
  public:
    BoostPointers(ColorbrewerQualitative9_Set1);

  public:
    const Color& color(unsigned i){
      return colors[i % numColors];
    }

  private:
    static const Color colors[];
    static const unsigned numColors;
  };

  
  class ColorbrewerQualitative9_Set1_Mod1 : public ColorGenerator {
  public:
    BoostPointers(ColorbrewerQualitative9_Set1_Mod1);

  public:
    const Color& color(unsigned i){
      return colors[i % numColors];
    }

  private:
    static const Color colors[];
    static const unsigned numColors;
  };

  class ColorbrewerQualitative9_Set1_Mod2 : public ColorGenerator {
  public:
    BoostPointers(ColorbrewerQualitative9_Set1_Mod2);

  public:
    const Color& color(unsigned i){
      return colors[i % numColors];
    }

  private:
    static const Color colors[];
    static const unsigned numColors;
  };
}

#endif
