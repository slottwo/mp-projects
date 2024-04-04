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

if [[ ! -x ./src/mpi/lib/build.sh ]]; then
    chmod +x ./src/mpi/lib/build.sh
fi

if command -v module > /dev/null 2>&1; then
    if ! module list 2>&1 | grep -q mpi; then
        module load mpi
    fi
fi

if [ "$3" ] && [ "$3" == "-D" ]; then
    ./src/mpi/lib/build.sh -D
else
    ./src/mpi/lib/build.sh
fi

if [ ! -d ".bin" ]; then
    mkdir ".bin"
fi
cd .bin

if [ "$1" == "sieve-of-Eratosthenes" ] || [ "$1" == "sieve-of-Sundaram" ] || [ "$1" == "sieve-of-Sundaram-cluster" ] ; then
    if [ "$3" ] && [ "$3" == "-D" ]; then
        mpicc -c ../src/mpi/$1.c -lm -g -fdiagnostics-color=always -D DEBUG
        mpicc -o program.out "$1".o ./lib/*.o -lm -g
    else
        mpicc -c ../src/mpi/$1.c -lm
        mpicc -o program.out "$1".o ./lib/*.o -lm
    fi
else
    mpicc -o program.out ../src/mpi/$1.c -lm
fi

N=$(($2 > 1 ? $2 : 2))

if [ -f program.out ]; then
    if [ "$3" ] && [ "$3" == "-D" ]; then
        mpiexec -n $N ./program.out &
        sleep 1 # Waits before attach
        PIDS=$(pgrep program)
        PORT=1234
        # Attach to debugger server
        for pid in ${PIDS[@]}; do
            gdbserver --attach :$PORT "$pid" &
            PORT=$((PORT + 1))
        done
        wait $!
    else
        if [ ! -d log ]; then
            mkdir log
        fi
        for f in log/.log* log/log* ; do
            rm "$f"
        done
        if [ "$4" ] && [ "$3" == "-n" ]; then
            mpirun -n 2 --output file=log/log ./program.out -n $4
        else
            mpiexec -n 2 --output file=log/log ./program.out
        fi
        #mpiexec -n 2 ./program.out -l -prepend-rank -ordered-output # mpihc only
    fi
    rm program.out
fi
