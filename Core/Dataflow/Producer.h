// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// Producer.h - Interface for dataflow objects that produce data of
// some type.  These can either be *originators* of data, such as an
// object that reads data from disk and then pushes it down a
// pipeline, or else objects that receive data from another producer,
// transform it, and then send it out again (i.e., a filter).

#ifndef PRODUCER_H
#define PRODUCER_H

// MTV headers.
#include <Core/Dataflow/Consumer.h>
#include <Core/Util/BoostForeach.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <iostream>
#include <list>
// #include <vector>

namespace MTV{
  template<typename Out>
  class Producer{
  public:
    BoostPointers(Producer<Out>);

  public:
    virtual ~Producer() {}

    virtual void produce(const Out& out) = 0;

    // NOTE(choudhury): g++ doesn't like parsing "Consumer<Out>::ptr".
    void addConsumer(boost::shared_ptr<Consumer<Out> > c){
      consumers.push_back(c);
    }

    bool removeConsumer(boost::shared_ptr<Consumer<Out> > c){
      // Search the list backwards by default (common case will be to
      // insert some items and then delete the same ones,
      // stack-style).
      //
      // for(typename std::list<typename Consumer<Out>::ptr>::reverse_iterator i = consumers.rbegin(); i != consumers.rend(); i++){
      //   if(*i == c){
      //     consumers.erase(i);
      //     break;
      //   }
      // }

      if(consumers.size() == 0){
        return false;
      }

      typename std::list<typename Consumer<Out>::ptr>::iterator i = consumers.end();
      --i;
      for( ; i != consumers.begin(); i--){
        if(*i == c){
          consumers.erase(i);
          return true;
        }
      }

      assert(i == consumers.begin());
      if(*i == c){
        consumers.erase(i);
        return true;
      }

      return false;
    }


  protected:
    void broadcast(const Out& out){
      // This method simply sends the output item to each registered
      // consumer.  The implementing class may make use of this method
      // in the produce() method to actually push the data out after
      // any processing that may be required.

      foreach(typename Consumer<Out>::ptr c, consumers){
        c->print();
        c->consume(out);
      }
    }

  protected:
    // std::vector<typename Consumer<Out>::ptr> consumers;
    std::list<typename Consumer<Out>::ptr> consumers;
    bool echo;
  };
}

#endif
