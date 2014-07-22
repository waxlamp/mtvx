// Copyright 2011 A.N.M. Imroz Choudhury
//
// CacheConstructor.cpp

// MTV headers.
#include <Core/Util/Boost.h>
#include <Tools/CacheSimulator/CacheConstructor.h>
#include <Tools/CacheSimulator/CacheSetConstructor.h>
using Daly::CacheConstructor;
using Daly::CacheSetConstructor;

// TinyXML headers.
#define TIXML_USE_STL
#include <tinyxml.h>

// System headers.
#include <map>

CacheSetConstructor::CacheSetConstructor(const std::string& filename, const std::string& bsfile, unsigned numstreams){
  // Create an XML document object.
  TiXmlDocument doc(filename.c_str());
  if(!doc.LoadFile()){
    *this << "error: could not open file '" << filename << "' for reading.";
    return;
  }

  // Parse the file.
  //
  // Begin by checking the name of the root element.
  TiXmlElement *root = doc.RootElement();
  if(std::string(root->Value()) != "CacheSet"){
    *this << "error: XML document '" << filename << "' does not contain a root CacheSet element.";
    return;
  }

  // Get the blocksize attribute.
  const char *text = root->Attribute("blocksize");
  if(!text){
    *this << "error: CacheSet element does not contain 'blocksize' attribute.";
    return;
  }

  unsigned blocksize = lexical_cast<unsigned>(text);
  //std::cout << "blocksize = " << blocksize << std::endl;

  // Find the SharedCacheLevels element and make sure it appears at most
  // once.
  TiXmlElement *shared_element = root->FirstChildElement("SharedCacheLevels");
  if(shared_element and shared_element->NextSiblingElement("SharedCacheLevels")){
    *this << "error: multiple SharedCacheLevels elements in file '" << filename << "'.";
    return;
  }

  // Process it if it's there.
  std::map<std::string, CacheLevel::ptr> shared_levels;
  if(shared_element){
    // The shared element consists of several CacheLevel elements.
    for(TiXmlElement *e = shared_element->FirstChildElement("CacheLevel"); e; e = e->NextSiblingElement("CacheLevel")){
      // Name attribute.
      const char *text = e->Attribute("name");
      if(!text){
        *this << "error: CacheLevel element missing 'name' attribute.";
        return;
      }
      std::string name(text);
      //std::cout << "Shared Cache Level: name = " << name << std::endl;

      // Block count attribute.
      text = e->Attribute("num_blocks");
      if(!text){
        *this << "error: CacheLevel element missing 'num_blocks' attribute.";
        return;
      }
      unsigned num_blocks = lexical_cast<unsigned>(text);
      //std::cout << "num_blocks = " << num_blocks << std::endl;

      // Associativity attribute.
      text = e->Attribute("associativity");
      if(!text){
        *this << "error: CacheLevel element missing 'associativity' attribute.";
        return;
      }
      unsigned associativity = lexical_cast<unsigned>(text);
      //std::cout << "associativity = " << associativity << std::endl;

      // Write policy attribute.
      text = e->Attribute("write_policy");
      WritePolicy policy;
      if(!text){
        *this << "error: CacheLevel element missing 'write_policy' attribute.";
        return;
      }
      if(std::string(text) == "WriteThrough"){
        policy = WriteThrough;
      }
      else if(std::string(text) == "WriteBack"){
        policy = WriteBack;
      }
      else{
        *this << "error: CacheLevel's write_policy is '" << text << "'; must be either 'WriteThrough' or 'WriteBack'.";
        return;
      }
      //std::cout << "write_policy = " << text << std::endl;

      // Construct a cache level object and store it in the shared
      // map.
      shared_levels[name] = CacheLevel::ptr(new CacheLevel(blocksize*num_blocks, blocksize, associativity, policy));
    }
  }

  // Get the containing path for the XML file (relative or absolute).
  const boost::filesystem::path filepath(filename);

  // const std::string location = boost::filesystem::path(filename).parent_path().string();
  const std::string location = filepath.parent_path().string();

  // Create a cache object for each Cache element appearing in the
  // file.
  ModtimeTable::ptr modtime = boost::make_shared<ModtimeTable>();
  for(TiXmlElement *e = root->FirstChildElement("Cache"); e; e = e->NextSiblingElement("Cache")){
    const char *text = e->Attribute("filename");
    if(!text){
      *this << "error: Cache element has no 'filename' attribute.";
      return;
    }

    std::string cache_file(text);

    // If the filename is not absolute, prepend the containing path of
    // the original file to its name.
    //
    // boost::filesystem::path p(text);
    // if(!p.is_absolute()){
    if(cache_file.length() > 0 and cache_file[0] != '/'){
      cache_file = location + "/" + cache_file;
    }

    //std::cout << "cache, filename = " << cache_file << std::endl;

    // Create a cache and store its pointer in the return list.
    CacheConstructor c(cache_file, shared_levels);

    // TODO(choudhury): error checking on cache construction.

    // Use a shared modtime table for all the caches in the set.
    Cache::ptr cachep = c.constructCache(TraceReader::ptr(), modtime);
    if(cachep){
      cacheset.push_back(cachep);
    }
  }
}
