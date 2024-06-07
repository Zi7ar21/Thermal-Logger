#!/bin/sh

CXX=clang++
#CXX=g++

mkdir -p build

${CXX} -march=native -O2 -pedantic -std=c++17 -Wall -Wextra -Wshadow -o build/thermal-logger src/main.cpp

#/bin/time -v ./build/thermal-logger
time ./build/thermal-logger
