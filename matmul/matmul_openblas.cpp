#include <cstdlib>
#include <cblas.h>

void matmul(const float *a, const float *b, float *c, int n) {
  static bool init = false;

  if (!init) {
    setenv("OMP_NUM_THREADS", "1", 0);
    setenv("MKL_NUM_THREADS", "1", 0);
    setenv("OPENBLAS_NUM_THREADS", "1", 0);
    setenv("BLIS_NUM_THREADS", "1", 0);
    setenv("VECLIB_MAXIMUM_THREADS", "1", 0);
    setenv("NUMEXPR_NUM_THREADS", "1", 0);
    goto_set_num_threads(1);
    openblas_set_num_threads(1);
    init = true;
  }

  openblas_set_num_threads(1);

  const float alpha = 1.0f;
  const float beta  = 0.0f;

  cblas_sgemm(
    CblasRowMajor, CblasNoTrans, CblasNoTrans,
    n, n, n,
    alpha,
    a, n,
    b, n,
    beta,
    c, n
  );
}
