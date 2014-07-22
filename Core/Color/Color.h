// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Color.h - RGB colors.

#ifndef COLOR_H
#define COLOR_H

// MTV headers.
//
// TODO(choudhury): this should appear in Core/ along with the Color
// class.
#include <Marino/Color.pb.h>

// System headers.
#include <ostream>
#include <sstream>
#include <stdexcept>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace MTV{
  class Color {
  public:
    // The constructor makes an uninitialized color.
    Color() {}

    Color(float r, float g, float b, float a=1.0)
      : r(r), g(g), b(b), a(a)
    {
#ifndef NDEBUG
      this->checkValues();
#endif
    }

    // TODO(choudhury): the color memento should have an alpha value.
    Color(const Marino::ColorCold& c)
      : r(c.r()), g(c.g()), b(c.b()), a(c.a())
    {
#ifndef NDEBUG
      this->checkValues();
#endif
    }

    // float red() const { return r; }
    // float green() const { return g; }
    // float blue() const { return b; }
    // float alpha() const { return a; }

    void glSet() const {
      glColor4f(r, g, b, a);
    }

    // Modulate a color by a numeric value.
    template<typename T>
    Color operator*(T t) const {
      return Color(t*r, t*g, t*b, a);
    }

    template<typename T>
    friend Color operator*(T t, const Color& color){
      return color*t;
    }

    Color operator+(const Color& c) const {
      Color sum(r + c.r, g + c.g, b + c.b);

      // Clamp the values to 1.0.
      if(sum.r > 1.0f)
        sum.r = 1.0f;

      if(sum.g > 1.0f)
        sum.g = 1.0f;

      if(sum.b > 1.0f)
        sum.b = 1.0f;

      if(sum.a > 1.0f)
        sum.a = 1.0f;

      return sum;
    }

    friend std::ostream& operator<<(std::ostream& out, const Color& c){
      out << "Color(" << c.r << ", " << c.g << ", " << c.b << ")";
      return out;
    }

    bool operator==(const Color& c) const {
      return (r == c.r and
              g == c.g and
              b == c.b);
    }

    bool operator!=(const Color& c) const {
      return !(*this == c);
    }

  public:
    float r, g, b, a;

  private:
    void checkValues(){
      if( (r < 0.0f or r > 1.0f) or
          (g < 0.0f or g > 1.0f) or
          (b < 0.0f or b > 1.0f) or
          (a < 0.0f or a > 1.0f) ){
        std::stringstream ss;
        ss << "Color(" << r << ", " << g << ", " << b << ") has entries out of range." << std::endl;

        *((char *)0) = 'a';

        throw std::domain_error(ss.str().c_str());
      }
    }

  public:
    static const Color black;
    static const Color white;
    static const Color red;
    static const Color green;
    static const Color blue;
    static const Color magenta;
  };
}
#endif
