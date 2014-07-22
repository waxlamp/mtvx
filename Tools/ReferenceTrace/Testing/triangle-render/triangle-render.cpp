
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <vector>
#include <queue>
#include <algorithm>

#include <Tools/ReferenceTrace/registration.h>

using namespace std;

struct float3 {
	float x,y,z;
	float3( float _x, float _y, float _z ){
		x = _x; y = _y; z = _z;
	}
};

struct uint3 {
	unsigned int u[3];

	uint3( unsigned int _u0, unsigned int _u1, unsigned int _u2 ){
		if( _u0 > _u1 && _u0 > _u2 ){
			u[0] = _u1; u[1] = _u2; u[2] = _u0; 
		}
		else if( _u1 > _u0 && _u1 > _u2 ){
			u[0] = _u2; u[1] = _u0; u[2] = _u1; 
		}
		else{
			u[0] = _u0; u[1] = _u1; u[2] = _u2;
		}
	}

	bool operator < ( const uint3 & right ) const {
		return (u[2] < right.u[2]);
	}
};


void ReadVertices( vector<float3> & verts, const char * fname ){
	float f0,f1,f2;
	FILE * infile = fopen(fname,"r");
	while( !feof(infile) ){
		fscanf( infile, " %f %f %f ", &f0, &f1, &f2 );
		verts.push_back( float3(f0,f1,f2) );		
	}
	fclose(infile);
}

void ReadFaces( vector<uint3> & faces, const char * fname ){
	unsigned int f0,f1,f2;
	FILE * infile = fopen(fname,"r");
	while( !feof(infile) ){
		fscanf( infile, " %u %u %u ", &f0, &f1, &f2 );
		faces.push_back( uint3(f0,f1,f2) );		
	}
	fclose(infile);
}

void RandomizeFaces( vector<uint3> & faces ){
	priority_queue< pair<int, uint3> > rdm;

	for(int i = 0; i < (int)faces.size(); i++){
		rdm.push( pair<int, uint3>( rand(), faces[i] ) );
	}

	faces.clear();

	while( !rdm.empty() ){
		faces.push_back( rdm.top().second );
		rdm.pop();
	}
}

void SortFaces( vector<uint3> & faces ){
	sort( faces.begin(), faces.end() );
}

void PerverseSortFaces( vector<uint3> & faces, int pieces ){
  SortFaces(faces);

  vector<uint3> work(faces);
  int skip = ceil(faces.size() / (float)pieces);
  int k=0;
  for(int i=0; i<skip; i++){
    for(unsigned j=i; j<faces.size(); j+=skip){
      faces[k++] = work[j];
    }
  }
}

void PrintFaces( vector<uint3> & faces ){
	for(int i = 0; i < (int)faces.size(); i++){
		printf("%i - %i %i %i\n",i,faces[i].u[0], faces[i].u[1], faces[i].u[2] );
	}
}

float DrawVertex( float x, float y, float z ){
	//printf("%f %f %f\n",x,y,z);
	return (x+y+z);
}

void DrawTriangle( uint3 & face, vector<float3> & verts ){
	DrawVertex( verts[ face.u[0] ].x, verts[ face.u[0] ].x, verts[ face.u[0] ].z );
	DrawVertex( verts[ face.u[1] ].x, verts[ face.u[1] ].x, verts[ face.u[1] ].z );
	DrawVertex( verts[ face.u[2] ].x, verts[ face.u[2] ].x, verts[ face.u[2] ].z );
	//printf("\n");
}

void DrawMesh( vector<uint3> & faces, vector<float3> & verts ){
	for(int i = 0; i < (int)faces.size(); i++){
		DrawTriangle( faces[i], verts );
	}
}

int main(int argc, char ** argv){
  if(argc < 3){
    fprintf(stderr, "usage: triangle-render <vertex-file> <face-file>\n");
    exit(1);
  }

  const char *vertfile = argv[1];
  const char *facefile = argv[2];

  vector<float3> verts;
  vector<uint3>  faces;

  ReadVertices( verts, vertfile );
  ReadFaces(  faces, facefile );

  if(argc >= 4){
    if(std::string(argv[3]) == "sorted"){
      SortFaces(faces);
    }
    else if(std::string(argv[3]) == "random"){
      RandomizeFaces(faces);
    }
    else if(std::string(argv[3]) == "perverse-sorted"){
      PerverseSortFaces(faces, 32);
    }
    else{
      fprintf(stderr, "error: secret mode must be 'sorted', 'random', or 'perverse-sorted'\n");
      exit(1);
    }

    for(unsigned i=0; i<faces.size(); i++){
      printf("%u %u %u\n", faces[i].u[0], faces[i].u[1], faces[i].u[2]);
    }
    exit(0);
  }

  MTR::Registrar r;
  r.array(reinterpret_cast<MTR::addr_t>(&verts[0]), verts.size()*sizeof(verts[0]), sizeof(verts[0].x));
  r.array(reinterpret_cast<MTR::addr_t>(&faces[0]), faces.size()*sizeof(faces[0]), sizeof(faces[0].u[0]));
  r.record("triangle-render.xml");

  //PrintFaces( faces );
  DrawMesh( faces, verts );

  return 0;
}

