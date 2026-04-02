#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv) {
    if (argc != 3) {
        printf("Usage: %s <binary_file> <num_to_print>\n", argv[0]);
        return 1;
    }

    char* filename = argv[1];
    int n_print = atoi(argv[2]);

    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open file %s\n", filename);
        return 1;
    }

    // Determine file size
    fseek(fp, 0, SEEK_END);
    long N_total = ftell(fp) / sizeof(double);
    fseek(fp, 0, SEEK_SET);

    if (n_print > N_total) n_print = N_total;

    double* data = malloc(n_print * sizeof(double));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(fp);
        return 1;
    }

    fread(data, sizeof(double), n_print, fp);
    fclose(fp);

    printf("First %d values in %s:\n", n_print, filename);
    for (int i = 0; i < n_print; i++) {
        printf("%f\n", data[i]);
    }

    free(data);
    return 0;
}