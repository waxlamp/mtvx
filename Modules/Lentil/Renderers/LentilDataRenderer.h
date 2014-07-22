// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilDataRenderer.h - Reads data from disk to render particles and
// (TODO:choudhury) grid, and accepts rendering commands to update the
// features of the rendered glyphs.

#ifndef LENTIL_DATA_RENDERER_H
#define LENTIL_DATA_RENDERER_H

// MTV includes.
#include <Core/Dataflow/UpdateNotifier.h>
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Modules/Lentil/Dataflow/DataRetrievalCommander.h>
#include <Modules/Lentil/Dataflow/ParticleStatusRenderCommand.h>
#include <Modules/Lentil/Graphics/LentilDataDisplay.h>

namespace MTV{
  class LentilDataRenderer : public Consumer<ParticleStatusRenderCommand>,
                             public Consumer<RetrieveData>,
                             public UpdateNotifier {
  public:
    BoostPointers(LentilDataRenderer);

  protected:
    LentilDataRenderer(const std::string& dir);

  public:
    static LentilDataRenderer::ptr New(const std::string& dir){
      return LentilDataRenderer::ptr(new LentilDataRenderer(dir));
    }

    LentilDataDisplay::ptr getWidget() { return display; }

    const Vector& getGridDim() const { return gridDim; }

    unsigned numParticles() const { return display->numParticles(); }

    void consume(const ParticleStatusRenderCommand& cmd){
      // Clear the marker on the last focus.
      focus->clearMarker();

      // Set the appropriate shell color on the right particle.
      display->particle(cmd.index)->setShellColor(cmd.color);
      
      // Mark this particle and save its identity.
      focus = display->particle(cmd.index);
      focus->setMarker();
      
      emit updated();
    }

    void consume(const RetrieveData&){
      this->loadDataFrame(++frame);
      emit updated();
    }

    void loadDataFrame(unsigned i, bool init=false);

  private:
    static void loadNrrd(const std::string& filename, std::vector<double>& data);

  private:
    const std::string dir;

    Vector initDim;
    Vector gridDim;

    unsigned frame;

    LentilDataDisplay::ptr display;
    ParticleGlyph::ptr focus;
  };
}

#endif
