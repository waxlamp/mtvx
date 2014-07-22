// -*- c++ -*-

#ifndef GAUSSIAN_ALPHA_GLYPH_H
#define GAUSSIAN_ALPHA_GLYPH_H


#include <Core/Graphics/AlphaGlyph.h>

namespace MTV{
class GaussianAlphaGlyph : 	public AlphaGlyph {
protected:

	float sigma;

	float G(float d, float sigma);

	virtual void Rebuild();


public:
  GaussianAlphaGlyph(int resX, int resY, float sigma);
	virtual ~GaussianAlphaGlyph(void);

	virtual void SetSigma( float sigma );

};
}

#endif
