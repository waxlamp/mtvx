// Copyright 2011 A.N.M. Imroz Choudhury
//
// mtr2blockstream.cpp - A program to convert a reference trace into
// its "block stream" format, consisting of several streams of
// appearances of each cache block represented in the reference trace.

// MTV headers.
#include <Core/Dataflow/BlockStream/BlockStreamReader.h>
#include <Core/Dataflow/BlockStream/LRUStreamPool.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Util/Boost.h>
#include <Core/BlockStreamHeader.pb.h>
using MTV::BlockStream::BlockStreamHeader;
using MTV::BlockStreamReader;
using MTV::LRUOfstreamPool;
using MTV::TraceReader;

// MTV headers.
#include <Core/Util/Util.h>

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <fstream>
#include <string>

void predump(const std::string& filename, unsigned prefix){
  std::ifstream in(filename.c_str());
  std::string buf(prefix, '\0');
  in.read(&(buf[0]), prefix);

  for(unsigned i=0; i<buf.length(); i++){
    printf("%02x ", buf[i]);
  }
  std::cout << std::endl;
}

int main(int argc, char *argv[]){
  std::string infile, outfile;
  unsigned blocksize, numstreams;

  try{
    TCLAP::CmdLine cmd("Convert a trace file to its 'block stream' format.");
    
    // Input file.
    TCLAP::ValueArg<std::string> infileArg("i",
                                           "input",
                                           "Input filename",
                                           true,
                                           "",
                                           "filename",
                                           cmd);

    // Output file.
    TCLAP::ValueArg<std::string> outfileArg("o",
                                            "output",
                                            "Output filename",
                                            true,
                                            "",
                                            "filename",
                                            cmd);

    // Block size.
    TCLAP::ValueArg<unsigned> blocksizeArg("b",
                                           "block-size",
                                           "Size of cache blocks (in bytes)",
                                           true,
                                           0,
                                           "size",
                                           cmd);

    // Number of filestreams to use.
    TCLAP::ValueArg<unsigned> numstreamsArg("n",
                                            "num-streams",
                                            "Number of file streams to use for caching",
                                            true,
                                            0,
                                            "number",
                                            cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract values.
    infile = infileArg.getValue();
    outfile = outfileArg.getValue();
    blocksize = blocksizeArg.getValue();
    numstreams = numstreamsArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open the trace file.
  TraceReader::ptr trace = boost::make_shared<TraceReader>();
  if(!trace->open(infile)){
    std::cerr << "error: could not open file '" << infile << "' for reading." << std::endl;
    exit(1);
  }

  // Open the output file.
  std::ofstream out(outfile.c_str());
  if(!out){
    std::cerr << "error: could not open file '" << outfile << "' for writing." << std::endl;
    exit(1);
  }

  // Make a pass through the trace file, counting (1) how many unique
  // block addresses show up and (2) how many times each appears in
  // the trace.
  boost::unordered_map<uint64_t, uint64_t> appearances;
  try{
    while(true){
      // Read out a record.
      const MTR::Record& rec = trace->nextRecord();

      // Skip non-memory-type records.
      if(MTR::type(rec) != MTR::Record::MType){
        continue;
      }

      // Compute the block address.
      const uint64_t blockAddr = rec.addr / blocksize;

      // Increment the count of appearances of this address.
      appearances[blockAddr]++;
    }
  }
  catch(TraceReader::End){}

  // Report the counts.
  for(boost::unordered_map<uint64_t, uint64_t>::const_iterator i = appearances.begin();
      i != appearances.end();
      i++){
    std::cerr << "block " << i->first << " appears " << i->second << " times." << std::endl;
  }

  // Write out a header to the output file.
  //
  // First create the header object and set its general attributes.
  BlockStreamHeader hdr;
  hdr.set_magicphrase(BlockStreamReader::magicphrase);
  hdr.set_blocksize(blocksize);

  // Add information about each block stream in the trace.
  std::streampos curpos = 0;
  for(boost::unordered_map<uint64_t, uint64_t>::const_iterator i = appearances.begin(); i != appearances.end(); i++){
    // Create a block stream message object.
    BlockStreamHeader::BlockStream *bs = hdr.add_blockstream();

    // Set the attributes for it.
    bs->set_block_addr(i->first);
    bs->set_offset(curpos);

    // Bump the position counter by the appropriate length.
    curpos += i->second*sizeof(uint64_t);
  }

  // Write the header out to disk.
  //
  // First, write out the size of the header.
  //
  // TODO(choudhury): use network byte order to write out the value.
  const int size = hdr.ByteSize();
  out.write(reinterpret_cast<const char *>(&size), sizeof(size));

  // Serialize the header to the output file.
  const std::streampos before = out.tellp();
  hdr.SerializeToOstream(&out);
  out.flush();
  const std::streampos after = out.tellp();

  // // DEBUG(choudhury): show the bytes in hex as they are in memory.
  // //
  // // Serialize to a string.
  // std::stringstream ss;
  // hdr.SerializeToOstream(&ss);
  // std::cout << "in memory: ";
  // for(unsigned i=0; i<ss.str().length(); i++){
  //   printf("%02x ", ss.str()[i]);
  // }
  // std::cout << std::endl;

  // std::cout << "on disk: ";
  // predump(outfile, size + sizeof(int));

  std::cerr << "before writing header: " << before << std::endl
            << "after writing header: " << after << std::endl
            << "diff: " << (after - before) << std::endl
            << "header size: " << size << std::endl;

  std::cerr << hdr.blockstream_size() << " unique blocks" << std::endl;

  // Save the current position of the put pointer.
  const std::streampos data_start = out.tellp();

  // Figure out how many streams are really needed.
  //
  // First, if the number of streams was requested as 0, perform a
  // probing test to see how many open files this process can actually
  // support.
  if(numstreams == 0){
    numstreams = MTV::probeMaxOpenStreams();
  }

  // Next, if the number of block streams is less than the nominal
  // number of filestreams, then reduce the number of filestreams used
  // for this run.
  if(static_cast<int>(numstreams) > hdr.blockstream_size()){
    numstreams = hdr.blockstream_size();
  }

  // Create an LRU stream pool object and populate it with the block
  // stream information.
  LRUOfstreamPool pool(outfile, numstreams);

  // Go through the header object, creating several blockstreams in
  // the LRU pool.  The fixed number of ofstream objects within it
  // will automatically match up in LRU manner with these block
  // streams.
  for(int i=0; i<hdr.blockstream_size(); i++){
    // Grab the block id and the file offset for the stream.
    const uint64_t blockAddr = hdr.blockstream(i).block_addr();
    const std::streampos offset = hdr.blockstream(i).offset();

    // Create a new block stream.
    std::cout << "adding block stream: id " << blockAddr << ", pos " << (data_start + offset) << std::endl;
    if(!pool.addBlockStream(blockAddr, data_start + offset)){
      std::cerr << "error: could not open file stream for block address " << blockAddr << std::endl;
      exit(1);
    }
  }

  // Report the number of block streams and the number of file
  // streams.
  std::cerr << pool.numBlockStreams() << " blockstreams." << std::endl
            << numstreams << " filestreams." << std::endl;

  // Make a second pass through the trace, this time inserting block
  // appearance information into the output trace as we go.
  //
  // Reopen the trace file and begin reading out records.
  trace = boost::make_shared<TraceReader>();
  if(!trace->open(infile)){
    std::cerr << "error: could not open file '" << infile << "' for reading." << std::endl;
    exit(1);
  }
  try{
    for(uint64_t trace_pos=0; ; trace_pos++){
      // Read out a record.
      const MTR::Record& rec = trace->nextRecord();

      // Compute its block address.
      const uint64_t blockAddr = rec.addr / blocksize;

      // Have the stream pool write out the appropriate data.
      pool.access(blockAddr)->write(reinterpret_cast<const char *>(&trace_pos), sizeof(trace_pos));

      // // DEBUG(choudhury): dump the header after each write.
      // predump(outfile, size + sizeof(int));
    }
  }
  catch(TraceReader::End){}

  // NOTE(choudhury): the stream pool's destructor will destroy its
  // file streams, flushing all data to disk.
  return 0;
}
