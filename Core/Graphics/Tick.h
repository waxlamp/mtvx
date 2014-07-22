// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// Tick.h - Tick widgets (with optional label) for indexing locations in other widgets.

#ifndef TICK_H
#define TICK_H

// MTV includes.
#include <Core/Graphics/TextWidget.h>
#include <Core/Graphics/SolidRectangle.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

namespace MTV{
  class Tick : public Widget {
  public:
    BoostPointers(Tick);

  public:
    Tick(Point location, const std::string& labelText = "", float extra=0.0);

    float tickWidth() const { return 3.0; }
    float tickHeight() const { return 10.0 + extra; }

    int labelWidth() const { return label? label->width() : 0; }
    int labelHeight() const { return label? label->height() : 0; }

    // Widget interface methods.
    bool contains(const Point& p){
      return childrenContain(p);
    }

    void draw() const {
      this->drawChildren();
    }

  private:
    SolidRectangle::ptr tick;
    TextWidget::ptr label;
    float extra;
  };
}

#endif
