// Copyright 2010 A.N.M. Imroz Choudhury
//
// ParticleGlyph.cpp

// MTV includes.
#include <Modules/Lentil/Graphics/ParticleGlyph.h>
using MTV::ParticleGlyph;

void ParticleGlyph::draw() const {
  static const float shellSqWidth = (1.0/(numQuadsPerSide+1))*width;
  static const float shellSqHeight = (1.0/(numQuadsPerSide+1))*height;

  // Draw the core.
  glBegin(GL_QUADS);
  {
    corecolor.glSet();

    glVertex2f(location.x - 0.5*width, location.y - 0.5*height);
    glVertex2f(location.x + 0.5*width, location.y - 0.5*height);
    glVertex2f(location.x + 0.5*width, location.y + 0.5*height);
    glVertex2f(location.x - 0.5*width, location.y + 0.5*height);
  }
  glEnd();

  // Draw the shell
  glBegin(GL_QUADS);
  {
    const Point starting(location.x - 0.5*width, location.y + 0.5*height);
    const int offset = 0;
    for(int i=0; i<numQuadsPerSide; i++){
      shellcolors[offset+i].glSet();

      glVertex2f(starting.x + i*shellSqWidth, starting.y);
      glVertex2f(starting.x + i*shellSqWidth, starting.y - shellSqHeight);
      glVertex2f(starting.x + i*shellSqWidth + shellSqWidth, starting.y - shellSqHeight);
      glVertex2f(starting.x + i*shellSqWidth + shellSqWidth, starting.y);
    }
  }

  {
    const Point starting(location.x - 0.5*width + numQuadsPerSide*shellSqWidth, location.y + 0.5*height);
    const int offset = numQuadsPerSide;
    for(int i=0; i<numQuadsPerSide; i++){
      shellcolors[offset+i].glSet();

      glVertex2f(starting.x, starting.y - i*shellSqHeight);
      glVertex2f(starting.x, starting.y - i*shellSqHeight - shellSqHeight);
      glVertex2f(starting.x + shellSqWidth, starting.y - i*shellSqHeight - shellSqHeight);
      glVertex2f(starting.x + shellSqWidth, starting.y - i*shellSqHeight);
    }
  }

  {
    const Point starting(location.x + 0.5*width - shellSqWidth, location.y - 0.5*height + shellSqHeight);
    const int offset = 2*numQuadsPerSide;
    for(int i=0; i<numQuadsPerSide; i++){
      shellcolors[offset+i].glSet();

      glVertex2f(starting.x - i*shellSqWidth, starting.y);
      glVertex2f(starting.x - i*shellSqWidth, starting.y - shellSqHeight);
      glVertex2f(starting.x - i*shellSqWidth + shellSqWidth, starting.y - shellSqHeight);
      glVertex2f(starting.x - i*shellSqWidth + shellSqWidth, starting.y);
    }
  }

  {
    const Point starting(location.x - 0.5*width, location.y - 0.5*height + shellSqHeight);
    const int offset = 3*numQuadsPerSide;
    for(int i=0; i<numQuadsPerSide; i++){
      shellcolors[offset+i].glSet();

      glVertex2f(starting.x, starting.y + i*shellSqHeight);
      glVertex2f(starting.x, starting.y + i*shellSqHeight - shellSqHeight);
      glVertex2f(starting.x + shellSqWidth, starting.y + i*shellSqHeight - shellSqHeight);
      glVertex2f(starting.x + shellSqWidth, starting.y + i*shellSqHeight);
    }
  }
  glEnd();

  // Draw an outline.
  Color::white.glSet();
  glBegin(GL_LINE_LOOP);
  {
    glVertex2f(location.x - 0.5*width, location.y - 0.5*height);
    glVertex2f(location.x + 0.5*width, location.y - 0.5*height);
    glVertex2f(location.x + 0.5*width, location.y + 0.5*height);
    glVertex2f(location.x - 0.5*width, location.y + 0.5*height);
  }
  glEnd();

  // If the particle is marked, draw a dot in its center.
  if(marked){
    Color::black.glSet();
    glPointSize(5);
    glBegin(GL_POINTS);
    {
      glVertex2f(location.x, location.y);
    }
    glEnd();
  }

}

// TODO(choudhury): v1.0
//
// void ParticleGlyph::draw() const {
//   static const float fraction = 0.5;

//   // Draw the shell rectangle first.
//   shellcolor.glSet();
//   glBegin(GL_QUADS);
//   {
//     glVertex2f(location.x - 0.5*width, location.y - 0.5*height);
//     glVertex2f(location.x + 0.5*width, location.y - 0.5*height);
//     glVertex2f(location.x + 0.5*width, location.y + 0.5*height);
//     glVertex2f(location.x - 0.5*width, location.y + 0.5*height);
//   }
//   glEnd();

//   // Draw a black wireframe over it.
//   Color::black.glSet();
//   glBegin(GL_LINE_LOOP);
//   {
//     glVertex2f(location.x - 0.5*width, location.y - 0.5*height);
//     glVertex2f(location.x + 0.5*width, location.y - 0.5*height);
//     glVertex2f(location.x + 0.5*width, location.y + 0.5*height);
//     glVertex2f(location.x - 0.5*width, location.y + 0.5*height);
//   }
//   glEnd();

//   // Draw the core rectangle on top.
//   corecolor.glSet();
//   glBegin(GL_QUADS);
//   {
//     glVertex2f(location.x - fraction*0.5*width, location.y - fraction*0.5*height);
//     glVertex2f(location.x + fraction*0.5*width, location.y - fraction*0.5*height);
//     glVertex2f(location.x + fraction*0.5*width, location.y + fraction*0.5*height);
//     glVertex2f(location.x - fraction*0.5*width, location.y + fraction*0.5*height);
//   }
//   glEnd();
      
//   // Draw a black wireframe over it.
//   Color::black.glSet();
//   glBegin(GL_LINE_LOOP);
//   {
//     glVertex2f(location.x - fraction*0.5*width, location.y - fraction*0.5*height);
//     glVertex2f(location.x + fraction*0.5*width, location.y - fraction*0.5*height);
//     glVertex2f(location.x + fraction*0.5*width, location.y + fraction*0.5*height);
//     glVertex2f(location.x - fraction*0.5*width, location.y + fraction*0.5*height);
//   }
//   glEnd();
// }
