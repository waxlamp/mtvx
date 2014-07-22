// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// WidgetPanel.h - A subclass of QGLWidget, which accepts Widgets for
// display and manipulation.

#ifndef WIDGET_PANEL_H
#define WIDGET_PANEL_H

// System headers.
#include <vector>

// Qt headers.
#include <QtOpenGL>

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Color/ColorProfile.h>
#include <Core/Graphics/MotionBlur.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class Color;

  class WidgetPanel : public QGLWidget {
  public:
    BoostPointers(WidgetPanel);

  public:
    WidgetPanel();

    // Widget membership control.
    void add(Widget::ptr w);
    void remove(Widget::ptr w);

    // Display control.
    void setBackgroundColor(const Color& c){
      clearColor = c;
      blur.SetClearColor(c.r, c.g, c.b);
    }

    void useColorProfile(const ColorProfile& profile);
    void text(int x, int y, const std::string& message, const QFont& font);

    void motionBlur(bool on){
      doMotionBlur = on;
    }

    // Framebuffer grab.
    //
    // NOTE(choudhury): for some reason, QGLWidget::grabFrameBuffer()
    // is not a const method, so neither can this method be.
    bool screencap(const std::string& filename){
      QImage screen = this->grabFrameBuffer();
      return screen.save(QString(filename.c_str()));
    }

    void setDumping(bool _dumping){
      dumping = _dumping;
    }

    bool getDumping() const {
      return dumping;
    }

    unsigned long long getFrame() const {
      return frame;
    }

  protected:
    // Returns the first widget in the widgets list that contains the
    // point p, or else a null pointer if there is no widget there.
    Widget::ptr getWidgetAt(Point p);

    // Inverts a QMouseEvent to place it in line with the lower-left
    // origin of the GL context (instead of the upper left origin of
    // the Qt drawing system).
    boost::shared_ptr<QMouseEvent> invert(QMouseEvent *e) const;

    // Sends a mouse event to the widget under the point p, if there
    // is one.
    Widget::ptr forwardMouseEvent(QMouseEvent *e);

    // Raises the named widget to the top of the drawing stack (so it
    // appears in front of all other widgets).
    void raiseToTop(Widget::ptr w);

  protected:
    // These functions all come from the QWidget interface.

    // Called once to set up the GL context.  Re-implemented from
    // QGLWidget.
    virtual void initializeGL();

    // Sets the orthographic camera projection and viewport from the new
    // widget size.
    virtual void resizeGL(int width, int height);

    // Renders each widget in turn.  Re-implemented from QGLWidget.
    virtual void paintGL();

  private:
    std::vector<Widget::ptr> widgets;
    Color clearColor, textColor;

    bool doMotionBlur;
    MotionBlur blur;

    bool dumping;

    unsigned long long frame;

  public:
    // TODO(choudhury): these should go in a global-singleton
    // FontProfile class.
    static const QFont serif, sans, computer;
  };
}

#endif
