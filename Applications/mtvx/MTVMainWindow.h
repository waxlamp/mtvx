// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// MTVMainWindow.h - Derived from QMainWindow, this class implements
// the GUI used to drive a TraceReader object, and to optionally
// manage several module panels (feeding the trace records to each of
// them).

#ifndef MTV_MAIN_WINDOW_H
#define MTV_MAIN_WINDOW_H

// MTV includes.
#include <Core/Dataflow/CacheSimulator.h>
#include <Core/Dataflow/DeltaMementoReader.h>
#include <Core/Dataflow/LineRecordFilter.h>
#include <Core/Dataflow/MemoryRecordFilter.h>
#include <Core/Dataflow/Module.h>
#include <Core/Dataflow/TraceReader.h>
#include <Modules/ReferenceTrace/UI/ReferenceTracePanel.h>
#include <Tools/CacheSimulator/Cache.h>
using Daly::Cache;

// Qt includes.
#include <QtGui>

namespace MTV{
  class MTVMainWindow : public QMainWindow {
    Q_OBJECT;

  public:
    MTVMainWindow();

  private slots:
    // Save state for reloading later.
    void checkpoint();

    // Open reference trace via dialog.
    void openReferenceTrace();

    // Open delta event trace via dialog.
    void openEventTrace();

    // Instantiate the example cache.
    void createExampleCache();

    // Instantiate a cache from interactive dialog.
    void createDialogCache();

    // Open various built-in modules via dialog.
    void openReferenceTraceModule();
    void openMulticacheReferenceTraceModule();
    // void openLentilModule();

    // "About" infobox.
    void about();

    // Button actions.
    void last_event();
    void rewind();
    void play();
    void pause();
    void ffwd();
    void next_event();

    // Speed control for trace playback.
    void setTraceSpeed(int value);

    // Update the trace position label.
    void updateTracePosition(unsigned long);

  private:
    bool useCache(Cache::ptr cache);

    void deactivateButtons(QToolButton *skip);

    static void unimplemented();

  private:
    // TODO(choudhury): implement these widgets and dock them to
    // appropriate places.
    //
    // // Control and display widgets that are tied directly to the data
    // // in the reference trace.
    // //

    // // Source code display widget.
    // SourceCodeDisplay *sourcecode;

    // // Cache history widget.
    // CacheHistoryMap *cachehistory;

    // This is the data source for all possible modules.
    TimedTraceReader::ptr trace;

    // Vis-driven event trace (computed from the trace in a
    // pre-process).
    DeltaMementoReader::ptr event;

    // Stream demuxers to extract M- and L-type records from the
    // trace.
    MemoryRecordFilter::ptr mfilter;
    LineRecordFilter::ptr lfilter;

    // An optional cache to simulate with records from the trace.
    CacheSimulator::ptr cache;

    // List of modules.
    std::vector<Module *> modules;

    // Named modules for special-purpose handling.
    ReferenceTracePanel *reftracePanel;

    // A label displaying the name of the currently open trace file in
    // the status bar.
    QLabel *filenameLabel;
    QLabel *traceRecordLabel;

    // A handle to the buttons whose actions, icons, and/or
    // clickability change in response to other events.
    QToolButton *playButton;
    QToolButton *lastEventButton;
    QToolButton *nextEventButton;
    QToolButton *rewindButton;
    QToolButton *fastForwardButton;

    // A handle to the "currently pressed" button.
    QToolButton *actionButton;

    // Min and max values for the slider.
    int min, max;
  };
}

#endif
