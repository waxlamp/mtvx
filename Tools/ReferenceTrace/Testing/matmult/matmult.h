/*
 * MatMult.h
 *
 * @author Jeff Jenkins
 * @version 3/6/11
 *
 * Performs various methods of representing and multiplying matrices.
 */

#ifndef MATMULT_H_
#define MATMULT_H_

#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <algorithm>

#include <Tools/ReferenceTrace/registration.h>

typedef double Real;

class MatMult {
public:
  MatMult(unsigned int n, unsigned int p = 0, unsigned int r = 0);
  virtual ~MatMult();

  // Matrix multiply where n is size of matrix and p is the ijk permutation
  Real** mdIjk(unsigned int p);
  Real* linIjk(unsigned int p);
  Real** mdBlock(unsigned int b);
  Real* linBlock(unsigned int b);

  void registerMatrices(MTR::Registrar& r) const {
    r.array(reinterpret_cast<MTR::addr_t>(linA), (N+pad)*N*sizeof(Real), sizeof(Real), "A");
    r.array(reinterpret_cast<MTR::addr_t>(linB), (N+pad)*N*sizeof(Real), sizeof(Real), "B");
    r.array(reinterpret_cast<MTR::addr_t>(linC), (N+pad)*N*sizeof(Real), sizeof(Real), "C");
  }

  // DEBUG
  void linPrintResults(Real *A);
  void mdPrintResults(Real **A);
  void printLinA();
  void printLinB();
  void printLinC();
  void printMdA();
  void printMdB();
  void printMdC();

private:
  unsigned int N; // number of rows and columns
  unsigned int pad; // amount of false padding at ends of rows

  // Matrices
  Real *linA, *linB, *linC;
  Real **mdA, **mdB, **mdC;
  
  // Assign random values to matrix
  void linRandM(Real *A);
  void mdRandM(Real **A);
  
  // Set matrix elements to zero
  void linZero(Real *A);
  void mdZero(Real **A);
};

#endif // MATMULT_H_
