#!/bin/bash

g++ -std=c++11 -O3 -o mapmaker mapmaker2.cpp
g++ -std=c++11 -O3 -o ma ma.cpp
g++ -std=c++11 -O3 -o ma_pr ma_prioritized.cpp

for doors in 6 8 10 12 14 16
do
  for agents in 6 8 10 12 14 16
  do
    for rows in 5 15 25 35 55 75
    do
      let a=
      $controls = 0
      if [ "$doors" == "8" ]
      then
        controls += 1
      fi
      if [ "$agents" == "8" ]
      then
        controls += 1
      fi
      if [ "$rows" == "25" ]
      then
        controls += 1
      fi
      if [ "$controls" >= "2" ]
      then
        echo "$doors $agents $rows"
        for inst in {1..50}
        do
          ./mapmaker ${doors} ${agents} ${rows} ${rows} > stats/map_${doors}_${agents}_${rows}_${inst}.txt
          ./ma < stats/map_${doors}_${agents}_${rows}_${inst}.txt > junk.txt
          mv stats.csv stats/complete_${doors}_${agents}_${rows}_${inst}.csv
          ./ma_pr < stats/map_${doors}_${agents}_${rows}_${inst}.txt > junk.txt
          mv stats.csv stats/greedy_${doors}_${agents}_${rows}_${inst}.csv
        done
      fi
    done
  done
done
