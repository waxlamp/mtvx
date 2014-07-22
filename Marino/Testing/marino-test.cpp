// Copyright 2010 A.N.M. Imroz Choudhury
//
// marino-test.cpp - Tests Marino functionality by instantiating a
// region renderer, making some changes to it, recording state
// objects, then applying those objects to a blank region renderer
// object.

// MTV includes.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/BoostPointers.h>
#include <Marino/Memento.pb.h>
#include <Modules/ReferenceTrace/Dataflow/CacheStatusRenderCommand.h>
#include <Modules/ReferenceTrace/Dataflow/RecordRenderCommand.h>
#include <Modules/ReferenceTrace/Renderers/RegionRenderer.h>
using MTV::CacheStatusRenderCommand;
using MTV::Color;
using MTV::Consumer;
using MTV::Marino::DeltaMemento;
using MTV::Marino::WarmMemento;
using MTV::Point;
using MTV::RecordRenderCommand;
using MTV::RegionRenderer;
using MTV::RegionRenderer$ComputeDelta;
using MTV::WidgetPanel;

// System includes.
#include <vector>

template<typename T>
class Saver : public Consumer<T> {
public:
  BoostPointers(Saver<T>);

public:
  void consume(const T& t){
    saved.push_back(t);
  }

  const std::vector<T>& getVector() const { return saved; }

protected:
  std::vector<T> saved;
};

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  // Create a small region renderer.
  RegionRenderer$ComputeDelta::ptr rr(new RegionRenderer$ComputeDelta(Point(10, 10),
                                                                      0, 16, 1,
                                                                      Color::magenta,
                                                                      "Test region"));

  Saver<DeltaMemento>::ptr saver(new Saver<DeltaMemento>);
  rr->addConsumer(saver);

  // Make some changes to it.
  rr->consume(RecordRenderCommand(MTR::Record::Read, 0));
  rr->consume(CacheStatusRenderCommand(0, Color::red));

  rr->consume(RecordRenderCommand(MTR::Record::Write, 3));
  rr->consume(CacheStatusRenderCommand(3, Color::magenta));

  rr->consume(RecordRenderCommand(MTR::Record::Read, 14));
  rr->consume(CacheStatusRenderCommand(14, Color::blue));

  // Save a warm snapshot.
  WarmMemento snap;
  rr->saveWarm(snap);

  // Create a fresh region renderer of the same kind as the testing
  // one.
  RegionRenderer::ptr fresh(new RegionRenderer(Point(10, 10),
                                               0, 16, 1,
                                               Color::magenta,
                                               "Fresh renderer"));

  // Apply the snapshot to the new renderer.
  fresh->loadWarm(snap);

  // Check to see whether the colors are the same.
  bool good = true;
  for(unsigned i=0; i<rr->getWidget()->numStripes(); i++){
    const Color& rr_data_color = rr->getWidget()->getDataColor(i);
    const Color& fresh_data_color = fresh->getWidget()->getDataColor(i);

    if(rr_data_color != fresh_data_color){
      std::cout << "[warm] data cell " << i << " does not match: " << rr_data_color << " -> " << fresh_data_color << std::endl;
      good = false;
    }

    const Color& rr_cache_color = rr->getWidget()->getCacheResultColor(i);
    const Color& fresh_cache_color = fresh->getWidget()->getCacheResultColor(i);

    if(rr_cache_color != fresh_cache_color){
      std::cout << "[warm] cache cell " << i << " does not match: " << rr_cache_color << " -> " << fresh_cache_color << std::endl;
      good = false;
    }
  }

  // Creater another fresh object, and apply the deltas to it.
  RegionRenderer::ptr fresher(new RegionRenderer(Point(10, 10),
                                                 0, 16, 1,
                                                 Color::magenta,
                                                 "Fresher renderer"));

  for(unsigned i=0; i<saver->getVector().size(); i++){
    fresher->applyDelta(saver->getVector()[i]);
  }

  for(unsigned i=0; i<rr->getWidget()->numStripes(); i++){
    const Color& rr_data_color = rr->getWidget()->getDataColor(i);
    const Color& fresh_data_color = fresher->getWidget()->getDataColor(i);

    if(rr_data_color != fresh_data_color){
      std::cout << "[diff] data cell " << i << " does not match: " << rr_data_color << " -> " << fresh_data_color << std::endl;
      good = false;
    }

    const Color& rr_cache_color = rr->getWidget()->getCacheResultColor(i);
    const Color& fresh_cache_color = fresher->getWidget()->getCacheResultColor(i);

    if(rr_cache_color != fresh_cache_color){
      std::cout << "[diff] cache cell " << i << " does not match: " << rr_cache_color << " -> " << fresh_cache_color << std::endl;
      good = false;
    }
  }

  return good ? 0 : 1;
}
