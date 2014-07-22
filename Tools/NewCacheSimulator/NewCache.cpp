// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCache.cpp

#include <Tools/NewCacheSimulator/NewCache.h>
#include <Tools/NewCacheSimulator/NewCacheConstructor.h>
using MTV::CacheLevel;
using MTV::NewCache;
using MTV::NewCacheConstructor;

NewCache::ptr NewCache::newFromSpec(const std::string& specfile, TraceReader::ptr trace, BlockStreamReader::ptr bsreader, std::string& error){
  NewCacheConstructor c(specfile);
  if(c.error() != ""){
    error = c.error();
    return NewCache::ptr();
  }

  return c.constructCache(trace, bsreader);
}

CacheLevel::ptr NewCache::add_level(unsigned num_blocks, unsigned num_sets, CacheLevel::WritePolicy write_policy, CacheLevel::ReplacementPolicy repl_policy){
  // Make sure the requested number of sets evenly divides the
  // requested number of blocks.
  if(num_blocks % num_sets != 0){
    throw UnevenSets();
  }

  if(num_blocks == num_sets){
    // Direct mapped caches have each block residing in its own
    // logical set.
    //
    // The replacement policy is not necessary for a direct-mapped
    // cache; the direct mapping determines which block to evict.
    CacheLevel::ptr level = boost::make_shared<DirectMappedCacheLevel>(levels.size(), write_policy, num_blocks);
    levels.push_back(level);
  }
  else{
    // Instantiate a set associative cache level with the requested
    // replacement policy.
    const unsigned num_blocks_per_set = num_blocks / num_sets;
    CacheLevel::ptr level;

    switch(repl_policy){
    case CacheLevel::LRU:
    case CacheLevel::MRU:
      {
        // Instantiate a replacement algorithm object.
        ReplacementAlgorithm::ptr repl;
        if(repl_policy == CacheLevel::LRU){
          repl = boost::make_shared<LRU>();
        }
        else if(repl_policy == CacheLevel::MRU){
          repl = boost::make_shared<MRU>();
        }
        else{
          std::cerr << "fatal error: logic error" << std::endl;
          abort();
        }

        // Create a vector of cache sets.
        std::vector<CacheLevel::ptr> sets(num_sets);
        for(unsigned i=0; i<num_sets; i++){
          OrderedCacheSet::ptr set = boost::make_shared<OrderedCacheSet>(write_policy, repl, num_blocks_per_set, i*num_blocks_per_set);
          sets[i] = set;
        }

        level = boost::make_shared<SetAssociativeCacheLevel>(sets);
      }
      break;

    case CacheLevel::OPT:
      {
        if(not this->bs_reader){
          throw UninitializedBlockstreamReader();
        }
        else if(not this->trace){
          throw UninitializedTraceReader();
        }

        // Create a vector of OPT cache sets.
        std::vector<CacheLevel::ptr> sets(num_sets);
        for(unsigned i=0; i<num_sets; i++){
          OPTCacheSet::ptr set = boost::make_shared<OPTCacheSet>(write_policy, num_blocks_per_set, i*num_blocks_per_set, this->trace, this->bs_reader);
          sets[i] = set;
        }

        level = boost::make_shared<SetAssociativeCacheLevel>(sets);
      }
      break;

    case CacheLevel::PES:
      {
        if(not this->bs_reader){
          throw UninitializedBlockstreamReader();
        }
        else if(not this->trace){
          throw UninitializedTraceReader();
        }

        // Create a vector of OPT cache sets.
        std::vector<CacheLevel::ptr> sets(num_sets);
        for(unsigned i=0; i<num_sets; i++){
          PESCacheSet::ptr set = boost::make_shared<PESCacheSet>(write_policy, num_blocks_per_set, i*num_blocks_per_set, this->trace, this->bs_reader);
          sets[i] = set;
        }

        level = boost::make_shared<SetAssociativeCacheLevel>(sets);
      }
      break;

    case CacheLevel::Random:
      {
        std::vector<CacheLevel::ptr> sets(num_sets);
        for(unsigned i=0; i<num_sets; i++){
          RandomReplacementCacheSet::ptr set = boost::make_shared<RandomReplacementCacheSet>(write_policy, num_blocks_per_set, i*num_blocks_per_set);
          sets[i] = set;
        }

        level = boost::make_shared<SetAssociativeCacheLevel>(sets);
      }
      break;
    }

    // Place the level into the cache's level set.
    levels.push_back(level);
  }

  return levels.back();
}

void NewCache::load(const uint64_t addr){
  // Empty the hit information vectors.
#ifdef USE_STRING_INFO
  hit_info_string.clear();
  eviction_info_string.clear();
  entrance_info_string.clear();
#endif

  hit_info.clear();
  eviction_info.clear();
  entrance_info.clear();

  // Compute the block address.
  const uint64_t block_addr = addr / blocksize;

  // Search for the block in each level of cache until it's found.
  // When it's not found in a given level, allocate the block there
  // and possibly take a write-back action.
  //
  // TODO(choudhury): throughout the process, record information
  // about the results of each operation.
  bool in_cache = false;
  for(unsigned L=0; L<levels.size(); L++){
    // CacheLevel::iterator i = levels[L]->find(block_addr);      
    // if(levels[L]->present(i)){
    if(levels[L]->has_block(block_addr)){
      // Block found in L; perform the read operation within the
      // level.
      levels[L]->read(block_addr);

      // Record the hit.
      CacheLevel::iterator i = levels[L]->find(block_addr);
#ifdef USE_STRING_INFO
      std::stringstream ss;
      ss << "read hit to " 
         << std::hex << "0x" << addr
         << " (block address 0x" << block_addr << ")"
         << " in level " << std::dec << (L+1) << ", cell " << i->cell;
      hit_info_string.push_back(ss.str());
#endif

      hit_info.push_back(Daly::CacheHitRecord(addr, L, i->cell, 'R'));

      // Stop looking for the block.
      in_cache = true;
      break;
    }
    else{
      // Block not found - allocate the block.
      Eviction e = levels[L]->allocate(block_addr, shared_from_this(), L);

      // Record entrace of block to this level.
#ifdef USE_STRING_INFO
      std::stringstream ss;
      ss << "entrance of "
         << "0x" << std::hex << addr
         << " (block address 0x" << block_addr << ")"
         << " to level " << std::dec << (L+1) << ", cell " << e.cell;
      entrance_info_string.push_back(ss.str());
#endif

      entrance_info.push_back(Daly::CacheEntranceRecord(addr, L, block_addr));

      // If there was an eviction during the allocation, it may be
      // necessary to perform write back actions.
      if(e.eviction){
        // if(e.dirty and levels[L]->write_policy() == CacheLevel::WriteBack){
        //   // NOTE(choudhury): Convert the block address into a
        //   // memory address.
        //   this->write_at_level(L+1, e.block_addr * blocksize);
        // }

        // Record the eviction.
#ifdef USE_STRING_INFO
        std::stringstream ss;
        ss << "eviction of " << (e.dirty ? "dirty" : "clean") << " block 0x" << std::hex << e.block_addr
           << " from level " << (L+1) << ", which is" << (levels[L]->write_policy() == CacheLevel::WriteBack ? " " : " not ") << "write-back";
        eviction_info_string.push_back(ss.str());
#endif

        eviction_info.push_back(Daly::CacheEvictionRecord(L, e.block_addr, levels[L]->write_policy() == CacheLevel::WriteBack, e.dirty));
      }

      // Finally, read from the newly allocated block.
      //
      // NOTE(choudhury): don't record a hit here - the block may be
      // found in a higher level, so the hit will be recorded AFTER
      // the search loop concludes.
      levels[L]->read(block_addr);
    }
  }

  // If the block was not found in any level of cache, record a
  // "hit" to main memory (i.e. a cache miss).
  if(not in_cache){
#ifdef USE_STRING_INFO
    std::stringstream ss;
    ss << "read miss to " 
       << std::hex << "0x" << addr
       << " (block address 0x" << block_addr << ")";
    hit_info_string.push_back(ss.str());
#endif

    hit_info.push_back(Daly::CacheHitRecord(addr, levels.size(), 0, 'R'));
  }
}

void NewCache::write_at_level(const unsigned level, const uint64_t addr){
  const uint64_t block_addr = addr / blocksize;

  // Go up through the levels of cache, looking for the block to
  // write to - stop when a write back cache level is found.
  unsigned L;
  CacheLevel::iterator i;
  for(L=level; L<levels.size(); L++){
    // i = levels[L]->find(block_addr);
    // if(levels[L]->present(i)){
    if(levels[L]->has_block(block_addr)){
      // Found the block; write to it.
      i = levels[L]->find(block_addr);
      levels[L]->write(block_addr);

      // If the level is write-back, we can stop looking for the
      // block.
      if(levels[L]->write_policy() == CacheLevel::WriteBack){
        break;
      }
    }
    else{
      if(write_miss_pol == WriteAllocate){
        // Allocate a block - if there is an eviction, it may be
        // necessary to perform a write-back.
        Eviction e = levels[L]->allocate(block_addr, shared_from_this(), L);

        // Record block entrance to the level.
#ifdef USE_STRING_INFO
        std::stringstream ss;
        ss << "entrance of "
           << "0x" << std::hex << addr
           << " (block address 0x" << block_addr << ")"
           << " to level " << std::dec << (L+1) << ", cell " << e.cell;
        entrance_info_string.push_back(ss.str());
#endif

        entrance_info.push_back(Daly::CacheEntranceRecord(addr, L, block_addr));

        // Perform write back actions if necessary.
        if(e.eviction){
          // Record an eviction event.
#ifdef USE_STRING_INFO
          std::stringstream ss;
          ss << "eviction of " << (e.dirty ? "dirty" : "clean") << " block 0x" << std::hex << e.block_addr
             << " from level " << (L+1) << ", which is" << (levels[L]->write_policy() == CacheLevel::WriteBack ? " " : " not ") << "write-back";
          eviction_info_string.push_back(ss.str());
#endif

          eviction_info.push_back(Daly::CacheEvictionRecord(L, e.block_addr, levels[L]->write_policy() == CacheLevel::WriteBack, e.dirty));
        }

        // Perform the write to the block.
        i = levels[L]->find(block_addr);
        levels[L]->write(block_addr);
      }
      else{
        // We don't have an implementation for "write no allocate"
        // caches.
        throw "WriteNoAllocate not implemented.";
      }
    }

    // Record the write hit.
#ifdef USE_STRING_INFO
    std::stringstream ss;
    ss << "write hit to " 
       << std::hex << "0x" << addr
       << " (block address 0x" << block_addr << ")"
       << " in level " << std::dec << (L+1) << ", cell " << i->cell;
    hit_info_string.push_back(ss.str());
#endif

    hit_info.push_back(CacheHitRecord(addr, L, i->cell, 'W'));
  }

  // If the last level is write-through and the last step of the
  // above loop therefore goes through to L == levels.size(), or if
  // the function is called with level=levels.size(), then one last
  // write must post to main memory.
  if(L == levels.size()){
#ifdef USE_STRING_INFO
    std::stringstream ss;
    ss << "write miss to "
       << std::hex << "0x" << addr
       << " (block address 0x" << block_addr << ")";
    hit_info_string.push_back(ss.str());
#endif

    hit_info.push_back(CacheHitRecord(addr, levels.size(), 0, 'W'));
  }
}

void NewCache::store(const uint64_t addr){
  // Empty the hit information vectors.
#ifdef USE_STRING_INFO
  hit_info_string.clear();
  eviction_info_string.clear();
  entrance_info_string.clear();
#endif

  hit_info.clear();
  eviction_info.clear();
  entrance_info.clear();

  // Initiate a write at the fastest cache level.
  this->write_at_level(0, addr);
}
