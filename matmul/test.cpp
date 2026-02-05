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

void naive_row_major(const float *a, const float *b, float *c, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      for (int k = 0; k < n; k++)
        c[i * n + j] += a[i * n + k] * b[k * n + j];
}

void naive_col_major(const float *a, const float *b, float *c, int n) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      for (int k = 0; k < n; k++)
        c[i + n * j] += a[i + n * k] * b[k + n * j];
}


int main(int argc, char *argv[]) {
  int n = (argc > 1 ? std::atoi(argv[1]) : 512);
  
  int m = (n + 3) / 4 * 4;

  f32 *a = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);
  f32 *b = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);
  f32 *c = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);
  f32 *R = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);
  f32 *C = (f32*) std::aligned_alloc(64, sizeof(f32)*m*m);

  std::mt19937 rng(2137);
  std::uniform_real_distribution<f32> dist(0, 1);

  for (int i = 0; i < n * n; i++) {
    a[i] = dist(rng);
    b[i] = dist(rng);
    c[i] = 0;
    R[i] = 0;
    C[i] = 0;
  }
  
  matmul(a, b, c, n);
  naive_row_major(a, b, R, n);
  naive_col_major(a, b, C, n); 

  f32 max_err = 0;
  for (int i = 0; i < n * n; i++) {
    f32 diffR = c[i] - R[i];
    f32 errR = fabs(diffR / fmax(1, fabs(R[i])));
    
    f32 diffC = c[i] - C[i];
    f32 errC = fabs(diffC / fmax(1, fabs(C[i])));
    
    f32 err = std::min(errR, errC);

    if (err > 1E-5) {
      std::cerr << "Wrong err: " << err << std::endl;
      std::cerr << "> diffR: " << diffR << " errR: " << errR << " diffC: " << diffC << " errC: " << errC << std::endl;
      std::cerr << "i: " << i << std::endl;
      return 1;
    }
    max_err = fmax(max_err, err);
  }

  std::cout << "OK " << std::fixed << std::setprecision(10) << max_err << std::endl;

  std::free(C);
  std::free(R);
  std::free(c);
  std::free(b);
  std::free(a);
  
  return 0;
}
