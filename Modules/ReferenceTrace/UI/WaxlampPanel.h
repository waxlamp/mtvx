// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampPanel.h - A panel that does reference trace animation with
// organic vis ideas.

#ifndef WAXLAMP_PANEL_H
#define WAXLAMP_PANEL_H

// MTV headers.
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/Ground.h>
#include <Core/Dataflow/Module.h>
#include <Core/UI/WidgetAnimationPanel.h>
#include <Core/Util/Timing.h>
#include <Modules/ReferenceTrace/Networks/WaxlampNetwork.h>
#include <Tools/ReferenceTrace/mtrtools.h>

// System headers.
#include <fstream>

namespace MTV{
  class WaxlampPanel : public WidgetAnimationPanel,
                       public Module {
  public:
    BoostPointers(WaxlampPanel);

  public:
    WaxlampPanel(Clock::ptr clock, WaxlampNetwork::ptr net);

    void installGroupers();

    Consumer<MTR::Record>::ptr memoryRecordEntryPoint(){
      return net->getTraceRepeater();
    }

    Consumer<MTR::Record>::ptr lineRecordEntryPoint(){
      static Ground<MTR::Record>::ptr ground(Ground<MTR::Record>::instance());
      return ground;
    }

    Consumer<CacheAccessRecord>::ptr cacheAccessRecordEntryPoint(){
      static Ground<CacheAccessRecord>::ptr ground(Ground<CacheAccessRecord>::instance());
      return ground;
    }

    Consumer<CacheStatusReport>::ptr cacheStatusReportEntryPoint(){
      static Ground<CacheStatusReport>::ptr ground(Ground<CacheStatusReport>::instance());
      return ground;
    }

    Consumer<TraceSignal>::ptr signalEntryPoint(){
      static Ground<TraceSignal>::ptr ground(Ground<TraceSignal>::instance());
      return ground;
    }

    void addRegion(MTR::addr_t base, MTR::addr_t size, MTR::size_t type){
      std::vector<Widget::ptr> widgets = net->addRegion(base, size, type);
      foreach(Widget::ptr w, widgets){
        this->add(w);
      }
    }

    virtual void paintGL(){
      // Make sure to call the parent paintGL().
      WidgetAnimationPanel::paintGL();

      static std::ofstream out;
      static bool init = false;

      // Dump the widget locations/colors to disk.
      if(this->getDumping()){
        // // Construct a filename.
        // const unsigned long long frame = this->getFrame();
        // std::stringstream filename;
        // filename << "frame" << frame << ".txt";

        if(!init){
          out.open("framedata.dat");
          if(!out){
            std::cerr << "error: could not open framedata.dat for output" << std::endl;
            exit(1);
          }

          out << "# ptr-addr color-r color-g color-b color-a pos-x pos-y radius" << std::endl;

          init = true;
        }

        // net->dumpPoints(filename.str());
        net->dumpPoints(out);

        if(this->getFrame() % 1000 == 0){
          std::cout << this->getFrame() << std::endl;
        }
      }
    }

  private:
    WaxlampNetwork::ptr net;
  };
}

#endif
