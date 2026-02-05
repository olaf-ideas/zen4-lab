#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>
#include <algorithm>
#include <iostream>

#define INLINE inline __attribute__ ((always_inline))

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

constexpr u32 MAX_N = 1 << 12;

// fma -> latency: 4 cycles, throughput: 1 per cycle
// 32 zmm registers

#undef ROWS_KER
#undef COLS_KER

constexpr u32 ROWS_KER = 4;
constexpr u32 COLS_KER = 2 * 16;

constexpr u32 ROW_N = (MAX_N + ROWS_KER - 1) / ROWS_KER * ROWS_KER;
constexpr u32 COL_N = (MAX_N + COLS_KER - 1) / COLS_KER * COLS_KER;

template <u32 val>
constexpr u32 roundup(u32 x) {
  return (x + val - 1) / val * val;
}

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {

  const u32 s3 = 128; // std::min((32U << 10) / 4, (32 << 20) / (4 * n) / ROWS_KER * ROWS_KER);
  const u32 s2 = 64; // (1  << 20) / (4 * n) / COLS_KER * COLS_KER;
  const u32 s1 = 64; // (32 << 10) / (4 * s3);

  const u32 n = _n;
  const u32 nx = roundup<s2>(roundup<ROWS_KER>(n));
  const u32 ny = roundup<s3>(roundup<COLS_KER>(n));

  alignas(64) static f32 A[ROW_N * COL_N];
  alignas(64) static f32 B[ROW_N * COL_N];
  alignas(64) static f32 C[ROW_N * COL_N];

  memset(A, 0, 4 * nx * ny);
  memset(B, 0, 4 * nx * ny);
  memset(C, 0, 4 * nx * ny);

  for (u32 i = 0; i < n; i++) {
    memcpy(A + (i * ny), a + i * n, 4 * n);
    memcpy(B + (i * ny), b + i * n, 4 * n);
  }

  for (u32 i3 = 0; i3 < n; i3 += s3) {
    for (u32 i2 = 0; i2 < n; i2 += s2) {
      for (u32 i1 = 0; i1 < n; i1 += s1) {
        for (u32 x = i2; x < i2 + s2; x += ROWS_KER) {
          for (u32 y = i3; y < i3 + s3; y += COLS_KER) {
            alignas(64) f32x16 d00 = {};
            alignas(64) f32x16 d01 = {};
            alignas(64) f32x16 d10 = {};
            alignas(64) f32x16 d11 = {};
            alignas(64) f32x16 d20 = {};
            alignas(64) f32x16 d21 = {};
            alignas(64) f32x16 d30 = {};
            alignas(64) f32x16 d31 = {};

            const f32 *a0 = &A[(x + 0) * ny + i1];
            const f32 *a1 = &A[(x + 1) * ny + i1];
            const f32 *a2 = &A[(x + 2) * ny + i1];
            const f32 *a3 = &A[(x + 3) * ny + i1];

            const f32 *b0 = &B[i1 * ny + y + 0];
            const f32 *b1 = &B[i1 * ny + y + 16];

            for (u32 i = s1; i --> 0; ) {
            // for (u32 k = l; k < r; k++) {
                f32x16 _b0 = *(f32x16*) b0;
                f32x16 _b1 = *(f32x16*) b1;

                f32x16 _a0 = _mm512_set1_ps(*a0);
                f32x16 _a1 = _mm512_set1_ps(*a1);
                f32x16 _a2 = _mm512_set1_ps(*a2);
                f32x16 _a3 = _mm512_set1_ps(*a3);

                d00 += _a0 * _b0;
                d01 += _a0 * _b1;
                d10 += _a1 * _b0;
                d11 += _a1 * _b1;
                d20 += _a2 * _b0;
                d21 += _a2 * _b1;
                d30 += _a3 * _b0;
                d31 += _a3 * _b1;
                
                a0++;
                a1++;
                a2++;
                a3++;

                b0 += ny;
                b1 += ny;
            }

            *(f32x16*) &C[(x + 0) * ny + y +  0] += d00;
            *(f32x16*) &C[(x + 0) * ny + y + 16] += d01;
            *(f32x16*) &C[(x + 1) * ny + y +  0] += d10;
            *(f32x16*) &C[(x + 1) * ny + y + 16] += d11;
            *(f32x16*) &C[(x + 2) * ny + y +  0] += d20;
            *(f32x16*) &C[(x + 2) * ny + y + 16] += d21;
            *(f32x16*) &C[(x + 3) * ny + y +  0] += d30;
            *(f32x16*) &C[(x + 3) * ny + y + 16] += d31;
          }
        }
      }
    }
  }

  for (u32 i = 0; i < n; i++) {
    memcpy(c + (i * n), C + (i * ny), 4 * n);
  }
}
