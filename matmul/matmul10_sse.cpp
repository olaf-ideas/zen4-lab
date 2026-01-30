#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

using u32 = std::uint32_t;
using f32 = float;
using f32x4 __attribute__ (( vector_size((16)) )) = f32;

constexpr u32 MAX_N = 2 << 10;

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {

  const u32 n = _n;
  const u32 m = (n + 3) / 4;

  alignas(16) static f32x4 A[MAX_N * MAX_N / 4];
  alignas(16) static f32x4 B[MAX_N * MAX_N / 4];

  memset(A, 0, 4 * 4 * n * m); 
  memset(B, 0, 4 * 4 * n * m);

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
        A[i * m + j / 4][j % 4] = a[i * n + j];
        B[i * m + j / 4][j % 4] = b[j * n + i];
    }
  }

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
      f32x4 s{};
      for (u32 k = 0; k < m; k++) {
        s += A[i * m + k] * B[j * m + k];
      }

      for (u32 k = 0; k < 4; k++) {
        c[i * n + j] += s[k];
      }
    }
  }
}
