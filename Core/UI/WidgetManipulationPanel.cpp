// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// WidgetManipulationPanel.cpp.

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/UI/WidgetManipulationPanel.h>
#include <Core/Util/BoostForeach.h>
using MTV::WidgetManipulationPanel;
using MTV::Widget;

// System headers.
#include <iostream>

void WidgetManipulationPanel::mouseDoubleClickEvent(QMouseEvent *e){
  boost::shared_ptr<QMouseEvent> ee = this->invert(e);
  this->forwardMouseEvent(ee.get());
  updateGL();
}

void WidgetManipulationPanel::mousePressEvent(QMouseEvent *e){
  // If the mouse click was the left button, then set the widget panel
  // up to move the widget under the pointer (if any).

  if(e->button() == Qt::LeftButton){
    // Grab whatever widget is under the cursor.
    boost::shared_ptr<QMouseEvent> ee = this->invert(e);
    grabbed = this->forwardMouseEvent(ee.get());

    // Promote the clicked widget to the top of the stack (so it pops to
    // the front in the drawing phase).
    if(grabbed){
      this->raiseToTop(grabbed);
      updateGL();
    }

    // Store the clicked position to compute movement in the
    // mouseMoveEvent() method.
    oldlocation = Point(ee->x(), ee->y());
  }
}

void WidgetManipulationPanel::mouseMoveEvent(QMouseEvent *e){
  // Directly move the "grabbed" widget, if there is one.  In this
  // case, do not forward any mouse events to the widget.
  //
  // TODO(choudhury): probably all move events SHOULD be forwarded to
  // the grabbed widget.
  if(grabbed){
    boost::shared_ptr<QMouseEvent> ee = this->invert(e);
    Point newlocation(ee->x(), ee->y());
    Vector movement = newlocation - oldlocation;
    grabbed->move(movement);
    oldlocation = newlocation;

    updateGL();
  }
}

void WidgetManipulationPanel::mouseReleaseEvent(QMouseEvent *e){
  // ONLY send unclick events to a widget that was "grabbed" by an
  // earlier mouse click.  Also "ungrab" that widget at this point.
  if(grabbed){
    boost::shared_ptr<QMouseEvent> ee = this->invert(e);
    grabbed->receiveMouse(ee.get());

    // Sets the grabbed ptr to null ("ungrabs").
    grabbed.reset();
  }

  updateGL();
}
