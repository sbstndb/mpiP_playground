#pragma once

#include <vector>

namespace simple {
constexpr std::size_t kPayloadSize = 1 << 20;  // 1 Mi éléments ~ 8 MiB

std::vector<double> make_payload(int rank);
void heavy_compute(std::vector<double>& buffer);
}
