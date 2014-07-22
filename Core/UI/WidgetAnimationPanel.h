// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// WidgetAnimationPanel.h - A subclass of WidgetPanel that uses
// the animation infrastructure to manipulate widgets.

#ifndef WIDGET_ANIMATION_PANEL_H
#define WIDGET_ANIMATION_PANEL_H

// MTV headers.
#include <Core/Animation/Animator.h>
#include <Core/Animation/Grouper.h>
#include <Core/UI/WidgetPanel.h>
#include <Core/Util/Boost.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>

// Qt headers.
#include <QtCore>

// System headers.
#include <list>
#include <typeinfo>
#include <vector>

namespace MTV{
  class WidgetAnimationPanel : public WidgetPanel {
    Q_OBJECT;

  public:
    BoostPointers(WidgetAnimationPanel);

  public:
    WidgetAnimationPanel(Clock::ptr clock);

    void addAnimator(Animator::ptr a){
      const unsigned f = a->getFlags();

      // See if this animator is to pre-empt another animator.
      std::pair<std::string, Widget::const_ptr> id = std::make_pair(typeid(*a).name(), a->getWidget());
      if(preemption.find(id) != preemption.end()){
        // Grab the iterator to the preempted animator.
        std::list<Animator::ptr>::iterator i = preemption[id];

        // Remove the animator from the list.
        animators.erase(i);
      }

      // Add the new animator at the end of the list.
      //
      // TODO(choudhury): is there a reason to "replace" the old
      // animator instead of using push_back() here?
      animators.push_back(a);

      if(f & Animator::Preemptible){
        // Update the preemption table entry.
        //
        // NOTE(choudhury): list iterators don't support operator--()
        // so we have to do it this way (and I don't want to use the
        // rbegin()/rend() functions because they are weird).
        std::list<Animator::ptr>::iterator i = animators.end();
        --i;

        preemption[id] = i;
      }
      else{
        // No preemption, so remove any entry that may be in the
        // preemption table.
        preemption.erase(id);
      }
    }

    void addGrouper(Grouper::ptr g){
      groupers.push_back(g);
    }

  public slots:
    void animationUpdate();

  private:
    std::vector<Widget::ptr> widgets;
    std::list<Animator::ptr> animators;
    std::vector<Grouper::ptr> groupers;

    boost::unordered_map<std::pair<std::string, Widget::const_ptr>, std::list<Animator::ptr>::iterator> preemption;

    boost::shared_ptr<QTimer> animationTimer;

    Clock::ptr clock;
  };
}

#endif
