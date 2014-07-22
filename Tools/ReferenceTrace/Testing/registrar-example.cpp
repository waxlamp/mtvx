// MTV includes.
#include <Tools/ReferenceTrace/registration.h>

// System includes.
#include <iostream>

int main(){
  MTR::Registrar r;

  int *data = new int[32];
  r.array(reinterpret_cast<MTR::addr_t>(data), 32*sizeof(data[0]), sizeof(data[0]), "Data");
  r.record("registrar-example.xml");

  int total = 0;
  for(int i=0; i<32; i++){
    total += data[i];
  }

  std::cout << total << std::endl;
  return 0;
}
