// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// Util.h - Utility functions.

#ifndef UTIL_H
#define UTIL_H

// Boost headers.
#include <boost/shared_ptr.hpp>

// System headers.
#include <cmath>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

namespace MTV{
  // Returns either the title passed in, or else a generic label based
  // on the rank when the title is the empty string.
  std::string makeTitle(const std::string& title, unsigned rank);

  // Functions for sorting pairs based on the first or second element.
  template<typename U, typename V>
  bool lessthanFirst(const std::pair<U,V>& p1, const std::pair<U,V>& p2){
    return p1.first < p2.first;
  }

  template<typename U, typename V>
  bool lessthanSecond(const std::pair<U,V>& p1, const std::pair<U,V>& p2){
    return p1.second < p2.second;
  }

  template<typename U, typename V>
  bool equalFirst(const std::pair<U,V>& p1, const std::pair<U,V>& p2){
    return p1.first == p2.first;
  }

  template<typename T>
  T sq(T x){
    return x*x;
  }

  inline unsigned probeMaxOpenStreams(){
    // This function tells how many files can be open, at the time of
    // the call, within the calling process.

    // Start opening files in a loop, counting them, until an
    // exception is thrown.
    std::vector<boost::shared_ptr<std::ofstream> > outs;
    unsigned i;
    for(i=0; ; i++){
      // TODO(choudhury): won't work on non-unix platforms.
      outs.push_back(boost::shared_ptr<std::ofstream>(new std::ofstream("/dev/null")));
      if(!(*(outs.back()))){
        break;
      }
    }

    return i;
  }

  inline std::vector<std::string> tokenize(const std::string& text){
    // Create a stream from the input text.
    //
    // NOTE(choudhury): the stream won't extract the final token if
    // there is no whitespace (i.e., delimiter) character after it.
    std::stringstream ss(text + ' ');

    // Read out tokens from the stream until none remain.
    std::vector<std::string> tokenlist;
    while(true){
      std::string token;
      ss >> token;
      if(ss.eof()){
        break;
      }

      tokenlist.push_back(token);
    }

    return tokenlist;
  }

  // This is a utility function for reading out a protocol buffer from
  // a file.  The function assumes the file is structured with an int
  // value expressing the size of the next protocol buffer, followed
  // by the serialized buffer itself.  The client must know the type
  // stored in the file.
  template<typename T>
  inline void readProtocolBuffer(std::ifstream& in, T& t){
    // Read out the size of the buffer.
    int size;
    in.read(reinterpret_cast<char *>(&size), sizeof(size));

    // Read the buffer out into a string of bytes.
    std::string buf(size, '\0');
    in.read(&buf[0], size);

    // Parse the protocol buffer out of the string.
    t.ParseFromString(buf);
  }

  // See http://tauday.com.
  const double Tau = 2*M_PI;
}

#endif
