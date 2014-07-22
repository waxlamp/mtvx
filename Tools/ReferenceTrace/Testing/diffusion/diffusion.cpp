// diffusion.cpp - Starts with a lump of material in one part of the
// 1D domain and simulates its diffusion across the domain.

// MTV headers.
#include <Tools/ReferenceTrace/registration.h>

// TCLAP headers.
#include <tclap/CmdLine.h>

// System headers.
#include <cmath>
#include <fstream>
#include <string>
#include <vector>

double gaussian(double s, double x){
  return exp((-x*x)/(2*s*s)) / sqrt(2*M_PI*s*s);
}

int main(int argc, char *argv[]){
  unsigned N;
  double dt;
  unsigned nsteps;
  std::string outfile;
  bool pingpong;

  try{
    TCLAP::CmdLine cmd("Material diffusion");

    TCLAP::ValueArg<unsigned> NArg("s",
                                   "size",
                                   "number of elements in domain array",
                                   true,
                                   0,
                                   "positive integer",
                                   cmd);

    TCLAP::ValueArg<double> dtArg("",
                                  "dt",
                                  "time step",
                                  true,
                                  0.0,
                                  "positive real",
                                  cmd);

    TCLAP::ValueArg<unsigned> nstepsArg("n",
                                        "num-timesteps",
                                        "number of time steps to simulate",
                                        true,
                                        0,
                                        "positive integer",
                                        cmd);

    TCLAP::ValueArg<std::string> outfileArg("o",
                                            "output-file",
                                            "file to write output to",
                                            false,
                                            "",
                                            "filename",
                                            cmd);

    TCLAP::SwitchArg pingpongArg("p",
                                 "ping-pong",
                                 "Whether to move back and forth along the array when computing.",
                                 cmd);

    cmd.parse(argc, argv);

    N = NArg.getValue();
    dt = dtArg.getValue();
    nsteps = nstepsArg.getValue();
    outfile = outfileArg.getValue();
    pingpong = pingpongArg.getValue();
  }
  catch(TCLAP::ArgException& e){
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    exit(1);
  }

  // Whether to do file output.
  const bool do_output = outfile != "";

  // Open output file.
  std::ofstream out;
  if(do_output){
    out.open(outfile.c_str());
    if(!out){
      std::cerr << "error: could not open file '" << outfile << "' for writing." << std::endl;
      exit(1);
    }
  }

  // Domain array.
  std::vector<double> domain(N);
  MTR::Registrar r;
  r.array(reinterpret_cast<MTR::addr_t>(&domain[0]), domain.size()*sizeof(domain[0]), sizeof(domain[0]));
  r.record("diffusion.xml");

  // Initialize domain with a gaussian.  Assume the domain is [-5,5].
  const double dx = 10.0 / (N-1);
  const double sigma = sqrt(0.2);
  for(unsigned i=0; i<domain.size(); i++){
    domain[i] = gaussian(sigma, -5.0 + i*dx);
  }

  // Run the algorithm for the requested number of timesteps.
  if(pingpong){
    for(unsigned i=0; i<nsteps; i++){
      if(i % 2 == 0){
      // Run the diffusion equation, updating over the domain.
        for(unsigned k=0; k<domain.size(); k++){
          // Compute  f's second space  derivative (i.e.  "d^2f/dx^2") via
          // finite differences.

          // Backward part of central difference.
          const double back = k == 0 ? domain[0] : domain[k-1];
          const double forward = k+1 == domain.size() ? domain.back() : domain[k+1];

          // Compute the second derivative with a central difference, and
          // update the domain.
          const double update = back - 2*domain[k] + forward;
          domain[k] += -update*dt;
        }

      }
      else{
        // Run the diffusion equation, but in reverse order.
        for(int k=domain.size()-1; k>=0; k--){
          // Compute  f's second space  derivative (i.e.  "d^2f/dx^2") via
          // finite differences.

          // Backward part of central difference.
          const double back = k == 0 ? domain[0] : domain[k-1];
          const double forward = k+1 == static_cast<int>(domain.size()) ? domain.back() : domain[k+1];

          // Compute the second derivative with a central difference, and
          // update the domain.
          const double update = back - 2*domain[k] + forward;
          domain[k] += -update*dt;
        }
      }

      // Save the results to disk.
      if(do_output){
        for(unsigned k=0; k<domain.size(); k++){
          out << domain[k] << ' ';
        }
        out << std::endl;
      }
    }
  }
  else{
    for(unsigned i=0; i<nsteps; i++){
      // Run the diffusion equation, updating over the domain.
      for(unsigned k=0; k<domain.size(); k++){
        // Compute  f's second space  derivative (i.e.  "d^2f/dx^2") via
        // finite differences.

        // Backward part of central difference.
        const double back = k == 0 ? domain[0] : domain[k-1];
        const double forward = k+1 == domain.size() ? domain.back() : domain[k+1];

        // Compute the second derivative with a central difference, and
        // update the domain.
        const double update = back - 2*domain[k] + forward;
        domain[k] += -update*dt;
      }

      // Save the results to disk.
      if(do_output){
        for(unsigned k=0; k<domain.size(); k++){
          out << domain[k] << ' ';
        }
        out << std::endl;
      }
    }
  }

  out.close();
  return 0;
}
