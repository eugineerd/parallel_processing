#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_VEC_SIZE 16

int int_cmp(const void *a, const void *b) { return *(int *)a - *(int *)b; }

int main() {
  // Init
  MPI_Init(NULL, NULL);

  int my_rank = 0;
  int world_size = 0;

  MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  // Read input
  int vec1_root[MAX_VEC_SIZE];
  if (my_rank == 0) {
    for (int i = 0; i < MAX_VEC_SIZE; ++i) {
      scanf("%d", &vec1_root[i]);
    }
  }
  int my_elements_n = MAX_VEC_SIZE / world_size;

  // Step 0
  int vec1_child[my_elements_n];
  MPI_Scatter(vec1_root, my_elements_n, MPI_INT, vec1_child, my_elements_n,
              MPI_INT, 0, MPI_COMM_WORLD);
  qsort(vec1_child, my_elements_n, sizeof(int), int_cmp);
  printf("%d: ", my_rank);
  for (int i = 0; i < MAX_VEC_SIZE; ++i) {
    printf("%d ", vec1_child[i]);
  }
  printf("\n");

  int coop_rank = 0;
  if (my_rank % 2 == 0) {
    coop_rank = my_rank + 1;
  } else {
    coop_rank = my_rank - 1;
  }
  MPI_Sendrecv(vec1_child, my_elements_n, MPI_INT, coop_rank, 1, vec1_child,
               my_elements_n, MPI_INT, coop_rank, 1, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  // Step n
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