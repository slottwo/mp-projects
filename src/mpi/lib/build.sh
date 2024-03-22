#!/bin/bash

if [ ! -d .bin ]; then
    mkdir .bin
fi
if [ ! -d .bin/lib ]; then
    mkdir .bin/lib
fi

for file in "$(dirname $0)"/*.c
do
    input="${file%.*}"
    mpicc -c "$input".c
    out=$(basename "$input").o
    mv "$out" .bin/lib
done

#for c in *.c; do
#    if [ "$c" == "benchmark.c" ]; then
#        continue
#    fi
#    gcc -c "$c"
#done
