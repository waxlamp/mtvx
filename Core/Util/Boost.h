// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// Boost.h - Convenience header to pull in useful boost stuff.

#ifndef BOOST_H
#define BOOST_H

// Lexical cast.
#include <boost/lexical_cast.hpp>
using boost::lexical_cast;

// Path object for manipulating file paths.
#define BOOST_FILESYSTEM_VERSION 3
#include <boost/filesystem.hpp>
using boost::filesystem::path;

// Gzip filters for I/O.
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/filter/gzip.hpp>
using boost::iostreams::filtering_istreambuf;
using boost::iostreams::filtering_ostreambuf;
using boost::iostreams::gzip_compressor;
using boost::iostreams::gzip_decompressor;

// Unordered associative containers (hash table/set).
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#endif
