void matmul(const float *a, const float *b, float *__restrict__ c, int n) {
  for (int i = 0; i < n; i++)
    for (int k = 0; k < n; k++)
      for (int j = 0; j < n; j++)
        c[i * n + j] += a[i * n + k] * b[k * n + j];
}
