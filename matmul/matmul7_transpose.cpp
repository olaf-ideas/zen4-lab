#include <cstdlib>
#include <immintrin.h>

const int MAX_N = 2 << 10;

void matmul(const float *a, const float *b, float *__restrict__ c, int n) {
  
  alignas(64) static float bb[MAX_N*MAX_N];
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      bb[i * n + j] = b[j * n + i];

  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++) {
      float s = 0;
      for (int k = 0; k < n; k++)
        s += a[i * n + k] * bb[j * n + k];
      c[i * n + j] = s;
    }
}
