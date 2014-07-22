// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// TextWidget.h - A widget for displaying text.

#ifndef TEXT_WIDGET_H
#define TEXT_WIDGET_H

// MTV includes.
#include <Core/Geometry/Point.h>
#include <Core/Geometry/Vector.h>
#include <Core/Graphics/Widget.h>
#include <Core/UI/WidgetManipulationPanel.h>
#include <Core/Util/BoostPointers.h>

// Qt includes.
#include <QtGui>

// Boost includes.
#include <boost/shared_ptr.hpp>

// System includes.
#include <string>

namespace MTV{
  class TextWidget : public Widget {
  public:
    BoostPointers(TextWidget);

  public:
    TextWidget(const Point& location, const std::string& t = "", const QFont& f = QFont());

    float width() const { return bbox.x; }
    float height() const { return bbox.y; }

    void setFont(const QFont& f);
    void setText(const std::string& t);

    bool contains(const Point& p);
    void draw() const;

  private:
    void recompute_bounding_box();

  private:
    std::string text;
    boost::shared_ptr<QFont> font;
    Vector bbox;
  };
}

#endif
