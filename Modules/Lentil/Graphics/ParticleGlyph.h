// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// ParticleGlyph.h - A widget representing a single MPM particle.

#ifndef PARTICLE_GLYPH_H
#define PARTICLE_GLYPH_H

// MTV includes.
#include <Core/Color/ColorProfile.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

// System includes.
#include <iostream>

namespace MTV{
  class ParticleGlyph : public Widget {
  public:
    BoostPointers(ParticleGlyph);

  protected:
    ParticleGlyph(const Point& location, const Color& corecolor, const Color& shellcolor, float width, float height)
      : Widget(location),
        corecolor(corecolor),
        // shellcolor(shellcolor),
        shellcolors(numQuadsInShell, MTV::Colors::Cache::cold),
        p(0),
        width(width),
        height(height),
        marked(false)
    {}

  public:
    static ParticleGlyph::ptr New(const Point& location, const Color& corecolor, const Color& shellcolor, float width, float height){
      return ParticleGlyph::ptr(new ParticleGlyph(location, corecolor, shellcolor, width, height));
    }

    float getWidth() const { return width; }
    float getHeight() const { return height; }

    const Color& getShellColor(unsigned i) const {
      return shellcolors[i];
    }

    void setMarker(){
      marked = true;
    }

    void clearMarker(){
      marked = false;
    }

    void setShellColor(const Color& c){
      // shellcolor = c;

      // Set the color of the correct shell cell and advance the
      // pointer (wrapping to 0 if necessary).
      shellcolors[p] = c;
      p = (p + 1) % numQuadsInShell;

      // Fade all the colors towards the cold cache color, except the
      // one just set.
      for(int i=p; i != (p + numQuadsInShell - 1) % numQuadsInShell; i = (i+1)%numQuadsInShell){
        shellcolors[i] = shellcolors[i]*0.9 + MTV::Colors::Cache::cold*0.1;
      }
    }

    const Color& getCoreColor() const {
      return corecolor;
    }

    void setCoreColor(const Color& c){
      corecolor = c;
    }

    // Widget interface.
    bool contains(const Point& p){
      const Vector into = location - p;
      return ((-0.5*width <= into.x and into.x <= 0.5*width) and
              (-0.5*height <= into.y and into.y <= 0.5*height));
    }

    void draw() const;

  private:
    // Color corecolor, shellcolor;
    Color corecolor;
    std::vector<Color> shellcolors;
    int p;

    float width, height;

    static const int numQuadsPerSide = 4;
    static const int numQuadsInShell = numQuadsPerSide * 4;

    bool marked;
  };
}

#endif
