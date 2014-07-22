// // MTV headers.
// #include <Tools/ReferenceTrace/registration.h>

// System headers.
#include <cstdio>
#include <stdint.h>

double add(double x, double y){
  double sum = x+y;
  printf("info: &sum = %p\n", &sum);
  return sum;
}

double addup(double *vals, int64_t N){
  double total = 0.0;
  printf("info: &addup::total = %p\n", &total);
  for(int64_t i=0; i<N; i++){
    printf("info: &addup::i = %p\n", &i);
    total = add(total, vals[i]);
  }

  return total;
}

int main(){
  // double vals[] = {1.0, 2.0, 3.0, 4.0, 5.0};
  int64_t N = 5;
  double *vals = new double[N];

  printf("info: &vals[0] = %p\n", vals);
  printf("info: &N = %p\n", &N);

  // MTR::Registrar r;
  // r.array(reinterpret_cast<MTR::addr_t>(vals), sizeof(double)*N, sizeof(double), "vals");
  // r.record("trace.xml");

  for(int64_t i=0; i<N; i++){
    printf("info: &i = %p\n", &i);
    vals[i] = static_cast<double>(i);
  }

  double total_value = addup(vals, N);

  printf("info: total_value = %f\n", total_value);

  return 0;
}
