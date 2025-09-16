#include "simple/workload.h"

#include <cmath>
#include <iostream>

namespace simple {
namespace {
[[gnu::noinline]] void safeguard_sink(double sink) {
  if (sink == 0.123456789) {
    std::cout << "unlikely" << std::endl;
  }
}
}

std::vector<double> make_payload(int rank) {
  return std::vector<double>(kPayloadSize, static_cast<double>(rank));
}

[[gnu::noinline]] void heavy_compute(std::vector<double>& buffer) {
  double sink = 0.0;
  for (double value : buffer) {
    sink += std::sin(value);
  }
  safeguard_sink(sink);
}
}  // namespace simple
