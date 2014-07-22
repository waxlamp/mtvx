// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Widget.h - Encapsulation for the concept of "object that can be
// drawn onscreen and interacted with."

#ifndef WIDGET_H
#define WIDGET_H

// Qt headers.
#include <QMouseEvent>

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Dataflow/UpdateNotifier.h>
#include <Core/Geometry/Point.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <iostream>
#include <string>
#include <vector>
#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace MTV{
  // NOTE(choudhury): this is a forward declaration to avoid an
  // include-dependency cycle.
  class WidgetPanel;

  class Widget : public UpdateNotifier {
  public:
    BoostPointers(Widget);

  public:
    Widget(const Point& location);

    virtual ~Widget() {}

    // TODO(choudhury): make these pure virtual, and implement for
    // every widget.
    virtual float width() const { return 0.0; }
    virtual float height() const { return 0.0; }

    // TODO(choudhury): why is draw() const but contains() isn't?
    virtual bool contains(const Point& p) = 0;
    virtual void draw() const = 0;

    void drawChildren() const {
      // Convenience function: simply draws each child widget.
      foreach(Widget::ptr child, children){
        child->draw();
      }
    }

    bool childrenContain(const Point& p){
      // Convenience function: returns true if ANY child widget
      // contains the point p.
      foreach(Widget::ptr child, children){
        if(child->contains(p)){
          return true;
        }
      }

      return false;
    }

    unsigned numChildren() const { return children.size(); }

    void setPanel(WidgetPanel *_panel);

    virtual const Point& getLocation() const { return location; }

    virtual void setLocation(const Point& p){
      // For children, the location is with respect to the parent
      // location - so for each child, compute the current vector
      // offset, and move it from the point p by that amount.
      foreach(Widget::ptr child, children){
        Vector offset = child->getLocation() - location;
        child->setLocation(p + offset);
      }

      // Set the location for this widget.
      location = p;
    }

    virtual void move(const Vector& v){
      // Move the current widget.
      location = location + v;

      // Move each child widget by the same amount.
      foreach(Widget::ptr child, children){
        child->move(v);
      }
    }

    std::string& extra() { return stringdata; }
    const std::string& extra() const { return stringdata; }

    // This method has a default implementation, which simply
    // dispatches one of the specific mouse handler functions below.
    // It can be reimplmented to provide arbitrary behavior.
    virtual void receiveMouse(QMouseEvent *e);

    // These are, by default, null actions.  Any or all can be
    // reimplemented to provide mouse interaction.
    virtual void mouseClick(QMouseEvent *e) {}
    virtual void mouseUnclick(QMouseEvent *e) {}
    virtual void mouseMove(QMouseEvent *e) {}
    virtual void mouseDoubleClick(QMouseEvent *e) {}

  protected:
    // NOTE(choudhury): this method is protected so as to only allow
    // derived class member functions to use it (so that widgets can't
    // be adopted to other widgets without the consent of the would-be
    // parent widget, so to speak).  If that is too restrictive, then
    // this function will be made public instead.
    //
    // TODO(choudhury): cause this method to connect the child's
    // updated() signal to this widget's own updated() signal, so they
    // chain in proper fashion.
    void addChild(Widget::ptr child){
      children.push_back(child);

      // Make sure the new child (and any children it may have) know
      // which panel the parent belongs to.
      child->setPanel(this->panel);
    }

    // Add a child, with new location *relative to the new parent*.
    void addChild(Widget::ptr child, const Point& loc){
      children.push_back(child);
      child->setLocation(this->location + Vector(loc.x, loc.y));
    }

    void addChild(Widget::ptr child, const Vector& offset){
      children.push_back(child);
      child->setLocation(this->location + offset);
    }

  protected:
    Point location;
    std::string stringdata;
    WidgetPanel *panel;

    std::vector<Widget::ptr> children;
  };

  namespace Testing{
    class FilledRectangle : public Widget {
    public:
      FilledRectangle(Point a, Point b)
        : Widget(a),
          offset(b - a),
          fill(0.7, 0.1, 0.3)
      {}

      FilledRectangle(Point a, Vector v)
        : Widget(a),
          offset(v),
          fill(0.7, 0.1, 0.3)
      {}

      bool contains(const Point& p){
        Vector into = p - location;
        return ( (0.0f <= into.x and into.x <= offset.x) and
                 (0.0f <= into.y and into.y <= offset.y) );
      }

      void draw() const {
        Point a = location;
        Point b = location + offset;

        fill.glSet();
        glBegin(GL_QUADS);
        {
          glVertex2f(a.x, a.y);
          glVertex2f(a.x, b.y);
          glVertex2f(b.x, b.y);
          glVertex2f(b.x, a.y);
        }
        glEnd();
      }

      void mouseClick(QMouseEvent *e){
        if(e->button() != Qt::LeftButton){
          return;
        }

        Vector into = Point(e->x(), e->y()) - location;
        std::cout << "FilledRectangle clicked at (" << into.x << ", " << into.y << ")" << std::endl;

        // Make the widget green while it's being clicked.
        fill = Color(0.3, 0.9, 0.2);

        // Save the click point.
        oldpos = Point(e->x(), e->y());
      }

      void mouseMove(QMouseEvent *e){
        if(e->buttons() != Qt::LeftButton){
          return;
        }

        // Figure out the movement offset and apply it to the widget.
        Point newpos = Point(e->x(), e->y());
        Vector movement = newpos - oldpos;

        // TODO(choudhury): implement operator+= for Point/Vector.
        location = location + movement;

        // Save the new "old position".
        oldpos = newpos;
      }

      void mouseUnclick(QMouseEvent *e){
        if(e->button() != Qt::LeftButton){
          return;
        }

        static bool blue = false;
        
        // Toggle the color between red and blue.
        blue = !blue;
        if(blue){
          fill = Color(0.1, 0.2, 0.8);
        }
        else{
          fill = Color(0.7, 0.1, 0.3);
        }
      }

    private:
      Vector offset;
      Color fill;
      Point oldpos;
    };
  }
}

#endif
