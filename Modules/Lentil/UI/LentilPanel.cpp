// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilPanel.cpp

// MTV includes.
#include <Core/Color/ColorProfile.h>
#include <Modules/Lentil/UI/LentilPanel.h>
using MTV::LentilPanel;
using MTV::PrintColorProfile;

// System includes.
#include <iostream>

LentilPanel::LentilPanel(const std::string& dir)
  : net(new LentilNetwork(dir))
{
  this->useColorProfile(PrintColorProfile());

  // Add the network's top-level widget to the panel.
  LentilDataDisplay::ptr w = net->getWidget();
  this->WidgetPanel::add(w);

  // Repaint whenever the widget updates.
  QObject::connect(net->getRenderer().get(), SIGNAL(updated()), this, SLOT(updateGL()));
}

void LentilPanel::resizeGL(int width, int height){
  const Vector& gridDim = net->getRenderer()->getGridDim();

  // Orthographic projection - make window big enough to hold the grid
  // itself.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  // gluOrtho2D(0, gridDim.x, 0, gridDim.y);
  gluOrtho2D(3.3, gridDim.x, 3.3, gridDim.y);

  // Identity matrix for modelview.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set viewport equal to window size.
  glViewport(0, 0, width, height);
}
