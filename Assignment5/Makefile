# Compiler
CC = gcc
MPICC = mpicc
CFLAGS = -O2 -Wall
MPIFLAGS = -O2 -Wall

# Targets
all: gsum generate_data print_data serial_sum

# MPI program
gsum: gsum.c
	$(MPICC) $(MPIFLAGS) -o gsum gsum.c

# Serial baseline program
serial_sum: serial_sum.c
	$(MPICC) $(MPIFLAGS) -o serial_sum serial_sum.c

# Data generator
generate_data: generate_data.c
	$(CC) $(CFLAGS) -o generate_data generate_data.c

# Print/check program
print_data: print_data.c
	$(CC) $(CFLAGS) -o print_data print_data.c

# Clean
clean:
	rm -f gsum serial_sum generate_data print_data *.o