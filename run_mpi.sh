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

# Associates each file to be compiled correctly
COMPILER=""

case "$1" in
  "sieve-of-Eratosthenes" | "sieve-of-Sundaram" | "sieve-of-Sundaram-cluster" | "sfs" | "sun")
    COMPILER=mpicc
  ;;
  "sieve-of-Sundaram-bitfull")
    COMPILER=mpicxx
  ;;
  *)
    mpicc -o program.out ../src/mpi/$1.c -lm
  ;;
esac

if [ "$COMPILER" ] ; then
    if [ "$3" ] && [ "$3" == "-D" ] ; then
        if [ "$(cat /proc/sys/kernel/yama/ptrace_scope)" != "0" ] ; then
            >&2 printf "\33[33mMake sure you have gdb process attachment permitions.\n\33[m"
            >&2 read -p "Run echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope if not [y/N]" c
            case "$c" in
              "y"* | "Y"*)
                echo 0 | sudo tee /proc/sys/kernel/yama/ptrace_scope
              ;;
            esac
        fi

        $COMPILER -c ../src/mpi/$1.c -lm -g -fdiagnostics-color=always -D DEBUG
        $COMPILER -o program.out "$1".o ./lib/*.o -lm -g
    else
        $COMPILER -c ../src/mpi/$1.c -lm
        $COMPILER -o program.out "$1".o ./lib/*.o -lm
    fi
fi

# Run the compiled files in the specified mode
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
