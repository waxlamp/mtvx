// Copyright 2011 A.N.M. Imroz Choudhury
//
// fdcat.cpp - Prints framedump information from protocol buffers.

// MTV headers.
#include <Core/FrameDump.pb.h>
namespace FD = MTV::FrameDump;

// System headers.
#include <fstream>
#include <iostream>
#include <string>

const char *yes_no(bool cond){
  return cond ? "yes" : "no";
}

int main(int argc, char *argv[]){
  // TODO(choudhury): replace with tclap - add option for displaying
  // header only.
  if(argc != 2){
    std::cerr << "usage: fdcat <framedumpfile>" << std::endl;
    exit(1);
  }

  // Open the file.
  const char *filename = argv[1];
  std::ifstream in(filename);
  if(!in){
    std::cerr << "error: could not open file '" << filename << "' for reading." << std::endl;
    exit(1);
  }

  // Read out the header.
  //
  // Start by reading out the size of the header.
  int hdr_size;
  in.read(reinterpret_cast<char *>(&hdr_size), sizeof(hdr_size));

  // Allocate a buffer to hold the header.
  std::string hdr_buf(hdr_size+1, '\0');
  in.read(&hdr_buf[0], hdr_size);

  // Parse out the header from the buffer.
  FD::FrameDumpHeader hdr;
  hdr.ParseFromString(hdr_buf);

  // Print out the header.
  std::cout << "Creation date: " << hdr.date() << std::endl
            << "Trace file: " << hdr.trace_path() << std::endl
            << "Registration file: " << (hdr.has_registration() ? hdr.registration().path() : "(none)") << std::endl
            << "Cache specification file: " << (hdr.has_cache_spec() ? hdr.cache_spec().path() : "(none)") << std::endl
            << "Source code info: " << yes_no(hdr.source_file_size() > 0) << std::endl
            << "Line visit count info: " << yes_no(hdr.visit_count()) << std::endl
            << "Line miss count info: " << yes_no(hdr.miss_count()) << std::endl
            << "Cache set utilization info: " << yes_no(hdr.cache_set_utilization()) << std::endl << std::endl;

  // Print out the registration file.
  if(hdr.has_registration()){
    std::cout << "Registration file " << hdr.registration().path() << ":" << std::endl;
    std::cout << hdr.registration().text() << std::endl;
  }

  // Print out the cache spec file.
  if(hdr.has_cache_spec()){
    std::cout << "Cache spec file " << hdr.cache_spec().path() << ":" << std::endl;
    std::cout << hdr.cache_spec().text() << std::endl;
  }

  // Print out each source file.
  for(int i=0; i<hdr.source_file_size(); i++){
    const FD::FrameDumpHeader::SourceFile& sf = hdr.source_file(i);

    std::cout << "Source file " << sf.index() << ", " << sf.path() << ":" << std::endl;
    std::cout << sf.text() << std::endl;
  }

  // Keep a count of the number of frames encountered.
  unsigned count = 0;

  // Begin reading out records from the file.
  while(true){
    // First read out an int.
    int size;
    in.read(reinterpret_cast<char *>(&size), sizeof(size));

    // Bail out if the read failed.
    if(in.eof()){
      break;
    }

    // Keep a count of the total number of frames.
    ++count;

    // Otherwise, read out that many bytes into a string object.
    std::string buf(size+1, '\0');
    in.read(&buf[0], size);

    // Parse the protobuffer from the string.
    FD::FrameDump frame;
    frame.ParseFromString(buf);

    // Print out the contents.
    std::cout << "Frame " << std::dec << count << ":" << std::endl;

    // First print out the source code (if any).
    if(frame.source_code_size() > 0){
      std::cout << "source_code:" << std::endl;

      for(int i=0; i<frame.source_code_size(); i++){
        std::cout << "\t" << "file: " << frame.source_code(i).file() << std::endl
                  << "\t" << "line: " << frame.source_code(i).line() << std::endl;
      }
    }

    // Next, the line visit counts.
    if(frame.has_visit_count()){
      std::cout << "line visit count:" << std::endl;
      const FD::FrameDump::LineVisitCount& lvc = frame.visit_count();
      for(int i=0; i<lvc.count_size(); i++){
        std::cout << "\t" << "Record " << i << std::endl
                  << "\t\t" << "file: " << lvc.count(i).file() << std::endl
                  << "\t\t" << "line: " << lvc.count(i).line() << std::endl
                  << "\t\t" << "count: " << lvc.count(i).count() << std::endl;
      }
    }

    // Line miss counts.
    if(frame.has_miss_count()){
      std::cout << "line miss count:" << std::endl;
      const FD::FrameDump::LineMissCount& lmc = frame.miss_count();
      for(int i=0; i<lmc.count_size(); i++){
        std::cout << "\t" << "Record " << i << std::endl
                  << "\t\t" << "file: " << lmc.count(i).file() << std::endl
                  << "\t\t" << "line: " << lmc.count(i).line() << std::endl
                  << "\t\t" << "misses: " << lmc.count(i).misses() << std::endl
                  << "\t\t" << "total: " << lmc.count(i).total() << std::endl;
      }
    }

    // Cache set utilization rates.
    if(frame.has_cache_set_utilization()){
      std::cout << "cache set utilization rates:" << std::endl;
      const FD::FrameDump::CacheSetUtilization& csu = frame.cache_set_utilization();
      for(int i=0; i<csu.utilization_size(); i++){
        std::cout << "\t" << "set " << i << ": " << csu.utilization(i) << std::endl;
      }
    }

    // Print the cache temperatures.
    if(frame.temperature_size() > 0){
      std::cout << "temperature:" << std::endl;
      for(int i=0; i<frame.temperature_size(); i++){
        std::cout << "\tL" << (i+1) << ": " << frame.temperature(i) << std::endl;
      }
    }

    for(int i=0; i<frame.glyph_size(); i++){
      std::cout << "Glyph " << i << ":" << std::endl;

      const FD::FrameDump::Glyph& g = frame.glyph(i);

      std::cout << "\t" << "id: " << std::hex << g.id() << std::endl
                << "\t" << "ghost: " << std::dec << g.ghost() << std::endl
                << "\t" << "x: " << g.x() << std::endl
                << "\t" << "y: " << g.y() << std::endl
                << "\t" << "color: " << g.color() << std::endl;
    }

    for(int i=0; i<frame.activity_size(); i++){
      std::cout << "Activity " << std::dec << i << ":" << std::endl;

      const FD::FrameDump::Activity& a = frame.activity(i);

      switch(a.type()){
      case FD::FrameDump::Activity::ColorPulseActivity:
        std::cout << "\t" << "type: color pulse" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl
                  << "\t" << "color: " << std::dec << a.color_pulse().color() << std::endl
                  << "\t" << "apex: " << a.color_pulse().apex() << std::endl;
        break;

      case FD::FrameDump::Activity::SizePulseActivity:
        std::cout << "\t" << "type: size pulse" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl
                  << "\t" << "size: " << std::dec << a.size_pulse().size() << std::endl
                  << "\t" << "apex: " << a.size_pulse().apex() << std::endl;
        break;

      case FD::FrameDump::Activity::ColorChangeActivity:
        std::cout << "\t" << "type: color change" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl
                  << "\t" << "color: " << std::dec << a.color_change().color() << std::endl;
        break;

      case FD::FrameDump::Activity::PolarInterpolationActivity:
        std::cout << "\t" << "type: polar interpolation" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl;
        break;

      case FD::FrameDump::Activity::LinearInterpolationActivity:
        std::cout << "\t" << "type: linear interpolation" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl;
        break;

      case FD::FrameDump::Activity::BirthActivity:
        std::cout << "\t" << "type: birth" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl;
        break;

      case FD::FrameDump::Activity::DeathActivity:
        std::cout << "\t" << "type: death" << std::endl
                  << "\t" << "id: " << std::hex << a.id() << std::endl;
        break;
      }
    }
  }

  std::cout << std::dec << count << " frames total." << std::endl;
  return 0;
}
