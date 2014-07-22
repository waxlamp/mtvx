// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Printer.h - A templated consumer that simply sends incoming data to
// a stream for printing.

#ifndef PRINTER_H
#define PRINTER_H

// MTV Headers.
#include <Core/Dataflow/Consumer.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <iostream>

namespace MTV{
  template<typename T>
  class Printer : public Consumer<T> {
  public:
    BoostPointers(Printer<T>);

  public:
    Printer(std::ostream& out = std::cout, bool printEndl = true, unsigned limit = 1)
      : out(out),
        printEndl(printEndl),
        limit(limit)
    {}

    void consume(const T& t){
      static unsigned count = 0;

      out << t;
      if(++count == limit){
        if(printEndl){
          out << std::endl;
        }

        count = 0;
      }
      else{
        out << ' ';
      }
    }

  private:
    std::ostream& out;
    bool printEndl;
    unsigned limit;
  };
}

#endif
