// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilDataRenderer.h

// MTV includes.
#include <Modules/Lentil/Renderers/LentilDataRenderer.h>
using MTV::LentilDataRenderer;
using MTV::Vector;

// Nrrd includes.
#include <teem/nrrd.h>

// System includes.
#include <fstream>

LentilDataRenderer::LentilDataRenderer(const std::string& dir)
  : dir(dir),
    frame(0)
    // , display(new LentilDataDisplay(Point::zero))
{
  // Open the cheating file that gives the grid dimensions and the
  // PPC.  Compute the intial volume from it.
  std::ifstream in((dir + "/cheat.txt").c_str());
  if(!in){
    return;
  }

  // Read in the grid extent.
  in >> gridDim.x >> gridDim.y;

  std::cout << "LentilDataRenderer: " << gridDim.x << ", " << gridDim.y << std::endl;

  // Read in the nodal dimensions.
  int gx, gy;
  in >> gx >> gy;

  // Read in the PPC dimensions.
  int ppcx, ppcy;
  in >> ppcx >> ppcy;
  in.close();

  // The initial particle volume is equal to the product of a single
  // particle's dimensions.
  initDim = Vector(gridDim.x / ((gx-1)*ppcx), gridDim.y / ((gy-1)*ppcy));

  // Initialize the data display widget.
  display = LentilDataDisplay::ptr(new LentilDataDisplay(Point::zero, gx, gy, gridDim.x, gridDim.y));

  // Load the initial data frame.
  this->loadDataFrame(0, true);

  // Let the focus arbitrarily be the first particle.
  focus = display->particle(0);
}

void LentilDataRenderer::loadDataFrame(unsigned i, bool init){
  std::vector<double> data;

  // Load the position data.
  {
    std::stringstream ss;
    ss << dir << "/mp-position-" << i << ".nrrd";
    loadNrrd(ss.str(), data);

    // Resize the glyph vector.
    // display->resizeParticles(data.size() / 2, Point::zero, Color::blue, Color::red, initDim.x, initDim.y);
    display->resizeParticles(data.size() / 2, Point::zero, Color(220.0/255., 213./255., 138./255.), Color::red, initDim.x, initDim.y);

    // Update the glyph positions.
    for(unsigned i=0; i<display->numParticles(); i++){
      display->particle(i)->setLocation(Point(data[2*i], data[2*i+1]));
    }
  }
  
  // Load the deformation gradient data.
  {
    std::stringstream ss;
    ss << dir << "/mp-deformation-gradient-" << i << ".nrrd";
    loadNrrd(ss.str(), data);

    // TODO(choudhury): Update the glyph deformations.
  }

  // Load the stress.
  {
    std::stringstream ss;
    ss << dir << "/mp-stress-" << i << ".nrrd";
    loadNrrd(ss.str(), data);

    // TODO(choudhury): Update the glyph colormap.
  }

  emit updated();
}

void LentilDataRenderer::loadNrrd(const std::string& filename, std::vector<double>& data){
  // Create a Nrrd.
  Nrrd *nin = nrrdNew();

  // Create an IO state object.
  NrrdIoState *nio = nrrdIoStateNew();
  nrrdIoStateSet(nio, nrrdIoStateSkipData, AIR_TRUE);

  // Read in the header.
  if(nrrdLoad(nin, filename.c_str(), nio)){
    char *err = biffGetDone(NRRD);
    std::cerr << "error: trouble reading '" << filename << "' header: " << err << std::endl;
    abort();
  }

  // Delete the IO state object.
  nio = nrrdIoStateNix(nio);

  // Compute the total size of this data.
  const unsigned dims = nin->dim;
  unsigned size = 1;
  for(unsigned i=0; i<dims; i++){
    size *= nin->axis[i].size;
  }

  // Resize the vector, and set it to receive data from the actual
  // nrrd load operation.
  data.resize(size);
  nin->data = &data[0];

  // Load the nrrd for real.
  if(nrrdLoad(nin, filename.c_str(), NULL)){
    char *err = biffGetDone(NRRD);
    std::cerr << "error: trouble reading '" << filename << "' data: " << err << std::endl;
    abort();
  }

  // Nix the nrrd struct (this will destroy the nrrd meta info, but
  // leave the data pointer alone).
  nrrdNix(nin);
}
