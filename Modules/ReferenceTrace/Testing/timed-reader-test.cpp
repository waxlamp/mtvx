// Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved
//
// timed-reader-test.cpp - Tests the trace reader by simply printing
// out the trace records at intervals.

// MTV headers.
#include <Core/Color/Color.h>
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/MemoryRecordFilter.h>
#include <Core/Dataflow/Printer.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/UI/WidgetPanel.h>
#include <Modules/ReferenceTrace/Dataflow/RecordRenderDirector.h>
#include <Modules/ReferenceTrace/Renderers/RegionRenderer.h>

// Qt headers.
#include <QtGui>

// System headers.
#include <cstdlib>
#include <iostream>

int main(int argc, char *argv[]){
  // Launch a Qt application so the timing loop can go.
  QApplication app(argc, argv);

  std::string filename;
  if(argc < 2){
    // No file argument supplied, so use the default.
    filename = "trace.mrt";
  }
  else{
    filename = argv[1];
  }

  // Open the trace file, which is assumed to be the one produced by
  // the "make-trace" program (unless a filename is supplied to the
  // command), using a timed trace reader object.
  MTV::TimedTraceReader::ptr trace(new MTV::TimedTraceReader);
  if(!trace->open(filename)){
    std::cerr << "error: could not open file 'trace.mrt' for reading." << std::endl;
    exit(1);
  }

  // Set the timeout value for the trace reader object.
  trace->setInterval(200);

  // Connect the trace reader to a printer object.
  MTV::Consumer<MTR::Record>::ptr p(new MTV::Printer<MTR::Record>);
  trace->addConsumer(p);

  // Furthermore, connect it to a sequence of filters meant to...
  //
  // ...cull out just the memory reference records.
  MTV::MemoryRecordFilter::ptr memory_records(new MTV::MemoryRecordFilter);
  trace->addConsumer(memory_records);

  // ...pass through only the records falling in a particular range...
  const MTR::size_t type1 = 4;
  const MTR::size_t type2 = 8;

  const MTR::addr_t base1 = 0x300000;
  const MTR::addr_t limit1 = 0x300080 - type1;

  const MTR::addr_t base2 = 0x5fffe10;
  const MTR::addr_t limit2 = 0x6000000 - type2;

  MTV::AddressRangePass::ptr region1(new MTV::AddressRangePass(base1, limit1));
  MTV::AddressRangePass::ptr region2(new MTV::AddressRangePass(base2, limit2));
  memory_records->addConsumer(region1);
  memory_records->addConsumer(region2);

  // ...convert those into appropriate rendering commands...
  MTV::RecordRenderDirector::ptr director1(new MTV::RecordRenderDirector(base1, limit1, type1));
  MTV::RecordRenderDirector::ptr director2(new MTV::RecordRenderDirector(base2, limit2, type2));
  region1->MTV::Producer<MTR::Record>::addConsumer(director1);
  region2->MTV::Producer<MTR::Record>::addConsumer(director2);

  // ...and finally, to render those records.
  MTV::RegionRenderer::ptr renderer1(new MTV::RegionRenderer(MTV::Point(10,10),
                                                             base1, limit1, type1,
                                                             MTV::Color::green,
                                                             "Region 1"));
  MTV::RegionRenderer::ptr renderer2(new MTV::RegionRenderer(MTV::Point(10,200),
                                                             base2, limit2, type2,
                                                             MTV::Color::blue,
                                                             "Region 2"));
  director1->addConsumer(renderer1);
  director2->addConsumer(renderer2);

  // Create a widget panel and register the RegionRenderer internal
  // widgets to it.
  MTV::WidgetPanel::ptr panel(new MTV::WidgetPanel);
  panel->setBackgroundColor(0.9*MTV::Color::white);
  panel->add(renderer1->getWidget());
  panel->add(renderer2->getWidget());

  // Connect the widgets' update() signals the to widget panel's
  // updateGL() slot.
  //
  // TODO(choudhury): need a method to handle this, rather than having
  // to do it manually.
  QObject::connect(renderer1.get(), SIGNAL(updated()),
                   panel.get(), SLOT(updateGL()));
  QObject::connect(renderer2.get(), SIGNAL(updated()),
                   panel.get(), SLOT(updateGL()));

  // Show the widget panel.
  panel->show();

  // Start the trace.
  trace->start();

  app.exec();
}
