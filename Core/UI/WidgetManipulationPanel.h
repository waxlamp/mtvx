// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// WidgetManipulationPanel.h - A subclass of WidgetPanel that uses
// mouse control to manipulate the widgets.

#ifndef WIDGET_MANIPULATION_PANEL_H
#define WIDGET_MANIPULATION_PANEL_H

// System headers.
#include <vector>

// MTV headers.
#include <Core/Graphics/Widget.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class WidgetManipulationPanel : public WidgetPanel {
  public:
    BoostPointers(WidgetManipulationPanel);

  protected:
    // Handles mouse events of different types.
    //
    // These functions all come from the QWidget interface.
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);

  private:
    Widget::ptr grabbed;
    Point oldlocation;
  };
}

#endif
