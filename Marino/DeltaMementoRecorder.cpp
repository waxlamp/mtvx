// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// DeltaMementoRecorder.cpp

// MTV includes.
#include <Marino/DeltaMementoRecorder.h>
using MTV::DeltaMementoRecorder;
using MTV::ImmediateDeltaMementoRecorder;

// // Protobuf includes.
// #include <google/protobuf/text_format.h>
// using google::protobuf::TextFormat;

// System includes.
#include <iostream>
#include <string>

// const unsigned DeltaMementoRecorder::None = 0;
// const unsigned DeltaMementoRecorder::Immediate = 1;
// const unsigned DeltaMementoRecorder::Chunked = 2;

ImmediateDeltaMementoRecorder::ImmediateDeltaMementoRecorder(const std::string& filename)
  : out(filename.c_str())
{
  if(!out){
    // The file wasn't able to be opened.  Just don't do anything here
    // and let the client check for "goodness" themselves.
    return;
  }

  // Write out the type code.
  const DeltaMementoRecorder::FileType code = DeltaMementoRecorder::Immediate;
  out.write(reinterpret_cast<const char *>(&code), sizeof(DeltaMementoRecorder::Immediate));
}

ImmediateDeltaMementoRecorder::~ImmediateDeltaMementoRecorder(){
  out.close();
}

void ImmediateDeltaMementoRecorder::consume(const DeltaMemento& delta){
  // Write the message size out to disk.
  const int size = delta.ByteSize();
  out.write(reinterpret_cast<const char *>(&size), sizeof(size));

  // Serialize the message itself.
  delta.SerializeToOstream(&out);
}
