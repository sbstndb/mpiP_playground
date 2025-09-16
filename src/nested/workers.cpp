#include "nested/workers.h"

#include <cmath>
#include <numeric>
#include <random>

namespace nested {
namespace {
constexpr double kFrequency = 0.75;

void mix_samples(std::vector<double>& data) {
  for (std::size_t i = 0; i < data.size(); ++i) {
    double x = data[i];
    x = std::sin(x * kFrequency) + std::cos(x * 0.25);
    data[i] = x;
  }
}
}

std::vector<double> build_local_segment(int rank, std::size_t sample_count) {
  std::vector<double> samples(sample_count);
  std::mt19937_64 rng(static_cast<unsigned int>(rank + 1234));
  std::normal_distribution<double> dist(static_cast<double>(rank), 1.0);

  for (double& v : samples) {
    v = dist(rng);
  }
  return samples;
}

void apply_workload(std::vector<double>& data) {
  mix_samples(data);
  // Ajoute une seconde passe pour simuler une phase CPU coûteuse.
  for (double& value : data) {
    value = std::sin(value) * std::cos(value * 0.5);
  }
}

double compute_metric(const std::vector<double>& data) {
  double accum = std::accumulate(data.begin(), data.end(), 0.0);
  double sq_accum = 0.0;
  for (double v : data) {
    sq_accum += v * v;
  }
  return sq_accum / (1.0 + std::abs(accum));
}
}  // namespace nested
