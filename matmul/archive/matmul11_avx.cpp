#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

using u32 = std::uint32_t;
using f32 = float;
using f32x8 __attribute__ (( vector_size((32)) )) = f32;

constexpr u32 MAX_N = 1 << 12;

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {
  const u32 n = _n;
  const u32 m = (n + 7) / 8;

  alignas(32) static f32x8 A[MAX_N * MAX_N / 8];
  alignas(32) static f32x8 B[MAX_N * MAX_N / 8];

  memset(A, 0, 4 * 8 * n * m); 
  memset(B, 0, 4 * 8 * n * m);

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
        A[i * m + j / 8][j % 8] = a[i * n + j];
        B[i * m + j / 8][j % 8] = b[j * n + i];
    }
  }

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
      f32x8 s{};
      for (u32 k = 0; k < m; k++) {
        s += A[i * m + k] * B[j * m + k];
      }

      for (u32 k = 0; k < 8; k++) {
        c[i * n + j] += s[k];
      }
    }
  }
}
