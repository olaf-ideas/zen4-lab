#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>
#include <cassert>

#define INLINE inline __attribute__ ((always_inline))

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

static constexpr u32 MAX_N = 1 << 12;
static constexpr u32 L1 = 32 << 10;
static constexpr u32 L2 = L1 << 5;
static constexpr u32 L3 = L2 << 5;

// based on https://arxiv.org/pdf/2302.08417

static constexpr u32 nr = 8;
static constexpr u32 mr = 2 * 16;  

static constexpr u32 kc = 512;  // L1 / nr / sizeof(f32) = 1024
static constexpr u32 mc = 512;  // L2 / kc / sizeof(f32) =  256
static constexpr u32 nc = 2048; // L3 / kc / sizeof(f32) = 8192

void matmul(const f32 *__restrict__ a, 
            const f32 *__restrict__ b, 
                  f32 *__restrict__ c, 
            int _n) {

  const u32 n = _n;
  assert(n % nc == 0);
  assert(n % mc == 0);
  assert(n % kc == 0);

  assert(nc % nr == 0);
  assert(mc % mr == 0);

  assert(mr % 16 == 0);

  assert(sizeof(f32) * kc * nc <= L3);
  assert(sizeof(f32) * mc * kc <= L2);
  assert(sizeof(f32) * kc * nr <= L1);

  alignas(64) static f32 A[L2];
  alignas(64) static f32 B[L3];

  for (u32 i5 = 0; i5 < n; i5 += nc) {
    for (u32 i4 = 0; i4 < n; i4 += kc) {
      f32* B_L3 = B;
      for (u32 c1 = 0; c1 < nc; c1 += nr) {
        for (u32 r1 = 0; r1 < kc; r1++) {
          for (u32 c2 = 0; c2 < nr; c2++) {
            *(B_L3++) = b[i5 * n + r1 + (c1 + c2) * n];
          }
        }
      }

      for (u32 i3 = 0; i3 < n; i3 += mc) {
        f32* A_L2 = A;
        for (u32 r1 = 0; r1 < mc; r1 += mr) {
          for (u32 c1 = 0; c1 < kc; c1++) {
            for (u32 r2 = 0; r2 < mr; r2++) {
              *(A_L2++) = a[i4 * n + (r1 + r2) + c1 * n];
            }
          }
        }

        for (u32 i2 = 0; i2 < nc; i2 += nr) {
          for (u32 i1 = 0; i1 < mc; i1 += mr) {
            alignas(64) f32x16 c_reg[mr][nr];

            for (u32 i = 0; i < nr; i++) {
              for (u32 j = 0; j < mr; j += 16) {
                c_reg[i][j] = *(f32x16*)&c[i5 * n + i3 + i2 * n + i * n + j];
              }
            }

            for (u32 k = 0; k < kc; k++) {
              for (u32 i = 0; i < nr; i++) {
                f32x16 b_reg = f32x16{} + B_L3[k * nr + i];
                for (u32 j = 0; j < mr; j += 16) {
                  f32x16 a_reg = *(f32x16*)&A_L2[k * mr + j];
                  c_reg[i][j] += a_reg * b_reg;
                }
              }
            }

            for (u32 i = 0; i < nr; i++) {
              for (u32 j = 0; j < mr; j += 16) {
                *(f32x16*)&c[i5 * n + i3 + i2 * n + i * n + j] = c_reg[i][j];
              }
            }
          }
        }
      }
    }
  }
}
