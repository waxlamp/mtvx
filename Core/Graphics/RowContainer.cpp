// Copyright 2010 A.N.M. Imroz Choudhury
//
// RowContainer.cpp

// MTV headers.
#include <Core/Graphics/RowContainer.h>
#include <Core/Util/BoostForeach.h>
using MTV::RowContainer;

RowContainer::RowContainer(const Point& location, float spacing, const std::vector<Widget::ptr>& initial_widgets)
  : Widget(location),
    spacing(spacing),
    insertion_offset(0.0),
    max_widget_height(0.0)
{
  // Add the initial widgets using the custom spacing.
  foreach(Widget::ptr w, initial_widgets){
    this->addWidget(w);
  }
}

bool RowContainer::contains(const Point& p){
  const Vector into = p - location;
  return ( (0.0 <= into.x and into.x <= insertion_offset) and
           (0.0 <= into.y and into.y <= max_widget_height) );
}

void RowContainer::addWidget(Widget::ptr w){
  if(this->numChildren() == 0){
    this->addWidget(w, 0.0);
  }
  else{
    this->addWidget(w, spacing);
  }
}

void RowContainer::addWidget(Widget::ptr w, float custom_spacing){
  // Place the widget at the right position.
  insertion_offset += custom_spacing;
  this->addChild(w, Vector(insertion_offset, 0.0));
  insertion_offset += w->width();

  // Keep track of the max height of the widgets.
  if(max_widget_height < w->height()){
    max_widget_height = w->height();
  }
}

void RowContainer::verticallyCenter(){
  // Compute the middle of the row widget, vertically.
  const float vc = 0.5*max_widget_height;

  // Move each widget so that its vertical center coincides with the
  // row's vertical center.
  foreach(Widget::ptr w, children){
    // w->setLocation(Point(w->getLocation().x, this->getLocation().y + vc - 0.5*w->height()));
    w->move(Vector(0.0, vc - 0.5*w->height()));
  }
}
