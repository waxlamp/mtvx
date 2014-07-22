// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// CacheSetDisplay.cpp

// MTV headers.
#include <Core/Graphics/TextWidget.h>
#include <Modules/ReferenceTrace/Graphics/CacheSetDisplay.h>
using MTV::CacheLevelGlyph;
using MTV::CacheSetDisplay;
using MTV::TextWidget;

CacheSetDisplay::CacheSetDisplay(const Point& location, const std::vector<std::vector<CacheLevel::ptr> >& levels, const std::vector<std::string>& labels)
  : Widget(location)
{
  // Each row of the display will contain the cache levels at the
  // corresponding element of the vector-of-vectors.  The row will
  // comprise a text label ("L1", "L2", etc.) followed by glyphs for
  // each cache level specified for that level of the cache set.
  float height_insertion_point = 0.0;
  for(unsigned i=0; i<levels.size(); i++){
    // Create a row widget for this level.
    RowContainer::ptr row(new RowContainer(location + Vector(0.0, height_insertion_point), 10.0));

    // Add a text label for the row.
    TextWidget::ptr label(new TextWidget(Point::zero, makeLabel(labels, i)));
    row->addWidget(label);

    // Add cache level glyphs for each cache level in the vector.
    foreach(CacheLevel::ptr c, levels[i]){
      // Create a glyph and place it in the row widget.
      CacheLevelGlyph::ptr g(new CacheLevelGlyph(Point::zero, c->numBlocks(), 100.0));
      row->addWidget(g);

      // Place the glyph in the map.
      glyphs[c] = g;
    }

    // Center the row widget elements.
    row->verticallyCenter();

    // Center the row itself horizontally.
    //
    // TODO(choudhury): see how this really works out.
    row->move(Vector(-0.5*row->width(), 0.0));

    // Claim the row as a child.
    this->addChild(row);

    // Add in the height of the row for the next one.
    height_insertion_point += row->height();
  }
}

CacheLevelGlyph::ptr CacheSetDisplay::getGlyph(CacheLevel::ptr which) const throw(std::domain_error) {
  std::map<CacheLevel::ptr, CacheLevelGlyph::ptr>::const_iterator i = glyphs.find(which);
  if(i == glyphs.end()){
    std::stringstream ss;
    ss << "CacheSetDisplay::getGlyph() received argument " << which << ", which does not exist in the glyph map.";

    throw std::domain_error(ss.str());
  }

  return i->second;
}

std::string CacheSetDisplay::makeLabel(const std::vector<std::string>& labels, unsigned rank){
  if(rank < labels.size()){
    return labels[rank];
  }
  else{
    std::stringstream ss;
    ss << "L" << (rank+1);
    return ss.str();
  }
}
