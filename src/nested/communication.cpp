#include "nested/communication.h"

#include <algorithm>
#include <array>
#include <iostream>

namespace nested {
namespace detail {
std::vector<double> gather_rank_segments(MPI_Comm comm, const std::vector<double>& local) {
  int world_size = 1;
  MPI_Comm_size(comm, &world_size);
  std::vector<double> aggregated(local.size() * static_cast<std::size_t>(world_size));
  MPI_Allgather(local.data(), static_cast<int>(local.size()), MPI_DOUBLE, aggregated.data(),
                static_cast<int>(local.size()), MPI_DOUBLE, comm);
  return aggregated;
}

double reduce_metric_sum(MPI_Comm comm, double metric) {
  double global_sum = 0.0;
  MPI_Allreduce(&metric, &global_sum, 1, MPI_DOUBLE, MPI_SUM, comm);
  return global_sum;
}

bool is_root(MPI_Comm comm, int root_rank) {
  int rank = -1;
  MPI_Comm_rank(comm, &rank);
  return rank == root_rank;
}

bool evaluate_threshold(double metric) {
  // Un critère arbitraire pour l'exemple.
  return metric < 1.0e3;
}

void share_decision(MPI_Comm comm, bool& decision, int root_rank) {
  int decision_value = decision ? 1 : 0;
  MPI_Bcast(&decision_value, 1, MPI_INT, root_rank, comm);
  decision = (decision_value != 0);
}
}  // namespace detail

std::vector<double> gather_global_snapshot(MPI_Comm comm,
                                          const std::vector<double>& local_segment) {
  return detail::gather_rank_segments(comm, local_segment);
}

double combine_metrics(MPI_Comm comm, double local_metric) {
  double summed = detail::reduce_metric_sum(comm, local_metric);
  int world_size = 1;
  MPI_Comm_size(comm, &world_size);
  return summed / static_cast<double>(world_size);
}

bool broadcast_convergence(MPI_Comm comm, double global_metric, int root_rank) {
  bool decision = false;
  if (detail::is_root(comm, root_rank)) {
    decision = detail::evaluate_threshold(global_metric);
  }
  detail::share_decision(comm, decision, root_rank);
  return decision;
}
}  // namespace nested
