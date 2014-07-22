// Copyright 2010 A.N.M. Imroz Choudhury
//
// ReferenceTracePanel.cpp

// MTV includes.
#include <Modules/ReferenceTrace/UI/ReferenceTracePanel.h>
using MTV::ReferenceTracePanel;
using MTV::RegionRenderer;

ReferenceTracePanel::ReferenceTracePanel()
  : net(new ReferenceTraceNetwork),
    signal_base(0),
    signal_limit(0)
{}

void ReferenceTracePanel::addRegion(MTR::addr_t base, MTR::addr_t limit, MTR::size_t type, const std::string& title){
  // Add the specified region to the network.  Get the new widget from
  // the newly added region renderer and add it to the parent instance
  // of WidgetPanel.
  RegionRenderer::ptr rr = net->addRegion(base, limit, type, title);
  this->WidgetPanel::add(rr->getWidget());

  // Repaint the panel whenever a region renderer changes.
  QObject::connect(rr.get(), SIGNAL(updated()), this, SLOT(updateGL()));
}

void ReferenceTracePanel::renderCache(Cache::const_ptr cache){
  CacheRenderer::ptr cacheRenderer = net->renderCache(cache);
  Widget::ptr w = cacheRenderer->getWidget();
  w->move(Vector(200, 200));
  this->WidgetPanel::add(w);

  QObject::connect(cacheRenderer.get(), SIGNAL(updated()), this, SLOT(updateGL()));
}

ReferenceTracePanel *ReferenceTracePanel::newFromSpec(const std::string& regfile){
  // Attempt to read in the registration file - if we cannot, then
  // bail with a null pointer return value.
  MTR::RegionRegistration reg;

  if(!reg.read(regfile.c_str())){
    std::cerr << reg.error() << std::endl;
    return 0;
  }

  // Create a reference trace panel object, then populate it with the
  // regions appearing in the registration.
  ReferenceTracePanel *panel = new ReferenceTracePanel;
  foreach(MTR::ArrayRegion& r, reg.arrayRegions){
    panel->addRegion(r.base, r.size, r.type, r.title);
  }

  if(reg.barriers.base != 0){
    panel->setSignalBase(reg.barriers.base);
    panel->setSignalLimit(reg.barriers.base + reg.barriers.size);
  }

  return panel;
}
