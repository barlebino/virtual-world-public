#!/bin/sh

cd gl/glew
unzip glew-2.1.0.zip
rm glew-2.1.0.zip
cd glew-2.1.0
make
cd ../../glfw
unzip glfw-3.2.1.zip
rm glfw-3.2.1.zip
cd glfw-3.2.1
/Applications/nondefaults/programming/cmake.app/Contents/bin/cmake CMakeLists.txt
cd ../../glm
unzip glm-0.9.8.1.zip
rm glm-0.9.8.1.zip
cd ../..
mkdir build
cd build
/Applications/nondefaults/programming/cmake.app/Contents/bin/cmake ..
make
./lab5 ../resources
cd ..
rm -rf build

