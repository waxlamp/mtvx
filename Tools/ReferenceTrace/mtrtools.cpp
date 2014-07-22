#include "mtrtools.h"

// MTV headers.
#include <Core/Util/Boost.h>

// Tinyxml headers.
#define TIXML_USE_STL
#include <tinyxml.h>

// System headers.
#include <iostream>

using namespace MTR;

Record::Type MTR::type(const Record& rec){
  Record::Type type = Record::NoType;

  switch(rec.code){
  case Record::Read:
  case Record::Write:
    type = Record::MType;
    break;

  case Record::LineNumber:
    type = Record::LType;
    break;

  case Record::FramePointer:
  case Record::ScopeEntry:
  case Record::ScopeExit:
  case Record::FunctionEntry:
  case Record::FunctionExit:
    type = Record::OtherType;
    break;

  case Record::NoCode:
    type = Record::NoType;
    break;
  }

  return type;
}

bool RegionRegistration::read(const char *filename){
  // Create an XML document object.
  TiXmlDocument doc(filename);
  if(!doc.LoadFile()){
    // std::stringstream ss;
    // ss << "error: could not load file '" << filename << "'.";
    // error_message = ss.str();
    *this << "error: could not load file '" << filename << "'.";
    return false;
  }

  // Parse the file.
  //
  // Begin my making sure the root is a registration element.
  TiXmlElement *root = doc.RootElement();
  if(std::string(root->Value()) != "AddressRegistration"){
    *this << "error: xml document does not contain a root AddressRegistration element.";
    return false;
  }

  // Process the TraceSignal element.
  TiXmlElement *e = root->FirstChildElement("TraceSignal");
  if(!e){
    *this << "error: file `" << filename << "' does not contain a 'TraceSignal' element.";
    return false;
  }
  else if(e->NextSiblingElement("TraceSignal")){
    *this << "error: file `" << filename << "' contains more than one 'TraceSignal' element.";
    return false;
  }

  // Get the "base" attribute - the starting address of the set of
  // trace signal addresses.
  const char *text  = e->Attribute("base");
  if(!text){
    *this << "error: element 'TraceSignal' in file `" << filename << "' has no 'base' attribute.";
    return false;
  }

  // Convert the (hex-formatted) string to an address value.
  std::stringstream(text) >> std::hex >> barriers.base;

  // Get the "num" attribute - the number of addresses used as signals.
  text = e->Attribute("num");
  if(!text){
    *this << "error: element 'TraceSignal' in file `" << filename << "' has no 'num' attribute.";
    return false;
  }
  barriers.size = lexical_cast<unsigned>(text);

  // Process the MessageSpace element.
  e = root->FirstChildElement("MessageSpace");
  if(!e){
    *this << "error: file `" << filename << "' does not contain a 'MessageSpace' element.";
    return false;
  }
  else if(e->NextSiblingElement("MessageSpace")){
    *this << "error: file `" << filename << "' contains more than one 'MessageSpace' element.";
    return false;
  }

  // Get the base attribute.
  text = e->Attribute("base");
  if(!text){
    *this << "error: element 'MessageSpace' in file `" << filename << "' has no 'base' attribute.";
    return false;
  }

  // Parse out the address.
  std::stringstream(text) >> std::hex >> messagespace.base;
  messagespace.size = 129;
  messagespace.type = 1;
  messagespace.title = "Message Space";

  // Process the Regions element.
  e = root->FirstChildElement("Regions");
  if(!e){
    *this << "error: file '" << filename << "' does not contain a 'Regions' element.";
    return false;
  }
  else if(e->NextSiblingElement("Regions")){
    *this << "error: file '" << filename << "' contains more than one 'Regions' element.";
    return false;
  }

  // Process the Region elements.
  for(TiXmlElement *ee = e->FirstChildElement("Region"); ee; ee = ee->NextSiblingElement("Region")){
    const char *layout = ee->Attribute("layout");
    const char *title = ee->Attribute("title");
    const char *base_text = ee->Attribute("base");
    const char *size_text = ee->Attribute("size");
    const char *type_text = ee->Attribute("type");

    if(!layout or !title or !base_text or !size_text or !type_text){
      *this << "error:  file '" << filename << "' is missing a 'layout', 'title', 'base', 'size', or 'type' attribute.";
      return false;
    }

    addr_t base;
    size_t size, type;

    std::stringstream(base_text) >> std::hex >> base;
    size = lexical_cast<addr_t>(size_text);
    type = lexical_cast<addr_t>(type_text);

    if(std::string(layout) == "array"){
      arrayRegions.push_back(ArrayRegion(base, size, type, title));
    }
    else if(std::string(layout) == "matrix"){
      const char *rows_text = ee->Attribute("rows");
      const char *cols_text = ee->Attribute("cols");

      int rows, cols;
      rows = lexical_cast<int>(rows_text);
      cols = lexical_cast<int>(cols_text);

      matrixRegions.push_back(MatrixRegion(base, size, type, rows, cols, title));
    }
    else{
      *this << "error: file '" << filename << "' has a Regions element of 'layout' other than 'array' or 'matrix'.";
      return false;
    }
  }

  return true;
}
