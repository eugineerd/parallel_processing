#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  double *prow;
  double *pvec;
  size_t n;

  double result;
} row_data_t;

double row_mult(row_data_t *data) {
  double result = 0;
  for (int j = 0; j < data->n; ++j) {
    result += data->prow[j] * data->pvec[j];
  }
  return result;
}

void *row_mult_routine(void *data) {
  row_data_t *data_ = (row_data_t *)data;
  data_->result = row_mult(data_);
  return NULL;
}

void run_row_mult() {
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
  row_data_t *data = malloc(n * sizeof(row_data_t));
  if (!handles || !data) {
    exit(1);
  }

  for (int i = 0; i < n; i++) {
    data[i].n = n;
    data[i].prow = &mat[i * n];
    data[i].pvec = vec;
    pthread_create(&handles[i], NULL, (void *(*)(void *)) & row_mult_routine,
                   &data[i]);
  }

  for (int i = 0; i < n; i++) {
    pthread_join(handles[i], NULL);
  }

  printf("\nresult: ");
  for (int i = 0; i < n; ++i) {
    printf("%.2lf ", data[i].result);
  }
  printf("\n");
}