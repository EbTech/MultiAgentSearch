import os

os.system("g++ -std=c++11 -O3 -o mapmaker benchmarkprocessor.cpp")
os.system("g++ -std=c++11 -O3 -o ma ma.cpp")
os.system("g++ -std=c++11 -O3 -o ma_pr ma_prioritized.cpp")

for doors in [6, 8, 10, 12, 14, 16]:
  for agents in [6, 8, 10, 12, 14, 16]:
      controls = (doors==8) + (agents==8)
      if controls >= 1:
        print(doors, agents)
        for inst in range(1, 2):
          print(inst)
          arglist = str(doors) + " " + str(agents) + " < ../map-read-only/sc1/FloodedPlains.map"
          suffix = "_" + str(doors) + "_" + str(agents) + "_" + "FloodedPlains" + "_" + str(inst)
          folder = "stats_bench/"
          os.system("./mapmaker " + arglist + " > " + folder + "map" + suffix + ".txt")
          os.system("./ma < " + folder + "map" + suffix + ".txt > junk.txt")
          os.system("mv stats.csv " + folder + "complete" + suffix + ".csv")
          os.system("./ma_pr < " + folder + "map" + suffix + ".txt > junk.txt")
          os.system("mv stats.csv " + folder + "greedy" + suffix + ".csv")
