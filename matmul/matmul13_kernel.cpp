#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

constexpr u32 MAX_N = 1 << 12;

// fma -> latency: 4 cycles, throughput: 2 per cycle
// 32 zmm registers

#undef ROWS_KER
#undef COLS_KER

constexpr u32 ROWS_KER = 1;
constexpr u32 COLS_KER = 1 * 16;

void kernel(const f32 *a, 
            const f32x16 *b, 
            f32x16 *__restrict__ c,
            u32 x, u32 y,
            u32 l, u32 r, 
            u32 n) {
  f32x16 d{};

  for (u32 k = l; k < r; k++) {
    f32x16 alpha = _mm512_set1_ps(a[x * n + k]);
    d += alpha * b[(k * n + y) / 16];
  }

  c[(x * n + y) / 16] += d;
}

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {

  const u32 n = _n;
  const u32 nx = (n + ROWS_KER - 1) / ROWS_KER * ROWS_KER;
  const u32 ny = (n + COLS_KER - 1) / COLS_KER * COLS_KER;

  alignas(64) static f32    A[MAX_N * MAX_N];
  alignas(64) static f32x16 B[MAX_N * MAX_N / 16];
  alignas(64) static f32x16 C[MAX_N * MAX_N / 16];

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
