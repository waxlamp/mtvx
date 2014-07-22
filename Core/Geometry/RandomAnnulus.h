// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// RandomAnnulus.h - Random points within an annular wedge.

#ifndef RANDOM_ANNULUS_H
#define RANDOM_ANNULUS_H

// MTV headers.
#include <Core/Geometry/Parametrized.h>
#include <Core/Math/Interpolation.h>
#include <Core/Util/BoostPointers.h>
#include <Core/Util/Util.h>

// System headers.
#include <cmath>

namespace MTV{
  class RandomAnnulus : public Parametrized {
  public:
    BoostPointers(RandomAnnulus);

  public:
    RandomAnnulus(const Point& center, float i_radius, float o_radius, float start_theta, float end_theta, unsigned N)
      : center(center)
    {
      // std::cout << "(" << i_radius << ", " << o_radius << ")" << std::endl;

      for(unsigned i=0; i<N; i++){
        // Generate a random angle and radius, then transform to
        // cartesian coordinates.
        const float randtheta = drand48()*(end_theta - start_theta) + start_theta;
        const float randradius = drand48()*(o_radius - i_radius) + i_radius;

        // std::cout << randradius << std::endl;

        theta.push_back(randtheta);
        radius.push_back(randradius);

        // // Return the point according to these values.
        // const Vector v = randradius*Vector(cos(randtheta), sin(randtheta));
        // std::cout << v << std::endl;

        // positions.push_back(center + v);
      }
    }

    Point position(float t){
      // Compute which interval to use.
      //
      // const unsigned i = t*(positions.size() - 1);
      const unsigned i = t*(theta.size() - 1);

      // Interpolate the appropriate points.
      //
      // const float tt = t*(positions.size() - 1)- i;
      const float tt = t*(radius.size() - 1)- i;
      // std::cout << tt << std::endl;

      // const Point p = LinearInterpolator<Point>::interpolate(positions[i], positions[i+1], tt);
      // return p;

      const float th = LinearInterpolator<float>::interpolate(theta[i], theta[i+1], tt);
      const float r = LinearInterpolator<float>::interpolate(radius[i], radius[i+1], tt);
      return center + r*Vector(cos(th), sin(th));
    }

  private:
    // std::vector<Point> positions;
    std::vector<float> theta, radius;
    Point center;
  };
}

#endif
