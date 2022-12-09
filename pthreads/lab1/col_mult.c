#include "matrix.h"
#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

matrix_t mat_a;
matrix_t mat_b;
// Т.к. все потоки работают с непересекающимеся столбцами,
// синхронизации доступв не требуется.
matrix_t result;

typedef struct {
  uint32_t idx_start;
  uint32_t n_rows;
} row_mult_workload_t;

void col_mult(row_mult_workload_t *data) {
  int idx_start = data->idx_start;
  int n_cols = data->n_rows;
  for (int col_idx = idx_start; col_idx < idx_start + n_cols; ++col_idx) {
    for (int k = 0; k < result.n; ++k) {
      for (int j = 0; j < mat_a.n; ++j) {
        matrix_data_t result_col_val = matrix_get(&result, j, col_idx);
        matrix_data_t mul =
            matrix_get(&mat_a, j, k) * matrix_get(&mat_b, k, col_idx);
        result_col_val += mul;
        matrix_set(&result, j, col_idx, result_col_val);
      }
    }
  }
}

clock_t run_row_mult_test(FILE *in_file, FILE *out_file, uint32_t num_threads) {
  mat_a = matrix_read(in_file);
  mat_b = matrix_read(in_file);

  if (mat_a.m != mat_b.n) {
    fprintf(stderr, "Matrix dimensions don't match: %ldx%ld and %ldx%ld\n",
            mat_a.n, mat_a.m, mat_b.n, mat_b.m);
    exit(EINVAL);
  }

  result = matrix_new_zero(mat_a.n, mat_b.m);

  pthread_t *handles = malloc((num_threads + 1) * sizeof(pthread_t));
  row_mult_workload_t *data =
      malloc((num_threads + 1) * sizeof(row_mult_workload_t));
  if (!handles || !data) {
    fprintf(stderr, "Failed to allocate memory for pthread workload\n");
    exit(1);
  }

  uint32_t n_per_thread = result.n / num_threads;
  uint32_t n_leftover = result.n % num_threads;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  for (int i = 0; i < num_threads; ++i) {
    data[i].idx_start = i * n_per_thread;
    data[i].n_rows = n_per_thread;
    pthread_create(&handles[i], NULL, (void *(*)(void *)) & col_mult, &data[i]);
  }
  if (n_leftover) {
    data[num_threads].idx_start = n_per_thread * num_threads;
    data[num_threads].n_rows = n_leftover;
    pthread_create(&handles[num_threads], NULL, (void *(*)(void *)) & col_mult,
                   &data[num_threads]);
  }
  for (int i = 0; i < num_threads; i++) {
    pthread_join(handles[i], NULL);
  }
  if (n_leftover) {
    pthread_join(handles[num_threads], NULL);
  }

  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_nsec - start.tv_nsec) / 1000;

  fprintf(out_file, "\nresult: \n");
  for (int i = 0; i < result.n; ++i) {
    for (int j = 0; j < result.m; ++j) {
      fprintf(out_file, "%6.2lf ", matrix_get(&result, i, j));
    }
    fputs("\n", out_file);
  }
  return delta_us;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("args: <in_file> <out_file> <num_threads>\n");
    exit(0);
  }

  FILE *in_file = fopen(argv[1], "r");
  FILE *out_file = NULL;
  if (strcmp(argv[2], "-") == 0) {
    out_file = stdout;
  } else {
    out_file = fopen(argv[2], "w");
  }

  clock_t run_time = run_row_mult_test(in_file, out_file, atoi(argv[3]));
  printf("%ld", run_time);

  fclose(in_file);
  fclose(out_file);
}