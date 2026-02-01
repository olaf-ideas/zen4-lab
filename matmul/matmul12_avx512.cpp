#include <cstdlib>
#include <immintrin.h>
#include <cstdint>
#include <cstring>

using u32 = std::uint32_t;
using f32 = float;
using f32x16 __attribute__ (( vector_size((64)) )) = f32;

constexpr u32 MAX_N = 1 << 12;

void matmul(const f32 *a, const f32 *b, f32 *__restrict__ c, int _n) {

  const u32 n = _n;
  const u32 m = (n + 15) / 16;

  alignas(64) static f32x16 A[MAX_N * MAX_N / 16];
  alignas(64) static f32x16 B[MAX_N * MAX_N / 16];

  memset(A, 0, 4 * 16 * n * m); 
  memset(B, 0, 4 * 16 * n * m);

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
        A[i * m + j / 16][j % 16] = a[i * n + j];
        B[i * m + j / 16][j % 16] = b[j * n + i];
    }
  }

  for (u32 i = 0; i < n; i++) {
    for (u32 j = 0; j < n; j++) {
      f32x16 s{};
      for (u32 k = 0; k < m; k++) {
        s += A[i * m + k] * B[j * m + k];
      }

      c[i * n + j] = _mm512_reduce_add_ps(s); // 12% speedup
      /*
      for (int k = 0; k < 16; k++) {
        c[i * n + j] += s[k];
      }
      */
    }
  }
}
