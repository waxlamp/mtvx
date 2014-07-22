// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// cacheset-display-test.cpp - A simple test for CacheSetDisplay.

// MTV headers.
#include <Core/UI/WidgetManipulationPanel.h>
#include <Core/Util/BoostForeach.h>
#include <Modules/ReferenceTrace/Graphics/CacheSetDisplay.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/CacheSimulator/CacheSet.h>
using Daly::Cache;
using Daly::CacheLevel;
using Daly::CacheSet;
using MTV::Color;
using MTV::Point;
using MTV::CacheSetDisplay;
using MTV::WidgetManipulationPanel;

// Qt headers.
#include <QtGui>

// System headers.
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  if(argc < 2){
    std::cerr << "usage: cacheset-display-test <cacheset-spec-file>" << std::endl;
    exit(1);
  }

  const std::string& bsfile = "";
  const unsigned numstreams = 0;

  std::string error;
  CacheSet::ptr cache(CacheSet::newFromSpec(argv[1], bsfile, numstreams, error));
  if(!cache){
    std::cerr << error << std::endl;
    exit(1);
  }

  std::vector<std::vector<CacheLevel::ptr> > levels = cache->getUnifiedCacheStructure();
  foreach(std::vector<CacheLevel::ptr> level, levels){
    foreach(CacheLevel::ptr p, level){
      std::cout << p << " ";
    }
    std::cout << std::endl;
  }

  std::vector<Cache::ptr> caches = cache->getCaches();
  std::cout << caches.size() << " caches present." << std::endl;

  CacheSetDisplay::ptr display(new CacheSetDisplay(Point(10, 10), cache->getUnifiedCacheStructure()));

  display->setShellColor(levels[0][0], Color::red);
  display->setBlockColor(levels[1][0], 0, Color::blue);

  WidgetManipulationPanel::ptr panel(new MTV::WidgetManipulationPanel);
  panel->add(display);
  panel->show();

  app.exec();
}
