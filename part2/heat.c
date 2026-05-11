#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define N 4096
#define C 0.25

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int local_n = N / size;
    int steps = (argc > 1) ? atoi(argv[1]) : 100;

    // Allocate two arrays with ghost cells on each side
    double* current = (double*)calloc(local_n + 2, sizeof(double));
    double* next    = (double*)calloc(local_n + 2, sizeof(double));

    // ---- Step 4: initialize ----
    if (rank == 0) {
        current[1] = 100.0;   // global position 0 = heat source
    }

    // Identify neighbors. MPI_PROC_NULL means "no neighbor" — Send/Recv to it is a no-op.
    int left  = (rank == 0)        ? MPI_PROC_NULL : rank - 1;
    int right = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

    // ---- Step 5: time loop ----
    int start = (rank == 0)        ? 2           : 1;
    int end   = (rank == size - 1) ? local_n - 1 : local_n;

    for (int t = 0; t < steps; t++) {
        MPI_Sendrecv(&current[1],           1, MPI_DOUBLE, left,  0,
                     &current[local_n + 1], 1, MPI_DOUBLE, right, 0,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&current[local_n], 1, MPI_DOUBLE, right, 1,
                     &current[0],       1, MPI_DOUBLE, left,  1,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = start; i <= end; i++) {
            next[i] = current[i] + C * (current[i-1] - 2.0 * current[i] + current[i+1]);
        }
        if (rank == 0)        next[1]       = 100.0;
        if (rank == size - 1) next[local_n] = current[local_n];

        double* tmp = current; current = next; next = tmp;
    }

    // ---- Step 6: gather and write CSV ----
    double* result = NULL;
    if (rank == 0) {
        result = (double*)malloc(N * sizeof(double));
    }

    MPI_Gather(&current[1], local_n, MPI_DOUBLE,
               result,      local_n, MPI_DOUBLE,
               0, MPI_COMM_WORLD);

    if (rank == 0) {
        FILE* f = fopen("output.csv", "w");
        fprintf(f, "position,temperature\n");
        for (int i = 0; i < N; i++) {
            fprintf(f, "%d,%.6f\n", i, result[i]);
        }
        fclose(f);
        free(result);
        printf("Done. Wrote output.csv (%d steps)\n", steps);
    }

    free(current);
    free(next);
    MPI_Finalize();
    return 0;
}