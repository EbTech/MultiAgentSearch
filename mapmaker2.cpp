#include <iostream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include <array>
#include <unordered_map>

using namespace std;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> lucky_dis(0, 199);
uniform_int_distribution<int> coin_dis(0, 1);

struct PositionInfo
{
    int number;
    int children;
    bool main;
    char value;
    vector<int> dirs;
    PositionInfo() : number(-2), children(0), main(false), value('#') {}
};

string doorIdToLoSymbol = "abcdefghijklmnopqrstuvwxyz-=[];/";
string doorIdToHiSymbol = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_+{}:?";
string doorIdToType     = "OOOOOOOOOOOOOOOOCCCCCCCCCCCCCCCC";
int numDoors, numAgents, R, C, goalNum, cellsToExplore;
array<int,4> dx = {1, 0, -1, 0};
array<int,4> dy = {0, 1, 0, -1};
vector<vector<pair<int,char> > > actions;
vector<vector<PositionInfo> > grid;

void dfsCarve(int i, int j)
{
    --cellsToExplore;
    vector<int> dirs;
    for (int d = 0; d < 4; ++d)
        dirs.push_back(d);
    shuffle(dirs.begin(), dirs.end(), gen);
    for (int d : dirs)
    {
        int i1 = i + dx[d];
        int j1 = j + dy[d];
        int i2 = i1 + dx[d];
        int j2 = j1 + dy[d];
        if (0 <= i2 && i2 < R && 0 <= j2 && j2 < C && grid[i2][j2].number == -2)
        {
            grid[i][j].dirs.push_back(d);
            grid[i1][j1].number = grid[i2][j2].number = -1;
            grid[i1][j1].children = cellsToExplore;
            dfsCarve(i2, j2);
            grid[i1][j1].children -= cellsToExplore;
        }
    }
}

void dfsLabel(int i, int j)
{
    bool isMain = grid[i][j].main;
    int total = 0;
    for (int d : grid[i][j].dirs)
    {
        int i1 = i + dx[d];
        int j1 = j + dy[d];
        total += grid[i1][j1].children;
    }
    if (total == 0) return;
    uniform_int_distribution<int> child_dis(0, total-1);
    int mainChild = child_dis(gen);
    
    for (int d : grid[i][j].dirs)
    {
        int i1 = i + dx[d];
        int j1 = j + dy[d];
        int i2 = i1 + dx[d];
        int j2 = j1 + dy[d];
        grid[i1][j1].main = grid[i2][j2].main = isMain;
        if (isMain)
        {
            grid[i1][j1].number = ++goalNum;
            grid[i2][j2].number = ++goalNum;
            dfsLabel(i2, j2);
            mainChild -= grid[i1][j1].children;
            if (mainChild < 0)
                isMain = false;
        }
        else
        {
            grid[i1][j1].number = goalNum;
            grid[i2][j2].number = goalNum;
            dfsLabel(i2, j2);
        }
    }
    assert(!isMain);
}

int main(int argc, char* argv[])
{
    if (argc != 5)
    {
        cout << "Usage: ./mapmaker <numDoors> <numAgents> <rows> <cols>" << endl;
        exit(-1);
    }
    numDoors = atoi(argv[1]);
    numAgents = atoi(argv[2]);
    R = atoi(argv[3]);
    C = atoi(argv[4]);
    cout << doorIdToLoSymbol << endl;
    cout << doorIdToHiSymbol << endl;
    cout << doorIdToType << endl;
    cout << numAgents << endl;
    
    actions.resize(numAgents);
    int openEnd = numDoors / 2;
    int closeEnd = 16 + numDoors - openEnd;
    int openLo = 0, openHi = 0, closeLo = 16, closeHi = 16, t = 0;
    uniform_int_distribution<int> agent_dis(0, numAgents-1);
    uniform_int_distribution<int> action_dis(0, 3);
    while (openHi < openEnd || closeHi < closeEnd)
    {
        int a = agent_dis(gen);
        switch (action_dis(gen))
        {
            case 0:
                if (openLo < openEnd)
                {
                    ++t;
                    pair<int,char> entry = make_pair(t, doorIdToLoSymbol[openLo++]);
                    actions[a].push_back(entry);
                }
                break;
            case 1:
                if (openHi < openLo)
                {
                    ++t;
                    pair<int,char> entry = make_pair(t, doorIdToHiSymbol[openHi++]);
                    actions[a].push_back(entry);
                }
                break;
            case 2:
                if (closeLo < closeEnd)
                {
                    ++t;
                    pair<int,char> entry = make_pair(t, doorIdToLoSymbol[closeLo++]);
                    actions[a].push_back(entry);
                }
                break;
            case 3:
                if (closeHi < closeLo)
                {
                    ++t;
                    pair<int,char> entry = make_pair(t, doorIdToHiSymbol[closeHi++]);
                    actions[a].push_back(entry);
                }
                break;
        }
    }
    ++t;
    uniform_int_distribution<int> row_dis(0, (R-1)/2);
    uniform_int_distribution<int> col_dis(0, (C-1)/2);
    for (int a = 0; a < numAgents; ++a)
    {
        do
        {
            grid = vector<vector<PositionInfo> >(R, vector<PositionInfo>(C));
            int startx = 2*row_dis(gen);
            int starty = 2*col_dis(gen);
            grid[startx][starty].number = 0;
            grid[startx][starty].main = true;
            goalNum = 0;
            cellsToExplore = ((R+1)/2)*((C+1)/2);
            dfsCarve(startx, starty);
            dfsLabel(startx, starty);
            assert(cellsToExplore == 0);
        }
        while (goalNum < t);
        
        unordered_map<int, char> numberToValue;
        numberToValue[0] = '!';
        numberToValue[goalNum] = '@';
        for (pair<int,char>& entry : actions[a])
        {
            numberToValue[entry.first * goalNum / t] = entry.second;
        }
        for (int i = 0; i < R; ++i)
        for (int j = 0; j < C; ++j)
        {
            int lucky = lucky_dis(gen);
            if (grid[i][j].number >= 0)
                grid[i][j].value = '.';
            if (grid[i][j].main)
            {
                if (numberToValue.find(grid[i][j].number) != numberToValue.end())
                    grid[i][j].value = numberToValue[grid[i][j].number];
            }
            else if ((0 <= lucky && lucky < openEnd) || (16 <= lucky && lucky < closeEnd))
            {
                if (coin_dis(gen))
                    grid[i][j].value = doorIdToLoSymbol[lucky];
                else
                    grid[i][j].value = doorIdToHiSymbol[lucky];
            }
        }
        cout << R << ' ' << C << endl;
        for (int i = 0; i < R; ++i)
        {
            for (int j = 0; j < C; ++j)
                cout << grid[i][j].value;
            cout << endl;
        }
    }
}
