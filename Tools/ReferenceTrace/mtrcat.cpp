// Copyright 2010 A.N.M. Imroz Choudhury
//
// mtrcat.cpp - A program for listing out a reference trace in
// human-readable form.

// TCLAP includes.
#include <tclap/CmdLine.h>

// MTV includes.
#include <Core/Dataflow/Printer.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Dataflow/TraceWriter.h>
using MTV::Printer;
using MTV::TraceReader;
using MTV::TraceWriter;

// System includes.
#include <string>
#include <vector>

bool readout(const std::string& filename){
  // Try to open the file; if it can't be opened, bail out.
  TraceReader::ptr in(new TraceReader);
  if(!in->open(filename)){
    return false;
  }

  switch(in->getEncoding()){
  case TraceWriter::Raw:
    std::cout << "encoding:raw" << std::endl;
    break;

  case TraceWriter::Gzip:
    std::cout << "encoding:gzip" << std::endl;

  case TraceWriter::AddressRaw:
  case TraceWriter::AddressGzip:
    std::cerr << "error: AddressRaw and AddressGzip encodings not yet implemented." << std::endl;
    abort();
  }

  // Create a Printer object.
  Printer<MTR::Record>::ptr p(new Printer<MTR::Record>);
  in->addConsumer(p);

  // Read out records from the trace until the End exception is thrown.
  try{
    while(true){
      // This call will extract the next record and forward it to the
      // printer.
      in->nextRecord();
    }
  }
  catch(TraceReader::End){}
  return true;
}

int main(int argc, char *argv[]){
  std::vector<std::string> files;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("List out reference trace files in a human readable format.");

    // The names of the files to work on.
    TCLAP::UnlabeledMultiArg<std::string> filesArg("filenames",
                                                   "Files to be processed by mtrcat.",
                                                   true,
                                                   "filenames",
                                                   cmd);

    // Parse the command line.
    cmd.parse(argc, argv);

    // Extract the list of filenames.
    files = filesArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Check to see that at least one filename was supplied.
  if(files.size() == 0){
    std::cerr << "error: Must specify at least one trace file." << std::endl;
    exit(1);
  }

  // Process each file by passing to the readout function.
  for(std::vector<std::string>::const_iterator i = files.begin(); i != files.end(); i++){
    if(!readout(*i)){
      std::cerr << argv[0] << ": " << *i << ": no such file" << std::endl;
    }
  }

  return 0;
}
