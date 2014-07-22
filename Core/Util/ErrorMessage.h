// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// ErrorMessage.h - An object that supports "streaming-to" to
// construct error messages, iostream style.

#ifndef ERROR_MESSAGE_H
#define ERROR_MESSAGE_H

// System headers.
#include <sstream>

namespace MTV{
  class ErrorMessage{
  public:
    virtual ~ErrorMessage() {}

    template<typename T>
    ErrorMessage& operator<<(const T& t){
      msg << t;
      return *this;
    }

    bool hasError() const { return msg.str().length() > 0; }
    std::string error() const { return msg.str(); }

  private:
    std::stringstream msg;
  };
}

#endif
