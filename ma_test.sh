#!/bin/bash

g++ -std=c++11 -O3 -o mapmaker mapmaker2.cpp
g++ -std=c++11 -O3 -o ma ma.cpp

./mapmaker 8 8 8 > genmap.txt
./ma < genmap.txt
