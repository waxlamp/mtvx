// Copyright 2010 A.N.M. Imroz Choudhury
//
// ColorGenerator.cpp

#include <Core/Color/ColorGenerator.h>
using MTV::Color;
using MTV::ColorbrewerQualitative9_Set1;
using MTV::ColorbrewerQualitative9_Set1_Mod1;
using MTV::ColorbrewerQualitative9_Set1_Mod2;
using MTV::ColorbrewerQualitative11_Paired;
using MTV::ColorGenerator;
using MTV::RGBCorners;

RGBCorners::RGBCorners(){
  // Add the corner colors, and then add in the midpoints.
  colors.push_back(Color(1.0, 0.0, 0.0)); // red
  colors.push_back(Color(0.0, 1.0, 0.0)); // green
  colors.push_back(Color(0.0, 0.0, 1.0)); // blue
  colors.push_back(Color(1.0, 1.0, 0.0)); // yellow
  colors.push_back(Color(1.0, 0.0, 1.0)); // magenta
  colors.push_back(Color(0.0, 1.0, 1.0)); // cyan
  colors.push_back(Color(0.4, 0.4, 0.4)); // dark gray
  colors.push_back(Color(0.7, 0.7, 0.7)); // light gray
  colors.push_back(Color(1.0, 1.0, 1.0)); // white
}

const Color& RGBCorners::color(unsigned i){
  // Fill in random colors if the requested color index is
  // out-of-bounds.
  if(i >= colors.size()){
    for(unsigned j=colors.size(); j<i+1; j++){
      colors.push_back(Color(drand48(), drand48(), drand48()));
    }
  }

  // Select the requested color.
  return colors[i];
}

const Color ColorbrewerQualitative11_Paired::colors[] = {
  Color(166./255, 206./255, 227./255),
  Color(31./255, 120./255, 180./255),
  Color(178./255, 223./255, 138./255),
  Color(51./255, 160./255, 44./255),
  Color(251./255, 154./255, 153./255),
  Color(227./255, 26./255, 28./255),
  Color(253./255, 191./255, 111./255),
  Color(255./255, 127./255, 0./255),
  Color(202./255, 178./255, 214./255),
  Color(106./255, 61./255, 154./255),
  Color(255./255, 255./255, 153./255)
};

const unsigned ColorbrewerQualitative11_Paired::numColors = sizeof(ColorbrewerQualitative11_Paired::colors) / sizeof(ColorbrewerQualitative11_Paired::colors[0]);

const Color ColorbrewerQualitative9_Set1::colors[] = {
  Color(228./255, 26./255, 28./255),
  Color(55./255, 126./255, 184./255),
  Color(77./255, 175./255, 74./255),
  Color(152./255, 78./255, 163./255),
  Color(255./255, 127./255, 0./255),
  Color(255./255, 255./255, 51./255),
  Color(166./255, 86./255, 40./255),
  Color(247./255, 129./255, 191./255),
  Color(153./255, 153./255, 153./255)
};

const unsigned ColorbrewerQualitative9_Set1::numColors = sizeof(ColorbrewerQualitative9_Set1::colors) / sizeof(ColorbrewerQualitative9_Set1::colors[0]);

const Color ColorbrewerQualitative9_Set1_Mod1::colors[] = {
  Color(0.0, 0.0, 0.0),
  Color(231./255, 41./255, 119./255), // More towards pink
  Color(55./255, 184./255, 184./255), // More towards cyan
  Color(77./255, 175./255, 74./255),
  Color(152./255, 78./255, 163./255),
  Color(255./255, 127./255, 0./255),
  Color(230./255, 171./255, 2./255), // Darker yellow
  Color(166./255, 86./255, 40./255),
  Color(247./255, 129./255, 191./255),
  Color(153./255, 153./255, 153./255)
};

const unsigned ColorbrewerQualitative9_Set1_Mod1::numColors = sizeof(ColorbrewerQualitative9_Set1_Mod1::colors) / sizeof(ColorbrewerQualitative9_Set1_Mod1::colors[0]);

const Color ColorbrewerQualitative9_Set1_Mod2::colors[] = {
  Color(152./255, 78./255, 163./255),
  Color(77./255, 175./255, 74./255),
  Color(255./255, 127./255, 0./255),
  Color(231./255, 41./255, 119./255), // More towards pink
  Color(55./255, 184./255, 184./255), // More towards cyan
  Color(230./255, 171./255, 2./255), // Darker yellow
  Color(166./255, 86./255, 40./255),
  Color(247./255, 129./255, 191./255),
  Color(153./255, 153./255, 153./255)
};

const unsigned ColorbrewerQualitative9_Set1_Mod2::numColors = sizeof(ColorbrewerQualitative9_Set1_Mod2::colors) / sizeof(ColorbrewerQualitative9_Set1_Mod2::colors[0]);
