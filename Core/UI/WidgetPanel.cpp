// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// WidgetPanel.cpp.

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/BoostForeach.h>
using MTV::WidgetPanel;
using MTV::Widget;

// System headers.
#include <iostream>

#ifdef __APPLE__
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

WidgetPanel::WidgetPanel()
  : QGLWidget(QGLFormat(QGL::DoubleBuffer | QGL::AlphaChannel)),
    textColor(Color::white),
    doMotionBlur(false),
    dumping(false),
    frame(0)
{
  blur.SetDecay(0.2);
  this->setBackgroundColor(Color::black);
}

void WidgetPanel::add(Widget::ptr w){
  // Add the widget to the list of widgets.
  widgets.push_back(w);

  // Register the widget as knowing about the panel it now belongs to.
  // This will cascade recursively down the descendant widgets of w as
  // well.  This will allow TextWidget instances to draw their text in
  // the proper widget panel.
  w->setPanel(this);
}

void WidgetPanel::remove(Widget::ptr w){
  for(std::vector<Widget::ptr>::iterator i = widgets.begin();
      i != widgets.end();
      i++){
    // Erase ALL instances of w occurring in the widgets list.
    if(*i == w){
      // Return value from vector::erase is iterator referring to NEXT
      // entry after the one that was erased.
      i = widgets.erase(i);
    }
  }
}

void WidgetPanel::useColorProfile(const ColorProfile& profile){
  clearColor = profile.background();
  textColor = profile.text();
}

void WidgetPanel::text(int x, int y, const std::string& message, const QFont& font){
  textColor.glSet();
  this->renderText(x, y, QString(message.c_str()), font);
}

Widget::ptr WidgetPanel::getWidgetAt(Point p){
  Widget::ptr selected;

  // NOTE(choudhury): Walk the list in REVERSE order so that widgets
  // higher in the stack (i.e., those that are more "uncovered") are
  // found first.
  foreach_reverse(Widget::ptr w, widgets){
    if(w->contains(p)){
      selected = w;
      break;
    }
  }

  // NOTE(choudhury): If no widget is found, then the "uninitialized"
  // pointer w is returned, but this is, by design, a null pointer.
  return selected;
}

boost::shared_ptr<QMouseEvent> WidgetPanel::invert(QMouseEvent *e) const {
  // Invert the y-coordinate of the mouse event to put it in line with
  // the lower-left origin in the drawing plane.
  QPoint inverted_pos(e->x(), this->height() - e->y());

  // TODO(choudhury): I don't know if this is the right way to invert
  // the global position.
  QPoint inverted_globalpos(e->globalX(), this->height() - e->globalY());

  // NOTE(choudhury): use a boost shared_ptr so it is deleted
  // automatically at the right time.
  return boost::shared_ptr<QMouseEvent>(new QMouseEvent(e->type(),
                                                        inverted_pos, inverted_globalpos,
                                                        e->button(), e->buttons(),
                                                        e->modifiers()));
}

Widget::ptr WidgetPanel::forwardMouseEvent(QMouseEvent *e){
  Widget::ptr clicked = this->getWidgetAt(Point(e->x(), e->y()));
  if(clicked){
    clicked->receiveMouse(e);
  }

  return clicked;
}

void WidgetPanel::raiseToTop(Widget::ptr w){
  // Find the widget in the widgets list, erasing it when it's found.
  size_t numWidgets = widgets.size();
  for(std::vector<Widget::ptr>::iterator i = widgets.begin(); i != widgets.end(); i++){
    if(*i == w){
      widgets.erase(i);
      break;
    }
  }

  // Post-condition: if the named widget is present in the widgets
  // list, then it must have been deleted in the previous for-loop.
  if(numWidgets != widgets.size() + 1){
    throw std::domain_error("WidgetPanel::raiseToTop(): argument widget not present in panel.");
  }

  // Add the widget back to the list, at the end (the top-most
  // position).
  widgets.push_back(w);
}

void WidgetPanel::initializeGL(){
  // Set up alpha blending in this widget.
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void WidgetPanel::resizeGL(int width, int height){
  // Orthographic projection.
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(0, width, 0, height);

  // Identity matrix for modelview.
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Set viewport equal to orthographic projection, to enable
  // pixelwise screen drawing.
  glViewport(0, 0, width, height);

  // Set the motion blur object to have these dimensions.
  blur.SetSize(width, height);
}

void WidgetPanel::paintGL(){
  // Clear the screen to the background color.
  //
  // NOTE(choudhury): this function is called here instead of in
  // setBackgroundColor() because here the GL context is guaranteed to
  // be current; to call it in the set method would require a call to
  // makeCurrent(), which itself might cause problems if, for
  // instance, that method is called while another GL context is
  // current (and the shift would violate some guarantee elsewhere).
  glClearColor(clearColor.r, clearColor.b, clearColor.g, clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT);

  if(doMotionBlur){
    blur.Draw();
  }

  // Draw the widgets.  Later widgets in the widgets list are drawn on
  // top of earlier widgets.
  foreach(Widget::ptr w, widgets){
    w->draw();
  }

  if(doMotionBlur){
    blur.Capture();
  }

  // NOTE(choudhury): Qt will call the swap buffer method
  // automatically; no need to do it here.

  // // If requested, dump the frame to disk.
  // if(dumping){
  //   std::stringstream filename;
  //   filename << "frame" << frame << ".png";

  //   this->screencap(filename.str());
  // }

  // Increment the frame counter.
  ++frame;
}

// // TODO(choudhury): instead of calling the updateGL() slot in the
// // methods below, allow the widgets to emit an "updated" signal, and
// // listen for that signal in this class (connected to the updateGL
// // slot).

// void WidgetPanel::mouseDoubleClickEvent(QMouseEvent *e){
//   // std::cout << "Double click at (" << e->x() << ", " << e->y() << ")" << std::endl;

//   boost::shared_ptr<QMouseEvent> ee = this->invert(e);
//   this->forwardMouseEvent(ee.get());
//   updateGL();
// }

// void WidgetPanel::mousePressEvent(QMouseEvent *e){
//   // std::cout << "Mouse press at (" << e->x() << ", " << e->y() << ")" << std::endl;

//   // If the mouse click was the left button, then set the widget panel
//   // up to move the widget under the pointer (if any).

//   if(e->button() == Qt::LeftButton){
//     // Grab whatever widget is under the cursor.
//     boost::shared_ptr<QMouseEvent> ee = this->invert(e);
//     grabbed = this->forwardMouseEvent(ee.get());

//     // Promote the clicked widget to the top of the stack (so it pops to
//     // the front in the drawing phase).
//     if(grabbed){
//       this->raiseToTop(grabbed);
//       updateGL();
//     }

//     // Store the clicked position to compute movement in the
//     // mouseMoveEvent() method.
//     oldlocation = Point(ee->x(), ee->y());
//   }
// }

// void WidgetPanel::mouseMoveEvent(QMouseEvent *e){
//   // std::cout << "Mouse move at (" << e->x() << ", " << e->y() << ")" << std::endl;

//   // Directly move the "grabbed" widget, if there is one.  In this
//   // case, do not forward any mouse events to the widget.
//   //
//   // TODO(choudhury): probably all move events SHOULD be forwarded to
//   // the grabbed widget.
//   if(grabbed){
//     boost::shared_ptr<QMouseEvent> ee = this->invert(e);
//     Point newlocation(ee->x(), ee->y());
//     Vector movement = newlocation - oldlocation;
//     grabbed->move(movement);
//     oldlocation = newlocation;

//     updateGL();
//   }
// }

// void WidgetPanel::mouseReleaseEvent(QMouseEvent *e){
//   // std::cout << "Mouse release at (" << e->x() << ", " << e->y() << ")" << std::endl;

//   // ONLY send unclick events to a widget that was "grabbed" by an
//   // earlier mouse click.  Also "ungrab" that widget at this point.
//   if(grabbed){
//     boost::shared_ptr<QMouseEvent> ee = this->invert(e);
//     grabbed->receiveMouse(ee.get());

//     // Sets the grabbed ptr to null ("ungrabs").
//     grabbed.reset();
//   }

//   updateGL();
// }

const QFont WidgetPanel::serif("Times", 14, QFont::Bold);
const QFont WidgetPanel::sans("Helvetica", 14, QFont::Bold);
const QFont WidgetPanel::computer("Courier", 14, QFont::Bold);
