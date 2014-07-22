// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// child-widget-test.cpp - Make sure that the child relationships for
// widgets work as expected.

// MTV headers.
#include <Core/Graphics/Widget.h>
#include <Core/UI/WidgetPanel.h>

using MTV::Testing::FilledRectangle;
using MTV::Point;
using MTV::Vector;
using MTV::Widget;
using MTV::WidgetPanel;

class ComposedWidget : public Widget {
public:
  ComposedWidget(Point p)
    : Widget(p)
  {
    Widget::ptr child1(new FilledRectangle(p, Vector(50, 30)));
    Widget::ptr child2(new FilledRectangle(p + Vector(0, 50), Vector(60, 20)));
    this->addChild(child1);
    this->addChild(child2);
  }

  bool contains(const Point& p){
    return childrenContain(p);
  }

  void draw() const {
    drawChildren();
  }

  void mouseDoubleClick(QMouseEvent *e){
    std::cout << "double click at (" << e->x() << ", " << e->y() << ")" << std::endl;
    this->setLocation(Point(e->x() + 100, e->y() - 40));
  }
};

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  Widget::ptr w1(new ComposedWidget(Point(30, 50)));

  WidgetPanel::ptr panel(new MTV::WidgetPanel);
  panel->add(w1);
  panel->show();

  app.exec();
}
