#include <Core/Graphics/GaussianAlphaGlyph.h>
using MTV::GaussianAlphaGlyph;

GaussianAlphaGlyph::GaussianAlphaGlyph(int resX, int resY, float sigma)
  : AlphaGlyph(resX, resY)
{
  this->SetSigma(sigma);
}

GaussianAlphaGlyph::~GaussianAlphaGlyph(void) {

}

float GaussianAlphaGlyph::G(float d, float sigma){
	float a = 1.0f / (2.0f * sigma * sigma);

	return sqrtf(a / PI) * exp( -a * (d*d) );
}

void GaussianAlphaGlyph::SetSigma( float s ){
	sigma = s;
	if(alpha){
		delete [] alpha;
		alpha = 0;
	}
}



void GaussianAlphaGlyph::Rebuild(){
	float malpha = 0;

	for(int v = 0; v < h; v++){
		for(int u = 0; u < w; u++){
			float dx = fabsf( (float)u - (float)(w-1) / 2.0f );
			float dy = fabsf( (float)v - (float)(h-1) / 2.0f );

			alpha[v*w+u] = G( sqrtf( dx*dx + dy*dy ), sigma );
			if(alpha[v*w+u] > malpha){ malpha = alpha[v*w+u]; }
		}
	}

	for(int i = 0; i < w*h; i++){
		alpha[i] /= malpha;
	}
}


