#pragma once

#include <mpi.h>

#include <vector>

namespace nested {
std::vector<double> gather_global_snapshot(MPI_Comm comm,
                                          const std::vector<double>& local_segment);
double combine_metrics(MPI_Comm comm, double local_metric);
bool broadcast_convergence(MPI_Comm comm, double global_metric, int root_rank);
}
