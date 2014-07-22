// Copyright 2011 A.N.M. Imroz Choudhury
//
// ShapeGrouper.cpp

// MTV headers.
#include <Core/Animation/PointToPointAnimator.h>
#include <Core/Animation/PolarPointAnimator.h>
#include <Core/Animation/ShapeGrouper.h>
#include <Core/Graphics/FadingPoint.h>
#include <Core/Math/Interpolation.h>
using MTV::DriftoutShapeGrouper;
using MTV::LinearInterpolator;
using MTV::PointToPointAnimator;
using MTV::PolarPointAnimator;
using MTV::ShapeGrouper;
using MTV::Widget;

ShapeGrouper::ShapeGrouper(Parametrized::ptr shape, bool animating, float duration, bool polar, const Point& center, bool print)
// ShapeGrouper::ShapeGrouper(Parametrized::ptr shape, float duration, bool print)
  : duration(duration),
    shape(shape),
    center(center),
    polar(polar),
    print(print),
    animating(animating)
{}

void ShapeGrouper::addWidget(Widget::ptr w, float time, bool marshal){
  // Add the requested widget and remarshal all of them.
  widgets.push_front(w);
  if(marshal){
    this->marshal(time);
  }
}

void ShapeGrouper::addWidget(Widget::ptr w, unsigned i, float time, bool marshal){
  // Add the requested widget at position i, and remarshal.
  i = widgets.size() - i;
  ContainerType::iterator in = widgets.begin();
  for(unsigned c=0; c<i; c++){
    in++;
  }

  widgets.insert(in, w);

  if(marshal){
    this->marshal(time);
  }
}

Widget::ptr ShapeGrouper::removeWidget(Widget::ptr w, float time, bool marshal){
  // Find the requested widget.
  ContainerType::iterator i = std::find(widgets.begin(), widgets.end(), w);
  if(i == widgets.end()){
    // If not found, return a null pointer.
    std::cout << "NOT FOUND" << std::endl;
    return Widget::ptr();
  }

  // Erase the widget and remarshal the remaining ones.
  widgets.erase(i);
  if(marshal){
    this->marshal(time);
  }

  return w;
}

Widget::ptr ShapeGrouper::removeLastWidget(float time, bool marshal){
  Widget::ptr w = widgets.back();
  widgets.pop_back();

  if(marshal){
    this->marshal(time);
  }

  return w;
}

bool ShapeGrouper::shiftWidget(Widget::ptr w, float time, bool marshal){
  // If the requested widget does not exist in the circle grouper,
  // return false.
  ContainerType::iterator i = std::find(widgets.begin(), widgets.end(), w);
  if(i == widgets.end()){
    return false;
  }

  // Erase the widget, then reinsert it at the front.
  widgets.erase(i);
  widgets.push_front(w);

  // Remarshal the widgets.
  if(marshal){
    this->marshal(time);
  }

  // Indicate success.
  return true;
}

bool ShapeGrouper::replaceWidget(Widget::ptr out, Widget::ptr in, float time, bool marshal){
  // Find the widget to be replaced.
  ContainerType::iterator i = std::find(widgets.begin(), widgets.end(), out);

  // Bail if the widget was not found.
  if(i == widgets.end()){
    return false;
  }

  // Insert the new widget before the one to be deleted.
  widgets.insert(i, in);

  // Erase the old widget.
  widgets.erase(i);

  // Remarshal the widgets.
  if(marshal){
    this->marshal(time);
  }

  return true;
}

bool ShapeGrouper::hasWidget(Widget::const_ptr w) const {
  return std::find(widgets.begin(), widgets.end(), w) != widgets.end();
}

void ShapeGrouper::marshal(float time){
  const int N = widgets.size() < 10 ? 10 : widgets.size();
  // const int N = widgets.size();

  if(print){
    std::cout << N << " items" << std::endl;
  }

  const float interval = 1.0 / N;
  {
    unsigned i=0;
    // foreach(Widget::ptr w, widgets){

    for(std::list<Widget::ptr>::iterator w = widgets.begin(); w != widgets.end(); w++){
      const Point p = shape->position((i + 0.5)*interval);

      if(polar){
        if(animating){
          Animator::ptr anim(boost::make_shared<PolarPointAnimator<LinearInterpolator<float> > > (*w, Animator::Preemptible, time, duration, center, (*w)->getLocation(), p));
          animators.push_back(anim);
        }
        else{
          (*w)->setLocation(p);

          // This block works the following way: if the widget's last
          // grouper was a linear interpolating grouper, then polar
          // interpolate to this grouper; if instead the widget is
          // coming from a polar grouper, use linear interpolation
          // here.
          if((*w)->extra() == "linear" or (*w)->extra() == ""){
            (*w)->extra() = "polar";
          }
          else{
            (*w)->extra() = "linear (polar)";
          }

          std::cout << "blerg" << std::endl;

          std::cout << (*w)->extra() << std::endl;

          // std::cout << "ShapreGrouper::marshal() - fill in polar interpolation codes here: " << __FILE__ << ":" << __LINE__ << std::endl;
          // abort();
        }
      }
      else{
        if(animating){
          Animator::ptr anim(new PointToPointAnimator<LinearInterpolator<float> >(*w, Animator::Preemptible, time, duration, (*w)->getLocation(), p));
          animators.push_back(anim);
        }
        else{
          (*w)->setLocation(p);
          (*w)->extra() = "linear";

          // std::cout << "ShapreGrouper::marshal() - fill in linear interpolation codes here: " << __FILE__ << ":" << __LINE__ << std::endl;
          // abort();
        }
      }

      if(print){
        std::cout << boost::static_pointer_cast<MTV::FadingPoint, Widget>(*w)->getColor() << " -> " << (i+0.5)*interval << " -> " << p << std::endl;
        std::cout << (*w)->getLocation() << std::endl;
      }

      ++i;
    }
  }
}

DriftoutShapeGrouper::DriftoutShapeGrouper(Parametrized::ptr shape, bool animating, float duration, float maxdrift, bool polar, const Point& center, bool print)
  : ShapeGrouper(shape, animating, duration, polar, center, print),
    maxdrift(maxdrift)
{}

void DriftoutShapeGrouper::addWidget(Widget::ptr w, float time, bool marshal){
  // Update the table of entry times.
  entrytime[w] = time;

  // Defer to the superclass addWidget() method.
  ShapeGrouper::addWidget(w, time, marshal);
}

void DriftoutShapeGrouper::addWidget(Widget::ptr w, unsigned i, float time, bool marshal){
  // Update the table of entry times.
  entrytime[w] = time;

  // Defer to the superclass addWidget() method.
  ShapeGrouper::addWidget(w, i, time, marshal);
}

Widget::ptr DriftoutShapeGrouper::removeWidget(Widget::ptr w, float time, bool marshal){
  // Remove the widget using the superclass method, and then remove
  // the timing entry from the hash table.
  Widget::ptr ww = ShapeGrouper::removeWidget(w, time, marshal);
  if(ww){
    entrytime.erase(ww);
  }

  return ww;
}

Widget::ptr DriftoutShapeGrouper::removeLastWidget(float time, bool marshal){
  // Remove the widget using the superclass method, and then remove
  // the timing entry from the hash table.
  Widget::ptr w = ShapeGrouper::removeLastWidget(time, marshal);
  if(w){
    entrytime.erase(w);
  }

  return w;
}

void DriftoutShapeGrouper::marshal(float time){
  // std::cout << "DriftoutShapeGrouper::marshal()" << std::endl;

  // Modify the position of each widget by pushing it out slightly -
  // the longer the widget has been present in the grouper, the
  // smaller the push.

  const int N = widgets.size() < 10 ? 10 : widgets.size();
  // const int N = widgets.size();

  if(print){
    std::cout << N << " items" << std::endl;
  }

  const float interval = 1.0 / N;
  {
    unsigned i=0;
    // foreach(Widget::ptr w, widgets){

    for(std::list<Widget::ptr>::iterator w = widgets.begin(); w != widgets.end(); w++){
      Point p = shape->position((i + 0.5)*interval);

      if(entrytime.find(*w) == entrytime.end()){
        std::cout << "SERIOUS ERROR!!" << std::endl;
      }

      // Compute how long the widget has been in the grouper.
      const float age = time - entrytime[*w];

      // std::cout << "widget entry at time " << entrytime[*w] << std::endl;

      // Push the widget outwards by an exponentially decreasing
      // amount - each second it goes half as far as in the last
      // second; in the first second, it moves half of maxdrift.
      const float push = (1 - pow(2.0, -age))*maxdrift;

      // Convert the widget position to polar coordinates.
      const Vector v = p - center;
      const float r = sqrt(v.x*v.x + v.y*v.y);
      float th = atan2(v.y, v.x);
      if(th < 0.0)
        th += 2*M_PI;

      // Convert back to rectangular coordinates, adding in the push.
      p = center + (r+push)*Vector(cos(th), sin(th));

      if(polar){
        if(animating){
          Animator::ptr anim(boost::make_shared<PolarPointAnimator<LinearInterpolator<float> > > (*w, Animator::Preemptible, time, duration, center, (*w)->getLocation(), p));
          animators.push_back(anim);
        }
        else{
          (*w)->setLocation(p);

          // This block works the following way: if the widget's last
          // grouper was a linear interpolating grouper, then polar
          // interpolate to this grouper; if instead the widget is
          // coming from a polar grouper, use linear interpolation
          // here.
          if((*w)->extra() == "linear" or (*w)->extra() == ""){
            (*w)->extra() = "polar";
          }
          else{
            (*w)->extra() = "linear (polar)";
          }

          // std::cout << "DriftoutShapeGrouper::marshal() - fill in polar interpolation codes here: " << __FILE__ << ":" << __LINE__ << std::endl;
          // abort();
        }
      }
      else{
        if(animating){
          Animator::ptr anim(new PointToPointAnimator<LinearInterpolator<float> >(*w, Animator::Preemptible, time, duration, (*w)->getLocation(), p));
          animators.push_back(anim);
        }
        else{
          (*w)->setLocation(p);
          (*w)->extra() = "linear";

          // std::cout << "DriftoutShapeGrouper::marshal() - fill in linear interpolation codes here: " << __FILE__ << ":" << __LINE__ << std::endl;
          // abort();
        }
      }

      if(print){
        std::cout << boost::static_pointer_cast<MTV::FadingPoint, Widget>(*w)->getColor() << " -> " << (i+0.5)*interval << " -> " << p << std::endl;
        std::cout << (*w)->getLocation() << std::endl;
      }

      ++i;
    }
  }

}
