#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double *prow;
  double *pvec;
  size_t n;

  double *result;
} col_data_t;

double *col_mult(col_data_t *data) {
  double *result = malloc(data->n * sizeof(double));
  for (int i = 0; i < data->n; ++i) {
    for (int j = 0; j < data->n; ++j) {
      result[j] = result[j] + data->prow[j*data->n + i] * data->pvec[i];
    }
  }
  return result;
}

void *col_mult_routine(void *data) {
  col_data_t *data_ = (col_data_t *)data;
  data_->result = malloc(data_->n * sizeof(double));
  for (int i = 0; i < data_->n; ++i) {
    data_->result = col_mult(data_);
  }
  return NULL;
}

void run_col_mult() {
  int n;
  scanf("%d", &n);

  double *vec = malloc(n * sizeof(double));
  double *mat = malloc(n * n * sizeof(double));
  if (!vec || !mat) {
    exit(1);
  }

  for (int i = 0; i < n; ++i) {
    scanf("%lf", &vec[i]);
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
    data[i].prow = mat;
    data[i].pvec = vec;
    pthread_create(&handles[i], NULL, (void *(*)(void *)) & col_mult_routine,
                   &data[i]);
  }

  for (int i = 0; i < n; i++) {
    pthread_join(handles[i], NULL);
  }
  
  for (int i = 0; i < n; ++i) {
    data[i].result;
  }
  printf("\nresult: ");
  for (int i = 0; i < data->n; ++i) {
    printf("%.2lf ", data[i].result[i]);
  }
  printf("\n");
}