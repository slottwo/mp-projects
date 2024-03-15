#!/bin/bash

if command -v module > /dev/null 2>&1; then
    if ! module list 2>&1 | grep -q mpi; then
        module load mpi
    fi
fi

mpicc -o program src/MPI/$1.c -lm

N=$(($2 > 1 ? $2 : 2))

if [ -f program ]; then
    mpiexec -n $N ./program
    rm program
fi
