// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampPanel.cpp

// MTV headers.
#include <Core/Graphics/FadingRing.h>
#include <Modules/ReferenceTrace/Animation/CacheGrouper.h>
#include <Modules/ReferenceTrace/UI/WaxlampPanel.h>
using MTV::CacheGrouper;
using MTV::Clock;
using MTV::FadingRing;
using MTV::WaxlampPanel;

WaxlampPanel::WaxlampPanel(Clock::ptr clock, WaxlampNetwork::ptr net)
  : WidgetAnimationPanel(clock),
    net(net)
{
  // Create a central backdrop.
  //
  // FadingPoint::ptr L2_backdrop = boost::make_shared<FadingPoint>(Point(360,360), Color(0.1, 0.2, 0.6), 1200);
  // FadingPoint::ptr L1_backdrop = boost::make_shared<FadingPoint>(Point(360,360), Color(0.1, 0.7, 0.2), 600);
  // this->add(L2_backdrop);
  // this->add(L1_backdrop);

  // Add the cache temperature glyph.
  Widget::ptr temp = net->getTemperatureRenderer();
  if(temp){
    this->add(temp);
  }

  // Turn on motion blur.
  //
  // this->motionBlur(true);
}

void WaxlampPanel::installGroupers(){
  // Add the groupers.
  foreach(Grouper::ptr g, net->getGroupers()){
    this->addGrouper(g);
  }
}
