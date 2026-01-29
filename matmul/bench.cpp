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
  u64 d = (argc > 2 ? std::atoll(argv[2]) : (1 << 30));

  f32 *a = new f32[n*n];
  f32 *b = new f32[n*n];
  f32 *c = new f32[n*n];

  std::mt19937 rng(2137);
  std::uniform_real_distribution<f32> dist(-1, +1);

  for (int i = 0; i < n * n; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
  }
  
  u32 laps = 0;
  u64 t0 = __rdtsc();
  u64 t1 = t0;

  while (t1 - t0 < d) {
    matmul(a, b, c, n);
    laps++;
  
    t1 = __rdtsc();
  }

  u64 avg_cycles = (t1 - t0 + laps - 1) / laps;

  std::cout << std::setw(15) << std::left << "avg cycles: "
            << std::setw(12) << std::right << avg_cycles << std::endl;

  return 0;
}
