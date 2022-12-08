#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double *prow;
  double *pvec;
  size_t n;
  size_t m;

  double *result;
} col_data_t;

double *col_mult(col_data_t *data) {
  double *result = malloc(data->n * data->n *sizeof(double));
  for (int i = 0; i < data->n; ++i) {
    for (int j = 0; j < data->n; ++j) {
      for (int q = 0; q < data->m; ++q) {
      result[q+i*data->n] += data->pvec[q*data->n + j] * data->prow[j*data->n + i];
      }
    }
  }
  return result;
}

void *col_mult_routine(void *data) {
  col_data_t *data_ = (col_data_t *)data;
  data_->result = malloc(data_->n * data_->n * sizeof(double));
  for (int i = 0; i < data_->n; ++i) {
    data_->result = col_mult(data_);
  }
  return NULL;
}

void run_col_mult() {
  int n, m, p;
  scanf("%d", &p);
  scanf("%d", &n);
  scanf("%d", &m);


  double *vec = malloc(n * n * sizeof(double));
  double *mat = malloc(n * n * sizeof(double));
  if (!vec || !mat) {
    exit(1);
  }

  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      scanf("%lf", &vec[i * n + j]);
    }
  }
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      scanf("%lf", &mat[i * n + j]);
    }
  }

  pthread_t *handles = malloc(n * sizeof(pthread_t));
  col_data_t *data = malloc(n * sizeof(col_data_t));
  if (!handles || !data) {
    exit(1);
  }

  for (int i = 0; i < n; i++) {
    data[i].n = n;
    data[i].m = m;
    data[i].prow = mat;
    data[i].pvec = vec;
    pthread_create(&handles[i], NULL, (void *(*)(void *)) & col_mult_routine,
                   &data[i]);
  }

  for (int i = 0; i < n; i++) {
    pthread_join(handles[i], NULL);
  }

  for (int i = 0; i < p; ++i) {
    data[i].result;
  }
  
  printf("\nresult: ");
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j < n; ++j) {
      if (j % n == 0) {
        printf("\n");
      }
      printf("%.2lf ", data->result[j*n + i]);
    }
    printf("\n");
  }
  printf("\n");
}


#include "col_mult.h"
#include <time.h>
#include <stdio.h>
#include <stdint.h>

int main() { 
  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);
  run_col_mult();
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
  printf("%ld", delta_us);
  return 0;
}