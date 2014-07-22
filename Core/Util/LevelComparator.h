// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LevelComparator.h - Used to find the maximum hit level in a list of
// hit info records.

#ifndef LEVEL_COMPARATOR_H
#define LEVEL_COMPARATOR_H

namespace MTV{
  template<typename T>
  class LevelComparator{
  public:
    bool operator()(const T& a, const T& b) const {
      return a.L < b.L;
    }
  };
}

#endif
