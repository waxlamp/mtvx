// -*- c++ -*-
//
// Copyright 2011 A.N.M. Imroz Choudhury
//
// BlockStreamReader.h - Reads a block stream file and reports on the
// next appearance in the associated trace of each block.

#ifndef BLOCK_STREAM_READER_H
#define BLOCK_STREAM_READER_H

// MTV headers.
#include <Core/Dataflow/BlockStream/LRUStreamPool.h>
#include <Core/Util/Boost.h>
#include <Core/Util/BoostPointers.h>

// System headers.
#include <fstream>
#include <stdint.h>
#include <string>

namespace MTV{
  class BlockStreamReader{
  public:
    BoostPointers(BlockStreamReader);

    struct Stream{
      std::streampos limit;
      uint64_t next;
    };

    typedef boost::unordered_map<uint64_t, Stream> Table;

  public:
    static const std::string magicphrase;
    static const uint64_t never;

  public:
    // Open a block stream trace file.
    bool open(const std::string& infile, unsigned numstreams);

    // Report on the next appearance of 'blockAddr' in the trace.
    uint64_t next(uint64_t blockAddr, uint64_t point);

    // Give the number of  block streams.
    uint64_t numStreams() const {
      return stream.size();
    }

    // Iterator for extracting information about the block streams.
    Table::const_iterator begin() const {
      return stream.begin();
    }

    // Iterator for extracting information about the block streams.
    Table::const_iterator end() const {
      return stream.end();
    }

  private:
    Table stream;
    boost::shared_ptr<LRUIfstreamPool> pool;
  };
}

#endif
