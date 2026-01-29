void matmul(const float *a, const float *b, float *c, int n) {
  for (int k = 0; k < n; k++)
    for (int i = 0; i < n; i++)
      for (int j = 0; j < n; j++)
        c[i * n + j] += a[i * n + k] * b[k * n + j];
}
