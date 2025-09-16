#include "simple/pipeline.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

#include "simple/workload.h"

namespace simple {
namespace {
constexpr int kTag = 42;

[[gnu::noinline]] void log_result(double max_elapsed) {
  std::cout << "Maximum elapsed time across ranks: " << std::fixed << std::setprecision(3)
            << max_elapsed << " s" << std::endl;
}

[[gnu::noinline]] void root_receive_loop(int world_size, MPI_Comm comm,
                                         std::vector<double>& buffer) {
  for (int src = 1; src < world_size; ++src) {
    MPI_Recv(buffer.data(), static_cast<int>(buffer.size()), MPI_DOUBLE, src, kTag, comm,
             MPI_STATUS_IGNORE);
    heavy_compute(buffer);
  }
}

[[gnu::noinline]] void worker_send_payload(MPI_Comm comm, const std::vector<double>& payload) {
  MPI_Ssend(payload.data(), static_cast<int>(payload.size()), MPI_DOUBLE, 0, kTag, comm);
}

[[gnu::noinline]] void emulate_additional_delay(int rank) {
  if (rank == 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
  }
}
}  // namespace

void run_rank(int rank, int world_size, MPI_Comm comm) {
  auto payload = make_payload(rank);
  std::vector<double> buffer(payload.size(), 0.0);

  MPI_Barrier(comm);
  double start = MPI_Wtime();

  if (rank == 0) {
    root_receive_loop(world_size, comm, buffer);
    emulate_additional_delay(rank);
  } else {
    worker_send_payload(comm, payload);
  }

  MPI_Barrier(comm);
  double elapsed = MPI_Wtime() - start;

  double max_elapsed = 0.0;
  MPI_Reduce(&elapsed, &max_elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, comm);

  if (rank == 0) {
    log_result(max_elapsed);
  }
}
}  // namespace simple
