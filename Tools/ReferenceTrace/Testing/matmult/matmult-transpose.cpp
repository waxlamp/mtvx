/*
 * MatMult.C
 *
 * @author Jeff Jenkins
 * @version 3/6/11
 */

#include <Tools/ReferenceTrace/Testing/matmult/matmult.h>
using namespace std;

MatMult::MatMult(unsigned int n, unsigned p, unsigned int r)
{
  if (r == 0)
    srand((unsigned) time(NULL));
  else
    srand(r);

  N = n;
  p = 0;

  // Matrices as linear arrays
  linA = new Real[N*N];
  linB = new Real[N*N];
  linC = new Real[N*N];

  // Matrices as 2d arrays
  mdA = new Real *[N];
  mdA[0] = new Real [N*N];
  for (unsigned int i = 1; i < N; i++)
    mdA[i] = mdA[i-1] + N;

  mdB = new Real *[N];
  mdB[0] = new Real [N*N];
  for (unsigned int i = 1; i < N; i++)
    mdB[i] = mdB[i-1] + N;

  mdC = new Real *[N];
  mdC[0] = new Real [N*N];
  for (unsigned int i = 1; i < N; i++)
    mdC[i] = mdC[i-1] + N;

  // Generate random matrices and zero result matrix
  linRandM(linA);
  linRandM(linB);
  linZero(linC);
  
  mdRandM(mdA);
  mdRandM(mdB);
  mdZero(mdC);
}

MatMult::~MatMult()
{
  delete[] linA;
  delete[] linB;
  delete[] linC;

  delete[] mdA[0];
  delete[] mdB[0];
  delete[] mdC[0];
  delete[] mdA;
  delete[] mdB;
  delete[] mdC;
}

Real* MatMult::linIjk(unsigned int p)
{
    // Set up indices
  unsigned int i, j, k;
  unsigned int *x, *y, *z;

  switch (p)
    {
    case 1: x = &i; y = &j; z = &k; break;
    case 2: x = &i; y = &k; z = &j; break;
    case 3: x = &j; y = &i; z = &k; break;
    case 4: x = &j; y = &k; z = &i; break;
    case 5: x = &k; y = &i; z = &j; break;
    case 6: x = &k; y = &j; z = &i; break;
    default: x = &i; y = &j; z = &k;
    }

  // Multiply
  for (*x = 0; *x < N; (*x)++)
    for (*y = 0; *y < N; (*y)++)
      for (*z = 0; *z < N; (*z)++)
	  linC[i*N + j] += linA[i*N + k] * linB[j*N + k];

  return linC;
}

Real** MatMult::mdIjk(unsigned int p)
{
  // Set up indices
  unsigned int i, j, k;
  unsigned int *x, *y, *z;

  switch (p)
    {
    case 1: x = &i; y = &j; z = &k; break;
    case 2: x = &i; y = &k; z = &j; break;
    case 3: x = &j; y = &i; z = &k; break;
    case 4: x = &j; y = &k; z = &i; break;
    case 5: x = &k; y = &i; z = &j; break;
    case 6: x = &k; y = &j; z = &i; break;
    default: x = &i; y = &j; z = &k;
    }

  // Multiply
  for (*x = 0; *x < N; (*x)++)
    for (*y = 0; *y < N; (*y)++)
      for (*z = 0; *z < N; (*z)++)
	  mdC[i][j] += mdA[i][k] * mdB[j][k];

  return mdC;
}

Real** MatMult::mdBlock(unsigned int b)
{
  Real r;

  for (unsigned int k0 = 0; k0 < N; k0 += b)
    for (unsigned int j0 = 0; j0 < N; j0 += b)
      for (unsigned int i = 0; i < N; i++)
	for (unsigned int k = k0; k < min(k0 + b, N); k++)
	  {
	    r = mdA[i][k];
	    for (unsigned int j = j0; j < min(j0 + b, N); j++)
	      mdC[i][j] += r*mdB[j][k]; 
	  }	  

  return mdC;
}

Real* MatMult::linBlock(unsigned int b)
{
  Real r;

  for (unsigned int k0 = 0; k0 < N; k0 += b)
    for (unsigned int j0 = 0; j0 < N; j0 += b)
      for (unsigned int i = 0; i < N; i++)
	for (unsigned int k = k0; k < min(k0 + b, N); k++)
	  {
	    r = linA[i*N + k];
	    for (unsigned int j = j0; j < min(j0 + b, N); j++)
	      linC[i*N + j] += r*linB[j*N + k];
	  }	  

  return linC;
}

void MatMult::linRandM(Real *A)
{
  for (unsigned int i = 0; i < N*N; i++)
    A[i] = ((Real) rand()) / RAND_MAX;
}

void MatMult::mdRandM(Real **A)
{
  for (unsigned int i = 0; i < N; i++)
      for (unsigned int j = 0; j < N; j++)
	A[i][j] = ((Real) rand()) / RAND_MAX;
}

void MatMult::linPrintResults(Real *A)
{
  printf("[");

  for (unsigned int i = 0; i < N*N; i++)
    {
      printf(" %f", A[i]);

      if ((i + 1) % N == 0)
	printf(";");
    }

  printf("]\n");
}

void MatMult::mdPrintResults(Real **A)
{
  printf("[");
  for (unsigned int i = 0; i < N; i++)
    {
      for (unsigned int j = 0; j < N; j++)
	{
	  printf(" %f", A[i][j]);
	}
      printf(";");
    }
  printf("]\n");
}

void MatMult::linZero(Real *A)
{
  for (unsigned int i = 0; i < N*N; i++)
    A[i] = 0;
}

void MatMult::mdZero(Real **A)
{
  for (unsigned int i = 0; i < N; i++)
    for (unsigned int j = 0; j < N; j++)
      A[i][j] = 0;
}

void MatMult::printLinA()
{
  linPrintResults(linA);
}
void MatMult::printLinB()
{
  linPrintResults(linB);
}
void MatMult::printLinC()
{
  linPrintResults(linC);
}
void MatMult::printMdA()
{
  mdPrintResults(mdA);
}
void MatMult::printMdB()
{
  mdPrintResults(mdB);
}
void MatMult::printMdC()
{
  mdPrintResults(mdC);
}
