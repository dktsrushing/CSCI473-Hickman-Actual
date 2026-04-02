#!/bin/bash

#SBATCH --partition=shared

#SBATCH --account=ccu108

#SBATCH --nodes=1

#SBATCH --ntasks=1

#SBATCH --mem=32G

#SBATCH --time=00:10:00

#SBATCH --job-name=generate_data

module purge
module load cpu
module load slurm
module load gcc/10.2.0
module load openmpi/4.1.3

cd /home/trushing/CSCI473-Hickman-Actual/Assignment5/mpi_data

./generate_data 160000000 data1.bin

./generate_data 320000000 data2.bin

./generate_data 640000000 data3.bin

./generate_data 1280000000 data4.bin
