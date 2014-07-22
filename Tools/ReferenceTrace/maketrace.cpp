// Copyright 2010 A.N.M. Imroz Choudhury
//
// maketrace.cpp - Reads in text-based reference records and writes
// them out in trace format.

// TCLAP includes.
#include <tclap/CmdLine.h>

// MTV includes.
#include <Core/Util/Boost.h>
#include <Core/Dataflow/TraceWriter.h>
#include <Tools/ReferenceTrace/mtrtools.h>
using MTV::TraceWriter;

// System includes.
#include <string>

int main(int argc, char *argv[]){
  std::string outfile;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("Read in human readable trace records and write them out to a trace file.");

    // The names of the files to work on.
    TCLAP::ValueArg<std::string> outfileArg("o",
                                            "output",
                                            "Output trace file",
                                            true,
                                            "",
                                            "filename",
                                            cmd);

    // Parse the command line.
    cmd.parse(argc, argv);

    // Extract the list of filenames.
    outfile = outfileArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Create the trace writer.
  //
  // TraceWriter::ptr writer(new TraceWriter(TraceWriter::Gzip, 1024));
  TraceWriter::ptr writer(new TraceWriter(TraceWriter::Raw, 1024));
  if(!writer->open(outfile)){
    std::cerr << "error: could not open file '" << outfile << "' for output" << std::endl;
    exit(1);
  }

  while(true){
    std::string code, value;

    std::cin >> code >> value;
    if(std::cin.eof()){
      break;
    }

    MTR::Record rec;
    if(code == "R"){
      rec.code = MTR::Record::Read;
    }
    else if(code == "W"){
      rec.code = MTR::Record::Write;
    }
    else{
      std::cerr << "error: code must be 'R' or 'W'" << std::endl;
      exit(1);
    }

    std::stringstream ss(value);
    ss >> std::hex >> rec.addr;

    // rec.addr = lexical_cast<MTR::addr_t>(value);

    writer->consume(rec);
  }

  // writer->flush();

  return 0;
}
