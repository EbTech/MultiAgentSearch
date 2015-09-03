#!/bin/bash

g++ -std=c++11 -O3 -o mapmaker mapmaker.cpp
g++ -std=c++11 -O3 -o ma ma.cpp

./mapmaker 32 8 8 > genmap.txt
./ma < genmap.txt
