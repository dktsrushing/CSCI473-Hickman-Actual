#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdio.h>
#include <mpi.h>

void global_sumA(double* result, int rank, int size, double my_value) {
    // ----------------------------
    // Check for power-of-2 processes
    // ----------------------------
    if ((size & (size - 1)) != 0) {
        if (rank == 0) 
            fprintf(stderr, "Error: number of processes (%d) must be a power of 2\n", size);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double io_time = 0.0;
    double compute_time = 0.0;
    double sum = my_value;  // local value

    if (rank == 0) {
        // Receive values from other ranks
        double t_io_start = MPI_Wtime();
        for (int src = 1; src < size; src++) {
            double recv_val;
            MPI_Recv(&recv_val, 1, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            double t_compute_start = MPI_Wtime();
            sum += recv_val;
            compute_time += MPI_Wtime() - t_compute_start;
        }
        io_time += MPI_Wtime() - t_io_start;

        // Send global sum back to all ranks
        t_io_start = MPI_Wtime();
        for (int dest = 1; dest < size; dest++) {
            MPI_Ssend(&sum, 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
        }
        io_time += MPI_Wtime() - t_io_start;

        *result = sum;

    } else {
        double t_io_start = MPI_Wtime();
        MPI_Ssend(&my_value, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
        MPI_Recv(result, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        io_time += MPI_Wtime() - t_io_start;
    }

    // Print communication & compute times (not including local sum)
    printf("GSUMA,%d,%d,%.8f,%.8f,%.8f\n", rank, size, io_time + compute_time, compute_time, io_time);
}

void global_sumB(double* result, int rank, int size, double my_value) {
    if ((size & (size - 1)) != 0) {
        if (rank == 0) 
            fprintf(stderr, "Error: number of processes (%d) must be a power of 2\n", size);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double io_time = 0.0;
    double compute_time = 0.0;
    double sum = my_value;

    // ----------------------------
    // Tree-based reduction
    // ----------------------------
    int step;
    for (step = 1; step < size; step *= 2) {
        if (rank % (2*step) == 0) {
            int src = rank + step;
            if (src < size) {
                double t_io_start = MPI_Wtime();
                double recv_val;
                MPI_Recv(&recv_val, 1, MPI_DOUBLE, src, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                io_time += MPI_Wtime() - t_io_start;

                double t_compute_start = MPI_Wtime();
                sum += recv_val;
                compute_time += MPI_Wtime() - t_compute_start;
            }
        } else {
            int dest = rank - step;
            double t_io_start = MPI_Wtime();
            MPI_Ssend(&sum, 1, MPI_DOUBLE, dest, 0, MPI_COMM_WORLD);
            io_time += MPI_Wtime() - t_io_start;
            break; // done after sending
        }
    }

    // ----------------------------
    // Tree-based broadcast
    // Use a new variable to avoid conflict with reduction step
    // ----------------------------
    int bstep;
    for (bstep = step/2; bstep >= 1; bstep /= 2) {
        if (rank % (2*bstep) == 0) {
            int dest = rank + bstep;
            if (dest < size) {
                double t_io_start = MPI_Wtime();
                MPI_Ssend(&sum, 1, MPI_DOUBLE, dest, 1, MPI_COMM_WORLD);
                io_time += MPI_Wtime() - t_io_start;
            }
        } else if (rank % bstep == 0) {
            int src = rank - bstep;
            double t_io_start = MPI_Wtime();
            MPI_Recv(&sum, 1, MPI_DOUBLE, src, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            io_time += MPI_Wtime() - t_io_start;
        }
    }

    *result = sum;
    printf("GSUMB,%d,%d,%.8f,%.8f,%.8f\n", rank, size, io_time + compute_time, compute_time, io_time);
}