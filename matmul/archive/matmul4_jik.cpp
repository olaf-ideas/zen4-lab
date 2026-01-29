void matmul(const float *a, const float *b, float *c, int n) {
  for (int j = 0; j < n; j++)
    for (int i = 0; i < n; i++)
      for (int k = 0; k < n; k++)
        c[i * n + j] += a[i * n + k] * b[k * n + j];
}
