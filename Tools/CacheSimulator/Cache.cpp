// Copyright 2010 A.N.M. Imroz Choudhury
//
// Cache.cpp - Implementation for cache simulator.

// MTV includes.
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/CacheSimulator/CacheConstructor.h>
using Daly::BlockRecord;
using Daly::Cache;
using Daly::CacheHitRecord;
using Daly::CacheLevel;
using Daly::ModtimeTable;
using Daly::ApproxOPT;
using Daly::ApproxPES;

// System includes.
#include <algorithm>
#include <iostream>
#include <limits>

unsigned CacheLevel::find(MTR::addr_t blockAddr) const throw(CacheLevel::BlockNotFound) {
  // Compute the set this block should belong to.
  unsigned setIndex = blockAddr % _numSets;

  // std::cout << "looking for block " << std::hex << (blockAddr * 32) << " in set " << std::dec << setIndex << std::endl;

  // Sweep that set looking for this block.
  for(unsigned i=this->blockIndex(setIndex,0); i < this->blockIndex(setIndex+1,0); i++){
    // std::cout << std::hex << (blocks[i].addr * 32) << std::dec << std::endl;

    if(blocks[i].mapped && blocks[i].addr == blockAddr){
      // std::cout << "found block in level" << std::endl;

      return i;
    }
  }

  // std::cout << "did not find block in level" << std::endl;

  // Reaching here means the block is not present.
  throw BlockNotFound();
}

void Cache::addCacheLevel(unsigned size, unsigned associativity, WritePolicy writePolicy){
  // The size should be a multiple of the cache's block size.
  if(size % _blocksize != 0){
    throw exceptions::BlocksizeDoesNotDivideSize(levels.size(), size, _blocksize);
  }

  // The associativity should be such that the blocks are split into 1 or more equal sized sets.
  unsigned numblocks = size / _blocksize;
  if(numblocks % associativity != 0){
    throw exceptions::UnevenSetSize(levels.size(), numblocks, associativity);
  }

  // The new cache level should be strictly larger than the last cache level.
  if(levels.size() > 0){
    unsigned lastCacheSize = (*(levels.end() - 1))->size();

    if(size <= lastCacheSize){
      throw exceptions::CacheTooSmall(levels.size(), size, lastCacheSize);
    }
  }

  // Create the new cache level.
  CacheLevel::ptr p(new CacheLevel(size, _blocksize, associativity, writePolicy));
  levels.push_back(p);

  // // Set the mod time for each new block to be 1 (since a reported
  // // modtime of 0 is a special signal).
  // for(std::vector<BlockRecord>::iterator i = p->blocksBegin(); i != p->blocksEnd(); i++){
  //   modTime[i->addr] = 1;
  // }
}

void Cache::infoReport() const {
  std::cout << "load: "
            << _hitInfo.size() << " hits, "
            << _evictionInfo.size() << " evictions, "
            << _entranceInfo.size() << " entrances" << std::endl;
  for(unsigned i=0; i<_hitInfo.size(); i++){
    std::cout << "Hit: " << std::hex << _hitInfo[i].addr << std::dec << " at level " << _hitInfo[i].L << std::endl;
  }
  for(unsigned i=0; i<_evictionInfo.size(); i++){
    std::cout << "Eviction: " << std::hex << _evictionInfo[i].blockaddr <<  " (" << _evictionInfo[i].blockaddr*_blocksize << ")" << std::dec << " at level " << _evictionInfo[i].L << std::endl;
  }
  for(unsigned i=0; i<_entranceInfo.size(); i++){
    std::cout << "Entrance: " << std::hex << _entranceInfo[i].blockaddr <<  " (" << _entranceInfo[i].addr << ")" << std::dec << " at level " << _entranceInfo[i].L << std::endl;
  }
}

void Cache::addCacheLevel(CacheLevel::ptr level){
  levels.push_back(level);
}

void Cache::load(MTR::addr_t addr){
  // std::cout << "load " << std::hex << addr << std::dec << std::endl;

  // Clear the hit records
  _hitInfo.resize(0);
  _evictionInfo.resize(0);
  _entranceInfo.resize(0);

  // Compute the *block* address
  MTR::addr_t blockAddr = addr / _blocksize;
  
  // Find the level of cache in which the block is found.
  CacheHitRecord found = this->find(blockAddr);
  found.addr = addr;
  found.op = 'R';

  // If not a cache miss, update the timestamp on the read block, and
  // add the accessed address to the block's list of addresses.
  if(found.L < levels.size()){
    // levels[found.L]->block(found.cell).modTime = stamp.nextTick();

    // this->updateModtime(levels[found.L]->block(found.cell).addr);
    // levels[found.L]->block(found.cell).modtime = stamp.nextTick();
    modtime->update(blockAddr);
  }

  // For each level in which the block was not found, place it there.
  //
  // NOTE(choudhury): allocate() may call evict(), which will record
  // any evictions.
  //
  // TODO(choudhury): we need to allocate the block in these cache
  // levels, but do we need to record a hit?  These should count as
  // *misses* in those levels.
  for(unsigned int L=0; L<found.L; L++){
    unsigned newblock = this->allocate(L, blockAddr);
    // _hitInfo.push_back(CacheHitRecord(addr, L, newblock, 'R', levels[L]));
    _entranceInfo.push_back(CacheEntranceRecord(addr, L, newblock));
  }

  // Record the hit in the appropriate level of cache (a read miss is
  // a "hit" to the level below the slowest cache, i.e. main memory).
  _hitInfo.push_back(found);

  // this->infoReport();
}

void Cache::store(MTR::addr_t addr){
  // std::cout << "store " << std::hex << addr << std::dec << std::endl;

  // Clear the hit records.
  _hitInfo.resize(0);
  _evictionInfo.resize(0);
  _entranceInfo.resize(0);

  // Initiate a write in the fastest cache level.
  writeAtLevel(0, addr);

  // this->infoReport();
}

Cache::Snapshot Cache::state() const {
  Snapshot snap;
  snap.state.resize(levels.size());
  for(unsigned i=0; i<levels.size(); i++){
    snap.state[i] = std::vector<BlockRecord>(levels[i]->blocks.begin(), levels[i]->blocks.end());
  }
  
  return snap;
}

void Cache::setState(const Cache::Snapshot& snap){
#ifndef NDEBUG
  if(snap.state.size() != levels.size()){
    throw exceptions::CacheSnapshotWrongSize();
  }
  
  for(unsigned i=0; i<snap.state.size(); i++){
    if(snap.state[i].size() != levels[i]->blocks.size()){
      throw exceptions::CacheSnapshotWrongSize();
    }
  }
#endif
  
  for(unsigned i=0; i<snap.state.size(); i++){
    levels[i]->blocks = std::vector<BlockRecord>(snap.state[i].begin(), snap.state[i].end());
  }
}

Cache::ptr Cache::newFromSpec(const std::string& specfile, TraceReader::ptr reader){
  CacheConstructor c(specfile);
  if(c.error() != ""){
    std::cerr << "[Cache] error: " << c.error() << std::endl;
    abort();
  }
  return c.constructCache(reader);
}

Cache::ptr Cache::newFromSpec(const std::string& specfile, TraceReader::ptr reader, const std::string& bsfile, unsigned numstreams){
  CacheConstructor c(specfile);
  if(c.error() != ""){
    std::cerr << "[Cache] error: " << c.error() << std::endl;
    abort();
  }
  return c.constructCache(reader, bsfile, numstreams);
}

CacheHitRecord Cache::find(MTR::addr_t blockAddr) const {
  for(unsigned L=0; L<levels.size(); L++){
    try{
      // This statement throws a blockNotFound exception if the block
      // is not present.
      unsigned cacheBlockIndex = levels[L]->find(blockAddr);

      // std::cout << "block at " << std::hex << blockAddr << std::dec << " found in level " << L << std::endl;

      // So if we reach here, the block is found and the function can
      // return.
      //
      // return CacheHitRecord(0, L, cacheBlockIndex, 'X', levels[L]);
      return CacheHitRecord(0, L, cacheBlockIndex, 'X');
    }
    // If the cacheLevel doesn't have the requested block, continue
    // looking in the next level.
    catch(CacheLevel::BlockNotFound){}
  }

  // std::cout << "block at " << std::hex << blockAddr << std::dec << " not found" << std::endl;

  // Reaching here means the block was not found in any cache level
  // (i.e. a read/write miss).
  return CacheHitRecord(0, levels.size(), 0);
}

// void Cache::updateModtime(MTR::addr_t blockAddr){
//   modTime[blockAddr] = stamp.nextTick();
// }

// void Cache::dumpModtime() const {
//   std::cout << "starting modtime dump" << std::endl;
//   for(boost::unordered_map<MTR::addr_t, unsigned long long>::const_iterator i = modTime.begin();
//       i != modTime.end();
//       i++){
//     std::cout << std::hex << (i->first * _blocksize) << std::dec << " -> " << i->second << std::endl;
//   }
//   std::cout << "done with modtime dump" << std::endl;
// }

void Cache::writeAtLevel(unsigned L, MTR::addr_t addr){
  // When the loop below ends, hit_level and hit_cell will contain the
  // proper write hit information for the phase of this operation
  // occurring in each cache level.  These values are initialized with
  // a write miss because if all the looping fails to find any block
  // in the cache, this represents a write miss (the special case of
  // called writeAtLevel() on too high a number, i.e., to main memory,
  // is also covered by this initialization).
  unsigned hit_level = levels.size();
  unsigned hit_cell = 0;

  // Compute the block address of addr.
  MTR::addr_t blockAddr = addr / _blocksize;

  // Repeat the actions in the loop until they have been performed for
  // a write-back level.
  for(unsigned level=L; level<levels.size(); level++){
    CacheLevel::ptr c = levels[level];
    
    // See if the block exists in this cache level.
    try{
      // Record possible hit information in this level.
      hit_cell = c->find(blockAddr);
      hit_level = level;

      //std::cout << "Found block in level " << hit_level << ", cell " << hit_cell << std::endl;

      // Modify a found block's timestamp and dirtiness.
      //
      // this->updateModtime(c->blocks[hit_cell].addr);
      // c->blocks[hit_cell].modtime = stamp.nextTick();
      modtime->update(blockAddr);

      c->blocks[hit_cell].dirty = true;
    }
    catch(CacheLevel::BlockNotFound){
      // The block is not present; allocate the block in this level,
      // and note the entrance of the address to this level of the
      // cache, before continuing (allocate() updates the timestamp on
      // the new block) and mark it dirty.
      hit_cell = this->allocate(level, blockAddr);
      c->blocks[hit_cell].dirty = true;
      _entranceInfo.push_back(CacheEntranceRecord(addr, L, hit_cell));

      //std::cout << "Block not found in level " << level << ", allocating at cell " << hit_cell << std::endl;

      // This block takes care of a special case- if the slowest cache
      // level is write through and we are in this cache block, the
      // system will need to write the value through to main memory.
      // This is a cache miss; unfortunately, this framework does not
      // automatically catch this occurrence.
      if(L == levels.size() - 1 && c->_writePolicy == WriteThrough){
	hit_level = levels.size();
	hit_cell = 0;
      }
    }

    // TODO(choudhury): why is this here?
    //
    // hit_level = level;

    // Record the write hit.
    //
    // TODO(choudhury): what if hit_level == levels.size()?
    //
    // this->_hitInfo.push_back(CacheHitRecord(addr, hit_level, hit_cell, 'W', hit_level == levels.size() ? CacheLevel::ptr() : levels[hit_level]));
    this->_hitInfo.push_back(CacheHitRecord(addr, hit_level, hit_cell, 'W'));

    // If c is a write-back level, don't continue the actions to
    // slower levels, just bail out.
    if(c->_writePolicy == WriteBack){
      break;
    }
  }
}

unsigned Cache::allocate(unsigned L, MTR::addr_t blockAddr){
  // Look for an unmapped block in the appropriate set of the cache.
  CacheLevel::ptr level = levels[L];
  unsigned setIndex = blockAddr % level->_numSets;
      
  // std::cout << "allocating " << std::hex << (blockAddr * _blocksize) << std::dec << " in level " << L << ", set " << setIndex << std::endl;

#if 0
  if(L == 0){
    std::cout << "level: " << L << std::endl;
    std::cout << "blockAddr: 0x" << std::hex << blockAddr << std::endl;
    std::cout << "setIndex: " << std::dec << setIndex << std::endl;
  }
#endif

  for(unsigned i=0; i<level->_numBlocksPerSet; i++){
    BlockRecord& blk = level->block(setIndex, i);
    if(!blk.mapped){
      blk.addr = blockAddr;
      blk.mapped = true;
      blk.dirty = false;

      // blk.modtime = stamp.nextTick();
      // this->updateModtime(blk.addr);
      modtime->update(blockAddr);

#if 0
      if(L == 0)
	std::cout << "unmapped block found: " << level.blockIndex(setIndex, i) << std::endl << std::endl;
#endif

      return level->blockIndex(setIndex, i);
    }
  }

  // No unmapped blocks were found, so evict a block and place the
  // block there.
  BlockRecord& blk = this->evict(L, setIndex);
  blk.addr = blockAddr;
  blk.mapped = true;
  blk.dirty = false;

  // blk.modtime = stamp.nextTick();
  // this->updateModtime(blk.addr);
  modtime->update(blockAddr);

#if 0
  if(L == 0)
    std::cout << "mapped block found: " << (&blk - &level.blocks[0]) << std::endl << std::endl;
#endif

  return &blk - &level->blocks[0];
}

BlockRecord& Cache::evict(unsigned L, unsigned setIndex){
#ifndef NDEBUG
  // This function assumes that the entire set in question is mapped
  // (eviction should not be happening in any case if there are
  // unmapped blocks).  This debugging block will do the check and
  // throw an exception if there are unmapped blocks in the set.

  for(unsigned i=0; i<levels[L]->_numBlocksPerSet; i++){
    BlockRecord& blk = levels[L]->block(setIndex, i);
    if(!blk.mapped){
      throw exceptions::EvictionWithUnmappedBlocks(L, setIndex, i);
    }
  }
#endif

  // // Find the least recently used block in the target set (this is an
  // // LRU replacement policy; code architecture needs to change if this
  // // policy needs to vary at runtime).
  // CacheLevel::ptr level = levels[L];
  // // std::vector<BlockRecord>::iterator LRU_block = min_element(level->blocks.begin() + level->blockIndex(setIndex,0),
  // //       						     level->blocks.begin() + level->blockIndex(setIndex+1,0));

  // std::vector<BlockRecord>::iterator LRU_block;
  // long long unsigned oldest = std::numeric_limits<unsigned long long>::max();
  // for(std::vector<BlockRecord>::iterator i = level->blocks.begin() + level->blockIndex(setIndex,0); i != level->blocks.begin() + level->blockIndex(setIndex+1,0); i++){
  //   assert(modTime.find(i->addr) != modTime.end());
  //   if(modTime[i->addr] < oldest){
  //     oldest = modTime[i->addr];
  //     LRU_block = i;
  //   }
  // }

  std::pair<std::vector<BlockRecord>::iterator, bool> pair = _evictionPolicy->select_eviction_block(this, L, setIndex);
  std::vector<BlockRecord>::iterator victim = pair.first;
  const bool writeback = pair.second;

  // std::cout << "evicting block at " << (LRU_block->addr * _blocksize) << " (modtime " << oldest << ")" << std::endl;

  // If the block is dirty and the cache is write-back, its contents
  // need to post through the cache levels to the next write-back
  // cache level (or main memory).  This is the same as asking for a
  // write to be initiated to the evicted block at the cache level one
  // higher than the evicting level.
  const bool do_writeback = victim->dirty and writeback;
  if(do_writeback){
    writeAtLevel(L+1, victim->addr);
  }

  // Unmap the block.  This really doesn't have any effect on the
  // logic of simulations but it's the "proper" thing to do here
  // (since, if the simulation were interrupted after this step, this
  // would be the way of knowing that an eviction had occurred).
  victim->mapped = false;

  _evictionInfo.push_back(CacheEvictionRecord(L, victim->addr, writeback, victim->dirty));

  // Return the block reference
  return *victim;
}

std::pair<std::vector<BlockRecord>::iterator, bool> ApproxOPT::select_eviction_block(Cache *c, unsigned L, unsigned setIndex){
  const uint64_t point = reader->getTracePoint();
  log << "point is " << point << std::endl;

  // Get the level and associativity set, and make a container set
  // from the entries.
  boost::unordered_set<MTR::addr_t> blocks;
  CacheLevel::ptr level = c->level(L);
  for(std::vector<BlockRecord>::iterator i = level->blocksBegin() + level->blockIndex(setIndex,0);
      i != level->blocksBegin() + level->blockIndex(setIndex+1,0);
      i++){
    blocks.insert(i->addr);
    log << "inserting block " << i->addr << std::endl;
  }

  // Rebuffer the trace reader, so we have a set of upcoming
  // addresses to look at.
  const unsigned size = reader->rebuffer();

  // Sweep through the addresses, tossing out entries from the
  // unordered set as their block addresses occur in the trace.
  // Continue until either (a) the trace runs out or (b) there is
  // only one entry left in the set.
  const std::vector<MTR::Record>& upcoming = reader->getBuffer();
  unsigned i;
  for(i=0; i<size; i++){
    if(blocks.size() == 1){
      log << "PASS: solved in " << i << " readaheads" << std::endl;
      break;
    }

    const uint64_t blockaddr = upcoming[i].addr / c->block_size();
    log << "block " << blockaddr << " next reference at " << (point + i) << std::endl;;
    blocks.erase(blockaddr);
  }

  if(blocks.size() > 1){
    log << "FAIL: " << blocks.size() << " blocks remaining in set" << std::endl;
  }

  // Find the iterator in the cache level for the first item in the
  // unordered set - this is either the unique item that is the
  // solution to the OPT problem, or else it's one of the ones that
  // remained past the end of the records array (and is therefore a
  // solution to the ApproxOPT problem).
  const MTR::addr_t blockaddr = *(blocks.begin());
  log << "Selected block " << blockaddr << " for eviction (next reference at point " << (point + i) << ")" << std::endl;
  for(std::vector<BlockRecord>::iterator i = level->blocksBegin() + level->blockIndex(setIndex,0);
      i != level->blocksBegin() + level->blockIndex(setIndex+1,0);
      i++){
    if(i->addr == blockaddr){
      return std::make_pair(i, level->writePolicy() == WriteBack);
    }
  }

  assert(false);
  return std::make_pair(level->blocksEnd(), false);
}

std::pair<std::vector<BlockRecord>::iterator, bool> ApproxPES::select_eviction_block(Cache *c, unsigned L, unsigned setIndex){
  // Get the level and associativity set, and make a container set
  // from the entries.
  boost::unordered_set<MTR::addr_t> blocks;

  CacheLevel::ptr level = c->level(L);
  for(std::vector<BlockRecord>::iterator i = level->blocksBegin() + level->blockIndex(setIndex,0);
      i != level->blocksBegin() + level->blockIndex(setIndex+1,0);
      i++){
    blocks.insert(i->addr);
  }

  // Rebuffer the trace reader, so we have a set of upcoming
  // addresses to look at.
  const unsigned size = reader->rebuffer();

  // Sweep through the addresses, looking for entries from the
  // unordered set as their block addresses occur in the trace.
  const std::vector<MTR::Record>& upcoming = reader->getBuffer();
  boost::unordered_set<MTR::addr_t>::const_iterator nearest = blocks.end();
  for(unsigned i=0; i<size; i++){
    nearest = blocks.find(upcoming[i].addr / c->block_size());

    // We have found a block, so break out of the loop here.
    if(nearest != blocks.end()){
      break;
    }
  }

  // In case the loop above terminated without finding any block,
  // select one at random (i.e., the first one in the set).
  if(nearest == blocks.end()){
    nearest = blocks.begin();
  }

  // Find the iterator in the cache level for the selected block -
  // this is either the unique item that is the solution to the PES
  // problem, or else it's one of the candidates (and is therefore a
  // solution to the ApproxPES problem).
  const MTR::addr_t blockaddr = *nearest;
  for(std::vector<BlockRecord>::iterator i = level->blocksBegin() + level->blockIndex(setIndex,0);
      i != level->blocksBegin() + level->blockIndex(setIndex+1,0);
      i++){
    if(i->addr == blockaddr){
      return std::make_pair(i, level->writePolicy() == WriteBack);
    }
  }

  assert(false);
  return std::make_pair(level->blocksEnd(), false);
}

Daly::Cache::ptr Daly::defaultCache(){
  static Daly::Cache::ptr _theCache;
  if(!_theCache){
    try{
#if 1
      // This is a very small example cache.
      _theCache = Daly::Cache::ptr(new Daly::Cache(32, WriteAllocate, boost::make_shared<LRU>(), boost::make_shared<ModtimeTable>()));
      _theCache->addCacheLevel(8*32, 2, WriteThrough);
      _theCache->addCacheLevel(16*32, 4, WriteBack);
#else
      // This is a G5 cache (too big for useful demonstrations of MTV)
      _theCache = Daly::Cache::ptr(new Daly::Cache(128, WriteAllocate));
      _theCache->addCacheLevel(32*1024, 2, WriteThrough);
      _theCache->addCacheLevel(512*1024, 8, WriteBack);
#endif
    }
    catch(Daly::exceptions::SetupProblem){
      std::cerr << "warning: cache parameters are bad, returning a null pointer!" << std::endl;
    }
  }

  return _theCache;
}
