#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

using u32 = std::uint32_t;

constexpr int L1 = 32 << 10;
constexpr int L2 = 1 << 20;
constexpr int L3 = 32 << 20;

const int MAX_N = 2 << 10;

constexpr int BI = 128;
constexpr int BJ = 256;
constexpr int BK = 512;

static_assert(sizeof(float) * (BI*BK + BK*BJ + BI*BJ) <= L2);

void matmul(const float *a, const float *b, float *__restrict__ c, int n) {

  const int ni = ((n + BI - 1) / BI) * BI;
  const int nj = ((n + BJ - 1) / BJ) * BJ;
  const int nk = ((n + BK - 1) / BK) * BK;

  alignas(64) static float A[MAX_N * MAX_N];
  alignas(64) static float B[MAX_N * MAX_N];
  alignas(64) static float C[MAX_N * MAX_N];

  for (u32 i = 0; i < n; i++) {
    memcpy(A + i * ni, a + i * n, sizeof(float) * n);
    memcpy(B + i * nk, b + i * n, sizeof(float) * n);
  }

  memset(C, 0, sizeof(float) * n);

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
    memcpy(c + i*n, C + i*ni, sizeof(float) * n);
  }
}
