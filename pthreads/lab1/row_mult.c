#include "row_mult.h"
#include "matrix.h"
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

matrix_t mat_a;
matrix_t mat_b;
// Т.к. все потоки работают с непересекающимеся столбцами,
// синхронизации доступв не требуется.
matrix_t result;

typedef struct {
  uint32_t mat_a_row;
  uint32_t mat_b_col;
} row_mult_workload_t;

void row_mult(row_mult_workload_t *data) {
  uint32_t row_idx = data->mat_a_row;
  uint32_t col_idx = data->mat_b_col;
  matrix_data_t sum = 0;
  for (int j = 0; j < mat_a.m; ++j) {
    sum += matrix_get(&mat_a, row_idx, j) * matrix_get(&mat_b, j, col_idx);
  }
  matrix_set(&result, row_idx, col_idx, sum);
}

clock_t run_row_mult_test(FILE *in_file, FILE *out_file) {
  mat_a = matrix_read(in_file);
  mat_b = matrix_read(in_file);

  if (mat_a.m != mat_b.n) {
    fprintf(stderr, "Matrix dimensions don't match: %ldx%ld and %ldx%ld\n",
            mat_a.n, mat_a.m, mat_b.n, mat_b.m);
    exit(EINVAL);
  }

  result = matrix_new_uninit(mat_a.n, mat_b.m);

  pthread_t *handles = malloc(result.n * result.m * sizeof(pthread_t));
  row_mult_workload_t *data =
      malloc(result.n * result.m * sizeof(row_mult_workload_t));
  if (!handles || !data) {
    fprintf(stderr, "Failed to allocate memory for pthread workload\n");
    exit(1);
  }

  clock_t start = clock() / (CLOCKS_PER_SEC / 1000);
  for (int i = 0; i < result.n; i++) {
    for (int j = 0; j < result.m; j++) {
      size_t workload_idx = result.m * i + j;
      data[workload_idx].mat_a_row = i;
      data[workload_idx].mat_b_col = j;
      pthread_create(&handles[workload_idx], NULL,
                     (void *(*)(void *)) & row_mult, &data[workload_idx]);
    }
  }

  for (int i = 0; i < result.n * result.m; i++) {
    pthread_join(handles[i], NULL);
  }
  clock_t time_diff = clock() / (CLOCKS_PER_SEC / 1000) - start;

  fprintf(out_file, "\nresult: \n");
  for (int i = 0; i < result.n; ++i) {
    for (int j = 0; j < result.m; ++j) {
      fprintf(out_file, "%6.2lf ", matrix_get(&result, i, j));
    }
    fputs("", out_file);
  }
  return time_diff;
}
