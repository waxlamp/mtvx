// System headers.
#include <sstream>
#include <stdexcept>

// MTV headers.
#include <Core/Graphics/Widget.h>

using namespace MTV;

Widget::Widget(const Point& location)
  : location(location),
    panel(0)
{}

void Widget::setPanel(WidgetPanel *_panel){
  // Record the widget panel.
  panel = _panel;

  // Recurse for the descendant widgets.
  foreach(Widget::ptr child, children){
    child->setPanel(panel);
  }
}

void Widget::receiveMouse(QMouseEvent *e){
  switch(e->type()){
  case QEvent::MouseButtonPress:
    this->mouseClick(e);
    break;

  case QEvent::MouseButtonRelease:
    this->mouseUnclick(e);
    break;

  case QEvent::MouseButtonDblClick:
    this->mouseDoubleClick(e);
    break;

  case QEvent::MouseMove:
    this->mouseMove(e);
    break;

  default:
    {
      std::stringstream ss;
      ss << "QMouseEvent contains a type field (" << e->type() << ") "
         << "besides MouseButtonPress, MouseButtonRelease, MouseButtonDblClick, and MouseMove";
      throw std::domain_error(ss.str().c_str());
    }
  }
}
