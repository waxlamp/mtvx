// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// TraceReader.h - Defines a data producer that reads a trace from
// disk and sends trace records downstream.

#ifndef TRACE_READER_H
#define TRACE_READER_H

// MTV headers.
#include <Core/Dataflow/Producer.h>
#include <Core/Dataflow/SignalRecordFilter.h>
#include <Core/Dataflow/TraceWriter.h>
#include <Core/Util/Boost.h>
#include <Core/Util/Timing.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// Qt headers.
#include <QtCore>

// System headers.
#include <iostream>
#include <fstream>
#include <vector>

namespace MTV{
  class TraceReader : public QObject,
                      public Producer<MTR::Record> {
    Q_OBJECT;

  public:
    BoostPointers(TraceReader);

  public:
    static const size_t default_bufsize = 10*1024;

  public:
    TraceReader(const size_t bufsize = default_bufsize);
    // ~TraceReader() {
    //   std::cerr << "TraceReader() destructor!" << std::endl;
    // }

    bool open(const std::string& filename);
    void seek(const size_t recID);

    void setSignalRange(MTR::addr_t base, MTR::addr_t limit);

    SignalRecordFilter::ptr getSignalFilter() { return sfilter; }

    TraceWriter::Encoding getEncoding() const { return encoding; }

    // This function rebuffers the upcoming content of the trace file
    // by copying the unread buffer contents to the start of the
    // buffer, and filling the rest from the disk.  This is used for
    // approximate OPT calculation, which needs to see the next N
    // records.
    unsigned rebuffer(){
      // NOTE(choudhury): this function is inline to avoid having a
      // library dependency cycle.  libdaly requires this function to
      // operate, but libmtvx-core already depends on libdaly.

      // Move the contents of the buffer, from the current position to the
      // end, to the start of the buffer.
      memmove(&buffer[0], &buffer[next], (curbufsize-next)*sizeof(MTR::Record));

      // Fill the remainder of the buffer with records from the trace.
      in.read(reinterpret_cast<char *>(&buffer[curbufsize-next]), (bufsize - (curbufsize - next))*sizeof(MTR::Record));

      // Set the new buffer size and reset the pointer.
      //
      // NOTE(choudhury): don't check for in.gcount() == 0 here, because
      // it doesn't matter - it will be detected in nextRecord() at the
      // appropriate time.
      curbufsize = (curbufsize - next) + in.gcount() / sizeof(MTR::Record);
      next = 0;

      return curbufsize;
    }

    // For use in OPT-style computations.
    const std::vector<MTR::Record>& getBuffer() const {
      return buffer;
    }

    // Reads out the next record from the trace and then produce()s
    // it.
    const MTR::Record& nextRecord();

    uint64_t getTracePoint() const {
      return globalPos;
    }

    // From the Producer interface.
    void produce(const MTR::Record& rec);

  signals:
    void onTraceRecord(uint64_t);

  protected:
    std::ifstream file;
    std::istream in;
    filtering_istreambuf inbuf;

    TraceWriter::Encoding encoding;

    std::vector<MTR::Record> buffer;
    const size_t bufsize;

    // "next" refers to the buffer position of the next item to be
    // read, while "curbufsize" is the number of objects that was read
    // out of the file on the last read operation.  When next ==
    // curbufsize, it means it is time to do a new read.  We need to
    // record the curbufsize on each read operation because when we
    // reach the end of the file we may receive fewer objects than the
    // full buffer size.
    int next, curbufsize;

    // The global index of the last produced trace record; this is
    // used to tell a GUI where the trace is.
    uint64_t globalPos;

    // These are used to detect signal references; those records
    // follow a separate path out of the trace reader.
    MTR::addr_t signalBase, signalLimit;
    SignalRecordFilter::ptr sfilter;

    // For "random access" to the result.
    MTR::Record out;

  public:
    // Thrown by nextRecord() when there are no more items to read.
    class End {};
  };

  class TimedTraceReader : public TraceReader {
    Q_OBJECT;

  public:
    BoostPointers(TimedTraceReader);

  public:
    TimedTraceReader(const size_t bufsize = TraceReader::default_bufsize);

  public slots:
    // This slot can be engaged to let the object know it should
    // produce a new record.
    void timeout();

    // Playback control.
    void start();
    void stop();
    void setInterval(int msec);

  private:
    boost::shared_ptr<QTimer> timer;
  };

  class ClockedTraceReader : public TraceReader {
    Q_OBJECT;

  public:
    BoostPointers(ClockedTraceReader);

  public:
    ClockedTraceReader(Clock::ptr clock, const size_t bufsize = TraceReader::default_bufsize);

  public slots:
    void timeout();

    void start();
    void stop();
    void setInterval(int msec);

  private:
    boost::shared_ptr<QTimer> timer;
    Clock::ptr clock;
    float interval;
    float last;
  };
}

#endif
