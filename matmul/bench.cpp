#include <iostream>
#include <iomanip>
#include <random>

#include <cstdint>
#include <cstdlib>

#include <x86intrin.h>

#include "matmul.hpp"

using f32 = float;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

int main(int argc, char *argv[]) {
  int n = (argc > 1 ? std::atoi(argv[1]) : 512);
  u64 t = (argc > 2 ? std::atoll(argv[2]) : (1ULL << 32));
  u32 l = (argc > 3 ? std::atoi(argv[3]) : 64);

  int m = (n + 3) / 4 * 4;

  f32 *a = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);
  f32 *b = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);
  f32 *c = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);

  std::mt19937 rng(2137);
  std::uniform_real_distribution<f32> dist(0, +1);

  for (int i = 0; i < n * n; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
    c[i] = 0;
  }
  
  u32 laps = 0;
  u64 t0 = __rdtsc();
  u64 t1 = t0;

  while (t1 - t0 < t && laps < l) {
    matmul(a, b, c, n);
    laps++;
  
    t1 = __rdtsc();
  }

  u64 avg_cycles = (t1 - t0 + laps - 1) / laps;

  std::cout << std::setw(15) << std::left << "cycles: "
            << std::setw(12) << std::right << avg_cycles << ' '
            << std::setw(10) << std::left << "  laps: "
            << std::setw(10) << std::right << laps << std::endl;

  free(c);
  free(b);
  free(a);

  return 0;
}
