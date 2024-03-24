#!/bin/bash

mkdir -p bin

gcc -fopenmp "$1" -o "${1%.c}" -lm

if [ -f "bin/$1" ]; then
    ./bin/"$1"
fi
