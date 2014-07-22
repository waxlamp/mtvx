// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// DeltaMementoReader.h - In the same style as TraceReader, opens a
// file of delta mementos, reads them one by one, and takes
// appropriate action.

#ifndef DELTA_MEMENTO_READER_H
#define DELTA_MEMENTO_READER_H

// MTV headers.
#include <Core/Util/BoostPointers.h>
#include <Marino/Memorable.h>

// Qt headers.
#include <QtCore>

// System headers.
#include <fstream>
#include <string>
#include <vector>

namespace MTV{
  class DeltaMementoReader : public QObject {
    Q_OBJECT;

  public:
    BoostPointers(DeltaMementoReader);

  public:
    static DeltaMementoReader::ptr open(const std::string& filename, const std::vector<Memorable::ptr>& mems);

  public:
    DeltaMementoReader(const std::vector<Memorable::ptr>& mems);

    virtual void nextEvent() = 0;

  public slots:
    // This slot is engaged to let the object know it should read out
    // the next event.
    void timeout();

    // Playback control.
    void start();
    void stop();
    void setInterval(int msec);

  protected:
    std::vector<Memorable::ptr> mems;

  private:
    boost::shared_ptr<QTimer> timer;

  public:
    // Thrown by nextEvent() when there are no more events to read.
    class End {};
  };

  class ImmediateDeltaMementoReader : public DeltaMementoReader {
    Q_OBJECT;

  public:
    BoostPointers(ImmediateDeltaMementoReader);

  public:
    ImmediateDeltaMementoReader(const std::vector<Memorable::ptr>& mems);

    ImmediateDeltaMementoReader(const std::string& filename, const std::vector<Memorable::ptr>& mems, std::streampos start)
      : DeltaMementoReader(mems),
        eventtrace(filename.c_str())
    {
      // Move the file read pointer to the requested position
      // (generally, to just past the type code at the head of the
      // file).
      eventtrace.seekg(start);
    }

    // bool open(const std::string& filename);

    void nextEvent();

  protected:
    std::ifstream eventtrace;
  };
}

#endif
