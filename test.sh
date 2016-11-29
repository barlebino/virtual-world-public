#!/bin/sh

mkdir build
cd build
cmake ..
make
./lab5 ../resources
cd ..
rm -rf build

