// Copyright 2011 A.N.M. Imroz Choudhury
//
// sort.cpp - A simple program that sorts a bunch of numbers according
// to a specified sorting algorithm.

// MTV headers.
#include <Tools/ReferenceTrace/registration.h>

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <cstdlib>
#include <fstream>
#include <string>

int doubleCompare(const void *a, const void *b){
  return *(const double *)a - *(const double *)b;
}

// void bubblesort(std::vector<double>& v){
void bubblesort(double *v, unsigned size){
  for(unsigned end=size-1; end >= 0; end--){
    bool swapped = false;
    for(unsigned i=0; i<end; i++){
      if(v[i] > v[i+1]){
        // std::swap(v[i], v[i+1]);

        const double t = v[i];
        v[i] = v[i+1];
        v[i+1] = t;

        swapped = true;
      }
    }
    if(!swapped){
      break;
    }
  }
}

int main(int argc, char *argv[]){
  // Get command line arguments.
  std::string inputfile, sortalgo;
  unsigned random;
  bool print, printbefore;

  try{
    // Create command line parser.
    TCLAP::CmdLine cmd("Sort a file's worth of numbers according to a chosen sorting algorithm.");

    // The file containing the numbers to sort.
    TCLAP::ValueArg<std::string> inputfileArg("i",
                                              "input-file",
                                              "Newline-separated file of numbers to sort.",
                                              false,
                                              "",
                                              "filename",
                                              cmd);

    // Instruct the system to use a random sequence of numbers instead
    // of ones found in a file.
    TCLAP::ValueArg<unsigned> randomArg("r",
                                        "random-input",
                                        "Number of random values to use for sorting.",
                                        false,
                                        0,
                                        "number",
                                        cmd);

    // The sorting algorithm to use.
    TCLAP::ValueArg<std::string> sortalgoArg("s",
                                             "sort",
                                             "Name of a sorting algorithm (bubble, qsortc, qsort, std::sort).",
                                             true,
                                             "/dev/stdin",
                                             "sorting algorithm",
                                             cmd);

    TCLAP::SwitchArg printArg("p",
                              "print-results",
                              "Print the results of the sorting pass.",
                              cmd);

    TCLAP::SwitchArg printbeforeArg("b",
                                    "print-before",
                                    "Print the numbers to be sorted.",
                                    cmd);

    // Parse command line.
    cmd.parse(argc, argv);

    // Extract the arguments.
    inputfile = inputfileArg.getValue();
    random = randomArg.getValue();
    sortalgo = sortalgoArg.getValue();
    print = printArg.getValue();
    printbefore = printbeforeArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // std::vector<double> values;
  double *values = 0;
  unsigned size;

  // Check whether we're doing file input or random values.
  if(random > 0){
    std::cerr << "generating " << random << " values for sorting..." << std::flush;

    values = new double[random];
    size = random;
    for(unsigned i=0; i<random; i++){
      // values.push_back(drand48());
      values[i] = drand48();
    }

    std::cerr << "done" << std::endl;
  }
  else{
    // Open the input file.
    std::ifstream in(inputfile.c_str());
    if(!in){
      std::cerr << "error: could not open file '" << inputfile << "' for reading." << std::endl;
      exit(1);
    }

    // Read in the numbers to sort.
    std::vector<double> v;
    while(true){
      double x;
      in >> x;

      if(!in.good()){
        in.close();
        break;
      }

      v.push_back(x);
    }

    values = new double[v.size()];
    size = v.size();

    for(unsigned i=0; i<size; i++){
      values[i] = v[i];
    }
  }

  // // Make sure some data was read in.
  // if(values.size() == 0){
  //   std::cerr << "error: no values found." << std::endl;
  //   exit(1);
  // }

  // Print the values if requested.
  if(printbefore){
    // std::cout << reinterpret_cast<void *>(&bubblesort) << std::endl;

    for(unsigned i=0; i<size; i++){
      std::cout << values[i] << std::endl;
    }
  }

  // Register the values array.
  MTR::Registrar r;
  r.array(reinterpret_cast<MTR::addr_t>(&values[0]), size*sizeof(values[0]), sizeof(values[0]), "Data");
  r.record("sort-registration.xml");

  // Dispatch a sorting method.
  if(sortalgo == "bubble"){
    bubblesort(values, size);
  }
  // else if(sortalgo == "qsortc"){
  //   qsort(&values[0], values.size(), sizeof(values[0]), doubleCompare);
  // }
  // else if(sortalgo == "std::sort"){
  //   std::sort(values.begin(), values.end());
  // }
  else{
    std::cerr << "error: unknown sorting algorithm '" << sortalgo << "'." << std::endl;
    exit(1);
  }

  // Print out the sorted values if requested.
  if(print){
    for(unsigned i=0; i<size; i++){
      std::cout << values[i] << std::endl;
    }
  }

  return 0;
}
