#!/bin/bash

g++ -std=c++11 -O3 -o mapmaker mapmaker2.cpp
g++ -std=c++11 -O3 -o ma ma.cpp
g++ -std=c++11 -O3 -o ma_pr ma_prioritized.cpp

for doors in 4
do
  for agents in 4
  do
    for mapsize in 4
    do
      ./mapmaker ${doors} ${agents} ${mapsize} > genmap.txt
      ./ma < genmap.txt
      mv stats.csv stats/voa_${doors}_${agents}_${mapsize}.csv
      ./ma_pr < genmap.txt
      mv stats.csv stats/greedy_${doors}_${agents}_${mapsize}.csv
    done
  done
done
