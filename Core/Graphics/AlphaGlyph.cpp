#include "AlphaGlyph.h"


float AlphaGlyph::PI = 3.14159265f;




AlphaGlyph::AlphaGlyph(int resX, int resY){
	tid = 0xffffffff;

	w = 1;
	h = 1;

	alpha = 0;

    this->SetSize(resX, resY);

}

AlphaGlyph::~AlphaGlyph(void){
	if( tid != 0xffffffff ){
		glDeleteTextures(1,&tid);
	}
	if( alpha ){
		delete [] alpha;
	}
}


void AlphaGlyph::SetSize( int resX, int resY ){
	w = resX;
	h = resY;
	if(alpha){
		delete [] alpha;
		alpha = 0;
	}
}


void AlphaGlyph::Draw( float x, float y, float scale ){
	if(tid == 0xffffffff){
		glGenTextures(1,&tid); 
	}
	if( alpha == 0 ){
		alpha = new float[w*h];

		Rebuild();

		glEnable(GL_TEXTURE_2D); 
			glBindTexture(GL_TEXTURE_2D,tid);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);
			glTexImage2D(GL_TEXTURE_2D,0,GL_ALPHA,w,h,0,GL_ALPHA,GL_FLOAT,alpha);
			//glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,w,h,0,GL_LUMINANCE,GL_FLOAT,alpha);
		glDisable(GL_TEXTURE_2D);
	}

	glEnable(GL_TEXTURE_2D); 
		glBindTexture(GL_TEXTURE_2D,tid);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0); glVertex3f(-scale + x,-scale + y,0);
			glTexCoord2f(1,0); glVertex3f( scale + x,-scale + y,0);
			glTexCoord2f(1,1); glVertex3f( scale + x, scale + y,0);
			glTexCoord2f(0,1); glVertex3f(-scale + x, scale + y,0);
		glEnd();
	glDisable(GL_TEXTURE_2D); 
}

void AlphaGlyph::EnableBlending(){
	glEnable(GL_BLEND);
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
}

void AlphaGlyph::DisableBlending(){
	glDisable(GL_BLEND);
}

