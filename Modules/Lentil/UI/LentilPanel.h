// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilPanel.h - UI panel for displaying Lentil simulation output
// along with cache status displays.

#ifndef LENTIL_PANEL_H
#define LENTIL_PANEL_H

// MTV includes.
#include <Core/Dataflow/Module.h>
#include <Core/UI/WidgetPanel.h>
#include <Modules/Lentil/Networks/LentilNetwork.h>

// System includes.
#include <iostream>

namespace MTV{
  class LentilPanel : public WidgetPanel,
                      public Module {
  public:
    LentilPanel(const std::string& dir);

    bool good() const { return net->good(); }

    // Module interface.
    Consumer<MTR::Record>::ptr memoryRecordEntryPoint() { return net->memoryRecordEntryPoint(); }
    Consumer<MTR::Record>::ptr lineRecordEntryPoint() { return net->lineRecordEntryPoint(); }
    Consumer<CacheAccessRecord>::ptr cacheAccessRecordEntryPoint() { return net->cacheAccessRecordEntryPoint(); }
    Consumer<CacheStatusReport>::ptr cacheStatusReportEntryPoint() { return net->cacheStatusReportEntryPoint(); }
    Consumer<TraceSignal>::ptr signalEntryPoint() { return net->signalEntryPoint(); }

  protected:
    void resizeGL(int width, int height);

  private:
    LentilNetwork::ptr net;
  };
}

#endif
