// -*- c++ -*-
//
// Copyright 2012 A.N.M. Imroz Choudhury
//
// CacheLevel.h

#ifndef CACHE_LEVEL_H
#define CACHE_LEVEL_H

// MTV headers.
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/BoostPointers.h>

// Boost headers.
#include <boost/unordered_map.hpp>

// System headers.
#include <list>
#include <stdint.h>
#include <vector>

namespace MTV{
  class NewCache;

  struct CacheBlock{
    CacheBlock(unsigned cell)
      : addr(static_cast<uint64_t>(-1)),
        dirty(false),
        cell(cell)
    {}

    uint64_t addr;
    bool dirty;

    const unsigned cell;
  };

  struct Eviction{
    Eviction(bool writeback, unsigned cell)
      : eviction(false),
        writeback(writeback),
        dirty(false),
        block_addr(static_cast<uint64_t>(-1)),
        cell(cell)
    {}

    bool eviction;
    bool writeback;
    bool dirty;
    uint64_t block_addr;
    unsigned cell;
  };

  class CacheLevel{
  public:
    BoostPointers(CacheLevel);

  public:
    enum WritePolicy{
      WriteThrough,
      WriteBack
    };

    enum ReplacementPolicy{
      LRU,
      MRU,
      OPT,
      PES,
      Random
    };

  public:
    typedef std::list<CacheBlock>::iterator iterator;

  public:
    class IllegalBlockAccess {};

  public:
    CacheLevel(WritePolicy write_pol);

    virtual ~CacheLevel(){}

    WritePolicy write_policy() const {
      return write_pol;
    }

    virtual bool has_block(uint64_t block_addr){
      return (this->find(block_addr) != blocks.end());
    }

    virtual iterator find(uint64_t block_addr) = 0;

    virtual bool present(CacheLevel::iterator i) const {
      return i != blocks.end();
    }

    virtual Eviction allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level) = 0;

    virtual void write_back(boost::shared_ptr<NewCache> cache, unsigned L, uint64_t block_addr);

    // This function can, e.g., set up the level's data structures to
    // make eviction easier, etc.
    virtual void read(uint64_t block_addr) = 0;

    // This function should be used like read() 
    virtual void write(uint64_t block_addr) = 0;

    virtual void print(std::ostream& out, const std::string& prefix = "") const {
      for(std::list<CacheBlock>::const_iterator i = blocks.begin(); i != blocks.end(); i++){
        out << prefix << "(" << std::dec << i->cell << ", 0x" << std::hex << i->addr << ", " << (i->dirty ? "dirty" : "clean") << ")" << std::endl;
      }
      out << std::dec;
    }

  protected:
    std::list<CacheBlock> blocks;
    const WritePolicy write_pol;
  };

  class DirectMappedCacheLevel : public CacheLevel {
  public:
    DirectMappedCacheLevel(unsigned level, WritePolicy write_policy, unsigned _num_blocks);

    CacheLevel::iterator find(uint64_t block_addr){
      // Direct mapping means there is precisely one position this block
      // can go - if the block is in that position, then return an
      // iterator to the entry.
      const unsigned index = block_addr % num_blocks;
      CacheLevel::iterator i = lookup[index];
      if(i != blocks.end() and i->addr == block_addr){
        return i;
      }
      else{
        // Otherwise, return the absent iterator.
        return blocks.end();
      }
    }

    Eviction allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level);

    void read(uint64_t block_addr){
      // Check that the block is actually present.
      //
      // TODO(choudhury): convert into an assertion.
      if(not block_present(block_addr)){
        throw IllegalBlockAccess();
      }

      // Otherwise, there's nothing that needs to be done.
      return;
    }

    void write(uint64_t block_addr){
      // Check that the block is actually present.
      //
      // TODO(choudhury): convert into an assertion.
      if(not block_present(block_addr)){
        throw IllegalBlockAccess();
      }

      // Set the dirty flag on the block.
      const unsigned i = block_addr % num_blocks;
      lookup[i]->dirty = true;
    }

  private:
    bool block_present(uint64_t block_addr){
      const unsigned i = block_addr % num_blocks;
      CacheLevel::iterator it = lookup[i];

      return it != blocks.end() and it->addr == block_addr;
    }

  private:
    const unsigned num_blocks;
    std::vector<CacheLevel::iterator> lookup;
  };

  class ReplacementAlgorithm{
  public:
    BoostPointers(ReplacementAlgorithm);

  public:
    virtual void insert(std::list<CacheBlock>& blocks, const CacheBlock& block) = 0;
    virtual void evict(std::list<CacheBlock>& blocks) = 0;
    virtual const CacheBlock& victim(const std::list<CacheBlock>& blocks) = 0;
    virtual CacheLevel::iterator last_touched(std::list<CacheBlock>& blocks) = 0;
    virtual void peek(std::list<CacheBlock>& blocks, CacheLevel::iterator i) = 0;
    virtual void poke(std::list<CacheBlock>& blocks, CacheLevel::iterator i) = 0;
  };

  class LRU : public ReplacementAlgorithm {
  public:
    void insert(std::list<CacheBlock>& blocks, const CacheBlock& block){
      blocks.push_front(block);
    }

    void evict(std::list<CacheBlock>& blocks){
      // Get iterator to rear element, and move it to the front.
      CacheLevel::iterator i = blocks.end();
      --i;
      blocks.splice(blocks.begin(), blocks, i);
    }

    const CacheBlock& victim(const std::list<CacheBlock>& blocks){
      // The victim is always at the back of the list.
      return blocks.back();
    }

    CacheLevel::iterator last_touched(std::list<CacheBlock>& blocks){
      return blocks.begin();
    }

    void peek(std::list<CacheBlock>& blocks, CacheLevel::iterator i){
      // Move touched elements to the front, AWAY from the victim position.
      blocks.splice(blocks.begin(), blocks, i);
    }

    void poke(std::list<CacheBlock>& blocks, CacheLevel::iterator i){
      // Move touched elements to the front, AWAY from the victim position.
      blocks.splice(blocks.begin(), blocks, i);
      blocks.front().dirty = true;
    }
  };

  class MRU : public ReplacementAlgorithm {
  public:
    void insert(std::list<CacheBlock>& blocks, const CacheBlock& block){
      // Place new blocks in the victim position.
      blocks.push_front(block);
    }

    void evict(std::list<CacheBlock>& blocks){
      // // Move the front element to the rear (front being the victim
      // // position).
      // blocks.splice(blocks.end(), blocks, blocks.begin());

      // This function is intentionally blank.
      //
      // NOTE(choudhury): to "evict" a block, we just leave the victim
      // block where it is - the new block will be the most recently
      // used block, so it should "move" into victim position, where
      // it already is.
    }

    const CacheBlock& victim(const std::list<CacheBlock>& blocks){
      // Victim position is the FRONT of the list.
      return blocks.front();
    }

    CacheLevel::iterator last_touched(std::list<CacheBlock>& blocks){
      return blocks.begin();
    }

    void peek(std::list<CacheBlock>& blocks, CacheLevel::iterator i){
      // When blocks are touched, move them into the victim position.
      if(blocks.begin() != i){
        blocks.splice(blocks.begin(), blocks, i);
      }
    }

    void poke(std::list<CacheBlock>& blocks, CacheLevel::iterator i){
      // When blocks are touched, move them into the victim position.
      if(blocks.begin() != i){
        blocks.splice(blocks.begin(), blocks, i);
      }

      // Mark the poked block dirty.
      blocks.front().dirty = true;
    }
  };

  class OrderedCacheSet : public CacheLevel {
  public:
    BoostPointers(OrderedCacheSet);

  public:
    OrderedCacheSet(WritePolicy write_policy, ReplacementAlgorithm::ptr repl, unsigned num_blocks, unsigned start_cell)
      : CacheLevel(write_policy),
        num_blocks(num_blocks),
        start_cell(start_cell),
        repl(repl)
    {}

    CacheLevel::iterator find(uint64_t block_addr){
      // Check the lookup table - if the address is found there, then
      // the table maps it to an iterator to the actual block;
      // otherwise, simply return the block list's end iterator.
      boost::unordered_map<uint64_t, iterator>::const_iterator i = lookup.find(block_addr);
      if(i != lookup.end()){
        return i->second;
      }
      else{
        return blocks.end();
      }
    }

    Eviction allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level);

    void read(uint64_t block_addr){
      // Make sure the block is actually present.
      //
      // TODO(choudhury): convert to an assertion.
      CacheLevel::iterator i = this->find(block_addr);
      if(not present(i)){
        throw IllegalBlockAccess();
      }

      // If the block is not already at the front of the list, move it
      // to the front.
      //
      // Ordering::peek(blocks, i);
      repl->peek(blocks, i);
    }

    void write(uint64_t block_addr){
      // Make sure the block is actually present.
      //
      // TODO(choudhury): convert to an assertion.
      CacheLevel::iterator i = this->find(block_addr);
      if(not present(i)){
        throw IllegalBlockAccess();
      }

      // If the block is not already at the front of the list, move it
      // to the front.
      //
      // Ordering::poke(blocks, i);
      repl->poke(blocks, i);
    }

  private:
    boost::unordered_map<uint64_t, CacheLevel::iterator> lookup;

    const unsigned num_blocks;
    const unsigned start_cell;

    ReplacementAlgorithm::ptr repl;
  };

  // typedef OrderedCacheSet<Orderings::LRU> LRUCacheSet;
  // typedef OrderedCacheSet<Orderings::MRU> MRUCacheSet;

  class RandomReplacementCacheSet : public CacheLevel {
  public:
    BoostPointers(RandomReplacementCacheSet);

  public:
    RandomReplacementCacheSet(WritePolicy write_policy, unsigned num_blocks, unsigned start_cell)
      : CacheLevel(write_policy),
        num_blocks(num_blocks)
    {}

    CacheLevel::iterator find(uint64_t block_addr){
      boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
      if(i == lookup.end()){
        return blocks.end();
      }
      else{
        return i->second;
      }
    }

    Eviction allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level);

    void read(uint64_t block_addr){
      // Check to make sure requested block is present.
      //
      // TODO(choudhury): convert into an assertion.
      boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
      if(i->second == blocks.end()){
        throw IllegalBlockAccess();
      }

      // NOTE(choudhury): Nothing else to do if we reach here.
      return;
    }

    void write(uint64_t block_addr){
      // Check to make sure requested block is present.
      //
      // TODO(choudhury): convert into an assertion.
      boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
      if(i->second == blocks.end()){
        throw IllegalBlockAccess();
      }

      // Set the dirty flag on the block.
      i->second->dirty = true;
    }

  private:
    boost::unordered_map<uint64_t, CacheLevel::iterator> lookup;
    std::vector<CacheLevel::iterator> random_access;
    const unsigned num_blocks;
  };

  class BeladyAlgorithmCacheSet : public CacheLevel {
  public:
    BoostPointers(BeladyAlgorithmCacheSet);

  public:
    BeladyAlgorithmCacheSet(WritePolicy write_policy, unsigned num_blocks, unsigned start_cell, TraceReader::const_ptr trace, BlockStreamReader::ptr bsreader)
      : CacheLevel(write_policy),
        trace(trace),
        bsreader(bsreader),
        num_blocks(num_blocks),
        start_cell(start_cell)
    {}

    CacheLevel::iterator find(uint64_t block_addr){
      boost::unordered_map<uint64_t, CacheLevel::iterator>::iterator i = lookup.find(block_addr);
      if(i != lookup.end() and i->second->addr == block_addr){
        return i->second;
      }
      else{
        return blocks.end();
      }
    }

    Eviction allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level);

    void read(uint64_t block_addr){
      // Ensure that the block is present.
      if(not present(find(block_addr))){
        throw IllegalBlockAccess();
      }

      // Nothing more to do if it is present.
    }

    void write(uint64_t block_addr){
      // Ensure that the block is present.
      if(not present(find(block_addr))){
        throw IllegalBlockAccess();
      }

      // Mark the block dirty.
      lookup[block_addr]->dirty = true;
    }

  private:
    virtual uint64_t select_victim() = 0;

  protected:
    boost::unordered_map<uint64_t, CacheLevel::iterator> lookup;
    TraceReader::const_ptr trace;
    BlockStreamReader::ptr bsreader;
    const unsigned num_blocks;
    const unsigned start_cell;
  };

  class OPTCacheSet : public BeladyAlgorithmCacheSet {
  public:
    BoostPointers(OPTCacheSet);

  public:
    OPTCacheSet(WritePolicy write_policy, unsigned num_blocks, unsigned start_cell, TraceReader::const_ptr trace, BlockStreamReader::ptr bsreader)
      : BeladyAlgorithmCacheSet(write_policy, num_blocks, start_cell, trace, bsreader)
    {}

  private:
    uint64_t select_victim(){
      // Get the current trace point.
      const uint64_t point = trace->getTracePoint();

      // For each block in the level, compute the FURTHEST re-use of
      // that block in the future.
      uint64_t furthest_reuse = 0, victim_block = 0;
      bool found = false;
      for(std::list<CacheBlock>::const_iterator i = blocks.begin(); i != blocks.end(); i++){
        // Get the next trace point at which the block is used.
        const uint64_t next = bsreader->next(i->addr, point);

        // Compare to the current furthest reuse.
        if(next > furthest_reuse){
          found = true;
          furthest_reuse = next;
          victim_block = i->addr;
        }
      }

      if(not found){
        std::cerr << "fatal error: no block was selected by OPTCacheLevel::select_victim()" << std::endl;
        abort();
      }

      return victim_block;
    }
  };

  class PESCacheSet : public BeladyAlgorithmCacheSet {
  public:
    BoostPointers(PESCacheSet);

  public:
    PESCacheSet(WritePolicy write_policy, unsigned num_blocks, unsigned start_cell, TraceReader::const_ptr trace, BlockStreamReader::ptr bsreader)
      : BeladyAlgorithmCacheSet(write_policy, num_blocks, start_cell, trace, bsreader)
    {}

  private:
    uint64_t select_victim(){
      // Get the current trace point.
      const uint64_t point = trace->getTracePoint();

      // For each block in the level, compute the NEAREST re-use of
      // that block in the future.
      uint64_t nearest_reuse = std::numeric_limits<uint64_t>::max(), victim_block = 0;
      bool found = false;
      for(std::list<CacheBlock>::const_iterator i = blocks.begin(); i != blocks.end(); i++){
        // Get the next trace point at which the block is used.
        const uint64_t next = bsreader->next(i->addr, point);

        // Compare to the current furthest reuse.
        if(next < nearest_reuse){
          found = true;
          nearest_reuse = next;
          victim_block = i->addr;
        }
      }

      if(not found){
        std::cerr << "fatal error: no block was selected by PESCacheLevel::select_victim()" << std::endl;
        abort();
      }

      return victim_block;
    }
  };

  class SetAssociativeCacheLevel : public CacheLevel {
  public:
    BoostPointers(SetAssociativeCacheLevel);

  public:
    SetAssociativeCacheLevel(const std::vector<CacheLevel::ptr>& init_sets);

    bool has_block(uint64_t block_addr){
      const uint64_t set_index = block_addr % sets.size();

      return sets[set_index]->has_block(block_addr);
    }

    bool present(CacheLevel::iterator i) const {
      // const uint64_t set_index = block_addr % sets.size();

      // TODO(choudhury): this is for testing whether this function is
      // really getting called.  Remove when testing complete.
      abort();

      // NOTE(choudhury): shared_set_index is computed in the find()
      // method, the only method that is capable of returning an
      // iterator.  The idea is that if a client called the present()
      // method, it will have needed to obtain the argument iterator via
      // a previous call to find().
      return sets[shared_set_index]->present(i);
    }

    CacheLevel::iterator find(uint64_t block_addr){
      shared_set_index = block_addr % sets.size();

      return sets[shared_set_index]->find(block_addr);
    }

    Eviction allocate(uint64_t block_addr, boost::shared_ptr<NewCache> cache, unsigned level){
      const uint64_t set_index = block_addr % sets.size();

      return sets[set_index]->allocate(block_addr, cache, level);
    }

    void read(uint64_t block_addr){
      const uint64_t set_index = block_addr % sets.size();

      sets[set_index]->read(block_addr);
    }

    void write(uint64_t block_addr){
      const uint64_t set_index = block_addr % sets.size();

      sets[set_index]->write(block_addr);
    }

    void print(std::ostream& out, const std::string& prefix = "") const {
      for(unsigned i=0; i<sets.size(); i++){
        out << prefix << "Set " << i << ":" << std::endl;
        sets[i]->print(out, prefix+prefix);
      }
    }

  // private:
  //   static Cache dummy_cache;

  private:
    std::vector<CacheLevel::ptr> sets;
    unsigned shared_set_index;
  };
}

#endif
