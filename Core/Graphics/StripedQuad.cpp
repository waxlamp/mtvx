// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// StripedQuad.cpp

// MTV headers.
#include <Core/Graphics/StripedQuad.h>

// System headers.
#include <iostream>

using namespace MTV;

StripedQuad::StripedQuad(Point location, int numStripes, float stripeWidth, float stripeHeight, float spacing, Color blank)
  : Widget(location),
    blank(blank),
    stripes(numStripes, blank),
    stripeWidth(stripeWidth),
    spacing(spacing),
    height(stripeHeight)
{}

bool StripedQuad::contains(const Point& p){
  const Vector offset = p - location;
  return ( (0.0 <= offset.y and offset.y <= height) and
           (0.0 <= offset.x and offset.x <= stripes.size()*stripeWidth + (stripes.size() - 1)*spacing) );
}

void StripedQuad::draw() const {
  const float width = stripes.size()*stripeWidth + (stripes.size() - 1)*spacing;
  // std::cerr << "width: " << width << std::endl;
  // std::cerr << "height: " << height << std::endl;

  // std::cerr << "location: " << location << std::endl;

  glBegin(GL_QUADS);
  {
    // Clear the space of the widget to black.
    glColor4f(0.0, 0.0, 0.0, 0.4);
    glVertex2f(location.x, location.y);
    glVertex2f(location.x + width, location.y);
    glVertex2f(location.x + width, location.y + height);
    glVertex2f(location.x, location.y + height);

    // Loop through the stripe data, laying down one stripe per item.
    for(unsigned i=0; i<stripes.size(); i++){
      Point start = location + Vector(i*(stripeWidth + spacing), 0.0);
      stripes[i].glSet();

      glVertex2f(start.x, start.y);
      glVertex2f(start.x + stripeWidth, start.y);
      glVertex2f(start.x + stripeWidth, start.y + height);
      glVertex2f(start.x, start.y + height);
    }
  }
  glEnd();
}
