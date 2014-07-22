// Copyright 2010 A.N.M. Imroz Choudhury
//
// LentilNetwork.cpp

// MTV includes.
#include <Modules/Lentil/Networks/LentilNetwork.h>
using MTV::AddressRangePass;
using MTV::DataRetrievalCommander;
using MTV::CacheAccessRecord;
using MTV::CacheStatusReport;
using MTV::Ground;
using MTV::LentilNetwork;
using MTV::ParticleSelector;

LentilNetwork::LentilNetwork(const std::string& dir)
  : MRepeater(new Repeater<MTR::Record>),
    LGround(Ground<MTR::Record>::instance()),
    CSRepeater(new Repeater<CacheStatusReport>),
    CAGround(Ground<CacheAccessRecord>::instance()),
    dataUpdateCommander(DataRetrievalCommander::New(0)),
    // pRange(pBase.size()),
    // gridNodeRange(gnBase.size()),
    // pSelector(pBase.size()),
    // gnSelector(gnBase.size()),
    renderer(LentilDataRenderer::New(dir)),
    ok(false)
{
  // Open the region registration.
  MTR::RegionRegistration reg;

  // TODO(choudhury): this module shouldn't assume the name of the
  // registration file.
  if(!reg.read((dir + "/region_registration.xml").c_str())){
    return;
  }

  // Grab the list of regions in the registration file.
  const std::vector<MTR::ArrayRegion>& arrays = reg.arrayRegions;
  pRange.resize(arrays.size());
  pSelector.resize(arrays.size());

  std::cout << arrays.size() << " arrays." << std::endl;

  // Get the number of particles.
  const unsigned numParticles = renderer->numParticles();

  std::cout << numParticles << " particles." << std::endl;

  // Instantiate range filters based on the base addresses for the
  // data ranges.
  for(unsigned i=0; i<pRange.size(); i++){
    std::cout << "array " << i << ": " << std::hex << arrays[i].base << " -> " << (arrays[i].base + numParticles*arrays[i].type) << std::endl;
    pRange[i] = AddressRangePass::ptr(new AddressRangePass(arrays[i].base, arrays[i].base + numParticles*arrays[i].type));
  }

  // Instantiate particle selectors based on the base address and the
  // type size.
  for(unsigned i=0; i<pSelector.size(); i++){
    pSelector[i] = ParticleSelector::New(arrays[i].base, arrays[i].type);
  }

  // Connect the network together.
  //
  // The memory records and cache statuses must go to the address
  // range filters.
  foreach(AddressRangePass::ptr p, pRange){
    MRepeater->addConsumer(p);
    CSRepeater->addConsumer(p);
  }

  // The address filters send their output (both M-records and cache
  // statuses) to the particle selectors.
  //
  // NOTE(choudhury): order matters here.  The particle selector
  // requires both a reference record and a cache status report to
  // generate a rendering command.  The protocol is that when it
  // consumes a reference record, it updates some internal state, then
  // when it consumes the status report, it will produce the rendering
  // command.
  for(unsigned i=0; i<pRange.size(); i++){
    pRange[i]->Producer<MTR::Record>::addConsumer(pSelector[i]);
    pRange[i]->Producer<CacheStatusReport>::addConsumer(pSelector[i]);
  }

  // Particle selectors send rendering commands to the renderer.
  foreach(ParticleSelector::ptr p, pSelector){
    p->addConsumer(renderer);
  }

  // The data retrieval trigger tells the renderer when to read in new
  // data from disk.
  dataUpdateCommander->addConsumer(renderer);

  // TODO(choudhury): need to add a datapath parallel to the particle
  // one for handling grid node accesses.

  // Indicate that everything is OK.
  ok = true;
}
