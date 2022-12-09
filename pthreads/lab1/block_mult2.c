#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"

void matrix_add(matrix_t *mat, size_t i, size_t j, matrix_data_t elem) {
  mat->data[mat->m * i + j] += elem;
}

matrix_t mat_a;
matrix_t mat_b;
// Т.к. все потоки работают с непересекающимеся столбцами,
// синхронизации доступв не требуется.
matrix_t result;

typedef struct {
  uint32_t idx_up_A;
  uint32_t n_rows_A;
  uint32_t idx_left_A;
  uint32_t n_cols_A;
  uint32_t idx_up_B;
  uint32_t n_rows_B;
  uint32_t idx_left_B;
  uint32_t n_cols_B;
} block_mult_workload_t;

void block_mult(block_mult_workload_t *data) {
  int idx_up_A = data->idx_up_A;
  int n_rows_A = data->n_rows_A;
  int idx_left_A = data->idx_left_A;
  int n_cols_A = data->n_cols_A;
  int idx_up_B = data->idx_up_B;
  int n_rows_B = data->n_rows_B;
  int idx_left_B = data->idx_left_B;
  int n_cols_B = data->n_cols_B;

  for (int row_idx_a = idx_up_A; row_idx_a < idx_up_A + n_rows_A; ++row_idx_a) {
    for (int col_idx_b = idx_left_B; col_idx_b < idx_left_B + n_cols_B;
         ++col_idx_b) {
      for (int col_idx_a = idx_left_A; col_idx_a < idx_left_A + n_cols_A;
           ++col_idx_a) {
        // matrix_data_t result_val = matrix_get(&result, row_idx_a, col_idx_b);
        matrix_data_t mul = matrix_get(&mat_a, row_idx_a, col_idx_a) *
                            matrix_get(&mat_b, col_idx_a, col_idx_b);
        matrix_add(&result, row_idx_a, col_idx_b, mul);
      }
    }
  }
}

clock_t run_block_mult_test(FILE *in_file, FILE *out_file,
                            uint32_t sqrt3_num_threads) {
  mat_a = matrix_read(in_file);
  mat_b = matrix_read(in_file);

  if (mat_a.m != mat_b.n) {
    fprintf(stderr, "Matrix dimensions don't match: %ldx%ld and %ldx%ld\n",
            mat_a.n, mat_a.m, mat_b.n, mat_b.m);
    exit(EINVAL);
  }

  result = matrix_new_zero(mat_a.n, mat_b.m);

  pthread_t *handles =
      malloc((sqrt3_num_threads * sqrt3_num_threads * sqrt3_num_threads) *
             sizeof(pthread_t));
  block_mult_workload_t *data =
      malloc((sqrt3_num_threads * sqrt3_num_threads * sqrt3_num_threads + 1) *
             sizeof(block_mult_workload_t));
  if (!handles || !data) {
    fprintf(stderr, "Failed to allocate memory for pthread workload\n");
    exit(1);
  }

  uint32_t n_per_thread = result.n / sqrt3_num_threads;
  uint32_t n_leftover = result.n % sqrt3_num_threads;

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  for (int i = 0; i < sqrt3_num_threads; ++i) {
    for (int j = 0; j < sqrt3_num_threads; ++j) {
      for (int k = 0; k < sqrt3_num_threads; ++k) {
        uint32_t ind = i * sqrt3_num_threads * sqrt3_num_threads +
                       j * sqrt3_num_threads + k;

        data[ind].idx_up_A = i * n_per_thread;
        data[ind].n_rows_A = n_per_thread;
        data[ind].idx_left_A = j * n_per_thread;
        data[ind].n_cols_A = n_per_thread;
        data[ind].idx_up_B = j * n_per_thread;
        data[ind].n_rows_B = n_per_thread;
        data[ind].idx_left_B = k * n_per_thread;
        data[ind].n_cols_B = n_per_thread;

        pthread_create(&handles[ind], NULL, (void *(*)(void *)) & block_mult,
                       &data[ind]);
      }
    }
  }
  //   if (n_leftover) {
  //     data[num_threads].idx_start = n_per_thread * num_threads;
  //     data[num_threads].n_rows = n_leftover;
  //     pthread_create(&handles[num_threads], NULL, (void *(*)(void *)) &
  //     block_mult,
  //                    &data[num_threads]);
  //   }
  for (int i = 0; i < sqrt3_num_threads * sqrt3_num_threads * sqrt3_num_threads;
       i++) {
    pthread_join(handles[i], NULL);
  }
  //   if (n_leftover) {
  //     pthread_join(handles[num_threads], NULL);
  //   }

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

  clock_t run_time = run_block_mult_test(in_file, out_file, atoi(argv[3]));
  printf("%ld", run_time);

  fclose(in_file);
  fclose(out_file);
}