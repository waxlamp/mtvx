// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// Timing.h - simple interface function to get the current time in
// seconds since some arbitrary, constant-per-program-run reference
// time.

#ifndef TIMING_H
#define TIMING_H

// MTV headers.
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Timing.h>

// System headers.
#include <sys/time.h>

namespace MTV{
  // inline float now(){
  //   // Capture a reference time the first time this function is
  //   // called.
  //   static timeval ref;
  //   static bool first = true;
  //   if(first){
  //     gettimeofday(&ref, 0);
  //     first = false;
  //   }

  //   // Use the POSIX system call to get the current time.
  //   struct timeval tv;
  //   gettimeofday(&tv, 0);

  //   // Construct a floating point value representing the time.
  //   return static_cast<float>(tv.tv_sec - ref.tv_sec) + static_cast<float>(tv.tv_usec - ref.tv_usec)*1e-6f;
  // }

  class Clock {
  public:
    BoostPointers(Clock);

  public:
    virtual void tick() = 0;
    virtual float noww() = 0;
  };

  // This clock simply returns the wall time (expressed as number of
  // seconds since it was created) every time it is queried.
  class WallClock : public Clock {
  public:
    BoostPointers(WallClock);

  public:
    WallClock(){
      // Capture a reference time when the clock is constructed.
      gettimeofday(&ref, 0);
    }

    void tick(){}

    float noww(){
      // Use the POSIX system call to get the current time.
      struct timeval tv;
      gettimeofday(&tv, 0);

      // Construct a floating point value representing the time.
      return static_cast<float>(tv.tv_sec - ref.tv_sec) + static_cast<float>(tv.tv_usec - ref.tv_usec)*1e-6f;
    }

  private:
    struct timeval ref;
  };

  // This clock simulates time ticking along at controllable
  // intervals, allowing for events to take place between ticks that
  // take longer than the tick itself, such as stopping to dump a
  // frame to disk, etc.
  class TickingClock : public Clock {
  public:
    BoostPointers(TickingClock);

  public:
    TickingClock(float interval)
      : interval(interval),
        time(0.0)
    {}

    void tick(){
      time += interval;
    }

    float noww(){
      // // Grab the current time.
      // const float current_time = time;

      // // Increment the running time counter.
      // time += interval;

      // // Return the current time.
      // return current_time;

      return time;
    }

  private:
    float interval, time;
  };

}

#endif
