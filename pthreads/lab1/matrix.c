#include "matrix.h"
#include <asm-generic/errno-base.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

matrix_t matrix_read(FILE *file) {
  size_t n, m;
  int res = 0;
  res += fscanf(file, "%ld", &n);
  res += fscanf(file, "%ld", &m);
  if (res != 2) {
    fprintf(stderr, "Failed to read matrix sizes\n");
    exit(EINVAL);
  }

  matrix_data_t *data = malloc(n * m * sizeof(matrix_data_t));
  if (!data) {
    fprintf(stderr, "Failed to allocate %ld bytes\n", n * m);
    exit(ENOMEM);
  }

  int elements_read = 0;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < m; ++j) {
      elements_read += fscanf(file, MATRIX_DATA_FORMAT_STR, &data[i * m + j]);
    }
  }
  if (elements_read != n * m) {
    fprintf(stderr, "Not enought data for %ldx%ld matrix\n", n, m);
    exit(EINVAL);
  }

  matrix_t mat = {.data = data, n = n, m = m};
  return mat;
}

matrix_data_t matrix_get(matrix_t *mat, size_t i, size_t j) {
  return mat->data[mat->m * i + j];
}

void matrix_set(matrix_t *mat, size_t i, size_t j, matrix_data_t elem) {
  mat->data[mat->m * i + j] = elem;
}

matrix_t matrix_new_uninit(size_t n, size_t m) {
  matrix_data_t *data = malloc(n * m * sizeof(matrix_data_t));
  if (!data) {
    fprintf(stderr, "Failed to allocate %ld bytes\n", n * m);
    exit(ENOMEM);
  }
  matrix_t mat = {.data = data, .m = m, .n = n};
  return mat;
}

matrix_t matrix_new_zero(size_t n, size_t m) {
  matrix_data_t *data = calloc(n * m, sizeof(matrix_data_t));
  if (!data) {
    fprintf(stderr, "Failed to allocate %ld bytes\n", n * m);
    exit(ENOMEM);
  }
  matrix_t mat = {.data = data, .m = m, .n = n};
  return mat;
}