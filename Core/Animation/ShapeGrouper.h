// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ShapeGrouper.h - A class for marshalling widgets onto a
// parametrized shape.

#ifndef SHAPE_GROUPER_H
#define SHAPE_GROUPER_H

// MTV headers.
#include <Core/Animation/Grouper.h>
#include <Core/Util/Boost.h>
#include <Core/Geometry/Parametrized.h>
#include <Core/Graphics/Widget.h>
#include <Core/Util/Timing.h>

// System headers.
#include <list>

namespace MTV{
  class ShapeGrouper : public Grouper {
  public:
    BoostPointers(ShapeGrouper);

  public:
    ShapeGrouper(Parametrized::ptr shape, bool animating, float duration, bool polar=false, const Point& center = Point(0.0,0.0), bool print=false);

    void resetShape(Parametrized::ptr _shape){
      shape = _shape;
    }

    // void addWidget(Widget::ptr w, float time = MTV::now(), bool marshal = true);
    // Widget::ptr removeWidget(Widget::ptr w, float time = MTV::now(), bool marshal = true);
    // Widget::ptr removeLastWidget(float time = MTV::now(), bool marshal = true);
    // bool shiftWidget(Widget::ptr w, float time = MTV::now(), bool marshal = true);
    // bool replaceWidget(Widget::ptr out, Widget::ptr in, float time = MTV::now(), bool marshal = true);

    virtual void addWidget(Widget::ptr w, float time, bool marshal = true);
    virtual void addWidget(Widget::ptr w, unsigned i, float time, bool marshal = true);
    virtual Widget::ptr removeWidget(Widget::ptr w, float time, bool marshal = true);
    virtual Widget::ptr removeLastWidget(float time, bool marshal = true);
    bool shiftWidget(Widget::ptr w, float time, bool marshal = true);
    bool replaceWidget(Widget::ptr out, Widget::ptr in, float time, bool marshal = true);

    bool hasWidget(Widget::const_ptr w) const;

    unsigned size() const {
      return widgets.size();
    }

    // void marshal(float time = MTV::now());
    virtual void marshal(float time);

  protected:
    float duration;

    typedef std::list<Widget::ptr> ContainerType;
    ContainerType widgets;

    Parametrized::ptr shape;

    Point center;

    bool polar, print, animating;
  };

  class DriftoutShapeGrouper : public ShapeGrouper {
  public:
    BoostPointers(DriftoutShapeGrouper);

  public:
    DriftoutShapeGrouper(Parametrized::ptr shape, bool animating, float duration, float maxdrift, bool polar=false, const Point& center = Point(0.0,0.0), bool print=false);

    // These versions of the functions record the time that a widget
    // entered the grouper - this is used to add an offset to its
    // position during marshaling operations.
    void addWidget(Widget::ptr w, float time, bool marshal = true);
    void addWidget(Widget::ptr w, unsigned i, float time, bool marshal = true);
    Widget::ptr removeWidget(Widget::ptr w, float time, bool marshal = true);
    Widget::ptr removeLastWidget(float time, bool marshal = true);

    void marshal(float time);

  private:
    float maxdrift;
    boost::unordered_map<Widget::ptr, float> entrytime;
  };
}

#endif
