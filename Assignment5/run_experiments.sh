#!/bin/bash

CSV_FILE="results.csv"

DATA_FILES=("data1.bin" "data2.bin" "data3.bin" "data4.bin")
ELEMENTS_LIST=(320000000 640000000 1280000000 2560000000)

PARALLEL_PROCS=(2 4 8)
METHODS=("A" "B")

RUNS=5

echo "Program,P,N,MeanIO,StdIO,MeanCompute,StdCompute,MeanTotal,StdTotal,GlobalSum" > $CSV_FILE

# ------------------------
# Helper: compute mean/std
# ------------------------
compute_stats() {
    local arr=("$@")
    local n=${#arr[@]}

    local sum=0
    for v in "${arr[@]}"; do
        sum=$(echo "$sum + $v" | bc -l)
    done
    local mean=$(echo "$sum / $n" | bc -l)

    local var=0
    for v in "${arr[@]}"; do
        var=$(echo "$var + ($v - $mean)^2" | bc -l)
    done
    var=$(echo "$var / $n" | bc -l)
    local std=$(echo "sqrt($var)" | bc -l)

    echo "$mean $std"
}

# ------------------------
# Run experiments
# ------------------------
for i in "${!DATA_FILES[@]}"; do
    DATA_FILE="${DATA_FILES[$i]}"
    N="${ELEMENTS_LIST[$i]}"

    echo "Using file $DATA_FILE (N=$N)..."

    # ------------------------
    # Serial
    # ------------------------
    IOs=()
    COMPs=()
    TOTALs=()
    GSUM=""

    echo "Running serial for N=$N..."

    for ((r=1; r<=RUNS; r++)); do
        SERIAL_OUT=$(mpirun -np 1 ./serial_sum $DATA_FILE)

        RESULT_LINE=$(echo "$SERIAL_OUT" | grep "^SERIAL")
        IFS=',' read -r tag PVAL dummy TOTAL COMP IO SUM <<< "$RESULT_LINE"

        IOs+=("$IO")
        COMPs+=("$COMP")
        TOTALs+=("$TOTAL")
        GSUM="$SUM"
    done

    read meanIO stdIO <<< $(compute_stats "${IOs[@]}")
    read meanC stdC <<< $(compute_stats "${COMPs[@]}")
    read meanT stdT <<< $(compute_stats "${TOTALs[@]}")

    echo "SERIAL,1,$N,$meanIO,$stdIO,$meanC,$stdC,$meanT,$stdT,$GSUM" >> $CSV_FILE

    # ------------------------
    # Parallel
    # ------------------------
    for P in "${PARALLEL_PROCS[@]}"; do
        for M in "${METHODS[@]}"; do

            IOs=()
            COMPs=()
            TOTALs=()
            GSUM=""

            echo "Running P=$P, method=$M, N=$N..."

            for ((r=1; r<=RUNS; r++)); do
                PAR_OUT=$(mpirun -np $P ./gsum $DATA_FILE $M)

                RESULT_LINE=$(echo "$PAR_OUT" | grep "^RESULT_")
                IFS=',' read -r tag PVAL IO COMP TOTAL SUM <<< "$RESULT_LINE"

                IOs+=("$IO")
                COMPs+=("$COMP")
                TOTALs+=("$TOTAL")
                GSUM="$SUM"
            done

            read meanIO stdIO <<< "$(compute_stats "${IOs[@]}")"
            read meanC stdC <<< "$(compute_stats "${COMPs[@]}")"
            read meanT stdT <<< "$(compute_stats "${TOTALs[@]}")"

            echo "$M,$P,$N,$meanIO,$stdIO,$meanC,$stdC,$meanT,$stdT,$GSUM" >> "$CSV_FILE"

        done
    done
done

echo "All experiments done. Results saved to $CSV_FILE"