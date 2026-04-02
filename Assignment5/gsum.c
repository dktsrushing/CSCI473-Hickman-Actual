#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

void global_sumA(double* result, int rank, int size, double my_value);
void global_sumB(double* result, int rank, int size, double my_value);

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <A|B>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];
    char  method   = argv[2][0];

    if (method != 'A' && method != 'B') {
        printf("Error: method must be A or B\n");
        return 1;
    }

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if ((size & (size - 1)) != 0) {
        if (rank == 0) fprintf(stderr, "Error: Number of processes must be a power of 2.\n");
        MPI_Finalize();
        return 1;
    }

    double compute_start, compute_end;
    double wall_start = MPI_Wtime();

    // All ranks open the file independently
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Rank %d: Failed to open %s\n", rank, filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    // Rank 0 gets N_total and sends to others
    long N_total = 0;
    if (rank == 0) {
        fseek(fp, 0, SEEK_END);
        N_total = ftell(fp) / sizeof(double);
        for (int r = 1; r < size; r++)
            MPI_Ssend(&N_total, 1, MPI_LONG, r, 10, MPI_COMM_WORLD);
    } else {
        MPI_Recv(&N_total, 1, MPI_LONG, 0, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    long chunk     = N_total / size;
    long remainder = N_total % size;
    long my_start  = rank * chunk + (rank < remainder ? rank : remainder);
    long my_count  = chunk + (rank < remainder ? 1 : 0);

    // Each rank seeks to its own chunk and reads independently
    double* local_data = malloc(my_count * sizeof(double));
    if (!local_data) {
        fprintf(stderr, "Rank %d: Failed to allocate local buffer\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    fseek(fp, my_start * sizeof(double), SEEK_SET);
    fread(local_data, sizeof(double), my_count, fp);
    fclose(fp);

    // Local sum
    compute_start = MPI_Wtime();
    double my_sum = 0.0;
    for (long i = 0; i < my_count; i++) my_sum += local_data[i];
    compute_end = MPI_Wtime();

    free(local_data);

    double result = 0.0;

    if (method == 'A') {
        global_sumA(&result, rank, size, my_sum);
        double wall_end = MPI_Wtime();
        if (rank == 0) {
            double total   = wall_end - wall_start;
            double compute = compute_end - compute_start;
            printf("RESULT_A,%d,%.8f,%.8f,%.8f,%.8f\n", size,
                total - compute, compute, total, result);
        }
    } else {
        global_sumB(&result, rank, size, my_sum);
        double wall_end = MPI_Wtime();
        if (rank == 0) {
            double total   = wall_end - wall_start;
            double compute = compute_end - compute_start;
            printf("RESULT_B,%d,%.8f,%.8f,%.8f,%.8f\n", size,
                total - compute, compute, total, result);
        }
    }

    MPI_Finalize();
    return 0;
}

void global_sumA(double* result, int rank, int size, double my_value) {
    if (rank == 0) {
        double sum = my_value;
        for (int r = 1; r < size; r++) {
            double tmp;
            MPI_Recv(&tmp, 1, MPI_DOUBLE, r, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            sum += tmp;
        }
        *result = sum;
        for (int r = 1; r < size; r++)
            MPI_Ssend(result, 1, MPI_DOUBLE, r, 2, MPI_COMM_WORLD);
    } else {
        MPI_Ssend(&my_value, 1, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
        MPI_Recv(result, 1, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
}

void global_sumB(double* result, int rank, int size, double my_value) {
    double tmp;

    for (int step = 1; step < size; step *= 2) {
        if (rank % (2*step) == 0) {
            int partner = rank + step;
            if (partner < size) {
                MPI_Recv(&tmp, 1, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                my_value += tmp;
            }
        } else if (rank % step == 0) {
            int partner = rank - step;
            MPI_Ssend(&my_value, 1, MPI_DOUBLE, partner, 0, MPI_COMM_WORLD);
            break;
        }
    }

    for (int step = size/2; step >= 1; step /= 2) {
        if (rank % (2*step) == 0) {
            int partner = rank + step;
            if (partner < size)
                MPI_Ssend(&my_value, 1, MPI_DOUBLE, partner, 1, MPI_COMM_WORLD);
        } else if (rank % step == 0) {
            int partner = rank - step;
            MPI_Recv(&my_value, 1, MPI_DOUBLE, partner, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }

    *result = my_value;
}