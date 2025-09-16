#include "nested/pipeline.h"

#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "nested/communication.h"
#include "nested/workers.h"

namespace nested {
namespace {
constexpr std::size_t kSegmentSize = 1 << 15;  // 32k éléments doubles (~256 kB)
constexpr int kRootRank = 0;

void synchronize_phase(MPI_Comm comm) {
  MPI_Barrier(comm);
}

double run_iteration(MPI_Comm comm, int rank, int world_size) {
  auto local_segment = build_local_segment(rank, kSegmentSize);
  apply_workload(local_segment);

  auto snapshot = gather_global_snapshot(comm, local_segment);
  double local_metric = compute_metric(snapshot);
  double global_metric = combine_metrics(comm, local_metric);
  return global_metric * static_cast<double>(world_size);
}

void emulate_root_processing(MPI_Comm comm, bool decision, int rank) {
  if (rank == kRootRank && !decision) {
    // Simule un goulot CPU sur le rang root.
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
  }
  synchronize_phase(comm);
}
}  // namespace

void run_pipeline(MPI_Comm comm, int rank, int size) {
  synchronize_phase(comm);

  double aggregated_value = run_iteration(comm, rank, size);
  bool converged = broadcast_convergence(comm, aggregated_value, kRootRank);
  emulate_root_processing(comm, converged, rank);

  if (rank == kRootRank) {
    std::cout << "[nested_bottleneck] valeur agrégée: " << aggregated_value
              << ", converged=" << std::boolalpha << converged << std::endl;
  }
}
}  // namespace nested
