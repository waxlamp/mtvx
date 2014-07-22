// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// SolidRectangle.h - A widget representing a solid rectangle of some
// color.

#ifndef SOLID_RECTANGLE_H
#define SOLID_RECTANGLE_H

// MTV includes.
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class SolidRectangle : public Widget {
  public:
    BoostPointers(SolidRectangle);

  public:
    SolidRectangle(const Point& upperleft, const Point& lowerright, const Color& color, bool outline)
      : Widget(upperleft),
        diagonal(lowerright - upperleft),
        color(color),
        outline(outline)
    {}

    SolidRectangle(const Point& upperleft, const Vector& diagonal, const Color& color, bool outline)
      : Widget(upperleft),
        diagonal(diagonal),
        color(color),
        outline(outline)
    {}

    bool contains(const Point& p){
      const Vector into = p - location;
      return ( (0.0f <= into.x and into.x <= diagonal.x) and
               (0.0f <= into.y and into.y <= diagonal.y) );
    }

    void draw() const {
      const Point& a = location;
      const Point b = location + diagonal;

      color.glSet();
      glBegin(GL_QUADS);
      {
        glVertex2f(a.x, a.y);
        glVertex2f(a.x, b.y);
        glVertex2f(b.x, b.y);
        glVertex2f(b.x, a.y);
      }
      glEnd();

      if(outline){
        Color::black.glSet();
        glBegin(GL_LINE_LOOP);
        {
          glVertex2f(a.x, a.y);
          glVertex2f(a.x, b.y);
          glVertex2f(b.x, b.y);
          glVertex2f(b.x, a.y);
        }
        glEnd();
      }
    }

    void setColor(const Color& c){
      std::cout << "SolidRectangle::setColor!" << std::endl;
      color = c;
    }

    const Color& getColor() const {
      return color;
    }

    float width() const {
      return diagonal.x;
    }

    float height() const {
      return diagonal.y;
    }

  private:
    Vector diagonal;
    Color color;
    bool outline;
  };
}

#endif
