#include <iostream>
#include <random>

#include <cassert>
#include <cstdint>
#include <cstdlib>

#include "matmul.hpp"

using f32 = float;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

void naive(const float *a, const float *b, float *c, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      for (int k = 0; k < n; k++)
        c[i * n + j] += a[i * n + k] * b[k * n + j];
}

int main(int argc, char *argv[]) {
  int n = (argc > 1 ? std::atoi(argv[1]) : 512);
  
  f32 *a = new f32[n*n];
  f32 *b = new f32[n*n];
  f32 *c = new f32[n*n];
  f32 *v = new f32[n*n];

  std::mt19937 rng(2137);
  std::uniform_real_distribution<f32> dist(-1, +1);

  for (int i = 0; i < n * n; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
  }
  
  matmul(a, b, c, n);
  naive(a, b, v, n);

  for (int i = 0; i < n * n; i++) {
    float diff = c[i] - v[i];
    assert(abs(diff) / fmax(1, diff) < 1E-6);
  }

  std::cout << "OK\n";

  return 0;
}
