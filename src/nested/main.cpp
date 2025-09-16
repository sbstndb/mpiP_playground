#include <mpi.h>

#include <iostream>

#include "nested/pipeline.h"

int main(int argc, char** argv) {
  MPI_Init(&argc, &argv);

  int rank = 0;
  int size = 1;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  if (size < 2) {
    if (rank == 0) {
      std::cerr << "nested_bottleneck requiert au moins deux processus." << std::endl;
    }
    MPI_Finalize();
    return 1;
  }

  nested::run_pipeline(MPI_COMM_WORLD, rank, size);

  MPI_Finalize();
  return 0;
}
