#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double get_time_sec() {
    // Returns current wall-clock time in seconds
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <binary_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    // ----------------------------
    // Start total timer
    // ----------------------------
    double total_start = get_time_sec();



    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        perror("fopen");
        return 1;
    }

    fseek(fp, 0, SEEK_END);
    long n_elements = ftell(fp) / sizeof(double);
    rewind(fp);

    double *data = malloc(n_elements * sizeof(double));
    if (!data) {
        perror("malloc");
        fclose(fp);
        return 1;
    }

    size_t read = fread(data, sizeof(double), n_elements, fp);
    if (read != n_elements) {
        fprintf(stderr, "Error reading file\n");
        free(data);
        fclose(fp);
        return 1;
    }

    fclose(fp);


    // ----------------------------
    // Compute phase
    // ----------------------------
    double comp_start = get_time_sec();

    double sum = 0.0;
    for (long i = 0; i < n_elements; i++) {
        sum += data[i];
    }

    double comp_end = get_time_sec();
    double compute_time = comp_end - comp_start;

    // ----------------------------
    // Total time
    // ----------------------------
    double total_end = get_time_sec();
    double total_time = total_end - total_start;

    double io_time = total_time - compute_time;

    // ----------------------------
    // Machine-readable CSV output
    // ----------------------------
    // Format: SERIAL,<rank>,<size>,<total_time>,<compute_time>,<io_time>,<sum>
    // Use rank=0, size=1 to match MPI structure
    printf("SERIAL,%d,%d,%.8f,%.8f,%.8f,%.8f\n",
           0, 1, total_time, compute_time, io_time, sum);

    free(data);
    return 0;
}