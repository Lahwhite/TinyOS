#!/bin/zsh

set -e

cd build

if [ "$1" = "clean" ]; then
    make clean
    cmake ..
fi

make
./minios