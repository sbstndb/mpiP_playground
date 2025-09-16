#pragma once

#include <cstddef>
#include <vector>

namespace nested {
std::vector<double> build_local_segment(int rank, std::size_t sample_count);
void apply_workload(std::vector<double>& data);
double compute_metric(const std::vector<double>& data);
}
