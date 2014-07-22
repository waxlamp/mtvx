// Copyright 2011 A.N.M. Imroz Choudhury
//
// blockstreamdump.cpp - Produces a human-readable dump of a
// blockstream file, for verification or inspection purposes.

// MTV headers.
#include <Core/BlockStreamHeader.pb.h>
#include <Core/Dataflow/BlockStream/LRUStreamPool.h>
using MTV::BlockStream::BlockStreamHeader;
using MTV::LRUIfstreamPool;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <fstream>
#include <string>

int main(int argc, char *argv[]){
  std::string infile;

  try{
    TCLAP::CmdLine cmd("Dump a block stream file to a human-readable format.");
    
    // Input file.
    TCLAP::ValueArg<std::string> infileArg("i",
                                           "input",
                                           "Input filename",
                                           true,
                                           "",
                                           "filename",
                                           cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract values.
    infile = infileArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open the input file.
  std::ifstream in(infile.c_str());
  if(!in){
    std::cerr << "error: could not open file '" << infile << "' for reading." << std::endl;
    exit(1);
  }

  // Read out the size of the header.
  //
  // TODO(choudhury): assume the value is written on disk in network
  // byte order.
  int size;
  in.read(reinterpret_cast<char *>(&size), sizeof(size));

  // Read out the header.
  //
  // First read out the appropriate number of bytes into a buffer.
  std::string buf(size, '\0');
  in.read(&buf[0], size);

  // Store the current file pointer - this is the "data start"
  // position.
  const std::streampos data_start = in.tellg();

  std::cout << "Data start position: " << data_start << std::endl;

  // Now extract the header message from the bytestream.
  std::stringstream ss(buf);
  BlockStreamHeader hdr;
  hdr.ParseFromIstream(&ss);

  std::cout << "Block size: " << hdr.blocksize() << " bytes" << std::endl;
  std::cout << hdr.blockstream_size() << " unique blocks" << std::endl;

  for(int i=0; i<hdr.blockstream_size(); i++){
    std::cout << "Block " << hdr.blockstream(i).block_addr() << " (offset " << hdr.blockstream(i).offset() << "):";

    // Grab the start position of the data.
    const std::streampos start = data_start + static_cast<std::streampos>(hdr.blockstream(i).offset());

    // Figure out where this stream ends.
    std::streampos end;
    if(i < hdr.blockstream_size() - 1){
      // The start of the next blockstream is the end of the current
      // blockstream.
      end = data_start + static_cast<std::streampos>(hdr.blockstream(i+1).offset());
    }
    else{
      // For the final blockstream, the end is the EOF position.
      const std::streampos save = in.tellg();
      in.seekg(0, std::ios_base::end);
      end = in.tellg();
      in.seekg(save);
    }

    // Start reading out numbers from the stream.
    in.seekg(start);
    while(in.tellg() != end){
      uint64_t next;
      in.read(reinterpret_cast<char *>(&next), sizeof(next));
      std::cout << " " << next;
    }
    std::cout << std::endl;
  }

  return 0;
}
