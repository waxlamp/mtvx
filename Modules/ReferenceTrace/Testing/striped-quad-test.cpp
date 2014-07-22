// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// striped-quad-test.cpp - Tests the StripedQuad widget.

// Qt headers.
#include <QtGui>

// MTV headers.
#include <Core/Graphics/StripedQuad.h>
#include <Core/UI/WidgetPanel.h>

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  MTV::WidgetPanel::ptr panel(new MTV::WidgetPanel());
  panel->add(MTV::Widget::ptr(new MTV::StripedQuad(MTV::Point(10, 10),
                                                   16, 20, 100, 3,
                                                   MTV::Color(0.6, 0.6, 0.6))));
  panel->show();

  return app.exec();
}
