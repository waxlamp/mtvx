// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// registration.h - facilities for specifying memory regions at
// runtime in traced programs.

#ifndef REGISTRATION_H
#define REGISTRATION_H

#include "mtrtools.h"

#include <ostream>
#include <string>
#include <vector>

namespace MTR{
  // extern "C"{
  //   void starttrace();
  //   void stoptrace();
  //   void toggletrace();
  // }

  // static char starttrace, stoptrace, toggletrace;

  // TODO(choudhury): this should probably not be a class.  We are
  // going for full-program instrumentation from a single point of
  // access.  In particular, if we add triggers to this module for
  // signalling to the instrumentation suite things about
  // starting/stopping instrumentation, then they need to live as
  // extern "C" objects in the namespace, not in the class scope.
  class Registrar{
  private:
    class IndentLevel{
    public:
      IndentLevel(int level=0) : level(level) {}

      void operator++(){
        ++level;
      }

      void operator--(){
        --level;
      }

      friend std::ostream& operator<<(std::ostream& stream, const IndentLevel& indent){
        for(int i=0; i<indent.level; i++){
          stream << "  ";
        }

        return stream;
      }

    private:
      int level;
    };

  public:
    // Register an array-type or matrix-type region.
    void array(addr_t base, size_t size, size_t type, const std::string& title="");
    void matrix(addr_t base, size_t size, size_t type, int rows, int cols, const std::string& title="");

    // Register several barriers (and create runtime data for using
    // them).
    void allocate_barriers(int n);

    // Use a barrier.
    void trigger_barrier(int i){
      barrierdata[i] = 'a';
    }

    // Place a message into the trace.
    void allocate_message_space();
    void message(const std::string& msg) const;

    // Signals to start/stop reference tracing.
    //
    // void start_tracing();
    // void stop_tracing();
    // void toggle_tracing();

    // Write out an XML rendition of the region registrations.
    void record(const char *filename=0) const;

  private:
    std::vector<ArrayRegion> arrayRegions;
    std::vector<MatrixRegion> matrixRegions;

    ArrayRegion barriers;
    char *barrierdata;

    ArrayRegion message_space;
    char *ascii;
    static const short message_toggle = 128;

    char c;
  };
}

#endif
