#include "matrix.h"
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>

#define EPSILON 1e-10

typedef struct {
  matrix_t *A;
  matrix_t *V;
  matrix_t *Q;
  matrix_t *R;
  size_t u_idx;
  size_t thread_idx;
  size_t num_threads;
} decomposition_data_t;

matrix_t proj_u(matrix_t *A, matrix_t *Q, size_t u, size_t a) {
  matrix_data_t s1 = 0, s2 = 0, proj;
  matrix_t out = matrix_new_uninit(A->n, 1);
  for (size_t i = 0; i < A->n; ++i) {
    s1 += matrix_get(Q, u, i) * matrix_get(A, i, a);
    s2 += matrix_get(Q, u, i) * matrix_get(Q, u, i);
  }
  proj = s1 / s2;
  for (size_t i = 0; i < A->n; ++i) {
    matrix_set(&out, i, 0, proj * matrix_get(Q, u, i));
  }
  return out;
}

void norm(matrix_t *u, size_t row_idx) {
  matrix_data_t s = 0;
  for (size_t i = 0; i < u->m; ++i) {
    s += matrix_get(u, i, 0) * matrix_get(u, i, 0);
  }
  if (fabs(s) > EPSILON) {
    s = sqrt(s);
    for (size_t i = 0; i < u->n; ++i) {
      matrix_set(u, i, 0, matrix_get(u, i, 0) / s);
    }
  }
}

pthread_mutex_t mutex;

void *decomposition_worker(void *args) {
  decomposition_data_t *data = (decomposition_data_t *)(args);

  pthread_mutex_lock(&mutex);
  int tread_idx = data->thread_idx++;
  pthread_mutex_unlock(&mutex);

  size_t M = data->A->n, N = data->A->m;
  size_t step = (data->u_idx + data->num_threads) / data->num_threads;
  for (size_t i = 0; i < data->num_threads; ++i) {
    for (size_t j = i * step;
         i == tread_idx && j < (i + 1) * step && j < data->u_idx; ++j) {
      // std::cout << "u :" << k << " a: " << row << '\n';
      matrix_t proj = proj_u(data->A, data->Q, j, data->u_idx);
      for (int k = 0; k < data->V->m; ++k) {
        matrix_set(data->V, j, k, matrix_get(&proj, k, 0));
      }
    }
  }

  pthread_exit(NULL);
}

void *summation_worker(void *args) {
  decomposition_data_t *data = (decomposition_data_t *)(args);

  pthread_mutex_lock(&mutex);
  int tread_idx = data->thread_idx++;
  pthread_mutex_unlock(&mutex);

  size_t M = data->A->n, N = data->A->m;
  size_t step = (M + data->num_threads - 1) / data->num_threads;
  for (size_t i = 0; i < data->num_threads; ++i) {
    for (size_t j = i * step; i == tread_idx && j < (i + 1) * step && j < M;
         ++j) {
      // std::cout << "u :" << k << " a: " << row << '\n';
      for (size_t e = 0; e < data->u_idx; ++e) {
        matrix_data_t tmp = matrix_get(data->Q, data->u_idx, j);
        tmp -= matrix_get(data->V, e, j);
        matrix_set(data->Q, data->u_idx, j, tmp);
      }
      matrix_data_t tmp = matrix_get(data->Q, data->u_idx, j);
      tmp += matrix_get(data->A, j, data->u_idx);
      matrix_set(data->Q, data->u_idx, j, tmp);
    }
  }

  pthread_exit(NULL);
  return NULL;
}

void *normalization_worker(void *args) {
  decomposition_data_t *data = (decomposition_data_t *)(args);

  pthread_mutex_lock(&mutex);
  int tread_idx = data->thread_idx++;
  pthread_mutex_unlock(&mutex);

  size_t N = data->Q->n, M = data->Q->m;
  size_t step = (N + data->num_threads - 1) / data->num_threads;
  for (size_t i = 0; i < data->num_threads; ++i) {
    for (size_t j = i * step; i == tread_idx && j < (i + 1) * step && j < N;
         ++j) {
      norm(data->Q, j);
    }
  }

  pthread_exit(NULL);
  return NULL;
}

void *multiply_worker(void *args) {
  decomposition_data_t *data = (decomposition_data_t *)(args);

  pthread_mutex_lock(&mutex);
  int tread_idx = data->thread_idx++;
  pthread_mutex_unlock(&mutex);

  size_t M = data->Q->n, N = data->Q->m;
  size_t step = (M + data->num_threads - 1) / data->num_threads;
  for (size_t i = 0; i < data->num_threads; ++i) {
    for (size_t j = i * step; i == tread_idx && j < (i + 1) * step && j < M;
         ++j) {
      for (size_t k = 0; k < N; ++k) {
        // std::cout << "j: " << j << " k: " << k << "\n";
        for (size_t e = 0; e < data->A->m; ++e) {
          matrix_data_t tmp = matrix_get(data->R, j, e);
          tmp += matrix_get(data->Q, j, k) * matrix_get(data->A, k, e);
          matrix_set(data->R, j, e, tmp);
        }
      }
    }
  }

  pthread_exit(NULL);
  return NULL;
}

clock_t decompose(matrix_t *m, size_t MAX_THREAD) {
  size_t M = m->n, N = m->m;

  matrix_t V = matrix_new_zero(N, 1);
  matrix_t Q = matrix_new_zero(N, M);
  matrix_t R = matrix_new_zero(N, N);

  decomposition_data_t args = {m, &V, &Q, &R, 0, 0, 0};

  struct timespec start, end;
  clock_gettime(CLOCK_MONOTONIC_RAW, &start);

  pthread_t threads[MAX_THREAD];
  int u = 0;
  for (; u < N; ++u) {
    args.u_idx = u;
    args.thread_idx = 0;
    args.num_threads = N < MAX_THREAD ? N : MAX_THREAD;
    for (int i = 0; i < args.num_threads; ++i) {
      pthread_create(&threads[i], NULL, &decomposition_worker, (void *)&args);
    }

    for (int i = 0; i < args.num_threads; ++i) {
      pthread_join(threads[i], NULL);
    }

    args.thread_idx = 0;
    args.num_threads = N < MAX_THREAD ? M : MAX_THREAD;
    for (int i = 0; i < args.num_threads; ++i) {
      pthread_create(&threads[i], NULL, &summation_worker, (void *)&args);
    }

    for (int i = 0; i < args.num_threads; ++i) {
      pthread_join(threads[i], NULL);
    }
    matrix_data_t s = 0;
    for (int e = 0; e < args.Q->m; ++e) {
      s += matrix_get(args.Q, args.u_idx, e);
    }
    if (fabs(s) < EPSILON) {
      //   N = u + 1;
      //   Q->resize(u + 1);
      //   R->resize(u + 1);
      break;
    }
  }

  args.thread_idx = 0;
  args.num_threads = N < MAX_THREAD ? N : MAX_THREAD;
  for (int i = 0; i < args.num_threads; ++i) {
    pthread_create(&threads[i], NULL, &normalization_worker, (void *)&args);
  }

  for (int i = 0; i < args.num_threads; ++i) {
    pthread_join(threads[i], NULL);
  }

  args.thread_idx = 0;
  args.num_threads = N < MAX_THREAD ? N : MAX_THREAD;
  for (int i = 0; i < args.num_threads; ++i) {
    pthread_create(&threads[i], NULL, &multiply_worker, (void *)&args);
  }

  for (int i = 0; i < args.num_threads; ++i) {
    pthread_join(threads[i], NULL);
  }
  clock_gettime(CLOCK_MONOTONIC_RAW, &end);
  uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 +
                      (end.tv_nsec - start.tv_nsec) / 1000;
  return delta_us;
}

int main(int argc, char *argv[]) {
  if (argc != 4) {
    printf("args: <in_file> <out_file> <num_threads>\n");
    exit(0);
  }

  FILE *in_file = fopen(argv[1], "r");
  matrix_t m = matrix_read(in_file);
  fclose(in_file);

  clock_t run_time = decompose(&m, atoi(argv[3]));
  printf("%ld", run_time);
}