#!/bin/bash

# segfaults under -O3 for some reason :/

g++ -std=c++11 -o mapmaker mapmaker.cpp
g++ -std=c++11 -o ma ma.cpp

./mapmaker 32 8 8 > genmap.txt
./ma < genmap.txt
