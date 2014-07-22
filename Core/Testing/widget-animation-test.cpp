// Copyright 2011 A.N.M. Imroz Choudhury, all rights reserved
//
// widget-animation-test.cpp - A simple test program to move a widget
// across the screen.


// Qt headers.
#include <QtGui>

// MTV headers.
#include <Core/Animation/ColorAnimator.h>
#include <Core/Animation/GlowAnimator.h>
#include <Core/Animation/ShapeGrouper.h>
#include <Core/Animation/PointToPointAnimator.h>
#include <Core/Animation/PolarPointAnimator.h>
#include <Core/Color/Color.h>
#include <Core/Geometry/ParametrizedCircle.h>
#include <Core/Geometry/Vector.h>
#include <Core/Graphics/GLPoint.h>
#include <Core/Math/Interpolation.h>
#include <Core/UI/WidgetAnimationPanel.h>
#include <Core/Util/Timing.h>

// System headers.
#include <cstdlib>

template<typename T>
T rando(int w, int h){
  T t(w*drand48(), h*drand48());
  std::cout << t << std::endl;
  return t;
}

MTV::Color randocolor(){
  return MTV::Color(drand48(), drand48(), drand48());
}

float factor(float frac){
  return 1.0 + 2.0*(drand48()-0.5)*frac;
}

int main(int argc, char *argv[]){
  QApplication app(argc, argv);

  // float right_now = MTV::now();
  MTV::Clock::ptr clock = boost::make_shared<MTV::WallClock>();
  float right_now = clock->noww();

  float duration = 5.0f;
  if(argc > 1){
    std::stringstream ss(argv[1]);
    ss >> duration;
  }

  int N = 0;
  if(argc > 2){
    std::stringstream ss(argv[2]);
    ss >> N;
  }

  MTV::WidgetAnimationPanel::ptr panel(new MTV::WidgetAnimationPanel(clock));
  const int w = panel->width();
  const int h = panel->height();

  // MTV::GLPoint::ptr w1(new MTV::GLPoint(rando<MTV::Point>(w,h), MTV::Color::red, 8));
  MTV::GLPoint::ptr w1(new MTV::GLPoint(MTV::Point(300, 300), MTV::Color::red, 8));
  MTV::GLPoint::ptr w2(new MTV::GLPoint(MTV::Point(300, 300), MTV::Color::green, 8));

  // MTV::GLPoint::ptr w2(new MTV::GLPoint(rando<MTV::Point>(w,h), MTV::Color::green, 8));
  // MTV::GLPoint::ptr w3(new MTV::GLPoint(rando<MTV::Point>(w,h), MTV::Color::blue, 8));

  panel->add(w1);
  panel->addAnimator(MTV::Animator::ptr(new MTV::PointToPointAnimator<MTV::SigmoidInterpolator>(w1, MTV::Animator::NoFlags, right_now, duration, w1->getLocation(), MTV::Point(0,300))));
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w1, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::white, right_now, duration)));
  panel->addAnimator(MTV::Animator::ptr(new MTV::ColorAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w1, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::white, 0.5, 2*duration)));

  panel->add(w2);
  panel->addAnimator(MTV::Animator::ptr(new MTV::PolarPointAnimator<MTV::SigmoidInterpolator>(w2, MTV::Animator::NoFlags, right_now, duration, MTV::Point(150, 300), w2->getLocation(), MTV::Point(150,150))));
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w2, MTV::Animator::NoFlags, w2->getColor(), MTV::Color::white, right_now, duration)));
  panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w2, MTV::Animator::NoFlags, w2->getColor(), MTV::Color::white, 0.35, duration)));

  // panel->add(w2);
  // panel->addAnimator(MTV::Animator::ptr(new MTV::PointToPointAnimator<MTV::SigmoidInterpolator>(w2, MTV::Animator::NoFlags, right_now, duration, w1->getLocation(), MTV::Vector(0,350))));
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::CubicInterpolator>(w2, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::white, right_now, duration*0.6)));

  // panel->add(w3);
  // panel->addAnimator(MTV::Animator::ptr(new MTV::PointToPointAnimator<MTV::SigmoidInterpolator>(w3, MTV::Animator::NoFlags, right_now, duration, w1->getLocation(), MTV::Vector(0,350))));
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::SigmoidInterpolator>(w3, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::white, right_now, duration*2)));

  // MTV::ShapeGrouper::ptr circle(boost::make_shared<MTV::ShapeGrouper>(boost::make_shared<MTV::ParametrizedCircle>(MTV::Point(300,300), 50), 1.0));
  // panel->addGrouper(circle);

  // for(int i=0; i<N; i++){
  //   MTV::GLPoint::ptr widget(new MTV::GLPoint(rando<MTV::Point>(w,h), randocolor(), 8));

  //   panel->add(widget);
  //   panel->addAnimator(MTV::Animator::ptr(new MTV::PointToPointAnimator<MTV::SigmoidInterpolator>(widget, MTV::Animator::NoFlags, right_now, duration*factor(0.5), w1->getLocation(), rando<MTV::Vector>(100,100) - MTV::Vector(50,50))));
  //   panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(widget, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::white, right_now, duration*factor(0.5))));

  //   circle->addWidget(widget, right_now + 1.5*duration);
  //   panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(widget, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::red, right_now + 1.5*duration, 2.0)));
  // }

  // circle->addWidget(w1, right_now + 1.5*duration + 2.5);
  // circle->addWidget(w2, right_now + 1.5*duration + 2.5);
  // circle->addWidget(w3, right_now + 1.5*duration + 2.5);
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w1, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::blue, right_now + 1.5*duration + 2.5, 2.0)));
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w2, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::blue, right_now + 1.5*duration + 2.5, 2.0)));
  // panel->addAnimator(MTV::Animator::ptr(new MTV::GlowAnimator<MTV::GLPoint,MTV::LinearInterpolator<float> >(w3, MTV::Animator::NoFlags, w1->getColor(), MTV::Color::blue, right_now + 1.5*duration + 2.5, 2.0)));

  panel->show();

  return app.exec();
}
