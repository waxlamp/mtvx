// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// RandomDisc.h - Randomly placed points within a circle's area.

#ifndef RANDOM_DISC_H
#define RANDOM_DISC_H

// MTV headers.
#include <Core/Geometry/Parametrized.h>
#include <Core/Math/Interpolation.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Util.h>

// System headers.
#include <cmath>

namespace MTV{
  class RandomDisc : public Parametrized {
  public:
    BoostPointers(RandomDisc);

  public:
    RandomDisc(const Point& center, float radius, unsigned N){
      for(unsigned i=0; i<N; i++){
        // Generate a random angle and radius, then transform to
        // cartesian coordinates.
        const float randtheta = drand48() * 2*M_PI;
        const float randradius = drand48() * radius;

        // Return the point according to these values.
        positions.push_back(center + randradius*Vector(cos(randtheta), sin(randtheta)));
      }
    }

    Point position(float t){
      // Compute which interval to use.
      const unsigned i = t*(positions.size() - 1);

      // Interpolate the appropriate points.
      const float tt = t*(positions.size() - 1)- i;
      const Point p = LinearInterpolator<Point>::interpolate(positions[i], positions[i+1], tt);

      return p;
    }

  private:
    std::vector<Point> positions;
  };
}

#endif
