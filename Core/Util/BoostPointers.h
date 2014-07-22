// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// BoostPointers.h - A macro to provide standardized typedefs for
// boost-style smart pointers to a given class.

#ifndef BOOST_POINTERS_H
#define BOOST_POINTERS_H

// Boost headers.
#include <boost/make_shared.hpp>
#include <boost/shared_ptr.hpp>

// Place this macro in a public section of a class definition to get
// standard, class-scoped typedefs for smart pointer types.
#define BoostPointers(CLASSNAME)\
  typedef boost::shared_ptr<CLASSNAME > ptr;\
  typedef boost::shared_ptr<const CLASSNAME > const_ptr

#define BoostPointers1(CLASSNAME, TEMPLATE1)\
  typedef boost::shared_ptr<CLASSNAME<TEMPLATE1> > ptr;\
  typedef boost::shared_ptr<const CLASSNAME<TEMPLATE1> > const_ptr

#define BoostPointers2(CLASSNAME, TEMPLATE1, TEMPLATE2)\
  typedef boost::shared_ptr<CLASSNAME<TEMPLATE1, TEMPLATE2> > ptr;\
  typedef boost::shared_ptr<const CLASSNAME<TEMPLATE1, TEMPLATE2> > const_ptr

#define BoostPointers3(CLASSNAME, TEMPLATE1, TEMPLATE2, TEMPLATE3)\
  typedef boost::shared_ptr<CLASSNAME<TEMPLATE1, TEMPLATE2, TEMPLATE3> > ptr;\
  typedef boost::shared_ptr<const CLASSNAME<TEMPLATE1, TEMPLATE2, TEMPLATE3> > const_ptr

#endif
