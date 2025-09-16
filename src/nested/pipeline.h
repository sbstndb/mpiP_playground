#pragma once

#include <mpi.h>

namespace nested {
void run_pipeline(MPI_Comm comm, int rank, int size);
}
