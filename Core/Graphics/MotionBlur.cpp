// MotionBlur.cpp

// MTV headers.
#include <Core/Graphics/MotionBlur.h>
using MTV::MotionBlur;

MotionBlur::MotionBlur(void){
  tid = 0xffffffff;
}

MotionBlur::~MotionBlur(void){
  if( tid != 0xffffffff ){
    glDeleteTextures(1,&tid);
  }
}

void MotionBlur::SetSize(int w, int h){
  width  = w;
  height = h;
}

void MotionBlur::SetClearColor( float _r, float _g, float _b ){
  r = _r;
  g = _g;
  b = _b;
}

void MotionBlur::SetDecay( float rate ){
  decay = rate;
}

void MotionBlur::Draw(){
  glDisable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  if(tid == 0xffffffff){
    glGenTextures(1,&tid);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tid);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,0);
    glDisable(GL_TEXTURE_2D);

    glColor4f(r,g,b,1.0f);
    glBegin(GL_QUADS);
    glVertex3f(-1,-1,0);
    glVertex3f( 1,-1,0);
    glVertex3f( 1, 1,0);
    glVertex3f(-1, 1,0);
    glEnd();

  }
  else{
    // Draw Prev Frame
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tid);
    glColor4f(1.0f,1.0f,1.0f,1.0f);
    glBegin(GL_QUADS);
    glTexCoord2f(0,0); glVertex3f(-1,-1,0);
    glTexCoord2f(1,0); glVertex3f( 1,-1,0);
    glTexCoord2f(1,1); glVertex3f( 1, 1,0);
    glTexCoord2f(0,1); glVertex3f(-1, 1,0);
    glEnd();
    glDisable(GL_TEXTURE_2D);

    // Wash Out Prev Frame
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable(GL_BLEND);
    glColor4f(r,g,b,decay);
    glBegin(GL_QUADS);
    glVertex3f(-1,-1,0);
    glVertex3f( 1,-1,0);
    glVertex3f( 1, 1,0);
    glVertex3f(-1, 1,0);
    glEnd();
    glDisable(GL_BLEND);
  }

  glPopMatrix();

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();

  glMatrixMode(GL_MODELVIEW);
}

void MotionBlur::Capture(){
  if(tid != 0xffffffff){
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,tid);
    glCopyTexImage2D(GL_TEXTURE_2D,0,GL_RGB16F_ARB,0,0,width,height,0);
    glDisable(GL_TEXTURE_2D);
  }
}
