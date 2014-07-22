// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Point.h - Geometric point.  Distinct from Vector in the operations
// allowed on it.

#ifndef POINT_H
#define POINT_H

// MTV headers.
#include <Core/Geometry/Vector.h>

// System headers.
#include <ostream>

namespace MTV{
  class Point {
  public:
    Point()
      : x(0.0f), y(0.0f)
    {}

    Point(float x, float y)
      : x(x), y(y)
    {}

    // Returns the vector pointing from p to this point.
    Vector operator-(const Point& p) const {
      return Vector(x - p.x, y - p.y);
    }

    // Returns the point lying at offset v from this point.
    Point operator+(const Vector& v) const {
      return Point(x + v.x, y + v.y);
    }

    Point operator-(const Vector& v) const {
      return *this + (-v);
    }

    friend std::ostream& operator<<(std::ostream& out, const Point& p){
      out << "Point(" << p.x << ", " << p.y << ")";
      return out;
    }

  public:
    float x, y;

  public:
    static const Point zero;
  };
}

#endif
