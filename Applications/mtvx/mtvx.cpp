// Copyright 2010 A.N.M. Imroz Choudhury
//
// mtvx.cpp - Application driver for MTV.

// MTV includes.
#include <Applications/mtvx/MTVMainWindow.h>

// Qt includes.
#include <QtGui>

int main(int argc, char *argv[]){
  // Create a controller application instance.
  QApplication app(argc, argv);

  // Instantiate a main window.
  MTV::MTVMainWindow mtv;
  mtv.setWindowTitle("MTV X");
  mtv.show();

  // Enter the Qt loop.
  app.exec();
}

