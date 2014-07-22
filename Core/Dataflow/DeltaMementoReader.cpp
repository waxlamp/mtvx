// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// DeltaMementoReader.cpp

// MTV includes.
#include <Core/Dataflow/DeltaMementoReader.h>
#include <Marino/DeltaMementoRecorder.h>
#include <Marino/Memento.pb.h>
#include <Marino/Memorable.h>
using MTV::DeltaMementoReader;
using MTV::DeltaMementoRecorder;
using MTV::ImmediateDeltaMementoReader;
using MTV::Memorable;

// Protobuf includes.
#include <google/protobuf/text_format.h>
using google::protobuf::TextFormat;

// System includes.
#include <iostream>

DeltaMementoReader::ptr DeltaMementoReader::open(const std::string& filename, const std::vector<Memorable::ptr>& mems){
  // Open the file.  Bail out (with a null pointer) if it can't be
  // opened.
  std::ifstream in(filename.c_str());
  if(!in){
    return DeltaMementoReader::ptr();
  }

  // Read out the type code.
  unsigned code;
  in.read(reinterpret_cast<char *>(&code), sizeof(code));

  DeltaMementoReader::ptr p;

  switch(code){
  case DeltaMementoRecorder::Immediate:
    p = DeltaMementoReader::ptr(new ImmediateDeltaMementoReader(filename, mems, in.tellg()));
    break;

  case DeltaMementoRecorder::Chunked:
    // TODO(choudhury): implement ChunkedDeltaMementoReader.
    //
    // return DeltaMementoReader::ptr(new ChunkedDeltaMementoReader(filename, mems, in.tellg())));
    p = DeltaMementoReader::ptr();
    break;

  case DeltaMementoRecorder::None:
    p = DeltaMementoReader::ptr();
    break;
  }

  return p;
}

DeltaMementoReader::DeltaMementoReader(const std::vector<Memorable::ptr>& mems)
  : mems(mems),
    timer(new QTimer(0))
{
  // Connect the timer's timeout signal to the class's timeout slot.
  QObject::connect(timer.get(), SIGNAL(timeout()), this, SLOT(timeout()));
}

void DeltaMementoReader::timeout(){
  // std::cout << "reading an event!" << std::endl;

  try{
    this->nextEvent();
  }
  catch(DeltaMementoReader::End){
    std::cout << "Event trace end reached!!" << std::endl;
    timer->stop();
  }
}

void DeltaMementoReader::start(){
  timer->start();
}

void DeltaMementoReader::stop(){
  timer->stop();
}

void DeltaMementoReader::setInterval(int msec){
  timer->setInterval(msec);
}

ImmediateDeltaMementoReader::ImmediateDeltaMementoReader(const std::vector<Memorable::ptr>& mems)
  : DeltaMementoReader(mems)
{}

// bool ImmediateDeltaMementoReader::open(const std::string& filename){
//   eventtrace.open(filename.c_str());
//   return static_cast<bool>(eventtrace);
// }

void ImmediateDeltaMementoReader::nextEvent(){
  // Read out the size of the next message.
  int size;
  eventtrace.read(reinterpret_cast<char *>(&size), sizeof(size));

  // Read out the bytes for the next message.
  std::string s;
  s.resize(size);
  eventtrace.read(&s[0], size);

  // Parse out the delta from the string bytes.
  DeltaMemento d;
  if(!d.ParseFromString(s)){
    throw DeltaMementoReader::End();
  }

  // Print the object.
  //
  // std::cout << "START PRINT" << std::endl;
  // std::string ss;
  // TextFormat::PrintToString(d, &ss);
  // std::cout << ss << std::endl;
  // std::cout << "END PRINT" << std::endl;

  // Examine the id field of the delta, and apply it directly to the
  // appropriate object.
  mems[d.id()]->applyDelta(d);
}
