#include <stdio.h>
#include <mpi.h>

double f(double x)
{
    return x * x;
}

void input(int my_rank, int comm_sz, double *a_p, double *b_p, int *n_p){
    if(my_rank == 0){
        printf("Enter a, b, n: ");
        scanf("%lf %lf %d", a_p, b_p, n_p);
        for(int dest = 1; dest < comm_sz; dest++) {
            MPI_Send(a_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(b_p, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            MPI_Send(n_p, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
        }
    } else{
        MPI_Recv(a_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(b_p, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(n_p, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

double Trap(double left_endp, double right_endp, int n, double h)
{
    double estimate = (f(left_endp) + f(right_endp)) / 2.0;
    for(int i = 1; i < n; ++i){
        double x = left_endp + i * h;
        estimate += f(x);
    }
    estimate *= h;
    return estimate;
}

int main() {

    int my_rank, comm_sz;
    double a = 0;
    double b = 3;
    int n = 1024;
    int local_n;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    input(my_rank, comm_sz, &a, &b, &n);

    double h = (b - a) / n;
    local_n = n / comm_sz;

    double local_a = a + my_rank * local_n * h;
    double local_b = local_a + local_n * h;
    double total_int = 0;
    double local_int = Trap(local_a, local_b, local_n, h);

    if(my_rank != 0)
    {
        MPI_Send(&local_int, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
    }
    else{
        total_int = local_int;
        for(int source = 1; source < comm_sz; source++){
            MPI_Recv(&local_int, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            total_int += local_int;
        }
    }

    if(my_rank == 0){
        printf("%lf\n", total_int);
    }

    MPI_Finalize();

    return 0;

}
