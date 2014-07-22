
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

void MergeSort(vector<double> & data, vector<double> & tmp, int i0, int i2){
	if((i2-i0) == 0 || (i2-i0) == 1){ return; }

	int i1 = (i0+i2)/2;

	// Merge sort sublists
	MergeSort( data, tmp, i0, i1 );
	MergeSort( data, tmp, i1, i2 );

	// Merge Lists
	int _i0 = i0;
	int _i1 = i1;
	for(int i = i0; i < i2; i++){
		if( _i1 >= i2 ){
			tmp[i] = data[_i0++];
		}
		else if( _i0 >= i1 ){
			tmp[i] = data[_i1++];
		}
		else if( data[_i0] < data[_i1] ){
			tmp[i] = data[_i0++];
		}
		else if( data[_i1] < data[_i0] ){
			tmp[i] = data[_i1++];
		}
	}

	// Copy data from temporary location
	for(int i = i0; i < i2; i++){
		data[i] = tmp[i];
	}

}





int main(int argc, char ** argv){
	srand(0);
        const int size = 128;
	vector<double> data(size);
	vector<double> tmp(size);

        MTR::Registrar r;
        r.array(reinterpret_cast<MTR::addr_t>(&data[0]), data.size()*sizeof(data[0]), sizeof(data[0]));
        // r.array(reinterpret_cast<MTR::addr_t>(&tmp[0]), tmp.size()*sizeof(tmp[0]), sizeof(tmp[0]));
        r.record("paulsort.xml");

        for(int i = 0; i < size; i++){
          data[i] = 100.0f*(double)rand()/(double)RAND_MAX;
        }

	InsertionSort( data );
	// MergeSort( data, tmp, 0, data.size() );

	return 0;
}
