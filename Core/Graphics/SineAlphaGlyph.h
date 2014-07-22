// -*- c++ -*-

#ifndef SINE_ALPHA_GLYPH_H
#define SINE_ALPHA_GLYPH_H

#include <Core/Graphics/AlphaGlyph.h>

class SineAlphaGlyph : public AlphaGlyph {
protected:

	virtual void Rebuild();

  float steepness;

public:
  SineAlphaGlyph(int resX, int resY, float steepness);
	~SineAlphaGlyph(void);
};

#endif
