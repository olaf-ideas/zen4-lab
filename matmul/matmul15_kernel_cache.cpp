#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iostream>

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

constexpr u32 MAX_N = 1 << 12;

// fma -> latency: 4 cycles, throughput: 2 per cycle
// 32 zmm registers

#undef ROWS_KER
#undef COLS_KER

constexpr u32 ROWS_KER = 4;
constexpr u32 COLS_KER = 2 * 16;

constexpr u32 ROW_N = (MAX_N + ROWS_KER - 1) / ROWS_KER * ROWS_KER;
constexpr u32 COL_N = (MAX_N + COLS_KER - 1) / COLS_KER * COLS_KER;

void kernel(const f32 *a, 
            const f32x16 *b, 
            f32x16 *__restrict__ c,
            u32 x, u32 y,
            u32 l, u32 r, 
            u32 n) {
  alignas(64) f32x16 d[ROWS_KER][COLS_KER / 16]{};

  for (u32 k = l; k < r; k++) {
    for (u32 i = 0; i < ROWS_KER; i++) {
      f32x16 alpha = _mm512_set1_ps(a[(x + i) * n + k]);
      for (u32 j = 0; j < COLS_KER; j += 16) {
        d[i][j / 16] += alpha * b[(k * n + y + j) / 16];
      }
    }
  }

  for (u32 i = 0; i < ROWS_KER; i++) {
    for (u32 j = 0; j < COLS_KER; j += 16) {
      c[((x + i) * n + y + j) / 16] += d[i][j / 16];
    }
  }
}

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {

  const u32 n = _n;
  const u32 nx = (n + ROWS_KER - 1) / ROWS_KER * ROWS_KER;
  const u32 ny = (n + COLS_KER - 1) / COLS_KER * COLS_KER;

  alignas(64) static f32    A[ROW_N * COL_N];
  alignas(64) static f32x16 B[ROW_N * COL_N / 16];
  alignas(64) static f32x16 C[ROW_N * COL_N / 16];

  memset(A, 0, 4 * nx * ny);
  memset(B, 0, 4 * nx * ny); 
  memset(C, 0, 4 * nx * ny);

  for (u32 i = 0; i < n; i++) {
    memcpy(A + (i * ny), a + i * n, 4 * n);
    memcpy(B + (i * ny) / 16, b + i * n, 4 * n);
  }

  const u32 s3 = std::min((32U << 10) / 4, (32 << 20) / (4 * n) / ROWS_KER * ROWS_KER);
  const u32 s2 = (1  << 20) / (4 * n) / COLS_KER * COLS_KER;
  const u32 s1 = (32 << 10) / (4 * s3);

  // std::cerr << "s3: " << s3 << " s2: " << s2 << " s1: " << s1 << std::endl;

  for (u32 i3 = 0; i3 < n; i3 += s3) {
    for (u32 i2 = 0; i2 < n; i2 += s2) {
      for (u32 i1 = 0; i1 < n; i1 += s1) {
        for (u32 x = i2; x < std::min(i2 + s2, n); x += ROWS_KER) {
          for (u32 y = i3; y < std::min(i3 + s3, n); y += COLS_KER) {
            kernel(A, B, C, x, y, i1, std::min(i1 + s1, n), ny);
          }
        }
      }
    }
  }

  for (u32 i = 0; i < n; i++) {
    memcpy(c + (i * n), C + (i * ny) / 16, 4 * n);
  }
}
