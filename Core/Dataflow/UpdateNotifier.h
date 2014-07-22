// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// UpdateNotifier.h - Interface for any class that wishes to broadcast
// that its state has changed.  Useful mainly for graphical objects
// (i.e., Widgets), but also for containers of such objects to allow a
// single point of notification for an external system.  Such
// contained objects' updated() signals can be connected to the
// containing object's updated() signal, so that it is emitted
// whenever any of the contained objects emit it.

#ifndef UPDATE_NOTIFIER_H
#define UPDATE_NOTIFIER_H

// Qt headers.
#include <QtCore>

namespace MTV{
  class UpdateNotifier : public QObject {
    Q_OBJECT;

  signals:
    void updated();
  };
}

#endif
