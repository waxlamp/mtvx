// MTV headers.
#include <Core/Util/Timing.h>
#include <Tools/CacheSimulator/Cache.h>
#include <Tools/NewCacheSimulator/NewCache.h>
using MTV::NewCache;
using MTV::CacheLevel;
using MTV::WallClock;

// System headers.
#include <iostream>

void do_load(NewCache::ptr c, uint64_t addr){
  std::cout << "loading 0x" << std::hex << addr << std::dec << std::endl;
  c->load(addr);
}

void do_store(NewCache::ptr c, uint64_t addr){
  std::cout << "storing 0x" << std::hex << addr << std::dec << std::endl;
  c->store(addr);
}

int main(int argc, char *argv[]){
  // Create a wall clock.
  WallClock clock;

  // Create a list of random addresses.
  //
  // std::vector<uint64_t> addrs(10*1024*1024);
  std::vector<uint64_t> addrs(1024*1024);
  for(unsigned i=0; i<addrs.size(); i++){
    addrs[i] = static_cast<uint64_t>(drand48()*10000);
  }

  // Instantiate a new-style cache.
  const unsigned blocksize = 1;

  NewCache::ptr c = NewCache::create(blocksize, NewCache::WriteAllocate);
  c->add_level(256, 2, CacheLevel::WriteThrough, CacheLevel::LRU);
  c->add_level(512, 16, CacheLevel::WriteBack, CacheLevel::LRU);

  // Instantiate an old-style cache.
  Daly::LRU::ptr lru = boost::make_shared<Daly::LRU>();
  Daly::ModtimeTable::ptr modtime = boost::make_shared<Daly::ModtimeTable>();
  Daly::Cache c2(blocksize, Daly::WriteAllocate, lru, modtime);
  c2.addCacheLevel(blocksize*256, 2, Daly::WriteThrough);
  c2.addCacheLevel(blocksize*512, 16, Daly::WriteBack);

  // for(int i=0; i<16; i++){
  //   do_load(c, i);
  //   c->print(std::cout);
  //   std::cout << "---" << std::endl;
  // }

  // Time the new cache.
  std::cout << "new cache running..." << std::flush;
  const float new_start = clock.noww();
  for(unsigned i=0; i<addrs.size(); i++){
    c->load(addrs[i]);
  }
  const float new_end = clock.noww();
  const float new_time = new_end - new_start;
  std::cout << "done" << std::endl;
  
  // Time the old cache.
  std::cout << "old cache running..." << std::flush;
  const float old_start = clock.noww();
  for(unsigned i=0; i<addrs.size(); i++){
    c2.load(addrs[i]);
  }
  const float old_end = clock.noww();
  const float old_time = old_end - old_start;
  std::cout << "done" << std::endl;

  std::cout << "new cache: " << new_time << " seconds." << std::endl;
  std::cout << "old cache: " << old_time << " seconds." << std::endl;

  return 0;
}
