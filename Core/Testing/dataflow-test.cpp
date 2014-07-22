// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// dataflow-test.cpp - A test program to test the dataflow
// infrastructure.

// System headers.
#include <iostream>

// MTV headers.
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/Filter.h>
#include <Core/Dataflow/Producer.h>

using MTV::Consumer;
using MTV::Filter;
using MTV::Producer;

class IntGenerator : public Producer<int> {
public:
  void produce(const int& i){
    broadcast(i);
  }
};

class Squarer : public Filter<int> {
public:
  void consume(const int& i){
    produce(i*i);
  }
};

class Adder : public Filter<int, float> {
public:
  Adder(float f)
    : f(f)
  {}

  void consume(const int& i){
    produce(static_cast<float>(i) + f);
  }

private:
  float f;
};

template<typename T>
class Printer : public Consumer<T> {
public:
  void consume(const T& t){
    std::cout << typeid(T).name() << ": " << t << std::endl;
  }
};

int main(){
  boost::shared_ptr<Squarer> sq(new Squarer);
  boost::shared_ptr<Adder> add(new Adder(0.3));

  IntGenerator gen;
  gen.addConsumer(sq);
  gen.addConsumer(add);

  sq->addConsumer(boost::shared_ptr<Printer<int> >(new Printer<int>));
  add->addConsumer(boost::shared_ptr<Printer<float> >(new Printer<float>));

  for(int i=0; i<10; i++){
    std::cout << "Generating " << i << "..." << std::endl;

    gen.produce(i);

    std::cout << "done." << std::endl;
  }
}
