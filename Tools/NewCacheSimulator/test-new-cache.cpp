#include <Tools/NewCacheSimulator/NewCache.h>
using MTV::NewCache;
using MTV::CacheLevel;

void do_load(NewCache::ptr c, uint64_t addr){
  std::cout << "loading 0x" << std::hex << addr << std::dec << std::endl;
  c->load(addr);
}

void do_store(NewCache::ptr c, uint64_t addr){
  std::cout << "storing 0x" << std::hex << addr << std::dec << std::endl;
  c->store(addr);
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

  std::cout << std::endl;
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

  std::cout << std::endl;
}
#endif

int main(){
  NewCache::ptr c = NewCache::create(32, NewCache::WriteAllocate);
  c->add_level(4, 1, CacheLevel::WriteThrough, CacheLevel::LRU);
  c->add_level(8, 4, CacheLevel::WriteBack, CacheLevel::LRU);

  do_load(c, 0x100);
  print_info(c);

  do_load(c, 0x120);
  print_info(c);

  do_load(c, 0x140);
  print_info(c);

  do_load(c, 0x160);
  print_info(c);

  do_load(c, 0x180);
  print_info(c);

  do_load(c, 0x100);
  print_info(c);


  do_load(c, 0x200);
  print_info(c);

  do_load(c, 0x220);
  print_info(c);

  do_load(c, 0x240);
  print_info(c);

  do_load(c, 0x260);
  print_info(c);

  do_load(c, 0x280);
  print_info(c);


  do_load(c, 0x300);
  print_info(c);

  do_load(c, 0x320);
  print_info(c);

  do_load(c, 0x340);
  print_info(c);

  do_load(c, 0x360);
  print_info(c);

  do_load(c, 0x380);
  print_info(c);


  do_load(c, 0x400);
  print_info(c);

  do_load(c, 0x420);
  print_info(c);

  do_load(c, 0x440);
  print_info(c);

  do_load(c, 0x460);
  print_info(c);

  do_load(c, 0x480);
  print_info(c);


  do_store(c, 0x100);
  print_info(c);


  c->print(std::cout);

  return 0;
}
