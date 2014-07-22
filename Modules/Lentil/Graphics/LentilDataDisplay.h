// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilDataDisplay.h - Widget for displaying lentil output data.

#ifndef LENTIL_DATA_DISPLAY_H
#define LENTIL_DATA_DISPLAY_H

// MTV includes.
#include <Core/Color/Color.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>
#include <Modules/Lentil/Graphics/ParticleGlyph.h>

namespace MTV{
  class LentilDataDisplay : public Widget {
  public:
    BoostPointers(LentilDataDisplay);

  public:
    LentilDataDisplay(const Point& location, int gx, int gy, float gridx, float gridy)
      : Widget(location),
        gx(gx),
        gy(gy),
        gridx(gridx),
        gridy(gridy)
    {}

    bool contains(const Point& p){
      return this->childrenContain(p);
    }

    void draw() const {
      static const float hx = gridx / (gx-1);
      static const float hy = gridy / (gy-1);

      // Draw the grid.
      Color::black.glSet();
      glBegin(GL_LINES);
      {
        for(int i=0; i<gx; i++){
          glVertex2f(i*hx, 0.0);
          glVertex2f(i*hx, gridy);
        }

        for(int i=0; i<gy; i++){
          glVertex2f(0.0, i*hy);
          glVertex2f(gridx, i*hy);
        }
      }
      glEnd();

      this->drawChildren();
    }

    void initialize(unsigned i, const Point& where, const Color& corecolor, const Color& shellcolor, float width, float height){
      glyphs[i] = ParticleGlyph::New(where, corecolor, shellcolor, width, height);
    }

    void addParticle(const Point& where, const Color& corecolor, const Color& shellcolor, float width, float height){
      ParticleGlyph::ptr p = ParticleGlyph::New(where, corecolor, shellcolor, width, height);
      this->addChild(p);
      glyphs.push_back(p);
    }

    void resizeParticles(unsigned size, const Point& where, const Color& corecolor, const Color& shellcolor, float width, float height){
      // Save the old size of the vector.
      const unsigned oldsize = glyphs.size();

      if(size < oldsize){
        std::cerr << "error!  shrinking the particle set is not supported!" << std::endl;
        abort();
      }

      // Resize the vector.
      glyphs.resize(size);

      // Add all the new widgets as children.
      for(unsigned i=oldsize; i<glyphs.size(); i++){
        glyphs[i] = ParticleGlyph::New(where, corecolor, shellcolor, width, height);
        this->addChild(glyphs[i]);
      }
    }

    unsigned numParticles() const { return glyphs.size(); }
    ParticleGlyph::ptr particle(unsigned i) { return glyphs[i]; }

  private:
    std::vector<ParticleGlyph::ptr> glyphs;
    int gx, gy; // The number of nodes in the x and y directions.
    float gridx, gridy; // The size of the grid in the x and y directions.
  };
}

#endif
