// Copyright 2010 A.N.M. Imroz Choudhury
//
// MTVMainWindow.cpp

// MTV includes.
#include <Applications/mtvx/MTVMainWindow.h>
// #include <Modules/Lentil/UI/LentilPanel.h>
#include <Modules/ReferenceTrace/UI/MulticacheReferenceTracePanel.h>
#include <Tools/CacheSimulator/Cache.h>
using MTV::Module;
// using MTV::LentilPanel;
using MTV::MTVMainWindow;
using MTV::MulticacheReferenceTracePanel;
using MTV::ReferenceTracePanel;

// System includes.
#include <iostream>

MTVMainWindow::MTVMainWindow()
  : trace(new TimedTraceReader),
    mfilter(new MemoryRecordFilter),
    lfilter(new LineRecordFilter),
    reftracePanel(0),
    filenameLabel(new QLabel("No trace open.")),
    traceRecordLabel(new QLabel("Record 0"))
{
  // Set the initial timeout of the trace reader to 1 second.
  trace->setInterval(1000);

  // Set up the status bar.
  this->statusBar()->addWidget(filenameLabel);
  this->statusBar()->addWidget(traceRecordLabel);

  // Connect the trace object's position reporting signal to the main
  // window.
  QObject::connect(trace.get(), SIGNAL(onTraceRecord(unsigned long)), this, SLOT(updateTracePosition(unsigned long)));

  // Set up the toolbar with playback buttons.
  QToolBar *toolbar = this->addToolBar("Playback Control");

  // Rewind button - "unplays" events continuously.
  rewindButton = new QToolButton(this);
  rewindButton->setIcon(QIcon(":/icons/rewind-button.png"));
  QObject::connect(rewindButton, SIGNAL(clicked()), this, SLOT(rewind()));
  rewindButton->setEnabled(false);
  toolbar->addWidget(rewindButton);

  // Last event button - "unplays" the last event in the trace.
  lastEventButton = new QToolButton(this);
  lastEventButton->setIcon(QIcon(":/icons/last-track-button.png"));
  QObject::connect(lastEventButton, SIGNAL(clicked()), this, SLOT(last_event()));
  lastEventButton->setEnabled(false);
  toolbar->addWidget(lastEventButton);

  // Play button - plays back trace records continuously.
  playButton = new QToolButton(this);
  playButton->setIcon(QIcon(":/icons/play-button.png"));
  QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(play()));
  playButton->setEnabled(false);
  toolbar->addWidget(playButton);

  // Next event button - plays the next event.
  nextEventButton = new QToolButton(this);
  nextEventButton->setIcon(QIcon(":/icons/next-track-button.png"));
  QObject::connect(nextEventButton, SIGNAL(clicked()), this, SLOT(next_event()));
  nextEventButton->setEnabled(false);
  toolbar->addWidget(nextEventButton);

  // Fast-forward button - plays back events continuously.
  fastForwardButton = new QToolButton(this);
  fastForwardButton->setIcon(QIcon(":/icons/ffwd-button.png"));
  QObject::connect(fastForwardButton, SIGNAL(clicked()), this, SLOT(ffwd()));
  fastForwardButton->setEnabled(false);
  toolbar->addWidget(fastForwardButton);

  // Place a speed-control dial in the toolbar.
  toolbar->addSeparator();
  QSlider *speed = new QSlider(Qt::Horizontal);

  // Record the min and max tick values of the slider.
  min = speed->minimum();
  max = speed->maximum();

  // Connect the slider to the speed control slot.
  QObject::connect(speed, SIGNAL(valueChanged(int)), this, SLOT(setTraceSpeed(int)));
  toolbar->addWidget(speed);

  // Create a "container" widget, it will contain the various MTV
  // panels, as requested by the user.
  QWidget *central = new QWidget;

  // Create a vertical box layout to use within the central widget.
  QVBoxLayout *layout = new QVBoxLayout;
  central->setLayout(layout);

  // Set the widget as the window's "central widget".  Doing so
  // reparents the central widget to this main window.  When this
  // window is destroyed, so will the child widgets.
  this->setCentralWidget(central);

  // Create menus.
  //
  // File menu.
  {
    QMenu *filemenu = this->menuBar()->addMenu("&File");
    QAction *openAction = filemenu->addAction("&Open Reference Trace...");
    QObject::connect(openAction, SIGNAL(triggered()), this, SLOT(openReferenceTrace()));

    QAction *openEventTraceAction = filemenu->addAction("Open &Event Trace...");
    QObject::connect(openEventTraceAction, SIGNAL(triggered()), this, SLOT(openEventTrace()));

    filemenu->addSeparator();

    QAction *exitAction = filemenu->addAction("E&xit");
    QObject::connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
  }

  // Modules menu.
  {
    QMenu *modulesMenu = this->menuBar()->addMenu("&Modules");

    // Reference Trace module submenu.
    {
      QMenu *reftraceSubmenu = modulesMenu->addMenu("&Reference Trace");
      QAction *newmodule = reftraceSubmenu->addAction("&New Reference Trace Module...");
      QObject::connect(newmodule, SIGNAL(triggered()), this, SLOT(openReferenceTraceModule()));

      reftraceSubmenu->addSeparator();

      QAction *newcache = reftraceSubmenu->addAction("Instantiate &New Cache...");
      QObject::connect(newcache, SIGNAL(triggered()), this, SLOT(unimplemented()));

      QAction *examplecache = reftraceSubmenu->addAction("Instantiate E&xample Cache");
      QObject::connect(examplecache, SIGNAL(triggered()), this, SLOT(createExampleCache()));
    }

    // // Lentil module submenu.
    // {
    //   QMenu *lentilSubmenu = modulesMenu->addMenu("&Lentil");
    //   QAction *newmodule = lentilSubmenu->addAction("&New Lentil Module...");
    //   QObject::connect(newmodule, SIGNAL(triggered()), this, SLOT(openLentilModule()));
    // }
  }

  // About menu.
  {
    QMenu *aboutMenu = this->menuBar()->addMenu("&About");
    QAction *aboutAction = aboutMenu->addAction("About MTV X...");
    QObject::connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
  }
}

void MTVMainWindow::checkpoint(){
  std::cout << "MTVMainWindow::checkpoint()" << std::endl;
}

void MTVMainWindow::openReferenceTrace(){
  // Get the user to select a file.  Bail out if the user clicked
  // "cancel".
  QString file = QFileDialog::getOpenFileName(this, tr("Open Reference Trace File"), ".");
  if(file == ""){
    return;
  }

  // TODO(choudhury): if there is already a trace open, warn the user
  // here, and then probably save the state of everything for
  // restoring later.
  trace->open(file.toStdString());

  // Modify the message in the status bar.
  filenameLabel->setText(file);

  // Connect the trace reader to the M- and L-filters.
  trace->addConsumer(mfilter);
  trace->addConsumer(lfilter);

  // If there is a cache simulator object, connect the trace to it.
  if(cache){
    trace->addConsumer(cache);
  }

  // Look for a region registration file in the same directory as the
  // trace file.  If found, initialize the S-filter (to pick out
  // signals from the trace).
  //
  // First find the position of the final slash in the string.
  int index = file.lastIndexOf("/");
  if(index < 0)
    index = 0;

  // // Extract the path and name the registration file.
  // const std::string registration = file.left(index).toStdString() + "/region_registration.xml";
  // std::cout << registration << std::endl;

  // // Attempt to open the registration file.
  // MTR::RegionRegistration reg;
  // if(reg.read(registration.c_str())){
  //   trace->setSignalRange(reg.barriers.base, reg.barriers.base + reg.barriers.size);
  // }

  // If a reference trace module has already been instantiated, then
  // the signalBase value will be nonzero if there are signals in the
  // trace; in that case, the trace will immediately learn what the
  // signal range is.  If there are no signals in the trace, or else
  // the reference trace module has not yet been opened, then no
  // signals will be set now; when the module is opened, and there are
  // signals specified in the module's config file, then they will be
  // registered at that time.
  if(reftracePanel and reftracePanel->getSignalBase() != 0){
    trace->setSignalRange(reftracePanel->getSignalBase(), reftracePanel->getSignalLimit());
  }

  // Connect the trace streams (and the simulator outputs, if there is
  // a cache simulator) to all the modules currently open.
  foreach(Module *i, modules){
    mfilter->addConsumer(i->memoryRecordEntryPoint());
    lfilter->addConsumer(i->lineRecordEntryPoint());

    if(cache){
      cache->Producer<CacheAccessRecord>::addConsumer(i->cacheAccessRecordEntryPoint());
      cache->Producer<CacheStatusReport>::addConsumer(i->cacheStatusReportEntryPoint());
    }

    if(trace->getSignalFilter()){
      trace->getSignalFilter()->addConsumer(i->signalEntryPoint());
    }
  }

  // Activate the play button.
  playButton->setEnabled(true);
}

void MTVMainWindow::openEventTrace(){
  if(!reftracePanel){
    QMessageBox::warning(this,
                         "Reference trace module not open yet",
                         "Please instantiate a reference trace module before opening an event trace.");
    return;
  }

  // Get the user to select a file.  Bail out if the user clicked
  // "cancel".
  QString file = QFileDialog::getOpenFileName(this, tr("Open Reference Trace File"), ".");
  if(file == ""){
    return;
  }

  // TODO(choudhury): if there is already a trace open, warn the user
  // here, and then probably save the state of everything for
  // restoring later.
  event = DeltaMementoReader::open(file.toStdString(), reftracePanel->getNetwork()->getMemorables());

  // Modify the message in the status bar.
  //
  // TODO(choudhury): this needs to play nicely with the message
  // written out by the reference trace file opening action.
  filenameLabel->setText(file);

  // Activate the appropriate buttons.
  lastEventButton->setEnabled(true);
  nextEventButton->setEnabled(true);
  fastForwardButton->setEnabled(true);
  rewindButton->setEnabled(true);
}

void MTVMainWindow::createExampleCache(){
  std::cout << "MTVMainWindow::createExampleCache()" << std::endl;

  this->useCache(Daly::defaultCache());
}

void MTVMainWindow::createDialogCache(){
  unimplemented();
}

void MTVMainWindow::openReferenceTraceModule(){
  // Get a region registration file (bail if they click cancel).
  QString file = QFileDialog::getOpenFileName(this, tr("Open region registration file"), ".");
  if(file == ""){
    return;
  }

  // Create a reference trace panel - bail out if errors.
  reftracePanel = ReferenceTracePanel::newFromSpec(file.toStdString());
  if(!reftracePanel){
    QMessageBox::warning(this,
                         "Could not open file",
                         "The selected file cannot be opened, "
                         "is not a registration file, "
                         "or is an ill-formed registration file.");
    return;
  }

  // Set color profile.
  reftracePanel->useColorProfile(MTV::PrintColorProfile());

  // Add the panel to the central widget.
  this->centralWidget()->layout()->addWidget(reftracePanel);

  // Store the pointer in the module list as well.
  modules.push_back(reftracePanel);

  // If there is already a trace open, connect it immediately, and set
  // the signal range if there is one.
  if(trace){
    mfilter->addConsumer(reftracePanel->memoryRecordEntryPoint());
    if(reftracePanel->getSignalBase() != 0){
      trace->setSignalRange(reftracePanel->getSignalBase(), reftracePanel->getSignalLimit());
    }
  }

  // If there is a cache simulator, connect that as well.  Also,
  // instantiate a cache renderer.
  if(cache){
    cache->Producer<CacheAccessRecord>::addConsumer(reftracePanel->cacheAccessRecordEntryPoint());
    cache->Producer<CacheStatusReport>::addConsumer(reftracePanel->cacheStatusReportEntryPoint());

    reftracePanel->renderCache(cache->getCache());
  }
}

void MTVMainWindow::openMulticacheReferenceTraceModule(){
  // Get a spec file.
  QString file = QFileDialog::getOpenFileName(this, tr("Open multicache specification fiel"), ".");
  if(file == ""){
    return;
  }

  // Create a multicache reference trace panel - bail if errors.
  MulticacheReferenceTracePanel *panel = MulticacheReferenceTracePanel::newFromSpec(file.toStdString());
  if(!panel){
    QMessageBox::warning(this,
                         "Could not open file",
                         "The selected file cannot be opened, "
                         "is not a multicache specification file, "
                         "or is an ill-formed specification file.");
    return;
  }

  // TODO(choudhury): set color profile for panel?

  // Add the panel to the central widget, and store the pointer in the
  // module list.
  this->centralWidget()->layout()->addWidget(panel);
  modules.push_back(panel);

  // If there is an open trace, connect it immediately.
  if(trace){
    mfilter->addConsumer(panel->memoryRecordEntryPoint());
  }
}

// void MTVMainWindow::openLentilModule(){
//   // Get a directory name (containing both the lentil simulation
//   // output, and a region registration).
//   QString dir = QFileDialog::getExistingDirectory(this, tr("Open Lentil data repository"), ".");
//   if(dir == ""){
//     return;
//   }

//   // // Construct vectors of bases, typesizes from the registered
//   // // regions.
//   // std::vector<MTR::addr_t> bases;
//   // std::vector<MTR::size_t> sizes;
//   // foreach(ArrayRegion r, reg.arrayRegions){
//   //   bases.push_back(r.base);
//   //   sizes.push_back(r.size);
//   // }

//   // Instantiate a Lentil panel.
//   LentilPanel *lentilPanel = new LentilPanel(dir.toStdString());
//   if(!lentilPanel->good()){
//     QMessageBox::warning(this,
//                          "Could not open file",
//                          "The selected directory does not contain 'region_registration.xml', "
//                          "'region_registration.xml' is not a registration file, "
//                          "is is an ill-formed registration file, "
//                          "or the necessary data files are missing.");
//     return;
//   }

//   // Add to central widget.
//   this->centralWidget()->layout()->addWidget(lentilPanel);

//   // Add it to the list of modules.
//   modules.push_back(lentilPanel);

//   // If there is a trace open, connect it immediately.
//   if(trace){
//     mfilter->addConsumer(lentilPanel->memoryRecordEntryPoint());
//   }

//   // If there is a cache simulator, connect that as well.
//   if(cache){
//     cache->Producer<CacheStatusReport>::addConsumer(lentilPanel->cacheStatusReportEntryPoint());
//   }

//   // Connect the signal source.
//   if(trace and trace->getSignalFilter()){
//     trace->getSignalFilter()->addConsumer(lentilPanel->signalEntryPoint());
//   }
// }

void MTVMainWindow::about(){
  QMessageBox::about(this, tr("About MTV X"),
                     tr("Following from MTV and MTV2, MTV X has videos again!\n\n"
                        "Written by A.N.M. Imroz Choudhury (special thanks to Kristin C. Potter).\n\n"
                        "Playback control icons, copyright 2010 Jojo Mendoza (http://www.deleket.com), used with permission under Creative Commons license.\n\n"
                        "Copyright 2010 A.N.M. Imroz Choudhury, all rights reserved."));
}

void MTVMainWindow::last_event(){
  std::cout << "MTVMainWindow::last_event()" << std::endl;
}

void MTVMainWindow::rewind(){
  std::cout << "MTVMainWindow::rewind()" << std::endl;
}

void MTVMainWindow::play(){
  // It should not be possible to click the play button unless a
  // reference trace is loaded.
  assert(trace);

  // Change the icon on the play/pause button.
  playButton->setIcon(QIcon(":/icons/pause-button.png"));

  // Save the identity of the play button (so the right buttons are
  // activated later on).
  actionButton = playButton;

  // Change the signal on the play button.
  QObject::disconnect(playButton, SIGNAL(clicked()), this, SLOT(play()));
  QObject::connect(playButton, SIGNAL(clicked()), this, SLOT(pause()));

  // Deactivate the other buttons.
  deactivateButtons(playButton);

  // Start the trace.
  trace->start();
}

void MTVMainWindow::pause(){
  static std::map<QToolButton *, std::pair<const char *, const char *> > icons;
  if(icons.size() == 0){
    icons[rewindButton] = std::make_pair(":/icons/rewind-button.png", SLOT(rewind()));
    icons[playButton] = std::make_pair(":/icons/play-button.png", SLOT(play()));
    icons[fastForwardButton] = std::make_pair(":/icons/ffwd-button.png", SLOT(ffwd()));
  }

  const char *iconResourceName = icons[actionButton].first;
  const char *slotName = icons[actionButton].second;

  // Change the icon on the "action" button (this is the button that
  // was clicked to start some kind of playback).
  actionButton->setIcon(QIcon(iconResourceName));

  // Change the signal on the button.
  QObject::disconnect(actionButton, SIGNAL(clicked()), this, SLOT(pause()));
  QObject::connect(actionButton, SIGNAL(clicked()), this, slotName);

  // If there is an event trace, activate the event-step buttons.
  if(event){
    nextEventButton->setEnabled(true);
    lastEventButton->setEnabled(true);
    rewindButton->setEnabled(true);
    fastForwardButton->setEnabled(true);
  }

  // Stop the trace(s).
  if(trace) trace->stop();
  if(event) event->stop();
}

void MTVMainWindow::ffwd(){
  // The button connected to this slot should be deactivated if there
  // is no event trace loaded.
  assert(event);

  // Change the icon on the ffwd button.
  fastForwardButton->setIcon(QIcon(":/icons/pause-button.png"));

  // Save the identity of the button.
  actionButton = fastForwardButton;

  // Change the signal on the ffwd button.
  QObject::disconnect(fastForwardButton, SIGNAL(clicked()), this, SLOT(ffwd()));
  QObject::connect(fastForwardButton, SIGNAL(clicked()), this, SLOT(pause()));
  
  // Deactivate the other buttons.
  deactivateButtons(fastForwardButton);

  // NOTE(choudhury): there is no need to stop the reference trace
  // here - the system is built so that this slot cannot be called
  // unless the system is "stopped".
  event->start();
}

void MTVMainWindow::next_event(){
  std::cout << "MTVMainWindow::next_event()" << std::endl;
}

void MTVMainWindow::setTraceSpeed(int value){
  // At the "fast end", simply unhinge the timer (let it run as fast
  // as possible).
  if(value == max){
    if(trace) trace->setInterval(0);
    if(event) event->setInterval(0);
    return;
  }

  // Normalize the slider value to [0.0, 1.0].
  const float x = static_cast<float>(value - min) / (max - min);

  // Set the timeout according to a cubic decay.
  const int msec = 1000.0*(1.0 - x)*(1.0 - x)*(1.0 - x);
  if(trace) trace->setInterval(msec);
  if(event) event->setInterval(msec);
}

void MTVMainWindow::updateTracePosition(unsigned long pos){
  std::stringstream ss;
  ss << "Record " << pos;
  traceRecordLabel->setText(ss.str().c_str());
  //std::cout << ss.str() << std::endl;
}

bool MTVMainWindow::useCache(Cache::ptr _cache){
  // Only allow a single cache per session.
  //
  // TODO(choudhury): clearly the user should be able to swap out the
  // cache.  Doing so needs to somehow update all the graphics
  // onscreen.  We need a "clear cache out" signal so the modules can
  // reset what they need to.
  if(cache){
    std::cout << "Already have a cache!  bailing" << std::endl;
    return false;
  }

  // Create a cache simulator filter.
  cache = CacheSimulator::ptr(new CacheSimulator(_cache));

  // Connect the M-filter to the cache simulator.
  mfilter->addConsumer(cache);

  // Connect the cache simulator outputs to all the modules.
  foreach(Module *i, modules){
    cache->Producer<CacheAccessRecord>::addConsumer(i->cacheAccessRecordEntryPoint());
    cache->Producer<CacheStatusReport>::addConsumer(i->cacheStatusReportEntryPoint());
  }

  // If there is a reference trace module, tell it to create a cache
  // renderer.
  if(reftracePanel){
    reftracePanel->renderCache(_cache);
  }

  return true;
}

void MTVMainWindow::deactivateButtons(QToolButton *skip){
  static QToolButton *buttons[] = { rewindButton, lastEventButton, playButton, nextEventButton, fastForwardButton };
  for(unsigned i=0; i<sizeof(buttons) / sizeof(buttons[0]); i++){
    if(buttons[i] != skip){
      buttons[i]->setEnabled(false);
    }
  }
}

void MTVMainWindow::unimplemented(){
  std::cerr << "unimplemented!" << std::endl;
}
