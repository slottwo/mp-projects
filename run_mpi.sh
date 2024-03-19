#!/bin/bash

if [ ! "$1" ]; then
    printf "\33[31m""MPI file name expected. Options above:\n\33[33m" >&2
    cd ./src/mpi/ && ls *.c | sed -e 's/\.c$//' >&2 && cd ../../
    printf "\33[m"
    exit 1
fi

if [ ! "$2" ]; then
    printf "\33[31m""Expected number of threads\n\33[m" >&2
    exit 2
fi

#chmod +x ./src/mpi/lib/build.sh
./src/mpi/lib/build.sh

if command -v module > /dev/null 2>&1; then
    if ! module list 2>&1 | grep -q mpi; then
        module load mpi
    fi
fi

if [ ! -d ".bin" ]; then
    mkdir ".bin"
fi
cd .bin

if [ "$1" == "sieve-of-Eratosthenes" ]; then
    mpicc -c ../src/mpi/$1.c -lm
    mpicc -o program.out "$1".o ./lib/*.o -lm
else
    mpicc -o program.out ../src/mpi/$1.c -lm
fi

N=$(($2 > 1 ? $2 : 2))

if [ -f program.out ]; then
    mpiexec -n $N ./program.out
    rm program.out
fi
