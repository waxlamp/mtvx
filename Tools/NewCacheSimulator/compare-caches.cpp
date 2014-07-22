// Copyright 2011 A.N.M. Imroz Choudhury

// MTV headers.
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/NewCacheSimulator/NewCache.h>
using MTV::NewCache;
using MTV::CacheLevel;

// System headers.
#include <iostream>

void do_load(Daly::Cache& c1, NewCache::ptr c2, uint64_t addr){
  std::cout << "loading 0x" << std::hex << addr << std::dec << std::endl;
  c1.load(addr);
  c2->load(addr);
}

void do_store(Daly::Cache& c1, NewCache::ptr c2, uint64_t addr){
  std::cout << "storing 0x" << std::hex << addr << std::dec << std::endl;
  c1.store(addr);
  c2->store(addr);
}

#ifdef USE_STRING_INFO
void print_info(NewCache::const_ptr c){
  if(c->hits_string().size() == 0){
    std::cout << "no hit info." << std::endl;
  }
  else{
    std::cout << "hit info:" << std::endl;
    for(unsigned i=0; i<c->hits_string().size(); i++){
      std::cout << "  " << c->hits_string()[i] << std::endl;
    }
  }

  if(c->entrances_string().size() == 0){
    std::cout << "no entrance info." << std::endl;
  }
  else{
    std::cout << "entrance info:" << std::endl;
    for(unsigned i=0; i<c->entrances_string().size(); i++){
      std::cout << "  " << c->entrances_string()[i] << std::endl;
    }
  }

  if(c->evictions_string().size() == 0){
    std::cout << "no eviction info." << std::endl;
  }
  else{
    std::cout << "eviction info:" << std::endl;
    for(unsigned i=0; i<c->evictions_string().size(); i++){
      std::cout << "  " << c->evictions_string()[i] << std::endl;
    }
  }
}
#else
void print_info(NewCache::const_ptr c){
  if(c->hits().size() == 0){
    std::cout << "no hit info." << std::endl;
  }
  else{
    std::cout << "hit info:" << std::endl;
    for(unsigned i=0; i<c->hits().size(); i++){
      std::cout << "  " << c->hits()[i] << std::endl;
    }
  }

  if(c->entrances().size() == 0){
    std::cout << "no entrance info." << std::endl;
  }
  else{
    std::cout << "entrance info:" << std::endl;
    for(unsigned i=0; i<c->entrances().size(); i++){
      std::cout << "  " << c->entrances()[i] << std::endl;
    }
  }

  if(c->evictions().size() == 0){
    std::cout << "no eviction info." << std::endl;
  }
  else{
    std::cout << "eviction info:" << std::endl;
    for(unsigned i=0; i<c->evictions().size(); i++){
      std::cout << "  " << c->evictions()[i] << std::endl;
    }
  }
}
#endif

void print_report(std::ofstream& oldout, std::ofstream& newout, const Daly::Cache& c1, NewCache::const_ptr c2){
  c1.infoReport();
  std::cout << std::endl;

  Daly::Cache::Snapshot snap = c1.state();
  for(unsigned i=0; i<snap.state.size(); i++){
    oldout << "level " << i << ":" << std::endl;
    for(unsigned j=0; j<snap.state[i].size(); j++){
      if(snap.state[i][j].mapped){
        oldout << "Cell " << std::dec << j << ": (0x" << std::hex << snap.state[i][j].addr << (snap.state[i][j].dirty ? ", dirty)" : ", clean)") << std::endl;
      }
    }
  }
  oldout << std::endl;

  print_info(c2);
  std::cout << "---" << std::endl;

  c2->print(newout);
  newout << std::endl;
}

int main(int argc, char *argv[]){
  std::ofstream oldcache("old.dump");
  std::ofstream newcache("new.dump");

  const unsigned blocksize = 1;

  // Instantiate an old-style cache.
  //
  // Daly::LRU::ptr lru = boost::make_shared<Daly::LRU>();
  Daly::MRU::ptr lru = boost::make_shared<Daly::MRU>();

  Daly::ModtimeTable::ptr modtime = boost::make_shared<Daly::ModtimeTable>();
  Daly::Cache c1(blocksize, Daly::WriteAllocate, lru, modtime);
  c1.addCacheLevel(blocksize*4, 2, Daly::WriteThrough);
  c1.addCacheLevel(blocksize*8, 4, Daly::WriteBack);

  // Instantiate a new-style cache.
  NewCache::ptr c = NewCache::create(blocksize, NewCache::WriteAllocate);
  c->add_level(4, 2, CacheLevel::WriteThrough, CacheLevel::MRU);
  c->add_level(8, 4, CacheLevel::WriteBack, CacheLevel::MRU);

  // Run some trace records.
  do_load(c1, c, 0x10);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x12);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x14);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x16);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x18);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x10);
  print_report(oldcache, newcache, c1, c);


  do_load(c1, c, 0x20);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x22);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x24);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x26);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x28);
  print_report(oldcache, newcache, c1, c);


  do_load(c1, c, 0x30);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x32);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x34);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x36);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x38);
  print_report(oldcache, newcache, c1, c);


  do_load(c1, c, 0x40);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x42);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x44);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x46);
  print_report(oldcache, newcache, c1, c);

  do_load(c1, c, 0x48);
  print_report(oldcache, newcache, c1, c);


  do_store(c1, c, 0x10);
  print_report(oldcache, newcache, c1, c);

  return 0;
}
