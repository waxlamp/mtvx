// Copyright 2012 A.N.M. Imroz Choudhury
//
// NewCacheConstructor.cpp

// MTV includes.
#include <Core/Util/Boost.h>
#include <Tools/NewCacheSimulator/NewCache.h>
#include <Tools/NewCacheSimulator/NewCacheConstructor.h>
using MTV::CacheLevel;
using MTV::NewCache;
using MTV::NewCacheConstructor;

// TinyXML includes.
#define TIXML_USE_STL
#include <tinyxml.h>

NewCacheConstructor::NewCacheConstructor(const std::string& filename, const SharedLevelTable& shared)
  : error_message("")
{
  // Create an XML document associated with the file.
  TiXmlDocument doc(filename);
  if(!doc.LoadFile()){
    std::stringstream ss;
    ss << "error: could not load file '" << filename << "'.";
    error_message = ss.str();
    return;
  }

  // Parse the file.
  //
  // Begin by making sure the root is a Cache element.
  TiXmlElement *root = doc.RootElement();
  if(std::string(root->Value()) != "Cache"){
    std::stringstream ss;
    ss << "error: xml document does not contain a root Cache element.";
    error_message = ss.str();
    return;
  }

  // Fill out the CacheConstructor fields from the data in the file.
  //
  // First the attributes of the root element - blocksize...
  const char *text = root->Attribute("blocksize");
  if(!text){
    std::stringstream ss;
    ss << "error: missing 'blocksize' attribute in root element Cache.";
    error_message = ss.str();
    return;
  }
  this->blocksize = lexical_cast<unsigned>(text);

  // write miss policy...
  text = root->Attribute("write_miss_policy");
  if(!text){
    std::stringstream ss;
    ss << "error: missing 'write_miss_policy' attribute in root element Cache.";
    error_message = ss.str();
    return;
  }
  if(std::string(text) == "WriteAllocate"){
    this->writeMissPolicy = NewCache::WriteAllocate;
  }
  else if(std::string(text) == "NoWriteAllocate"){
    this->writeMissPolicy = NewCache::WriteNoAllocate;
  }
  else{
    std::stringstream ss;
    ss << "error: write_miss_policy attribute of Cache element does not contain either 'WriteAllocate' or 'NoWriteAllocate'.";
    error_message = ss.str();
    return;
  }

  // ...and block replacement policy.
  text = root->Attribute("replacement_policy");
  CacheLevel::ReplacementPolicy repl_policy;
  if(!text){
    std::stringstream ss;
    std::cerr << "warning: missing 'replacement_policy' attribute in root element Cache, using default \"LRU\"." << std::endl;
    text = "LRU";
  }
  if(std::string(text) == "LRU"){
    repl_policy = CacheLevel::LRU;
  }
  else if(std::string(text) == "MRU"){
    repl_policy = CacheLevel::MRU;
  }
  else if(std::string(text) == "OPT"){
    repl_policy = CacheLevel::OPT;
  }
  else if(std::string(text) == "PES"){
    repl_policy = CacheLevel::PES;
  }
  else if(std::string(text) == "RANDOM"){
    repl_policy = CacheLevel::Random;
  }
  else{
    std::stringstream ss;
    ss << "error: replacement_policy attribute of Cache element does not contain either 'LRU' or 'ApproxOPT'.";
    error_message = ss.str();
    return;
  }

  // Then the cache level elements.
  for(TiXmlElement *e = root->FirstChildElement("CacheLevel"); e; e = e->NextSiblingElement("CacheLevel")){
    // TODO(choudhury): the following should be dead code; the for
    // loop condition prevents executing the loop in the case that
    // "!e" holds true.
    if(!e){
      std::stringstream ss;
      ss << "fatal error: found a null child of root node with value 'CacheLevel'.";
      error_message = ss.str();
      return;
    }

    // First check to see if the cache level element indicates a
    // shared cache level.
    text = e->Attribute("shared_name");
    if(text){
      // This path is for a shared cache level...

      // Find the shared cache level in the map and add it.
      SharedLevelTable::const_iterator i = shared.find(text);
      if(i == shared.end()){
        std::stringstream ss;
        ss << "error: shared_name value of '" << text << "' indicates non-existent shared cache level.";
        error_message = ss.str();
        return;
      }

      CacheLevel::ptr level = i->second;
      this->addLevelConstructor(level);
    }
    else{
      // ...and this path is for a non-shared cache level.
      unsigned numBlocks, associativity;
      text = e->Attribute("num_blocks");
      if(!text){
        std::stringstream ss;
        ss << "error: CacheLevel element missing 'num_blocks' attribute.";
        error_message = ss.str();
        return;
      }
      numBlocks = lexical_cast<unsigned>(text);

      text = e->Attribute("associativity");
      if(!text){
        std::stringstream ss;
        ss << "error: CacheLevel element missing 'associativity' attribute.";
        error_message = ss.str();
        return;
      }
      associativity = lexical_cast<unsigned>(text);

      text = e->Attribute("write_policy");
      if(!text){
        std::stringstream ss;
        ss << "error: CacheLevel element missing 'write_policy' attribute.";
        error_message = ss.str();
        return;
      }
      CacheLevel::WritePolicy writePolicy;
      if(std::string(text) == "WriteThrough"){
        writePolicy = CacheLevel::WriteThrough;
      }
      else if(std::string(text) == "WriteBack"){
        writePolicy = CacheLevel::WriteBack;
      }
      else{
        std::stringstream ss;
        ss << "error: write_policy attribute of CacheLevel element does not contain either 'WriteThrough' or 'WriteBack'.";
        error_message = ss.str();
        return;
      }

      this->addLevelConstructor(numBlocks, associativity, writePolicy, repl_policy);
    }
  }
}

NewCache::ptr NewCacheConstructor::constructCache(TraceReader::ptr trace, BlockStreamReader::ptr bsreader) const {
  NewCache::ptr p = NewCache::create(this->blocksize, this->writeMissPolicy);
  for(unsigned i=0; i<levels.size(); i++){
    if(levels[i].level){
      p->add_level(levels[i].level);
    }
    else{
      const CacheLevelParams& params = levels[i].params;
      p->add_level(params.num_blocks, params.num_sets, params.write_policy, params.repl_policy);
    }
  }

  return p;
}
