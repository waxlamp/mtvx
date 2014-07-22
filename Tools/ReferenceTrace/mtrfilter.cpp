// Copyright 2010 A.N.M. Imroz Choudhury
//
// mtrfilter.cpp - A program for reading in a reference trace,
// transforming it by only keeping certain records, and saving the
// result.

// TCLAP headers.
#include <tclap/CmdLine.h>

// MTV includes.
#include <Core/Dataflow/AddressRangeFilter.h>
#include <Core/Dataflow/BaseAddressLocator.h>
#include <Core/Dataflow/Consumer.h>
#include <Core/Dataflow/Filter.h>
#include <Core/Dataflow/LineRecordFilter.h>
#include <Core/Dataflow/MemoryRecordFilter.h>
#include <Core/Dataflow/TraceReader.h>
#include <Core/Dataflow/TraceWriter.h>
using MTV::AddressRangePass;
using MTV::ConstantBaseAddress;
using MTV::Consumer;
using MTV::Filter;
using MTV::LineRecordFilter;
using MTV::MemoryRecordFilter;
using MTV::TraceReader;
using MTV::TraceWriter;

class RecordHolder : public Consumer<MTR::Record> {
public:
  BoostPointers(RecordHolder);

public:
  void consume(const MTR::Record& r){
    rec = r;
  }

  MTR::Record rec;
};

int main(int argc, char *argv[]){
  std::string inputfile, outputfile, regfile, encodingSpec;
  std::vector<std::string> rangetext;
  bool stackInfo;
  // bool invert;

  try{
    // Create a command line parser.
    TCLAP::CmdLine cmd("Filter a reference trace into a new reference trace");

    // The input file.
    TCLAP::ValueArg<std::string> inputfileArg("i",
                                              "input",
                                              "input reference trace file",
                                              true,
                                              "",
                                              "filename",
                                              cmd);

    // The output file.
    TCLAP::ValueArg<std::string> outputfileArg("o",
                                               "output",
                                               "output reference trace file",
                                               true,
                                               "",
                                               "filename",
                                               cmd);

    // Multiple address ranges by which to filter.
    TCLAP::MultiArg<std::string> rangesArg("a",
                                           "address-range",
                                           "Range of addresses to filter (may be specified multiple times).",
                                           false,
                                           "address pair (comma separated)",
                                           cmd);

    // Optional registration file by which to filter.
    TCLAP::ValueArg<std::string> regfileArg("r",
                                            "registration-file",
                                            "Registration file by which to filter.",
                                            false,
                                            "",
                                            "filename",
                                            cmd);

    TCLAP::ValueArg<std::string> encodingSpecArg("e",
                                                 "encoding",
                                                 "Encoding to use for the output file.",
                                                 false,
                                                 "",
                                                 "string ('raw' or 'gzip')",
                                                 cmd);

    TCLAP::SwitchArg stackInfoArg("s",
                                  "stack-info",
                                  "Whether stack information is contained within the trace file.",
                                  cmd,
                                  false);

    // // Switch to indicate whether to exclude the registration file,
    // // rather than include.
    // TCLAP::SwitchArg invertArg("x",
    //                            "exclude-registration",
    //                            "Excludes (rather than includes) the address ranges in the registration file.",
    //                            cmd);

    // Parse the command line.
    cmd.parse(argc, argv);

    // Extract the arguments.
    inputfile = inputfileArg.getValue();
    outputfile = outputfileArg.getValue();
    rangetext = rangesArg.getValue();
    regfile = regfileArg.getValue();
    encodingSpec = encodingSpecArg.getValue();
    stackInfo = stackInfoArg.getValue();
    // invert = invertArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Open a trace reader.
  TraceReader::ptr reader(new TraceReader);
  if(!reader->open(inputfile)){
    std::cerr << "error: could not open input trace file '" << inputfile << "'." << std::endl;
    exit(1);
  }

  // Figure out what output encoding to use.
  TraceWriter::Encoding encoding;
  if(encodingSpec == "raw"){
    encoding = TraceWriter::Raw;
  }
  else if(encodingSpec == "gzip"){
    encoding = TraceWriter::Gzip;
  }
  else if(encodingSpec == ""){
    encoding = reader->getEncoding();
  }
  else{
    std::cerr << "error: illegal encoding spec '" << encodingSpec << "' (must be 'raw' or 'gzip')." << std::endl;
    exit(1);
  }

  // Open a trace writer.
  TraceWriter::ptr writer(new TraceWriter(encoding));
  if(!writer->open(outputfile)){
    std::cerr << "error: could not open output file '" << outputfile << "'." << std::endl;
    exit(1);
  }

  // If there are no address ranges or registration file specified,
  // then just crop out everything before the first line record.
  if(rangetext.size() == 0 and regfile == ""){
    std::cerr << "cropping prelude..." << std::flush;

    RecordHolder::ptr h = boost::make_shared<RecordHolder>();
    reader->addConsumer(h);

    bool copy = false;
    try{
      if(not stackInfo){
        while(true){
          // Produce a record.
          reader->nextRecord();

          // If we have seen a non-memory record already, then copy this
          // record straight to the output; otherwise, check for
          // non-memory records until one is found.
          if(copy){
            writer->consume(h->rec);
          }
          else{
            if(type(h->rec) != MTR::Record::MType){
              copy = true;

              // Copy this record to the output.
              writer->consume(h->rec);
            }
          }
        }
      }
      else{
        MTR::addr_t func = 0x0;
        while(true){
          // Produce a record.
          reader->nextRecord();

          // If we have seen a FunctionEntry record already, then copy
          // this record straight to the output; otherwise, check for
          // FunctionEntry records until one is found.  Continue with
          // such copying until the intially entered function is exited.
          if(copy){
            writer->consume(h->rec);
            if(h->rec.code == MTR::Record::FunctionExit and h->rec.addr == func){
              std::cerr << "found exit of initial function" << std::endl;
              throw TraceReader::End();
            }
          }
          else{
            if(h->rec.code == MTR::Record::FunctionEntry){
              copy = true;
              func = h->rec.addr;

              // Copy this record to the output.
              writer->consume(h->rec);
            }
          }
        }
      }
    }
    catch(TraceReader::End){}

    std::cerr << "done" << std::endl;

    writer->flush();
    return 0;
  }

  // TODO(choudhury): Create address range filters from the text in
  // the ranges argument, hook the trace reader to them, and then hook
  // them to the trace writer object.

  // Create a memory record and a line record filter.
  MemoryRecordFilter::ptr mfilter = boost::make_shared<MemoryRecordFilter>();
  LineRecordFilter::ptr lfilter = boost::make_shared<LineRecordFilter>();

  // Hook the reader to both.
  reader->addConsumer(mfilter);
  reader->addConsumer(lfilter);

  // Hook the line record filter directly up to the writer.
  lfilter->addConsumer(writer);

  // Open the registration file, if it was specified.
  if(regfile != ""){
    MTR::RegionRegistration reg;
    reg.read(regfile.c_str());
    if(reg.hasError()){
      std::cerr << "error: " << reg.error() << std::endl;
      exit(1);
    }

    // Create an address range filter for all the ranges in the file,
    // hook the memory record filter to it, and then hook it to the
    // writer.
    AddressRangePass::ptr filter(new AddressRangePass);
    for(unsigned i=0; i<reg.arrayRegions.size(); i++){
      filter->addRange(reg.arrayRegions[i].base, reg.arrayRegions[i].base + reg.arrayRegions[i].size);
      // filter->addRange(boost::make_shared<ConstantBaseAddress>(reg.arrayRegions[i].base),
      //                  boost::make_shared<ConstantBaseAddress>(reg.arrayRegions[i].base + reg.arrayRegions[i].size));
    }
    mfilter->addConsumer(filter);
    filter->Filter<MTR::Record>::addConsumer(writer);
  }
  else{
    // Just connect the reader to the writer.
    reader->addConsumer(writer);
  }

  // Run the entire trace file.
  try{
    while(true){
      reader->nextRecord();
    }
  }
  catch(TraceReader::End){}

  // Flush the writer object.
  writer->flush();

  // Upon exit, the writer will close the file and clean up after
  // itself.
  return 0;
}
