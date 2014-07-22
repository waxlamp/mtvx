// -*- c++ -*-
//
// MotionBlur.h - Adds a motion blur effect to a GL context.

#ifndef MOTION_BLUR_H
#define MOTION_BLUR_H

// Qt headers.
#include <QtOpenGL>

namespace MTV{
  class MotionBlur{
  protected:
    int width, height;
    float decay;
    float r,g,b;
    GLuint tid;

  public:
    MotionBlur(void);
    ~MotionBlur(void);

    void SetSize(int width, int height);
    void SetClearColor( float r, float g, float b );
    void SetDecay( float rate );

    void Draw();
    void Capture();
  };
}

#endif
