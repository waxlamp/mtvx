// Copyright 2010 A.N.M. Imroz Choudhury
//
// mtrconvert.cpp - Converts a trace file to an address-only format
// (for when the read code isn't needed, and there are no line number
// records).

// MTV headers.
#include <Core/Dataflow/TraceReader.h>
#include <Core/Dataflow/TraceWriter.h>
#include <Core/Util/Boost.h>
using MTV::TraceReader;
using MTV::TraceWriter;

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <fstream>
#include <string>
#include <vector>

void bufflush(std::ostream& out, const std::vector<MTR::addr_t>& buf, unsigned& p){
  // Dump the current buffer contents to disk and reset the pointer.
  out.write(reinterpret_cast<const char *>(&buf[0]), p*sizeof(buf[0]));
  p = 0;
}

int main(int argc, char *argv[]){
  std::string infile, outfile;
  bool zip;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("Convert a trace file to a binary address-only format.");

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

    // Whether to use gzip filters.
    TCLAP::SwitchArg zipArg("z",
                            "gzip",
                            "Use gzip filters",
                            cmd);

    // Parse.
    cmd.parse(argc, argv);

    // Collect arguments.
    infile = infileArg.getValue();
    outfile = outfileArg.getValue();
    zip = zipArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open the trace file.
  TraceReader::ptr trace(boost::make_shared<TraceReader>());
  if(!trace->open(infile)){
    std::cerr << "error: cannot open trace file '" << infile << "' for reading." << std::endl;
    exit(1);
  }

  // Open the output file.
  std::ofstream file(outfile.c_str());
  if(!file){
    std::cerr << "error: cannot open output file '" << outfile << "' for writing." << std::endl;
    exit(1);
  }

  // Write out the magic number and the encoding.
  file << TraceWriter::magicphrase << std::flush;
  const unsigned code = static_cast<unsigned>(zip ? TraceWriter::AddressGzip : TraceWriter::AddressRaw);
  file.write(reinterpret_cast<const char *>(&code), sizeof(code));

  // Set up a filtering ostreambuf.
  filtering_ostreambuf outbuf;
  if(zip){
    outbuf.push(gzip_compressor());
  }
  outbuf.push(file);

  // Set up an ostream object associated with the ostreambuf.
  std::ostream out(&outbuf);

  // Set up a buffer and a pointer into it.
  const unsigned bufsize = 1024*1024;
  std::vector<MTR::addr_t> buffer(bufsize);
  unsigned p = 0;

  // Read out reference trace records and save their addresses in the
  // buffer.
  try{
    while(true){
      const MTR::Record& rec = trace->nextRecord();

      buffer[p++] = rec.addr;
      if(p == bufsize){
        bufflush(out, buffer, p);
      }
    }
  }
  catch(TraceReader::End){
    // Perform cleanup in case there is a partial buffer when the last
    // record is read.
    if(p > 0){
      bufflush(out, buffer, p);
    }
  }

  return 0;
}
