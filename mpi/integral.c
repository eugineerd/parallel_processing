#include <mpi.h>
#include <stdio.h>

#define MAX_MSG_SIZE 100
#define MPI_PROC_MAX 10

int n = 100;

double calc_trap(double h, double left, double right, int n) {
    double est = (left*left+right*right) / 2.0;
    for (int i = 0; i < n; ++i){
        double x = left + i*h;
        est += x*x;
    }
    return est * h;
}

typedef struct {
    char a;
    double b;
    int c;
} TEST;


int main(){
    MPI_Init(NULL, NULL);
    int rank = 0;
    int world_size = 0;
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    int my_n = n / world_size;
    TEST msg = {
        0, 0, 0
    };
    
    if (rank == 0){
        for (int i = 1; i<world_size; ++i){
            MPI_Recv((char*)&msg, sizeof(TEST), MPI_CHAR, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            printf("%c, %lf, %d\n", msg.a, msg.b, msg.c);
        }
    } else {
        msg.a = 'A'+rank;
        msg.b = 20.1+rank;
        msg.c = -1+rank;
        MPI_Send((char*)&msg, sizeof(TEST), MPI_CHAR, 0, 0, MPI_COMM_WORLD);
    }
        
    MPI_Finalize();
}