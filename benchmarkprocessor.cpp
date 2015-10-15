#include <iostream>
#include <cstdlib>
#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <array>
#include <queue>
#include <unordered_map>

using namespace std;
typedef pair<int,int> pii;

constexpr int DIRS = 8;
constexpr int INFINITE = 100000000;
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<int> lucky_dis(0, 199);
uniform_int_distribution<int> coin_dis(0, 1);

struct PositionInfo
{
    int startD;
    int minD;
    int number;
    char value;
    PositionInfo() : startD(INFINITE), minD(-1), number(-2), value('@') {}
};

string bla;
string doorIdToLoSymbol = "abcdefghijklmnopqrstuvwxyz-=[];/";
string doorIdToHiSymbol = "ABCDEFGHIJKLMNOPQRSTUVWXYZ_+{}:?";
string doorIdToType     = "OOOOOOOOOOOOOOOOCCCCCCCCCCCCCCCC";
int numDoors, numAgents, R, C;
array<int,DIRS> dx = {1,1,-1,-1,1,0,-1,0};
array<int,DIRS> dy = {1,-1,1,-1,0,1,0,-1};
vector<vector<pair<int,char> > > actions;
vector<vector<PositionInfo> > grid;
vector<vector<char> > raw_grid;
pii start, goal;
map<int,int> numCount;

PositionInfo& getCell(pii coords)
{
    return grid[coords.first][coords.second];
}

pii tryMove(pii coords, int dir)
{
    int x = coords.first + dx[dir];
    int y = coords.second + dy[dir];
    if (0 <= x && x < R && 0 <= y && y < C && getCell(pii(x,y)).value != '@')
        return pii(x, y);
    else
        return coords;
}

pii randomCell()
{
    uniform_int_distribution<int> rand_row(0, R-1);
    uniform_int_distribution<int> rand_col(0, C-1);
    while (true)
    {
        pii pos(rand_row(gen), rand_col(gen));
        if (getCell(pos).value == '.')
            return pos;
    }
}

void readMap()
{
    cin >> bla >> bla >> bla >> R >> bla >> C >> bla;
    raw_grid = vector<vector<char> >(R, vector<char>(C));
    for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
    {
        char& val = raw_grid[i][j];
        cin >> val;
        if (val == 'G' || val == 'S')
            val = '.';
        if (val == 'O' || val == 'T' || val == 'W')
            val = '@';
        assert(val == '.' || val == '@');
    }
}

void processMap()
{
    for (int i = 0; i < R; ++i)
    for (int j = 0; j < C; ++j)
        grid[i][j].value = raw_grid[i][j];
    start = randomCell();
    getCell(start).value = '!';
    goal = randomCell();
    getCell(goal).value = '#';
    
    queue<pii> q, q2;
    getCell(start).startD = 0;
    q.push(start);
    while (!q.empty())
    {
        pii cur = q.front();
        q.pop();
        int nextD = getCell(cur).startD + 1;
        for (int d = 0; d < DIRS; ++d)
        {
            pii next = tryMove(cur, d);
            if (getCell(next).startD == INFINITE)
            {
                getCell(next).startD = nextD;
                q.push(next);
            }
        }
    }
    numCount.clear();
    int D = getCell(goal).minD = getCell(goal).startD;
    q.push(goal);
    if (D == INFINITE)
        return;
    for (; D >= 0; --D)
    {
        while (!q.empty())
        {
            pii cur = q.front();
            q.pop();
            if (getCell(cur).startD < getCell(cur).minD) // D is always the smaller of the two
            {
                ++numCount[getCell(cur).startD];
                getCell(cur).number = getCell(cur).startD;
            }
            for (int d = 0; d < DIRS; ++d)
            {
                pii next = tryMove(cur, d);
                if (getCell(next).minD == -1)
                {
                    getCell(next).minD = D;
                    if (getCell(next).startD >= D)
                        q.push(next);
                    else
                        q2.push(next);
                }
            }
        }
        swap(q, q2);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cout << "Usage: ./benchmarkprocessor <numDoors> <numAgents> < [benchmark file]" << endl;
        exit(-1);
    }
    numDoors = atoi(argv[1]);
    numAgents = atoi(argv[2]);
    readMap();
    cout << doorIdToLoSymbol << endl;
    cout << doorIdToHiSymbol << endl;
    cout << doorIdToType << endl;
    cout << numAgents << endl;
    
    actions.resize(numAgents);
    int openEnd = numDoors / 2;
    int closeEnd = 16 + numDoors - openEnd;
    int openLo = 0, openHi = 0, closeLo = 16, closeHi = 16;
    vector<int> t(numAgents, 1);
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
                    pair<int,char> entry = make_pair(t[a]++, doorIdToLoSymbol[openLo++]);
                    actions[a].push_back(entry);
                }
                break;
            case 1:
                if (openHi < openLo)
                {
                    pair<int,char> entry = make_pair(t[a]++, doorIdToHiSymbol[openHi++]);
                    actions[a].push_back(entry);
                }
                break;
            case 2:
                if (closeLo < closeEnd)
                {
                    pair<int,char> entry = make_pair(t[a]++, doorIdToLoSymbol[closeLo++]);
                    actions[a].push_back(entry);
                }
                break;
            case 3:
                if (closeHi < closeLo)
                {
                    pair<int,char> entry = make_pair(t[a]++, doorIdToHiSymbol[closeHi++]);
                    actions[a].push_back(entry);
                }
                break;
        }
    }
    for (int a = 0; a < numAgents; ++a)
    {
        do
        {
            grid = vector<vector<PositionInfo> >(R, vector<PositionInfo>(C));
            processMap();
        }
        while (getCell(goal).startD < 2*t[a]-1 || getCell(goal).startD == INFINITE);
        
        vector<pii> cnts;
        for (int i = 1; i+1 < getCell(goal).startD; ++i)
        {
            int relativeSize = numCount[i] + numCount[i+1] - numCount[i-1] - numCount[i+2];
            cnts.emplace_back(100*relativeSize + lucky_dis(gen), i);
            ++i;
        }
        sort(cnts.begin(), cnts.end());
        vector<int> keyNums;
        
        for (int i = 0; i < t[a]-1; ++i)
        {
            keyNums.push_back(cnts[i].second);
        }
        sort(keyNums.begin(), keyNums.end());
        
        unordered_map<int, char> numberToValue;
        //numberToValue[0] = '!';
        //numberToValue[getCell(goal).startD] = '#';
        for (int i = 0; i < t[a]-1; ++i)
        {
            numberToValue[keyNums[i]] = actions[a][i].second;
            numberToValue[keyNums[i]+1] = actions[a][i].second;
        }
        
        for (int i = 0; i < R; ++i)
        for (int j = 0; j < C; ++j)
        {
            int lucky = lucky_dis(gen);
            //if (grid[i][j].number >= 0)
            //    grid[i][j].value = '.';
            //if (grid[i][j].main)
            //{
                if (numberToValue.find(grid[i][j].number) != numberToValue.end())
                    grid[i][j].value = numberToValue[grid[i][j].number];
            //}
            /*else if ((0 <= lucky && lucky < openEnd) || (16 <= lucky && lucky < closeEnd))
            {
                if (coin_dis(gen))
                    grid[i][j].value = doorIdToLoSymbol[lucky];
                else
                    grid[i][j].value = doorIdToHiSymbol[lucky];
            }*/
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
