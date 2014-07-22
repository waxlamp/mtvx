// Copyright 2010 A.N.M. Imroz Choudhury
//
// Tick.cpp

// MTV includes.
#include <Core/Graphics/Tick.h>
#include <Core/UI/WidgetPanel.h>
using MTV::WidgetPanel;
using MTV::Tick;

Tick::Tick(Point location, const std::string& labelText, float extra)
  : Widget(location),
    extra(extra)
{
  // Create the tickmark.
  const float height = this->tickHeight(), width = this->tickWidth();
  // tick = SolidRectangle::ptr(new SolidRectangle(location - Vector(0.5*width, 0.5*height), Vector(width, height), Color::white, false));
  tick = SolidRectangle::ptr(new SolidRectangle(location - Vector(0.5*width, 0.5*height), Vector(width, height), Color::black, false));
  this->addChild(tick);

  // If there is a label to place, create a text widget for it.
  if(labelText.length() > 0){
    // Horizontally centered, and below the tick mark.
    label = TextWidget::ptr(new TextWidget(Point::zero, labelText, WidgetPanel::computer));
    label->setLocation(Point(location - Vector(0.5*label->width(), height*1.2 + label->height())));

    this->addChild(label);
  }
}
