// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// region-display-test.cpp - A simple test for RegionDisplay.

// MTV headers.
#include <Core/UI/WidgetPanel.h>
#include <Modules/ReferenceTrace/Graphics/RegionDisplay.h>

// Qt headers.
#include <QtGui>

using MTV::Color;
using MTV::Point;
using MTV::RegionDisplay;
using MTV::WidgetPanel;

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  RegionDisplay::ptr region(new RegionDisplay(Point(10, 10),
                                              0x0, 32, 1,
                                              Color(1,0,0),
                                              100, "Region Display"));

  for(int i=0; i<16; i++){
    const float redlevel = static_cast<float>(i) / 16;
    const float bluelevel = 1.0 - redlevel;

    region->setDataColor(2*i, Color(redlevel, 0.0, bluelevel));

    region->setCacheResultColor(2*i, Color(0.7, redlevel, 0.3));
    // region->setCacheResultColor(2*i, Color(redlevel, 0.0, bluelevel));
  }

  WidgetPanel::ptr panel(new MTV::WidgetPanel);
  panel->add(region);
  panel->show();

  app.exec();
}
