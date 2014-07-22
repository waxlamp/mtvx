// Copyright 2010 A.N.M. Imroz Choudhury
//
// TextWidget.cpp

// MTV includes.
#include <Core/Graphics/TextWidget.h>
using MTV::Point;
using MTV::TextWidget;
using MTV::Vector;

TextWidget::TextWidget(const Point& location, const std::string& t, const QFont& f)
  : Widget(location)
{
  this->setFont(f);
  this->setText(t);
}

void TextWidget::setFont(const QFont& f){
  // Make a copy of the font object.
  font = boost::shared_ptr<QFont>(new QFont(f));

  // Measure the bounding box.
  this->recompute_bounding_box();
}

void TextWidget::setText(const std::string& t){
  // Save the text.
  text = t;

  // Measure the bounding box.
  this->recompute_bounding_box();
}

bool TextWidget::contains(const Point& p){
  Vector into = p - location;
  return ( (0.0 <= into.x and into.x <= bbox.x) and
           (0.0 <= into.y and into.y <= bbox.y) );
}

void TextWidget::draw() const {
  panel->text(location.x, panel->height() - location.y, text, *font);

  // NOTE(choudhury): the following block is debugging code, it
  // visualizes the bounding rectangle computed for the text.
  //
  // glBegin(GL_LINE_LOOP);
  // {
  //   glColor3f(1.0, 0.0, 0.0);
  //   glVertex2f(location.x, location.y);
  //   glVertex2f(location.x + bbox.x, location.y);
  //   glVertex2f(location.x + bbox.x, location.y + bbox.y);
  //   glVertex2f(location.x, location.y + bbox.y);
  // }
  // glEnd();
}

void TextWidget::recompute_bounding_box(){
  // TODO(choudhury): figure out why the bounding rectangle is too
  // short.  The 1.3 in the bbox assignment is a total hack.
  QFontMetrics fm(*font);
  QRect rect = fm.boundingRect(QString(text.c_str()));
  bbox = Vector(rect.width()*1.3, rect.height());
}
