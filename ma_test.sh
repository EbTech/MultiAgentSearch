#!/bin/bash

g++ -std=c++11 -O3 -o mapmaker mapmaker2.cpp
g++ -std=c++11 -O3 -o ma ma.cpp
g++ -std=c++11 -O3 -o ma_pr ma_prioritized.cpp

for doors in 1
do
  for agents in 1
  do
    for rows in 1
    do
      for inst in 11
      do
        ./mapmaker ${doors} ${agents} ${rows} ${rows} > stats/map_${doors}_${agents}_${rows}_${inst}.txt
        ./ma < stats/map_${doors}_${agents}_${rows}_${inst}.txt
        mv stats.csv stats/complete_${doors}_${agents}_${rows}_${inst}.csv
        ./ma_pr < stats/map_${doors}_${agents}_${rows}_${inst}.txt
        mv stats.csv stats/greedy_${doors}_${agents}_${rows}_${inst}.csv
      done
    done
  done
done
