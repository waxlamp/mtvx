/*
 * TestMatMult.C
 *
 * @author Jeff Jenkins
 * @version 3/6/11
 */

#include <cstdlib>
#include <iostream>
#include <cctype>

#include <Tools/ReferenceTrace/Testing/matmult/matmult.h>

#include <tclap/CmdLine.h>

using namespace std;

int main(int argc, char *argv[])
{
  // Print usage if no argument is given
  if (argc == 1)
    {
      cout << endl;
      cout << "Usage: MatMult [OPTIONS]" << endl;
      cout << "`s` and `i` and either `b` or `p` must be specified." << endl;
      cout << endl;
      cout << "-s N\tuse N by N matrices." << endl;
      cout << "-i N\tN = 1 for direct indexing, 2 for 2d array." << endl;
      cout << "-p N\tN = [1-6], one for each permutation of ijk loops." << endl;
      cout << "-b N\tBlocksize must divide matrix size." << endl;
      cout << "-v\twill print each matrix." << endl;
      cout << "-r N\tSeed N to the RNG that generates matrice enties." << endl;
      cout << endl;
    }
  
  
  // Handle program options
  unsigned int size, pad, permutation, index, block;
  size = permutation = index = block = 0;  // Program options
  string s = "";

  // Holds result matrix to determine main return value
  Real** mdResult = (Real**) 0;
  Real* linResult = (Real*) 0;
  
  unsigned int seed = 0;

  bool verbose = false; // Whether to print matrices

  try{
    TCLAP::CmdLine cmd("matrix multiply");

    TCLAP::ValueArg<unsigned> sizeArg("s",
                                      "size",
                                      "square dimension of matrices",
                                      true,
                                      0,
                                      "dimension",
                                      cmd);

    TCLAP::ValueArg<unsigned> permutationArg("p",
                                             "permutation",
                                             "which of six permutations to use",
                                             true,
                                             0,
                                             "integer",
                                             cmd);

    TCLAP::ValueArg<unsigned> padArg("e",
                                     "extra-padding",
                                     "how much extra padding to place at the ends of the rows",
                                     false,
                                     0,
                                     "integer",
                                     cmd);

    TCLAP::ValueArg<unsigned> indexArg("i",
                                       "index",
                                       "which indexing scheme to use",
                                       true,
                                       0,
                                       "integer",
                                       cmd);

    TCLAP::ValueArg<unsigned> blockArg("b",
                                       "block",
                                       "block size to use (0 for no blocking)",
                                       false,
                                       0,
                                       "integer",
                                       cmd);

    TCLAP::SwitchArg verboseArg("v",
                                "verbose",
                                "print out matrices",
                                cmd,
                                false);

    TCLAP::ValueArg<unsigned> seedArg("r",
                                      "random-seed",
                                      "random seed for RNG",
                                      false,
                                      0,
                                      "integer",
                                      cmd);

    cmd.parse(argc, argv);

    size = sizeArg.getValue();
    permutation = permutationArg.getValue();
    pad = padArg.getValue();
    index = indexArg.getValue();
    block = blockArg.getValue();
    verbose = verboseArg.getValue();
    seed = seedArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Multiply
  MatMult *mm = new MatMult(size, pad, seed);
  
  // Register matrix arrays.
  MTR::Registrar r;
  mm->registerMatrices(r);
  r.record("matmult.xml");

  if (block != 0) // Blocking
    {
    if (index == 2)
      mdResult = mm->mdBlock(block);
    else 
      linResult = mm->linBlock(block);
    }
  else // ijk w/ permutations
    {
      if (index == 1)
	linResult = mm->linIjk(permutation);
      else
	mdResult = mm->mdIjk(permutation);
    }

  // Print matrices
  if (verbose)
    {
      if (block != 0)
	{
	  if (index == 1)
	    {
	      mm->printLinA();
	      printf("\n");
	      mm->printLinB();
	      printf("\n");
	      mm->printLinC();
	    }
	  else
	    {
	      mm->printMdA();
	      printf("\n");
	      mm->printMdB();
	      printf("\n");
	      mm->printMdC();
	    }
	}
      else
	{
	  if (index == 1)
	    {
	      mm->printLinA();
	      printf("\n");
	      mm->printLinB();
	      printf("\n");
	      mm->printLinC();
	    }
	  else
	    {
	      mm->printMdA();
	      printf("\n");
	      mm->printMdB();
	      printf("\n");
	      mm->printMdC();
	    }
	}

      printf("size: %u\n", size);
      if (index == 1)
	printf("indexing: direct\n");
      else
	printf("indexing: 2d\n");
      printf("block size: %u\n", block);
    }
  
  delete mm;
  
  if (linResult != (Real*) 0 || mdResult != (Real**) 0)
    return 0;
  return 1;
}
