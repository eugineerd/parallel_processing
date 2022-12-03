#include <mpi.h>
#include <stdio.h>

#define MAX_VEC_SIZE 4

int main() {
  MPI_Init(NULL, NULL);
  int my_rank = 0;
  int world_size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  int vec1_root[MAX_VEC_SIZE];
  int vec2_root[MAX_VEC_SIZE];
  if (my_rank == 0) {
    for (int i = 0; i < MAX_VEC_SIZE; ++i) {
      scanf("%d", &vec1_root[i]);
    }
    for (int i = 0; i < MAX_VEC_SIZE; ++i) {
      scanf("%d", &vec2_root[i]);
    }
  }
  int my_elements_n = MAX_VEC_SIZE / world_size;

  int vec1_child[my_elements_n];
  int vec2_child[my_elements_n];

  MPI_Scatter(vec1_root, my_elements_n, MPI_INT, vec1_child, my_elements_n,
              MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Scatter(vec2_root, my_elements_n, MPI_INT, vec2_child, my_elements_n,
              MPI_INT, 0, MPI_COMM_WORLD);

  for (int i = 0; i < my_elements_n; ++i) {
    vec1_child[i] += vec2_child[i];
  }

  MPI_Gather(vec1_child, my_elements_n, MPI_INTEGER, vec1_root, my_elements_n,
             MPI_INTEGER, 0, MPI_COMM_WORLD);

  if (my_rank == 0) {
    for (int i = 0; i < MAX_VEC_SIZE; ++i) {
      printf("%d ", vec1_root[i]);
    }
    printf("\n");
  }

  MPI_Finalize();
}