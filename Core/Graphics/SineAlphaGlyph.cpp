#include "SineAlphaGlyph.h"

SineAlphaGlyph::SineAlphaGlyph(int resX, int resY, float steepness)
  : AlphaGlyph(resX, resY),
    steepness(steepness)
{
}

SineAlphaGlyph::~SineAlphaGlyph(void)
{
}



void SineAlphaGlyph::Rebuild(){
	float malpha = 0;

	float mw = ((float)(w-1) / 2.0f);
	float mh = ((float)(w-1) / 2.0f);

	float mr = mw;
	if(mh < mw){ mr = mh; }

	for(int v = 0; v < h; v++){
		for(int u = 0; u < w; u++){
			float dx = fabsf( (float)u - (float)(w-1) / 2.0f );
			float dy = fabsf( (float)v - (float)(h-1) / 2.0f );

			float r = sqrtf( dx*dx + dy*dy );

			alpha[v*w+u] = 0;
			if(r < mr){
				float t = 3.14159265f * r / mr;
				alpha[v*w+u] = sinf( t );
			}

			alpha[v*w+u] = powf( alpha[v*w+u], steepness );
// 				printf("%f ",alpha[v*w+u]);

			if(alpha[v*w+u] > malpha){ malpha = alpha[v*w+u]; }
		}
	}

	for(int i = 0; i < w*h; i++){
		alpha[i] /= malpha;
	}
}
