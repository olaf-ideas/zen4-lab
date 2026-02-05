#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

#define INLINE inline __attribute__ ((always_inline))

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

template <u32 val>
constexpr u32 roundup(u32 x) { return (x + val - 1) / val * val; }

constexpr u32 MAX_N = 1 << 12;

#ifndef KERNEL_R
constexpr u32 KERNEL_R = 8;
#endif

#ifndef KERNEL_C
constexpr u32 KERNEL_C = 2;
#endif

constexpr u32 ROWS_WORDS = KERNEL_R;
constexpr u32 COLS_WORDS = KERNEL_C * 16;

constexpr u32 ROW_N = roundup<ROWS_WORDS>(MAX_N);
constexpr u32 COL_N = roundup<COLS_WORDS>(MAX_N);

static INLINE void kernel(
  const f32 *__restrict__ a,
  const f32 *__restrict__ b,
        f32 *__restrict__ c,
  u32 x, u32 y,
  u32 l, u32 r,
  u32 n) {

  alignas(64) f32x16 c_reg[KERNEL_R][KERNEL_C] = {};
  alignas(64) f32x16 a_reg[KERNEL_R];
  alignas(64) f32x16 b_reg[KERNEL_C];

  for (u32 k = l; k < r; k++) {
    for (u32 i = 0; i < KERNEL_R; i++) {
      a_reg[i] = f32x16{} + a[(x + i) * n + k];
    }

    for (u32 i = 0; i < KERNEL_C; i++) {
      b_reg[i] = *(f32x16*)&b[k * n + y + 16 * i];
    }

    for (u32 i = 0; i < KERNEL_R; i++) {
      for (u32 j = 0; j < KERNEL_C; j++) {
        c_reg[i][j] += a_reg[i] * b_reg[j];
      }
    }
  }

  for (u32 i = 0; i < KERNEL_R; i++) {
    for (u32 j = 0; j < KERNEL_C; j++) {
      *(f32x16*)&c[(x + i) * n + y + 16 * j] += c_reg[i][j];
    }
  }
}

void matmul(const f32 *__restrict__ a, 
            const f32 *__restrict__ b, 
                  f32 *__restrict__ c, 
            int _n) {

  const u32 n = _n;
  const u32 nx = roundup<ROWS_WORDS>(n);
  const u32 ny = roundup<COLS_WORDS>(n);
  
  alignas(64) static f32 A[ROW_N * COL_N];
  alignas(64) static f32 B[ROW_N * COL_N];
  alignas(64) static f32 C[ROW_N * COL_N];

  memset(A, 0, 4 * nx * ny);
  memset(B, 0, 4 * nx * ny); 
  memset(C, 0, 4 * nx * ny);

  for (u32 i = 0; i < n; i++) {
    memcpy(A + i * ny, a + i * n, 4 * n);
    memcpy(B + i * ny, b + i * n, 4 * n);
  }

  for (u32 x = 0; x < n; x += ROWS_WORDS) {
    for (u32 y = 0; y < n; y += COLS_WORDS) {
      kernel(A, B, C, x, y, 0, n, ny);
    }
  }

  for (u32 i = 0; i < n; i++) {
    memcpy(c + i * n, C + i * ny, 4 * n);
  }
}
