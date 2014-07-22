// -*- c++ -*-

#ifndef ALPHA_GLYPH_H
#define ALPHA_GLYPH_H

// System headers.
#include <cmath>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

class AlphaGlyph {

protected:
	static float PI;

	float * alpha;
	int w,h;
	GLuint tid;

	virtual void Rebuild() = 0;

public:
  AlphaGlyph(int resX, int resY);

	virtual ~AlphaGlyph(void);

	virtual void SetSize( int resX, int resY );

	virtual void EnableBlending();
	virtual void Draw( float x, float y, float scale );
	virtual void DisableBlending();
};

#endif
