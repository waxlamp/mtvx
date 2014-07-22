// Copyright 2011 A.N.M. Imroz Choudhury
//
// WidgetAnimationPanel.cpp

// MTV headers.
#include <Core/UI/WidgetAnimationPanel.h>
#include <Core/Util/BoostForeach.h>
#include <Core/Util/Timing.h>
using MTV::Clock;
using MTV::WidgetAnimationPanel;

WidgetAnimationPanel::WidgetAnimationPanel(Clock::ptr clock)
  : animationTimer(new QTimer(0)),
    clock(clock)
{
  // Set the update loop to 30Hz.
  //
  // TODO(choudhury): make this adjustable.
  QObject::connect(animationTimer.get(), SIGNAL(timeout()), this, SLOT(animationUpdate()));
  animationTimer->setInterval(1000.0 / 30);
  animationTimer->start();
}

void WidgetAnimationPanel::animationUpdate(){
  // const float time = MTV::now();
  const float time = clock->noww();

  // Collect all groupers' latest animator lists and append.
  foreach(Grouper::ptr g, groupers){
    // Append this grouper's animator vector.
    const std::vector<Animator::ptr>& grouping = g->getGrouping();
    // animators.insert(animators.end(), grouping.begin(), grouping.end());
    foreach(Animator::ptr a, grouping){
      this->addAnimator(a);
    }

    // Empty the grouper's vector.
    g->clearGrouping();
  }

  // Send the current time to each animator object.
  //
  // std::cout << animators.size() << " animators" << std::endl;
  for(std::list<Animator::ptr>::iterator i = animators.begin(); i != animators.end(); ){
    Animator::ptr a = *i;

    // std::cout << a << std::endl;

    if(!a->update(time)){
      // Check the preemption table, and remove any entry found there
      // for this animator and its widget.
      std::pair<std::string, Widget::const_ptr> id = std::make_pair(typeid(*a).name(), a->getWidget());
      preemption.erase(id);

      // If the animator is a widget-killing animator, delete the
      // widget at this point.
      if(a->getFlags() & Animator::WidgetKilling){
        this->remove(a->getWidget());
      }

      // We are about to delete the iterator, so we need to save the
      // "next" iterator BEFOREHAND.
      std::list<Animator::ptr>::iterator j = ++i;
      --i;
      animators.erase(i);
      i = j;
    }
    else{
      i++;
    }
  }

  // Issue a request to re-draw.
  updateGL();
}
