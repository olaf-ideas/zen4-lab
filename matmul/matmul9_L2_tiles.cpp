#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

using f32 = float;
using u32 = std::uint32_t;

constexpr u32 L1 = 32 << 10;
constexpr u32 L2 = 1 << 20;
constexpr u32 L3 = 32 << 20;

constexpr u32 MAX_N = 1 << 12;

constexpr u32 BI = 128;
constexpr u32 BJ = 256;
constexpr u32 BK = 512;

static_assert(sizeof(f32) * (BI*BK + BK*BJ + BI*BJ) <= L2);

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {

  const u32 n = _n;
  const u32 ni = ((n + BI - 1) / BI) * BI;
  const u32 nj = ((n + BJ - 1) / BJ) * BJ;
  const u32 nk = ((n + BK - 1) / BK) * BK;

  alignas(64) static f32 A[MAX_N * MAX_N];
  alignas(64) static f32 B[MAX_N * MAX_N];
  alignas(64) static f32 C[MAX_N * MAX_N];

  for (u32 i = 0; i < n; i++) {
    memcpy(A + i * ni, a + i * n, sizeof(f32) * n);
    memcpy(B + i * nk, b + i * n, sizeof(f32) * n);
  }

  memset(C, 0, sizeof(f32) * ni * nj);

  for (u32 i = 0; i < ni; i += BI) {
    for (u32 k = 0; k < nk; k += BK) {
      for (u32 j = 0; j < nj; j += BJ) {

        for (u32 i2 = 0; i2 < BI; i2++) {
          for (u32 k2 = 0; k2 < BK; k2++) {
            for (u32 j2 = 0; j2 < BJ; j2++) {
              u32 i3 = i + i2;
              u32 j3 = j + j2;
              u32 k3 = k + k2;
              C[i3*ni + j3] += A[i3*ni + k3] * B[k3*nk + j3];
            }
          }
        }

      }
    }
  }

  for (u32 i = 0; i < n; i++) {
    memcpy(c + i*n, C + i*ni, sizeof(f32) * n);
  }
}
