#include <math.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_STEPS 250
#define MAX_ITER 10000
#define DR 0.005
#define DT 0.01
#define M 100.0
#define G 10.0
#define P 2000.0

#define THREE_HALF_PI 3.0 * M_PI_2

typedef struct __attribute__((__packed__)) {
  // Iter step
  double x1;
  double x2;
  double y;
  double phi1;
  double phi2;
  // Global step
  double Ax;
  double Ay;
  double Bx;
  double By;
  double C;
  double vy;
  double time;
} Results;

Results results[MAX_STEPS];
#define NUM_RES sizeof(Results) / sizeof(double)
#define NUM_VARS 5

int main() {
  MPI_Init(NULL, NULL);
  int my_rank = 0;
  int world_size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Initial values
  Results init_vals = {
      .x1 = -0.1,
      .x2 = 0.1,
      .y = 0.0,
      .phi1 = 2.0,
      .phi2 = 2.0,
      .Ax = -0.353,
      .Ay = 0.3,
      .Bx = 0.353,
      .By = 0.3,
      .vy = 0.0,
      .C = 3.0 * M_PI / 8.0,
      .time = 0.0,
  };
  results[0] = init_vals;

  for (int i = 0; i < MAX_STEPS - 1; ++i) {
    Results *my_r = &results[i];

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC_RAW, &start);

    for (int j = 0; j < MAX_ITER; ++j) {
#ifdef MULTITHREAD
      MPI_Bcast(my_r, NUM_VARS, MPI_DOUBLE, 0, MPI_COMM_WORLD);
      switch (my_rank) {
      case 0: {
        double f_x =
            (my_r->x1 + my_r->y * cos(THREE_HALF_PI - my_r->phi1) - my_r->Ax);
        my_r->x1 -= f_x * DR;
        break;
      }
      case 1: {
        double f_x =
            (my_r->x2 + my_r->y * cos(THREE_HALF_PI + my_r->phi2) - my_r->Bx);
        my_r->x2 -= f_x * DR;
        break;
      }
      case 2: {
        double f_x =
            (my_r->y + my_r->y * sin(THREE_HALF_PI - my_r->phi1) - my_r->Ay);
        my_r->y -= f_x * DR;
        break;
      }
      case 3: {
        double f_x = ((my_r->phi1 + my_r->phi2) * my_r->y +
                      (my_r->x2 - my_r->x1) - my_r->C);
        my_r->phi1 -= f_x * DR;
        break;
      }
      case 4: {
        double f_x =
            (my_r->y + my_r->y * sin(THREE_HALF_PI + my_r->phi2) - my_r->By);
        my_r->phi2 -= f_x * DR;
        break;
      }
      }
      MPI_Gather(((double *)my_r) + my_rank, 1, MPI_DOUBLE, my_r, 1, MPI_DOUBLE,
                 0, MPI_COMM_WORLD);
#else
      double f_x1 =
          (my_r->x1 + my_r->y * cos(THREE_HALF_PI - my_r->phi1) - my_r->Ax);
      double f_x2 =
          (my_r->x2 + my_r->y * cos(THREE_HALF_PI + my_r->phi2) - my_r->Bx);
      double f_y =
          (my_r->y + my_r->y * sin(THREE_HALF_PI - my_r->phi1) - my_r->Ay);
      double f_phi1 = ((my_r->phi1 + my_r->phi2) * my_r->y +
                       (my_r->x2 - my_r->x1) - my_r->C);
      double f_phi2 =
          (my_r->y + my_r->y * sin(THREE_HALF_PI + my_r->phi2) - my_r->By);
      my_r->x1 -= f_x1 * DR;
      my_r->x2 -= f_x2 * DR;
      my_r->y -= f_y * DR;
      my_r->phi1 -= f_phi1 * DR;
      my_r->phi2 -= f_phi2 * DR;
      //   if (fabs(my_r->phi1) > 2.0 * M_PI) {
      //     my_r->phi1 -= (float)((int)(my_r->phi1 / (2.0 * M_PI)) * 2.0 *
      //     M_PI);
      //   }
#endif
    }
    clock_gettime(CLOCK_MONOTONIC_RAW, &end);
    uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 +
                        (end.tv_nsec - start.tv_nsec) / 1000;

    Results *next_r = &results[i + 1];
    if (my_rank == 0) {
      *next_r = *my_r;
      next_r->Ay += my_r->vy * DT;
      next_r->By = next_r->Ay;
      next_r->time = (double)delta_us;
      next_r->vy += (P * (my_r->x2 - my_r->x1) - M * G) / M * DT;
    }
#ifdef MULTITHREAD
    MPI_Bcast(next_r, NUM_RES, MPI_DOUBLE, 0, MPI_COMM_WORLD);
#endif
  }

  if (my_rank == 0) {
    FILE *f = fopen("results.csv", "w");
    fprintf(f, "n, x1, x2, y, phi1, phi2, Ax, Ay, Bx, By, C, time\n");
    for (int i = 0; i < MAX_STEPS; ++i) {
      fprintf(f, "%d, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf, %lf\n",
              i, results[i].x1, results[i].x2, results[i].y, results[i].phi1,
              results[i].phi2, results[i].Ax, results[i].Ay, results[i].Bx,
              results[i].By, results[i].C, results[i].time);
    }
    fclose(f);
  }

  MPI_Finalize();
}