#!/bin/bash

#g++ -std=c++11 -O3 -o mapmaker mapmaker2.cpp
#g++ -std=c++11 -O3 -o ma ma.cpp
#g++ -std=c++11 -O3 -o ma_pr ma_prioritized.cpp

for doors in 9
do
  for agents in 4
  do
    for rows in 9
    do
      for inst in 1
      do
        ./mapmaker ${doors} ${agents} ${rows} ${rows} > genmap.txt
        ./ma < genmap.txt
        mv stats.csv stats/complete_${doors}_${agents}_${rows}_${inst}.csv
        ./ma_pr < genmap.txt
        mv stats.csv stats/greedy_${doors}_${agents}_${rows}_${inst}.csv
      done
    done
  done
done
