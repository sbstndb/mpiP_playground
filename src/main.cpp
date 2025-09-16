#include <mpi.h>

#include <iostream>

#include "simple/pipeline.h"

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int world_rank = 0;
  int world_size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  if (world_size < 2) {
    if (world_rank == 0) {
      std::cerr << "This example requires au moins deux processus." << std::endl;
    }
    MPI_Finalize();
    return 1;
  }

  simple::run_rank(world_rank, world_size, MPI_COMM_WORLD);

  MPI_Finalize();
  return 0;
}
