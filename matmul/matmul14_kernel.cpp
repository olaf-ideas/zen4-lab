#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

#define INLINE inline __attribute__ ((always_inline))

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

constexpr u32 MAX_N = 1 << 12;

// fma -> latency: 4 cycles, throughput: 2 per cycle
// 32 zmm registers

#ifndef ROWS_KER
constexpr u32 ROWS_KER = 4;
#endif

#ifndef COLS_KER
constexpr u32 COLS_KER = 2 * 16;
#endif

constexpr u32 ROW_N = (MAX_N + ROWS_KER - 1) / ROWS_KER * ROWS_KER;
constexpr u32 COL_N = (MAX_N + COLS_KER - 1) / COLS_KER * COLS_KER;

static INLINE void kernel(
  const f32 *a,
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

  for (u32 x = 0; x < n; x += ROWS_KER) {
    for (u32 y = 0; y < n; y += COLS_KER) {
      kernel(A, B, C, x, y, 0, n, ny);
    }
  }

  for (u32 i = 0; i < n; i++) {
    memcpy(c + (i * n), C + (i * ny) / 16, 4 * n);
  }
}
