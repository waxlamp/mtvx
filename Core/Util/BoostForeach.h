// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// BoostForeach.h - Includes the foreach header from the Boost
// library, and creates two better-looking aliases for the boost
// macros.

#ifndef BOOST_FOREACH_H
#define BOOST_FOREACH_H

#include <boost/foreach.hpp>

#ifndef foreach
# define foreach BOOST_FOREACH
# define foreach_reverse BOOST_REVERSE_FOREACH
#else
# define mtv_foreach BOOST_FOREACH
# define foreach_reverse BOOST_REVERSE_FOREACH
#endif


#endif
