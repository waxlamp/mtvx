// Copyright 2011 A.N.M. Imroz Choudhury
//
// BlockStreamReader.cpp

// MTV headers.
#include <Core/BlockStreamHeader.pb.h>
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
#include <Core/Util/Util.h>
using MTV::BlockStream::BlockStreamHeader;
using MTV::BlockStreamReader;

// System headers.
#include <algorithm>
#include <iostream>

const std::string BlockStreamReader::magicphrase = "MTV:BlockStream";

const uint64_t BlockStreamReader::never = std::numeric_limits<uint64_t>::max();

bool BlockStreamReader::open(const std::string& infile, unsigned numstreams){
  // Try to open the file.
  std::ifstream in(infile.c_str());
  if(!in){
    return false;
  }

  // The first value in the file is a size for a protobuf
  int hdr_size;
  in.read(reinterpret_cast<char *>(&hdr_size), sizeof(hdr_size));

  // Allocate a buffer and read in the appropriate bytes from the
  // file.
  std::string s(hdr_size, '\0');
  in.read(&s[0], hdr_size);

  // Parse from the buffer into a protobuf message.
  BlockStreamHeader hdr;
  bool success = hdr.ParseFromString(s);
  if(!success){
    return false;
  }
  
  // Check the magic phrase.
  if(!hdr.has_magicphrase() or hdr.magicphrase() != BlockStreamReader::magicphrase){
    return false;
  }

  // Record the current position (which is also where the data
  // begins).  Find the eof filepos - seek to the end of the file and
  // record the position there, then close the file (the stream is no
  // longer needed).
  const std::streampos data_start = in.tellg();
  in.seekg(0, std::ios_base::end);
  const std::streampos eofpos = in.tellg();
  in.close();

  // Set up the stream pool.
  //
  // If the number of streams isn't specified, compute the maximum
  // number of possible streams supported in this environment.
  if(numstreams == 0){
    numstreams = MTV::probeMaxOpenStreams();
  }

  // For the actual number of streams use the minimum of the requested
  // number of sterams, and the number of streams actually present in
  // the file.
  numstreams = std::min(numstreams, static_cast<unsigned>(hdr.blockstream_size()));

  // Create the stream pool.
  pool = boost::make_shared<LRUIfstreamPool>(infile, numstreams);

  // Create block streams for the pool according to data in the block
  // stream header.
  for(int i=0; i<hdr.blockstream_size(); i++){
    // Grab the data for the block stream.
    const uint64_t blockAddr = hdr.blockstream(i).block_addr();
    const std::streampos offset = hdr.blockstream(i).offset();

    // Ensure that it's not a duplicate.
    if(stream.find(blockAddr) != stream.end()){
      // TODO(choudhury): use the error-message passing mechanism
      // instead of writing to cerr.
      std::cerr << "error: block address 0x" << std::hex << blockAddr << std::hex << " appears multiple times in blockstream file header." << std::endl;
      return false;
    }

    // Add the block stream.
    if(!pool->addBlockStream(blockAddr, data_start + offset)){
      // TODO(choudhury): use the error-message passing mechanism
      // instead of writing to cerr.
      std::cerr << "error: could not add block stream for block address 0x" << std::hex << blockAddr << std::dec << "." << std::endl;
      return false;
    }

    // Create a new information entry for the block stream.
    Stream& s = stream[blockAddr];

    // Set the limit point for the block stream - it is equal to
    // either the start point of the next block stream, or simple
    // end-of-file for the last block stream.
    s.limit = i < static_cast<int>(hdr.blockstream_size() - 1) ? data_start + static_cast<std::streampos>(hdr.blockstream(i+1).offset()) : eofpos;

    // Set the "next" value to 0 - this will be updated to the correct
    // value once the cache is warmed up.
    s.next = 0;
  }

  return true;
}

uint64_t BlockStreamReader::next(uint64_t blockAddr, uint64_t point){
  static const bool verbose = false;

  if(verbose){
    std::cout << "next() called for block address " << blockAddr << " at point " << point << std::endl;
  }

  // Enforce monotonically increasing points in calls to this
  // function.
  //
  // TODO(choudhury): is this necessary?
  static uint64_t curpoint = 0;
  if(point < curpoint){
    std::cerr << "error: requested point lies in the past." << std::endl;
    abort();
  }
  curpoint = point;

  // Unforeseen blocks don't need to be handled.
  if(stream.find(blockAddr) == stream.end()){
    std::cerr << "error: requested block address 0x" << std::hex << blockAddr << " not present in blockstream file." << std::endl;
    abort();
  }

  // Check to see if the information for the requested block is up to
  // date (i.e. its current value is BEYOND the specified trace point)
  // - if not, update it by reading from the file.
  Stream& s = stream[blockAddr];
  if(s.next < point){
    if(verbose){
      std::cout << "stream for " << blockAddr << " is out of date (next = " << s.next << ")" << std::endl;
    }

    // Read out records until the next appearance of the block exceeds
    // the current point.
    //
    // TODO(choudhury): use binary search to find the right value in
    // the file.
    std::ifstream *stream = pool->access(blockAddr);
    while(s.next < point and stream->tellg() < s.limit){
      const std::streampos cur = stream->tellg();
      stream->read(reinterpret_cast<char *>(&s.next), sizeof(s.next));
      if(verbose){
        std::cout << "...read next value " << s.next << " from file position " << cur << " (limit at " << s.limit << ")" << std::endl;
      }
    }

    // Check to see if we reached the limit for this stream - if so,
    // it means that the block does not appear again after the point.
    assert(stream->tellg() <= s.limit);
    if(stream->tellg() == s.limit and s.next < point){
      if(verbose){
        std::cout << "stream reached limit, setting next to max possible" << std::endl;
      }

      s.next = never;
    }
  }

  if(verbose){
    std::cout << "next() returning " << s.next << std::endl;
  }

  return s.next;
}
