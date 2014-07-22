// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// GLPoint.h - A widget consisting just of a GL point and its color.

#ifndef GL_POINT_H
#define GL_POINT_H

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Geometry/Point.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class GLPoint : public Widget {
  public:
    BoostPointers(GLPoint);

  public:
    GLPoint(const Point& p, const Color& c = Color::white, int size = 1)
      : Widget(p),
        color(c),
        size(size)
    {}

    GLPoint(const Color& c = Color::white, int size = 1)
      : Widget(Point::zero),
        color(c),
        size(size)
    {}

    GLPoint(const Point& p, int size)
      : Widget(p),
        color(Color::white),
        size(size)
    {}

    GLPoint(int size)
      : Widget(Point::zero),
        color(Color::white),
        size(size)
    {}

    const Color& getColor() const { return color; }

    void setColor(const Color& c){
      color = c;
    }

    // TODO(choudhury): fix this.
    bool contains(const Point& p) { return false; }

    void draw() const {
      color.glSet();
      glPointSize(size);
      glBegin(GL_POINTS);
      {
        glVertex2f(location.x, location.y);
      }
      glEnd();
    };

  private:
    Color color;
    int size;
  };
}

#endif
