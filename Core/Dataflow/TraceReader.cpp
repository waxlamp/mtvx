// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// TraceReader.cpp

// MTV includes.
#include <Core/Dataflow/TraceReader.h>
using MTV::Clock;
using MTV::ClockedTraceReader;
using MTV::TimedTraceReader;
using MTV::TraceReader;
using MTV::TraceWriter;

// System includes.
#include <iostream>

TraceReader::TraceReader(const size_t bufsize)
  : in(&inbuf),
    buffer(bufsize),
    bufsize(bufsize),
    next(0),
    curbufsize(0),
    globalPos(0),
    signalBase(0),
    signalLimit(0)
{}

bool TraceReader::open(const std::string& filename){
  // Open the file.
  file.open(filename.c_str());
  if(!file){
    return false;
  }

  // Reset the filtering streambuf object.
  inbuf.reset();

  // Read the magic bytes.  If they are not present, assume the file
  // is pre-magic and contains a raw encoding.
  std::string magic(TraceWriter::magicphrase.length(), '\0');
  file.read(&magic[0], TraceWriter::magicphrase.length());

  if(magic == TraceWriter::magicphrase){
    // Read out an unsigned int.
    unsigned int code;
    file.read(reinterpret_cast<char *>(&code), sizeof(code));

    encoding = static_cast<TraceWriter::Encoding>(code);
    if(encoding == TraceWriter::Gzip){
      inbuf.push(gzip_decompressor());
    }
  }
  else{
    // Reset the file pointer to the start.
    file.seekg(0);

    // The encoding is Raw.
    encoding = TraceWriter::Raw;
  }

  inbuf.push(file);

  // return static_cast<bool>(in);
  return true;
}

void TraceReader::seek(const size_t recID){
  // TODO(choudhury): more robust behavior.
  if(encoding == TraceWriter::Raw){
    // Seek to the right place in the file.
    file.seekg(recID*sizeof(MTR::Record));

    // Save the new global position.
    globalPos = recID;
  }
}

void TraceReader::setSignalRange(MTR::addr_t base, MTR::addr_t limit){
  signalBase = base;
  signalLimit = limit;

  std::cout << "signalBase, signalLimit: " << std::hex << signalBase << ", " << signalLimit << std::dec << std::endl;

  sfilter = SignalRecordFilter::New(base);
}

const MTR::Record& TraceReader::nextRecord(){
  // Check to see if the pointer is at the end of the buffer; if so,
  // we should read in more data.
  if(next == curbufsize){
    in.read(reinterpret_cast<char *>(&buffer[0]), bufsize*sizeof(MTR::Record));
    // if(!in.good()){
    //   std::cerr << "ran out!" << std::endl;
    //   throw TraceReader::End();
    // }

    // Reset the "next" pointer and the current buffer size.
    next = 0;
    curbufsize = in.gcount() / sizeof(MTR::Record);

    // If no items were read, then signal the caller that we have
    // finished reading the entire trace.
    if(curbufsize == 0){
      throw TraceReader::End();
    }
  }

  // Broadcast the global position of the trace.
  if(globalPos % 100 == 0){
    emit onTraceRecord(globalPos);
  }
  ++globalPos;

  // Check to see if the is a memory record, and its address field is
  // a signal.
  if(MTR::type(buffer[next]) == MTR::Record::MType and
     (signalBase <= buffer[next].addr and buffer[next].addr < signalLimit)){
    sfilter->consume(buffer[next++]);
  }
  else{
    // Increment the pointer and produce the appropriate record.
    //
    // NOTE(choudhury): save a copy, because downstream objects may
    // possibly change the next pointer under us (by requesting the
    // trace reader to rebuffer, as needed for OPT-style
    // computations).
    out = buffer[next++];
    this->produce(out);
  }

  return out;
}

void TraceReader::produce(const MTR::Record& rec){
  this->broadcast(rec);
}

TimedTraceReader::TimedTraceReader(const size_t bufsize)
  : TraceReader(bufsize),
    timer(new QTimer(0))
{
  // Have the reader produce a new record whenever the timer times
  // out.
  QObject::connect(timer.get(), SIGNAL(timeout()), this, SLOT(timeout()));
}

void TimedTraceReader::timeout(){
  try{
    // Produce records until a memory record appears.
    //
    // TODO(choudhury): this behavior ought to be controllable.
    while(MTR::type(this->nextRecord()) != MTR::Record::MType){
      // Empty while loop.
    }
  }
  catch(TraceReader::End){
    std::cout << "Trace end reached!!" << std::endl;
    timer->stop();
    exit(0);
  }
}

void TimedTraceReader::start(){
  timer->start();
}

void TimedTraceReader::stop(){
  timer->stop();
}

void TimedTraceReader::setInterval(int msec){
  timer->setInterval(msec);
}

ClockedTraceReader::ClockedTraceReader(Clock::ptr clock, const size_t bufsize)
  : TraceReader(bufsize),
    timer(new QTimer(0)),
    clock(clock)
{
  // Have the reader produce a new record whenever the timer times
  // out.
  QObject::connect(timer.get(), SIGNAL(timeout()), this, SLOT(timeout()));

  // Check the clock as often as possible.
  timer->setInterval(1);
}

void ClockedTraceReader::timeout(){
  if(clock->noww() - last > interval){
    try{
      // Produce records until a memory record appears.
      //
      // TODO(choudhury): this behavior ought to be controllable.
      while(MTR::type(this->nextRecord()) != MTR::Record::MType){
        // Empty while loop.
      }
    }
    catch(TraceReader::End){
      std::cout << "Trace end reached!!" << std::endl;
      timer->stop();
      exit(0);
    }

    last = clock->noww();
  }
}

void ClockedTraceReader::start(){
  timer->start();
}

void ClockedTraceReader::stop(){
  timer->stop();
}

void ClockedTraceReader::setInterval(int msec){
  interval = 0.001*msec;
}
