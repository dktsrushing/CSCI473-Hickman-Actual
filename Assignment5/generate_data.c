#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <total_elements> <output_file>\n", argv[0]);
        return 1;
    }

    long N_total = atol(argv[1]);
    char* filename = argv[2];

    double* data = malloc(N_total * sizeof(double));
    if (!data) {
        fprintf(stderr, "Failed to allocate memory for %ld doubles\n", N_total);
        return 1;
    }

    // Seed random number generator
    srand(time(NULL));

    for (long i = 0; i < N_total; i++) {
        data[i] = (double)rand() / RAND_MAX;  // random double in [0,1)
    }

    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open %s for writing\n", filename);
        free(data);
        return 1;
    }

    fwrite(data, sizeof(double), N_total, fp);
    fclose(fp);
    free(data);

    printf("Generated %ld random doubles in %s\n", N_total, filename);
    return 0;
}