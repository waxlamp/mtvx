#include "registration.h"

#include <fstream>
#include <iostream>

using namespace MTR;

// extern "C"{
//   static char stoptrace = 'a';
//   static char starttrace = 'a';
//   static char toggletrace = 'a';
// }

// void Registrar::start_tracing(){
//   c = reinterpret_cast<char *>(&starttrace)[0];
// }

// void Registrar::stop_tracing(){
//   c = reinterpret_cast<char *>(&stoptrace)[0];
// }

// void Registrar::toggle_tracing(){
//   c = reinterpret_cast<char *>(&toggletrace)[0];
// }

void Registrar::array(addr_t base, size_t size, size_t type, const std::string& title){
  arrayRegions.push_back(ArrayRegion(base, size, type, title));
}

void Registrar::matrix(addr_t base, size_t size, size_t type, int rows, int cols, const std::string& title){
  matrixRegions.push_back(MatrixRegion(base, size, type, rows, cols, title));
}

void Registrar::allocate_barriers(int n){
  // Allocate data for the barriers.
  barrierdata = new char[n];

  // Create a record of the barriers themselves.
  barriers = ArrayRegion(reinterpret_cast<addr_t>(barrierdata), n, 1, "barriers");
}

void Registrar::allocate_message_space(){
  // Allocate data for 128 ascii codes, plus a control code meaning
  // "message start/stop".
  static const size_t len = 129;
  ascii = new char[len];

  // Create a record for trace processing.
  message_space = ArrayRegion(reinterpret_cast<addr_t>(ascii), len, sizeof(char), "ascii message space");
}

void Registrar::message(const std::string& msg) const {
  // Toggle "message mode" by writing to the message toggle control
  // location.
  ascii[message_toggle] = 'a';

  // Write out the message character by character, by indexing into
  // the ascii array at positions encoding the values of the
  // characters in the message.
  for(unsigned i=0; i<msg.length(); i++){
    ascii[static_cast<int>(msg[i])] = 'a';
  }

  // Toggle "message mode" once more, to signify the end of the
  // message (analogous to a null byte in C-style strings).
  ascii[message_toggle] = 'a';
}

void Registrar::record(const char *filename) const {
  // TODO(choudhury): may need to change this to use a higher-level
  // random filename that eventually is placed inside the reference
  // trace repository by a shell script (or something).
  if(!filename)
    filename = "region_registration.xml";

  std::ofstream out(filename);
  if(!out){
    // TODO(choudhury): replace with appropriate exception.
    std::cerr << "error: file `" << filename << "' could not be opened for output." << std::endl;
    return;
  }

  IndentLevel indent;

  // XML header and root element tag.
  out << indent
      << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>" << std::endl
      << "<AddressRegistration>" << std::endl;

  // Barriers element.
  ++indent;
  out << indent
      << "<TraceSignal "
      << "base=\"0x" << std::hex << barriers.base << "\" "
      << "num=\"" << barriers.size << "\" />"
      << std::endl;

  // Ascii element.
  out << indent
      << "<MessageSpace base=\"0x" << std::hex << message_space.base << "\" />" << std::endl;

  // Regions element.
  out << indent
      << "<Regions num=\"" << (arrayRegions.size() + matrixRegions.size()) << "\">" << std::endl;

  // Region elements.
  ++indent;
  for(std::vector<ArrayRegion>::const_iterator i=arrayRegions.begin(); i != arrayRegions.end(); i++){
    out << indent
        << "<Region layout=\"array\" "
        << "title=\"" << i->title << "\" "
        << "base=\"0x" << std::hex << i->base << "\" "
        << "size=\"" << std::dec << i->size << "\" "
        << "type=\"" << std::dec << i->type << "\" "
        << "/>" << std::endl;

    // ++indent;
    // out << indent << "<title>" << i->title << "</title>" << std::endl
    //     << indent << "<base>0x" << std::hex << i->base << "</base>" << std::endl
    //     << indent << "<size>" << std::dec << i->size << "</size>" << std::endl
    //     << indent << "<type>" << i->type << "</type>" << std::endl;
    // --indent;

    // // Leave a blank line between elements.
    // if( (matrixRegions.size() > 0) || (i+1 != arrayRegions.end()) )
    //   out << std::endl;
  }
  --indent;

  for(std::vector<MatrixRegion>::const_iterator i=matrixRegions.begin(); i != matrixRegions.end(); i++){
    out << indent
        << "<Region layout=\"matrix\" "
        << "title=\"" << i->title << "\" "
        << "base=\"0x" << std::hex << i->base << "\" "
        << "size=\"" << std::dec << i->size << "\" "
        << "type=\"" << std::dec << i->type << "\" "
        << "rows=\"" << std::dec << i->rows << "\" "
        << "cols=\"" << std::dec << i->cols << "\" "
        << "/>" << std::endl;

    ++indent;

    // out << indent << "<title>" << i->title << "</title>" << std::endl
    //     << indent << "<base>0x" << std::hex << i->base << "</base>" << std::endl
    //     << indent << "<size>" << std::dec << i->size << "</size>" << std::endl
    //     << indent << "<type>" << i->type << "</type>" << std::endl
    //     << indent << "<rows>" << i->rows << "</rows>" << std::endl
    //     << indent << "<cols>" << i->cols << "</cols>" << std::endl;

    // --indent;

    // out << indent
    //     << "</region>" << std::endl;

    // Leave a blank line between elements.
    if(i+1 != matrixRegions.end())
      out << std::endl;
  }

  out << indent
      << "</Regions>" << std::endl;

  --indent;
  out << indent
      << "</AddressRegistration>" << std::endl;
}
