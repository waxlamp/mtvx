// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// widget-panel-test.cpp - A simple test program to make sure the
// widget panel works as expected.

// Qt headers.
#include <QtGui>

// MTV headers.
#include <Core/UI/WidgetManipulationPanel.h>
#include <Core/UI/WidgetPanel.h>

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  MTV::WidgetPanel::ptr panel;
  if(argc > 1 and std::string(argv[1]) == "-m"){
    panel = MTV::WidgetPanel::ptr(new MTV::WidgetManipulationPanel);
  }
  else{
    panel = MTV::WidgetPanel::ptr(new MTV::WidgetPanel);
  }

  panel->add(MTV::Widget::ptr(new MTV::Testing::FilledRectangle(MTV::Point(200, 200), MTV::Point(400, 300))));
  panel->add(MTV::Widget::ptr(new MTV::Testing::FilledRectangle(MTV::Point(400, 400), MTV::Vector(400, 100))));
  panel->show();

  return app.exec();
}
