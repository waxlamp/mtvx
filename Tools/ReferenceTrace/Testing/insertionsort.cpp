#include <Tools/ReferenceTrace/registration.h>

#include <stdio.h>
#include <stdlib.h>

#include <vector>

using namespace std;

void InsertionSort(vector<double> & data){

	for(unsigned i = 0; i < data.size()-1; i++){
		int idx = i;
		for(unsigned j = i+1; j < data.size(); j++){
			if(data[j] < data[idx]){
				idx = j;
			}
		}
		swap( data[i], data[idx] );
	}

}

int main(int argc, char ** argv){
	srand(0);
        const int size = 96;
	vector<double> data(size);
	vector<double> tmp(size);

        MTR::Registrar r;
        r.array(reinterpret_cast<MTR::addr_t>(&data[0]), data.size()*sizeof(data[0]), sizeof(data[0]));
        r.record("sort.xml");

        for(int i = 0; i < size; i++){
          data[i] = 100.0f*(double)rand()/(double)RAND_MAX;
        }

	InsertionSort( data );

        for(int i=0; i<size; i++){
          printf("%g\n", data[i]);
        }

	return 0;
}
