// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCacheSetConstructor.cpp

// MTV headers.
#include <Tools/NewCacheSimulator/CacheLevel.h>
#include <Tools/NewCacheSimulator/NewCacheConstructor.h>
#include <Tools/NewCacheSimulator/NewCacheSetConstructor.h>
using MTV::BlockStreamReader;
using MTV::CacheLevelParams;
using MTV::NewCacheConstructor;
using MTV::NewCacheSetConstructor;
using MTV::TraceReader;

// Boost headers.
#include <boost/unordered_map.hpp>

// TinyXML headers.
#define TIXML_USE_STL
#include <tinyxml.h>

NewCacheSetConstructor::NewCacheSetConstructor(const std::string& filename, TraceReader::ptr trace, BlockStreamReader::ptr bsreader)
  : trace(trace),
    bsreader(bsreader)
{
  // Create a dummy cache for use in constructing shared cache level
  // objects.
  NewCache::ptr dummy = NewCache::dummy(trace, bsreader);

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

  // unsigned blocksize = lexical_cast<unsigned>(text);
  //std::cout << "blocksize = " << blocksize << std::endl;

  // Get the replacement policy attribute.
  text = root->Attribute("replacement_policy");
  if(!text){
    *this << "error: CacheSet element does not contain 'replacement_policy' attribute.";
    return;
  }

  CacheLevel::ReplacementPolicy repl_policy;
  {
    std::string policy_text(text);
    if(policy_text == "LRU"){
      repl_policy = CacheLevel::LRU;
    }
    else if(policy_text == "MRU"){
      repl_policy = CacheLevel::MRU;
    }
    else if(policy_text == "OPT"){
      repl_policy = CacheLevel::OPT;
    }
    else if(policy_text == "PES"){
      repl_policy = CacheLevel::PES;
    }
    else if(policy_text == "Random"){
      repl_policy = CacheLevel::Random;
    }
    else{
      *this << "error: illegal replacement policy '" << policy_text << "' in CacheSet tag";
      return;
    }
  }

  // Find the SharedCacheLevels element and make sure it appears at most
  // once.
  TiXmlElement *shared_element = root->FirstChildElement("SharedCacheLevels");
  if(shared_element and shared_element->NextSiblingElement("SharedCacheLevels")){
    *this << "error: multiple SharedCacheLevels elements in file '" << filename << "'.";
    return;
  }

  // Process it if it's there.
  NewCacheConstructor::SharedLevelTable shared_levels;
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
      CacheLevel::WritePolicy policy;
      if(!text){
        *this << "error: CacheLevel element missing 'write_policy' attribute.";
        return;
      }
      if(std::string(text) == "WriteThrough"){
        policy = CacheLevel::WriteThrough;
      }
      else if(std::string(text) == "WriteBack"){
        policy = CacheLevel::WriteBack;
      }
      else{
        *this << "error: CacheLevel's write_policy is '" << text << "'; must be either 'WriteThrough' or 'WriteBack'.";
        return;
      }
      //std::cout << "write_policy = " << text << std::endl;

      // // Save the level parameters in a table for construction later.
      // shared_levels[name] = CacheLevelParams(num_blocks, associativity, policy, repl_policy);

      // Construct the level immediately, and save it in the table.
      shared_levels[name] = dummy->add_level(num_blocks, associativity, policy, repl_policy);
    }
  }

  // Get the containing path for the XML file (relative or absolute).
  const boost::filesystem::path filepath(filename);

  // const std::string location = boost::filesystem::path(filename).parent_path().string();
  const std::string location = filepath.parent_path().string();

  // Create a cache object for each Cache element appearing in the
  // file.
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
    NewCacheConstructor c(cache_file, shared_levels);

    // TODO(choudhury): error checking on cache construction.

    // Use a shared modtime table for all the caches in the set.
    NewCache::ptr cachep = c.constructCache(trace, bsreader);
    if(cachep){
      cacheset.push_back(cachep);
    }
  }
}
