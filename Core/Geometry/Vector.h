// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Vector.h - Geometric vector.  Distinct from Point in the operations
// allowed on it.

#ifndef VECTOR_H
#define VECTOR_H

// System includes.
#include <ostream>

namespace MTV{
  class Vector {
  public:
    Vector()
      : x(0.0f), y(0.0f)
    {}

    Vector(float x, float y)
      : x(x), y(y)
    {}

    // Negation.
    Vector operator-() const {
      return Vector(-x, -y);
    }

    // Addition/subtraction.
    Vector operator+(const Vector& v) const {
      return Vector(x - v.x, y - v.y);
    }

    Vector operator-(const Vector& v) const {
      return *this + (-v);
    }

    // Scalar multiplication.
    Vector operator*(const float t) const {
      return Vector(t*x, t*y);
    }

    friend Vector operator*(const float t, const Vector& v){
      return v*t;
    }

    friend std::ostream& operator<<(std::ostream& out, const Vector& p){
      out << "Vector(" << p.x << ", " << p.y << ")";
      return out;
    }

  public:
    // NOTE(choudhury): members are public to avoid the need for set/get
    // methods.  This is basically a value type, so I think it's ok
    // here.
    float x, y;

  public:
    static const Vector zero;
  };
}

#endif
