#include <iostream>
#include <iomanip>
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
  std::uniform_real_distribution<f32> dist(0, 1);

  for (int i = 0; i < n * n; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
    c[i] = 0;
    v[i] = 0;
  }
  
  matmul(a, b, c, n);
  naive(a, b, v, n);

  f32 max_err = 0;
  for (int i = 0; i < n * n; i++) {
    f32 diff = c[i] - v[i];
    f32 err = fabs(diff / fmax(1, fabs(v[i])));
    // std::cerr << "diff: " << diff << " err: " << err << '\n';
    assert(err < 1E-5);
    max_err = fmax(max_err, err);
  }

  std::cout << "OK " << std::fixed << std::setprecision(10) << max_err << std::endl;

  delete[] v;
  delete[] c;
  delete[] b;
  delete[] a;

  return 0;
}
