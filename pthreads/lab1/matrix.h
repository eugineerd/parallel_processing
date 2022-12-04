#pragma once
#include <stdio.h>
#include <stdlib.h>

#define MATRIX_DATA_FORMAT_STR "%lf"

typedef double matrix_data_t;

typedef struct {
  matrix_data_t *data;
  size_t n;
  size_t m;
} matrix_t;

matrix_t matrix_read(FILE *file);
matrix_t matrix_new_uninit(size_t n, size_t m);
void matrix_set(matrix_t *mat, size_t i, size_t j, matrix_data_t elem);
matrix_data_t matrix_get(matrix_t *mat, size_t i, size_t j);