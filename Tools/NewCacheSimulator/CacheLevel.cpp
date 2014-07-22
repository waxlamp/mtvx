// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCache.cpp

#include <Tools/NewCacheSimulator/CacheLevel.h>
#include <Tools/NewCacheSimulator/NewCache.h>
using MTV::BeladyAlgorithmCacheSet;
using MTV::NewCache;
using MTV::CacheLevel;
using MTV::DirectMappedCacheLevel;
using MTV::Eviction;
using MTV::OrderedCacheSet;
using MTV::OPTCacheSet;
using MTV::RandomReplacementCacheSet;
using MTV::SetAssociativeCacheLevel;

CacheLevel::CacheLevel(WritePolicy write_pol)
  : write_pol(write_pol)
{}

void CacheLevel::write_back(boost::shared_ptr<NewCache> cache, unsigned L, uint64_t block_addr){
  cache->write_at_level(L, block_addr*cache->block_size());
}

DirectMappedCacheLevel::DirectMappedCacheLevel(unsigned level, WritePolicy write_policy, unsigned _num_blocks)
  : CacheLevel(write_policy),
    num_blocks(_num_blocks),
    lookup(num_blocks, blocks.end())
{}

Eviction DirectMappedCacheLevel::allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level){
  // If the entry is present already, this function should not have
  // been called - signal by throwing an exception.
  CacheLevel::iterator i = this->find(block_addr);
  if(i != blocks.end()){
    throw AllocateExisting();
  }

  // Set up an eviction report object - initially set it up to
  // report no eviction, and a non-dirty block (modify later as
  // needed).
  //
  // Eviction e;
  // e.eviction = false;
  // e.writeback = (this->write_policy() == WriteBack);
  // e.dirty = false;
  // e.cell = i->cell;
  Eviction e(this->write_policy() == WriteBack, i->cell);

  // Compute the target block index.
  const unsigned index = block_addr % num_blocks;

  // Allocate the block to the target index.
  if(blocks.size() < num_blocks){
    // If the target block is unmapped, allocate the new block to it
    // immediately, and install an iterator to the new element in
    // the lookup table.
    const unsigned new_cell = blocks.size();
    blocks.push_back(CacheBlock(new_cell));
    lookup[index] = blocks.end();
    lookup[index]--;

    // Place the correct values in the new entry.
    lookup[index]->addr = block_addr;
    lookup[index]->dirty = false;
  }
  else{
    // If the target block is mapped, whatever is there needs to be
    // evicted before the block is allocated.
    //
    // First record the appropriate details for the eviction report.
    e.eviction = true;
    e.dirty = lookup[index]->dirty;
    e.block_addr = lookup[index]->addr;
    e.cell = lookup[index]->cell;

    // If the evicted block is dirty, perform a write back.
    if(lookup[index]->dirty){
      this->write_back(cache, level + 1, lookup[index]->addr);
    }

    // Next fill in the block with the appropriate information
    // (effectively performing the eviction and allocating the
    // block).
    lookup[index]->addr = block_addr;
    lookup[index]->dirty = false;
  }

  return e;
}

Eviction OrderedCacheSet::allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level){
  // Check the lookup table and signal an error if the sought block
  // is already in the level.
  boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
  if(i != lookup.end()){
    throw AllocateExisting();
  }

  // Create an eviction object.
  Eviction e(this->write_policy() == WriteBack, -1);

  // If the level is not yet full, allocate the block immediately.
  if(blocks.size() < num_blocks){
    // Create a cache block record.
    const unsigned new_cell = start_cell + blocks.size();
    CacheBlock new_block(new_cell);
    new_block.addr = block_addr;

    // Place it in the block list.
    //
    // Ordering::insert(blocks, new_block);
    repl->insert(blocks, new_block);

    // Place an iterator in the lookup table.
    //
    // lookup[block_addr] = Ordering::last_touched(blocks);
    lookup[block_addr] = repl->last_touched(blocks);

    // Set the cell number on the eviction report.
    e.cell = new_cell;
  }
  else{
    // Fill in the appropriate eviction information.
    e.eviction = true;
    // e.dirty = Ordering::victim(blocks).dirty;
    // e.block_addr = Ordering::victim(blocks).addr;
    // e.cell = Ordering::victim(blocks).cell;
    e.dirty = repl->victim(blocks).dirty;
    e.block_addr = repl->victim(blocks).addr;
    e.cell = repl->victim(blocks).cell;

    // If the victim block is dirty, perform a write back.
    if(repl->victim(blocks).dirty){
      this->write_back(cache, level + 1, repl->victim(blocks).addr);
    }

    // Perform the eviction
    //
    // Ordering::evict(blocks);
    repl->evict(blocks);

    // Modify the lookup table information.
    //
    // First assert that the evicted block has an entry in the
    // table.
    i = lookup.find(e.block_addr);
    if(i == lookup.end()){
      throw "illegal state";
    }

    // Then, install the OLD iterator under the NEW block address
    // (so that the new address points to the old cache block
    // entry).
    lookup[block_addr] = i->second;

    // Finally, delete the old entry.
    lookup.erase(i);
  }

  // CacheBlock& b = blocks.front();
  CacheLevel::iterator b = repl->last_touched(blocks);
  b->addr = block_addr;
  b->dirty = false;

  return e;
}

Eviction RandomReplacementCacheSet::allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level){
  // Check for error condition - allocating a block that is already
  // present.
  boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
  if(i != lookup.end()){
    throw AllocateExisting();
  }

  // Eviction e;
  // e.eviction = false;
  // e.writeback = this->write_policy() == WriteBack;
  // e.dirty = false;
  // e.cell = i->second->cell;
  Eviction e(this->write_policy() == WriteBack, -1);

  if(blocks.size() < num_blocks){
    // If the blocks have not all been filled up yet, allocate the
    // requested block immediately.
    unsigned new_cell = blocks.size();
    blocks.push_back(CacheBlock(new_cell));

    // Get an iterator to the new block.
    CacheLevel::iterator new_i = blocks.end();
    new_i--;

    // Point the lookup table to the new entry;
    lookup[block_addr] = new_i;

    // Place an iterator in the random access vector.
    random_access.push_back(new_i);

    // Fill in the block with the appropriate information.
    new_i->addr = block_addr;
    new_i->dirty = false;
  }
  else{
    // Select a block (at random) to evict, then replace the lookup
    // info with the info for that block.
    //
    // TODO(choudhury): see if boost has RNG classes.  If so, keep
    // one private to this class.
    const unsigned index = drand48() * blocks.size();
    CacheLevel::iterator i = random_access[index];

    // Update the eviction data.
    e.eviction = true;
    e.dirty = i->dirty;
    e.block_addr = i->addr;
    e.cell = i->cell;
      
    // If the block is dirty, perform a write back.
    if(i->dirty){
      this->write_back(cache, level + 1, i->addr);
    }

    // Update the lookup table.
    //
    // First, delete the old record.
    lookup.erase(lookup.find(i->addr));

    // Install a new record.
    lookup[block_addr] = i;

    // Install the new block information.
    i->addr = block_addr;
    i->dirty = false;
  }

  return e;
}

Eviction BeladyAlgorithmCacheSet::allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level){
  // Check the lookup table for the block - if it is there already,
  // signal an error.
  boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
  if(i != lookup.end()){
    throw AllocateExisting();
  }

  // Create an eviction object.
  Eviction e(this->write_policy() == WriteBack, -1);

  if(blocks.size() < num_blocks){
    // If the level is not yet full, allocate the block immiedately.
    //
    // TODO(choudhury): grab a reference to the entry instead of
    // looking it up multiple times.
    const unsigned new_cell = start_cell + blocks.size();
    blocks.push_back(CacheBlock(new_cell));
    lookup[block_addr] = blocks.end();
    lookup[block_addr]--;

    // Place the correct values in the new entry.
    lookup[block_addr]->addr = block_addr;
    lookup[block_addr]->dirty = false;
  }
  else{
    // Evict the chosen block.
    //
    // First select the victim block by address.
    const uint64_t victim_block_addr = this->select_victim();
    CacheLevel::iterator victim = lookup[victim_block_addr];

    // Fill in the eviction object's information.
    e.eviction = true;
    e.dirty = victim->dirty;
    e.block_addr = victim->addr;
    e.cell = victim->cell;

    // If the evicted block is dirty, perform a write back.
    if(victim->dirty){
      this->write_back(cache, level + 1, victim->addr);
    }

    // Emplace the new block information.
    //
    // TODO(choudhury): the dirty flag should not automatically be
    // false, it should actually be equal to whatever the flag is
    // set to in the next level in which it resides (or false, if
    // the block is found in main memory).
    victim->addr = block_addr;
    victim->dirty = false;

    // Remove the old entry from the lookup table and install the
    // new one.
    lookup.erase(victim_block_addr);
    lookup[block_addr] = victim;
  }

  return e;
}

SetAssociativeCacheLevel::SetAssociativeCacheLevel(const std::vector<CacheLevel::ptr>& init_sets)
  // NOTE(choudhury): supply dummy values to the base class
  // constructor, as this class is just an adapter to the CacheLevel
  // objects contained in the "init_sets" vector.
  : CacheLevel(CacheLevel::WriteThrough),
    sets(init_sets)
{}
