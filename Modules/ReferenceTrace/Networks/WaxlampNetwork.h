// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// WaxlampNetwork.h - Data processing network for waxlamp (organic vis
// take on reference trace visualization).

#ifndef WAXLAMP_NETWORK_H
#define WAXLAMP_NETWORK_H

// MTV headers.
#include <Core/FrameDump.pb.h>
#include <Core/Animation/Grouper.h>
#include <Core/Dataflow/LineCacheMissCounter.h>
#include <Core/Dataflow/LineVisitCounter.h>
#include <Core/Dataflow/SetUtilizationCounter.h>
#include <Core/Dataflow/Repeater.h>
#include <Core/Util/BoostPointers.h>
#include <Core/UI/WidgetPanel.h>
#include <Tools/CacheSimulator/Cache.h>

// System headers.
#include <fstream>
#include <string>

namespace MTV{
  using Daly::Cache;
  namespace FD = MTV::FrameDump;

  // TODO(choudhury): this class should go in its own source file.
  class LineDumper : public Consumer<MTR::Record> {
  public:
    BoostPointers(LineDumper);

  public:
    LineDumper(WidgetPanel::ptr panel)
      : panel(panel),
        textfile(false)
    {}

    LineDumper(WidgetPanel::ptr panel, const std::string& outfile)
      : panel(panel),
        out(outfile.c_str()),
        textfile(true)
    {
      out << "# framenum filenum linenum" << std::endl;
    }

    ~LineDumper(){
      if(textfile){
        out.close();
      }
    }

    void consume(const MTR::Record& rec){
      static unsigned count = 0;

      assert(rec.code == MTR::Record::LineNumber);

      if(textfile){
        // Text output mode.
        out << panel->getFrame() << " " << rec.file << " " << rec.line << std::endl;

        if(++count == 100){
          out << std::flush;
        }
      }
      else{
        // Protocol buffer mode.
        FD::FrameDump::SourceCode new_sc;
        new_sc.set_file(rec.file);
        new_sc.set_line(rec.line);

        sc.push_back(new_sc);
      }
    }

    const std::vector<FD::FrameDump::SourceCode>& getSourceCodePB() const {
      return sc;
    }

    void clearSourceCodePB(){
      sc.resize(0);
    }

  private:
    WidgetPanel::ptr panel;
    std::ofstream out;
    std::vector<FD::FrameDump::SourceCode> sc;
    bool textfile;
  };

  class WaxlampNetwork{
  public:
    BoostPointers(WaxlampNetwork);

  public:
    virtual std::vector<Grouper::ptr> getGroupers() = 0;
    virtual Widget::ptr getTemperatureRenderer() = 0;
    virtual Repeater<MTR::Record>::ptr getTraceRepeater() = 0;
    virtual std::vector<Widget::ptr> addRegion(MTR::addr_t base, MTR::addr_t size, MTR::size_t type) = 0;
    // virtual void dumpPoints(const std::string& filename) const = 0;
    virtual void dumpPoints(std::ofstream& out) const = 0;
    virtual LineDumper::ptr lineDumper(WidgetPanel::ptr, const std::string&) = 0;
    virtual LineVisitCounter::ptr visitCounter(WidgetPanel::ptr, const std::string&) = 0;
    virtual LineVisitCounter::ptr visitCounter(WidgetPanel::ptr) = 0;
    virtual LineCacheMissCounter::ptr missCounter(WidgetPanel::ptr, const std::string&) = 0;
    virtual LineCacheMissCounter::ptr missCounter(WidgetPanel::ptr) = 0;
    virtual SetUtilizationCounter::ptr setUtilizationCounter(WidgetPanel::ptr panel, Cache::const_ptr c, unsigned window, unsigned period, const std::string& filename) = 0;
    virtual SetUtilizationCounter::ptr setUtilizationCounter(WidgetPanel::ptr panel, Cache::const_ptr c, unsigned window, unsigned period) = 0;
    virtual void hitLevelCounter(WidgetPanel::ptr panel, const std::string& filename) = 0;
  };
}

#endif
