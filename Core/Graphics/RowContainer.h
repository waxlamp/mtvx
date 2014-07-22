// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// RowContainer.h - A layout widget that keeps track of several
// widgets and simply displays them side-by-side.

#ifndef ROW_CONTAINER_H
#define ROW_CONTAINER_H

// MTV headers.
#include <Core/Graphics/Widget.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <vector>

namespace MTV{
  class RowContainer : public Widget {
  public:
    BoostPointers(RowContainer);

  public:
    RowContainer(const Point& location, float spacing, const std::vector<Widget::ptr>& w = std::vector<Widget::ptr>());

    bool contains(const Point& p);

    void draw() const {
      this->drawChildren();
    }

    float width() const { return insertion_offset; }
    float height() const { return max_widget_height; }

    void addWidget(Widget::ptr w);
    void addWidget(Widget::ptr w, float custom_spacing);

    void verticallyCenter();

  private:
    float spacing;
    float insertion_offset;
    float max_widget_height;
  };
}

#endif
