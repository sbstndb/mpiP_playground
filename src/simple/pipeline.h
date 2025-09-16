#pragma once

#include <mpi.h>

namespace simple {
void run_rank(int rank, int world_size, MPI_Comm comm);
}
