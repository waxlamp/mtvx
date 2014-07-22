// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// timer-test.cpp - Test the timing utility function.

// MTV headers.
#include <Core/Util/Timing.h>
using MTV::Clock;
using MTV::WallClock;

// System headers.
#include <iostream>

int main(){
  Clock::ptr clock = boost::make_shared<WallClock>();

  while(true){
    std::cout << clock->noww() << std::endl;
  }

  return 0;
}
