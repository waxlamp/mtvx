// -*- c++ -*-
//
// Copyright 2010 A.N.M. Imroz Choudhury
//
// Cache.h - An object for simulating a cache's response to memory
// access requests.

#ifndef CACHE_H
#define CACHE_H

/// \file
/// \brief Defines a cache simulation system.

// MTV includes.
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/Boost.h>
#include <Core/Util/BoostPointers.h>
#include <Tools/ReferenceTrace/mtrtools.h>
using MTV::BlockStreamReader;
using MTV::TraceReader;

// System includes.
#include <exception>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/// \namespace cacheSim
/// \brief Contains classes needed for cache simulation.

namespace Daly {
  /// \class Timestamp
  ///
  /// \brief Provides a "timestamp" for cache events by counting up
  /// from zero.  Higher numbers mean later logical times.
  class Timestamp{
  public:
    /// \brief Creates a timestamp and initializes to zero.
    Timestamp() : _tick(1) {};

    /// \brief Returns the current time, without changing it.
    unsigned long long currentTick() const { return _tick; };

    /// \brief Sets the current time.
    void setTick(unsigned long long newTick) { _tick = newTick; };

    /// \brief Returns the current time, and increments the timer.
    unsigned long long nextTick() { return _tick++; };

  private:
    /// \brief The current time, in logical units.
    unsigned long long _tick;
  };

  /// \brief Enumerates the write policies a cache may choose to use.
  typedef enum {WriteThrough, WriteBack} WritePolicy;

  /// \brief Enumerates the write miss policies a cache may choose to use.
  typedef enum {WriteAllocate, NoWriteAllocate} WriteMissPolicy;

  /// \struct BlockRecord
  ///
  /// \brief Describes the state of a cache block, including the
  /// location in memory of its contents.
  ///
  /// A CacheLevel consists of several cache blocks, each one
  /// described by a BlockRecord.
  struct BlockRecord{
    /// \brief Construct a default cache block.
    BlockRecord() : addr(0), dirty(false), mapped(false) {};

    /// \brief The address of the data block contained in this cache
    /// block.  This is equivalent to the address of the first data
    /// item stored in this block.
    MTR::addr_t addr; 

    /// \brief The particular item addresses that used this cache
    /// block while it was resident.
    //
    // boost::unordered_set<MTR::addr_t> items;

    /// \brief This flag should be set when the data in this block is
    /// modified.
    bool dirty;

    /// \brief This flag should be set when this block is associated
    /// with a block of data in memory.
    bool mapped;
  };

  class ModtimeTable{
  public:
    BoostPointers(ModtimeTable);

  public:
    void update(MTR::addr_t addr){
      _modtime[addr] = stamp.nextTick();
    }

    unsigned long long modtime(MTR::addr_t addr) const {
      boost::unordered_map<MTR::addr_t, unsigned long long>::const_iterator i = _modtime.find(addr);
      assert(i != _modtime.end());

      return i->second;
    }

  private:
    boost::unordered_map<MTR::addr_t, unsigned long long> _modtime;
    Timestamp stamp;
  };

  /// \class CacheLevel
  /// \brief A single level of a cache hierarchy.
  ///
  /// This class emulates a single level of a cache.  A Cache is
  /// simply a thin wrapper that manages several CacheLevel instances.
  class CacheLevel{
    friend class Cache;

  public:
    BoostPointers(CacheLevel);

  public:
    /// \class BlockNotFound.
    /// \brief An exception class used during cache::find().
    class BlockNotFound {};

  public:
    /// \brief Construct a CacheLevel given a size, block size, associativity, and a write policy.
    ///
    /// This method is private because it should not be instantiated
    /// by general users.  Instead, a friend class (i.e., a Cache)
    /// will be able to create CacheLevel instances as it needs them.
    CacheLevel(unsigned _size, unsigned _blocksize, unsigned _associativity, WritePolicy _writePolicy) :
      _size(_size), _numBlocks(_size/_blocksize), _numSets(_associativity),
      // _numBlocksPerSet(_numBlocks/_numSets), _writePolicy(_writePolicy), blocks(_size) {};
      _numBlocksPerSet(_numBlocks/_numSets), _writePolicy(_writePolicy), blocks(_numBlocks) {};

    /// \brief Returns the size of the cache level, in bytes.
    unsigned size() const { return _size; };

    /// \brief Returns the number of cache blocks present in the cache
    /// level.
    unsigned numBlocks() const { return _numBlocks; };

    /// \brief Returns the number of sets (the associativity) of this
    /// cache level.
    unsigned numSets() const { return _numSets; };

    /// \brief Returns the size of each set, in blocks.
    unsigned numBlocksPerSet() const { return _numBlocksPerSet; };

    /// \brief A synonym for numSets().
    unsigned associativity() const { return _numSets; };

    /// \brief Returns the write policy used by this level of cache.
    WritePolicy writePolicy() const { return _writePolicy; };

    std::vector<BlockRecord>::iterator blocksBegin() { return blocks.begin(); }
    std::vector<BlockRecord>::iterator blocksEnd() { return blocks.end(); }

    /// \brief Returns a block's global index, given a set index and a
    /// cell number within that set.
    unsigned blockIndex(unsigned setIndex, unsigned cell) const { return setIndex*_numBlocksPerSet + cell; };

    /// \brief An output function.
    friend std::ostream& operator<<(std::ostream&, const CacheLevel&);

  private:
    // /// \brief Construct a CacheLevel given a size, block size, associativity, and a write policy.
    // ///
    // /// This method is private because it should not be instantiated
    // /// by general users.  Instead, a friend class (i.e., a Cache)
    // /// will be able to create CacheLevel instances as it needs them.
    // CacheLevel(unsigned _size, unsigned _blocksize, unsigned _associativity, WritePolicy _writePolicy) :
    //   _size(_size), _numBlocks(_size/_blocksize), _numSets(_associativity),
    //   // _numBlocksPerSet(_numBlocks/_numSets), _writePolicy(_writePolicy), blocks(_size) {};
    //   _numBlocksPerSet(_numBlocks/_numSets), _writePolicy(_writePolicy), blocks(_numBlocks) {};

    /// \brief Returns the index of the block containing blockAddr, or
    /// throws BlockNotFound if it is not present.
    unsigned find(MTR::addr_t blockAddr) const throw(BlockNotFound);

    /// \brief Block access via set index and cell number within that set.
    BlockRecord& block(unsigned setIndex, unsigned cell) { return blocks[this->blockIndex(setIndex, cell)]; };

    /// \brief Block access via global cell number.
    BlockRecord& block(unsigned cell) { return blocks[cell]; };

    /// \brief const version of block(unsigned, unsigned)
    const BlockRecord& block(unsigned setIndex, unsigned cell) const { return blocks[this->blockIndex(setIndex, cell)]; };

    /// \brief const version of block(unsigned)
    const BlockRecord& block(unsigned cell) const { return blocks[cell]; };

  private:
    unsigned _size, _numBlocks, _numSets, _numBlocksPerSet;
    WritePolicy _writePolicy;
    std::vector<BlockRecord> blocks;
  };

  /// \struct CacheHitRecord
  ///
  /// \brief This struct describes an access to a cache.
  struct CacheHitRecord{
    /// \brief Construct a default CacheHitRecord (with an invalid
    /// operation code).
    CacheHitRecord()
      : op('X'), addr(0), L(0), cell(0)
    {}

    /// \brief Construct a CacheHitRecord from an address, cache level
    /// identifier, cell number, and (optionally) an operation code.
    CacheHitRecord(MTR::addr_t addr, unsigned L, unsigned cell, char op = 'X')
      : op(op), addr(addr), L(L), cell(cell)
    {}

    /// \brief The operation represented by this access ('R' or 'W').
    char op;

    /// \brief The address associated with this access.
    MTR::addr_t addr;

    /// \brief The cache level it hit to.  A cache miss is signified
    /// by setting L to the number one larger than the last cache
    /// level (as though the main memory were considered as part of
    /// the cache, receiving its own cache level id).
    unsigned L;

    /// \brief A pointer to the cache level itself, in case this is
    /// useful in multiple-cache situations.
    //
    // CacheLevel::ptr p;

    /// \brief The cache block involved.
    unsigned cell;

    friend std::ostream& operator<<(std::ostream& out, const CacheHitRecord& r){
      // Save format flags.
      const std::ios_base::fmtflags flags = out.flags();

      // Output to stream.
      out << "CacheHitRecord("
          << "addr = 0x" << std::hex << r.addr << ", "
          << "L = " << std::dec << r.L << ", "
          << "cell = " << r.cell << ", "
          << "op = " << r.op << ")";

      // Restore format flags.
      out.flags(flags);

      return out;
    }
  };

  struct CacheEvictionRecord{
    CacheEvictionRecord()
      : L(0), blockaddr(0), writeback(false)
    {}

    CacheEvictionRecord(unsigned L, MTR::addr_t blockaddr, bool writeback, bool dirty)
      : L(L),
        blockaddr(blockaddr),
        writeback(writeback),
        dirty(dirty)
    {}

    unsigned L;
    MTR::addr_t blockaddr;
    bool writeback, dirty;

    friend std::ostream& operator<<(std::ostream& out, const CacheEvictionRecord& r){
      // Save format flags.
      const std::ios_base::fmtflags flags = out.flags();

      // Output to stream.
      out << "CacheEvictionRecord("
          << "L = " << std::dec << r.L << ", "
          << "blockaddr = 0x" << std::hex << r.blockaddr << ", "
          << "writeback = " << (r.writeback ? "true" : "false") << ", "
          << "dirty = " << (r.dirty ? "true" : "false") << ")";

      // Restore format flags.
      out.flags(flags);

      return out;
    }
  };

  struct CacheEntranceRecord{
    CacheEntranceRecord()
      : addr(0),
        L(0),
        blockaddr(0)
    {}

    CacheEntranceRecord(MTR::addr_t addr, unsigned L, MTR::addr_t blockaddr)
      : addr(addr),
        L(L),
        blockaddr(blockaddr)
    {}

    MTR::addr_t addr;
    unsigned L;
    MTR::addr_t blockaddr;

    friend std::ostream& operator<<(std::ostream& out, const CacheEntranceRecord& r){
      // Save format flags.
      const std::ios_base::fmtflags flags = out.flags();

      // Output to stream.
      out << "CacheEntranceRecord("
          << "addr = 0x" << std::hex << r.addr << ", "
          << "L = " << std::dec << r.L << ", "
          << "blockaddr = " << r.blockaddr << ")";

      // Restore format flags.
      out.flags(flags);

      return out;
    }
  };

  /// \class Cache
  ///
  /// \brief A class that builds on top of CacheLevel instances in
  /// order to present a unified cache simulation interface.
  class EvictionBlockSelector;
  class Cache{
  public:
    BoostPointers(Cache);

  public:
    /// \class Snapshot
    ///
    /// \brief A memento class containing the state of a Cache object.
    class Snapshot{
      friend class Cache;

    public:
      std::vector<std::vector<BlockRecord> > state;
    };

  public:
    /// \brief Construct a Cache from a block size and a write miss
    /// policy.
    ///
    /// Cache levels are added separately, with the addCacheLevel()
    /// method.
    Cache(unsigned _blocksize, WriteMissPolicy _writeMissPolicy, boost::shared_ptr<EvictionBlockSelector> _evictionPolicy, ModtimeTable::ptr modtime) :
      _blocksize(_blocksize),
      _writeMissPolicy(_writeMissPolicy),
      _evictionPolicy(_evictionPolicy),
      modtime(modtime)
    {}

    /// \brief Returns the block size, in bytes.
    ///
    /// This block size will be implicitly passed to all cache levels
    /// created for use with this cache.
    unsigned block_size() const { return _blocksize; }

    /// \brief Returns the write miss policy used by this cache.
    WriteMissPolicy writeMissPolicy() const { return _writeMissPolicy; }

    boost::shared_ptr<EvictionBlockSelector> evictionPolicy() { return _evictionPolicy; }

    // virtual BlockRecord& select_eviction_block(unsigned L, unsigned setIndex) const = 0;

    /// \brief Returns a const reference to one of the cache levels.
    ///
    /// This method is useful for querying the configuration of a
    /// particular cache level.
    CacheLevel::const_ptr level(unsigned i) const { return levels[i]; }

    CacheLevel::ptr level(unsigned i) { return levels[i]; }

    /// \brief Returns the number of cache levels in the cache.
    unsigned num_levels() const { return levels.size(); }

    bool hasLevel(unsigned L) const { return L < levels.size(); }

    void setEvictionPolicy(boost::shared_ptr<EvictionBlockSelector> policy){
      _evictionPolicy = policy;
    }

    /// \brief Returns a sequence of CacheHitRecords describing the
    /// actions performed by the cache in the last access.
    const std::vector<CacheHitRecord>& hitInfo() const { return _hitInfo; }

    const std::vector<CacheEvictionRecord>& evictionInfo() const { return _evictionInfo; }

    const std::vector<CacheEntranceRecord>& entranceInfo() const { return _entranceInfo; }

    void infoReport() const;

    /// \brief Add a cache level given a size, associativity, and a
    /// write policy.  The block size of the cache level will be the
    /// same as that specified for the cache itself.
    void addCacheLevel(unsigned size, unsigned associativity, WritePolicy writePolicy);

    /// \brief Add a cache level that already exists somewhere else
    /// (use this to implement a split cache level backed by a unified
    /// cache level, etc.).
    void addCacheLevel(CacheLevel::ptr level);

    /// \brief Simulate a load-type instruction at the given address.
    void load(MTR::addr_t addr);

    /// \brief Simulate a store-type instruction at the given address.
    void store(MTR::addr_t addr);

    /// \brief Returns a memento object describing the full state of
    /// the cache.
    Snapshot state() const;

    /// \brief Restores the state described by the given memento
    /// object.
    void setState(const Snapshot& snap);

    void updateModtime(MTR::addr_t blockAddr){
      modtime->update(blockAddr);
    }

    unsigned long long getModtime(MTR::addr_t blockAddr) const {
      return modtime->modtime(blockAddr);
    }

    static Cache::ptr newFromSpec(const std::string& specfile, TraceReader::ptr reader);
    static Cache::ptr newFromSpec(const std::string& specfile, TraceReader::ptr reader, const std::string& bsfile, unsigned numstreams);

  private:
    /// \brief Searches the cache levels for a particular block --
    /// called by load() and writeAtLevel()
    CacheHitRecord find(MTR::addr_t addr) const;

    /// \brief Posts a write to the specified level of cache,
    /// propagating the write past that level if required by each
    /// level's write policy.
    void writeAtLevel(unsigned L, MTR::addr_t addr);

    /// \brief Places a block in level \e L of the cache, evicting an
    /// existing block if necessary -- called by load() and store().
    /// Returns the cell index of the allocated block.
    unsigned allocate(unsigned L, MTR::addr_t blockAddr);

    /// \brief Unmaps a block from the cache in the specified set, and
    /// carries out write-back actions if necessary -- called by
    /// allocate().  Returns the block index of the evicted block.
    BlockRecord& evict(unsigned L, unsigned setIndex);

  private:
    unsigned _blocksize;
    WriteMissPolicy _writeMissPolicy;
    boost::shared_ptr<EvictionBlockSelector> _evictionPolicy;

    std::vector<CacheLevel::ptr> levels;

    std::vector<CacheHitRecord> _hitInfo;
    std::vector<CacheEvictionRecord> _evictionInfo;
    std::vector<CacheEntranceRecord> _entranceInfo;

    // Timestamp stamp;
    // boost::unordered_map<MTR::addr_t, unsigned long long> modTime;

    ModtimeTable::ptr modtime;
  };
  
  class EvictionBlockSelector{
  public:
    BoostPointers(EvictionBlockSelector);

  public:
    virtual ~EvictionBlockSelector() {}

    virtual std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex) = 0;
  };

  class RANDOM : public EvictionBlockSelector {
  public:
    BoostPointers(RANDOM);

  public:
    std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex){
      CacheLevel::ptr level = c->level(L);

      // Select a block from the appropriate set at random: take the
      // begin iterator, then add the index of the block that is the
      // randomly chosen block from that set.
      std::vector<BlockRecord>::iterator victim = level->blocksBegin() + level->blockIndex(setIndex,static_cast<unsigned>(drand48()*level->numBlocksPerSet()));

      return std::make_pair(victim, level->writePolicy() == WriteBack);
    }
  };

  class LRU : public EvictionBlockSelector {
  public:
    BoostPointers(LRU);

  public:
    std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex){
      CacheLevel::ptr level = c->level(L);

      std::vector<BlockRecord>::iterator LRU_block;
      long long unsigned oldest = std::numeric_limits<unsigned long long>::max();
      for(std::vector<BlockRecord>::iterator i = level->blocksBegin() + level->blockIndex(setIndex,0);
          i != level->blocksBegin() + level->blockIndex(setIndex+1,0);
          i++){
        const unsigned long long mtime = c->getModtime(i->addr);

        if(mtime < oldest){
          oldest = mtime;
          LRU_block = i;
        }
      }

      return std::make_pair(LRU_block, level->writePolicy() == WriteBack);
    }
  };

  class MRU : public EvictionBlockSelector {
  public:
    BoostPointers(MRU);

  public:
    std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex){
      CacheLevel::ptr level = c->level(L);

      std::vector<BlockRecord>::iterator MRU_block;
      long long unsigned newest = 0;
      for(std::vector<BlockRecord>::iterator i = level->blocksBegin() + level->blockIndex(setIndex,0);
          i != level->blocksBegin() + level->blockIndex(setIndex+1,0);
          i++){
        const unsigned long long mtime = c->getModtime(i->addr);

        if(mtime > newest){
          newest = mtime;
          MRU_block = i;
        }
      }

      return std::make_pair(MRU_block, level->writePolicy() == WriteBack);
    }
  };

  class ApproxOPT : public EvictionBlockSelector {
  public:
    BoostPointers(ApproxOPT);

  public:
    ApproxOPT(TraceReader::ptr reader = TraceReader::ptr())
      : reader(reader),
        log("approx-opt.log")
    {}

    ~ApproxOPT(){
      log.close();
    }

    void setReader(TraceReader::ptr p){
      reader = p;
    }

    std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex);

  private:
    TraceReader::ptr reader;

    std::ofstream log;
  };

  class OPT : public EvictionBlockSelector {
  public:
    BoostPointers(OPT);

  public:
    // NOTE(choudhury): blockstreamfile is a filename for the block
    // stream information, while trace is used to query the current
    // trace record number, which is needed for indexing into the
    // block stream data.
    OPT(const std::string& blockstreamfile, unsigned numstreams, TraceReader::const_ptr trace = TraceReader::const_ptr())
      : trace(trace),
        blockstreams(boost::make_shared<BlockStreamReader>())
    {
      if(!blockstreams->open(blockstreamfile, numstreams)){
        // TODO(choudhury): the error handling needs to be much more
        // robust.
        std::cerr << "fatal error: OPT object could not be instantiated because block stream reader could not open file '" << blockstreamfile << "' with " << numstreams << " requested filestreams." << std::endl;
        abort();
      }
    }

    void setReader(TraceReader::const_ptr p){
      trace = p;
    }

    std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex){
      // Capture the appropriate cache level.
      CacheLevel::ptr level = c->level(L);

      // Get the current trace point.
      const uint64_t point = trace->getTracePoint();

      // For each block in the evicting set, keep track of the one that
      // reports the least imminent re-use time.
      //
      // TODO(choudhury): abstract out the initial value of "furthest" and
      // the comparator used in the if statement to make this class able
      // to perform both OPT and PES.
      uint64_t furthest_reuse = 0;
      std::vector<BlockRecord>::iterator furthest = level->blocksEnd();

      // std::cout << (furthest - level->blocksBegin()) << std::endl;
      // std::cout << (level->blockIndex(setIndex+1,0) - level->blockIndex(setIndex,0)) << std::endl;

      for(std::vector<BlockRecord>::iterator i=level->blocksBegin() + level->blockIndex(setIndex, 0);
          i != level->blocksBegin() + level->blockIndex(setIndex+1, 0);
          i++){
        const uint64_t next = blockstreams->next(i->addr, point);

        // std::cout << "block " << i->addr << ": next access " << next;

        if(next > furthest_reuse){
          // std::cout << " (furthest reuse)";

          furthest_reuse = next;
          furthest = i;
        }

        // std::cout << std::endl;
      }

      // Sanity check.
      assert(furthest != level->blocksEnd());

      // std::cout << "selected block " << furthest->addr << std::endl;

      return std::make_pair(furthest, level->writePolicy() == WriteBack);
    }

  private:
    TraceReader::const_ptr trace;
    BlockStreamReader::ptr blockstreams;
  };

  class ApproxPES : public EvictionBlockSelector {
  public:
    BoostPointers(ApproxPES);

  public:
    ApproxPES(TraceReader::ptr reader = TraceReader::ptr())
      : reader(reader)
    {}

    void setReader(TraceReader::ptr p){
      reader = p;
    }

    std::pair<std::vector<BlockRecord>::iterator, bool> select_eviction_block(Cache *c, unsigned L, unsigned setIndex);

  private:
    TraceReader::ptr reader;
  };

  /// \namespace cacheSim::exceptions
  ///
  /// \brief Contains all cacheSim exceptions.
  namespace exceptions{
    // \class CacheSimException
    //
    /// \brief Base class for all cache simulation exceptions.
    class CacheSimException : public std::exception {};

    /// \class SetupProblem
    ///
    /// \brief Base class for exceptions thrown if cache setup
    /// parameters are bad.
    class SetupProblem : public CacheSimException {};

    /// \class UnevenSetSize
    ///
    /// \brief A cache level's requested size is smaller than its
    /// associativity.
    class UnevenSetSize : public SetupProblem {
    public:
      /// \brief Construct an UnevenSetSize exception given the cache
      /// level identifier in which the error occurred, as well as the
      /// problematic number of blocks and associativity involved.
      UnevenSetSize(unsigned levelIndex, unsigned numBlocks, unsigned associativity){
	std::stringstream s;
	s << "Cache level L" << levelIndex+1 << "'s " << numBlocks << " blocks do not divide evenly into " << associativity << " sets.";
	msg = s.str();
      }

      ~UnevenSetSize() throw() {}

      const char *what() const throw() { return msg.c_str(); }

    private:
      std::string msg;
    };
    
    /// \class BlocksizeDoesNotDivideSize
    ///
    /// \brief A cache level's requested size is not a multiple of its
    /// associativity.
    class BlocksizeDoesNotDivideSize : public SetupProblem {
    public:
      /// \brief Construct a BlocksizeDoesNotDivideSize exception
      /// given the cache level in which the error occurred, and the
      /// problematic size and block size values.
      BlocksizeDoesNotDivideSize(unsigned levelIndex, unsigned size, unsigned blockSize){
	std::stringstream s;
	s << "Cache level L" << levelIndex+1 << "'s size (" << size << " bytes) is not a multiple of its block size (" << blockSize << ").";
	msg = s.str();
      }

      ~BlocksizeDoesNotDivideSize() throw () {}

      const char *what() const throw() { return msg.c_str(); }

    private:
      std::string msg;
    };

    /// \class CacheTooSmall
    ///
    /// \brief A new cache level's requested size is smaller than or
    /// equal to that of the last cache level.
    class CacheTooSmall : public SetupProblem {
    public:
      /// \brief Construct a CacheTooSmall exception given the cache
      /// level in which the error occurred, and the problematic size
      /// values.
      CacheTooSmall(unsigned levelIndex, unsigned size, unsigned lastSize){
	std::stringstream s;
	s << "Cache level L" << levelIndex+1 << "'s size (" << size << " bytes) is not larger than the last level (" << lastSize << " bytes).";
	msg = s.str();
      }

      ~CacheTooSmall() throw () {}

      const char *what() const throw() { return msg.c_str(); }

    private:
      std::string msg;
    };

#ifndef NDEBUG
    /// \class EvictionWithUnmappedBlocks
    ///
    /// \brief The system has executed an eviction on a cache set that
    /// already has unmapped (available) blocks.  This exception
    /// indicates a logic error in the system and should never occur
    /// during bug-free operation.
    class EvictionWithUnmappedBlocks : public CacheSimException {
    public:
      /// \brief Construct an EvictionWithUnmappedBlocks exception
      /// given the cache level in which the error occurred, as well
      /// as the set index and cell number that were involved.
      EvictionWithUnmappedBlocks(unsigned levelIndex, unsigned setIndex, unsigned cell){
	std::stringstream s;
	s << "Cache level L" << levelIndex+1 << " experienced an eviction in set " << setIndex
	  << " when block " << cell << " of that set is already unmapped.";
      }

      ~EvictionWithUnmappedBlocks() throw() {}

      const char *what() const throw() { return msg.c_str(); }

    private:
      std::string msg;
    };
#endif

    /// \class CacheSnapshotWrongSize
    ///
    /// \brief This exception is thrown when a cache snapshot of the
    /// wrong size is presented to the Cache::setState() method.
    class CacheSnapshotWrongSize : public CacheSimException {
    public:
      const char *what() const throw() { return "A cache snapshot object of the wrong dimensions was passed to cache::setState()."; }
    };
  }

  /// \brief An example cache for other modules to use.
  //
  // TODO(choudhury): this function should return a Cache::const_ptr.
  Cache::ptr defaultCache();
}

#endif
