#include <iostream>
#include <cstdlib>
#include <string>
#include <vector>
#include <random>

using namespace std;
random_device rd;
mt19937 gen(rd());

string doorIdToLoSymbol = "abcdefghijklmnopqrstuvwxyz$%^&';";
string doorIdToHiSymbol = "ABCDEFGHIJKLMNOPQRSTUVWXYZ*()_+=";
string doorIdToType     = "OOOOOOOOOOOOOOOOCCCCCCCCCCCCCCCC";
int numDoors, numAgents, mapSize;
vector<vector<pair<int,char> > > actions;

char& getCell(vector<vector<char> >& grid, int linearPos, int offset = 0)
{
    int C = grid[0].size();
    int bigrow = linearPos / (C-2);
    int row = (mapSize+1) * bigrow + offset;
    int col;
    if (bigrow % 2 == 0)
        col = 1 + linearPos % (C-2);
    else
        col = C-2 - linearPos % (C-2);
    return grid[row][col];
}

int main(int argc, char* argv[])
{
    if (argc != 4)
    {
        cout << "Usage: ./mapmaker <numDoors> <numAgents> <mapSize>" << endl;
        exit(-1);
    }
    numDoors = atoi(argv[1]);
    numAgents = atoi(argv[2]);
    mapSize = atoi(argv[3]);
    cout << doorIdToLoSymbol << endl;
    cout << doorIdToHiSymbol << endl;
    cout << doorIdToType << endl;
    cout << numAgents << endl;
    actions.resize(numAgents);
    
    int openN = numDoors / 2;
    int closeN = 16 + numDoors - openN;
    int openLo = 0, openHi = 0, closeLo = 16, closeHi = 16, t = 0;
    uniform_int_distribution<int> agent_dis(0, numAgents-1);
    uniform_int_distribution<int> action_dis(0, 3);
    uniform_int_distribution<int> move_dis(1, 2*mapSize);
    while (openHi < openN || closeHi < closeN)
    {
        int a = agent_dis(gen);
        switch (action_dis(gen))
        {
            case 0:
                if (openLo < openN)
                {
                    t += move_dis(gen);
                    pair<int,char> entry = make_pair(t, doorIdToLoSymbol[openLo++]);
                    actions[a].push_back(entry);
                }
                break;
            case 1:
                if (openHi < openLo)
                {
                    t += move_dis(gen);
                    pair<int,char> entry = make_pair(t, doorIdToHiSymbol[openHi++]);
                    actions[a].push_back(entry);
                }
                break;
            case 2:
                if (closeLo < closeN)
                {
                    t += move_dis(gen);
                    pair<int,char> entry = make_pair(t, doorIdToLoSymbol[closeLo++]);
                    actions[a].push_back(entry);
                }
                break;
            case 3:
                if (closeHi < closeLo)
                {
                    t += move_dis(gen);
                    pair<int,char> entry = make_pair(t, doorIdToHiSymbol[closeHi++]);
                    actions[a].push_back(entry);
                }
                break;
        }
    }
    t += move_dis(gen);
    for (int a = 0; a < numAgents; ++a)
    {
        int C = 60;
        int R = (mapSize+1) * (1 + t / (C-2)) - 1;
        vector<vector<char> > grid(R, vector<char>(C, '.'));
        for (int i = mapSize; i < R; i += mapSize+1)
        {
            for (int j = 1; j < C-1; ++j)
                grid[i][j] = '#';
            if (i % (2*mapSize+2) == mapSize)
                grid[i][0] = '#';
            else
                grid[i][C-1] = '#';
        }
        getCell(grid, 0) = '!';
        getCell(grid, t) = '@';
        for (pair<int,char> entry : actions[a])
        {
            for (int i = 0; i < mapSize; ++i)
                getCell(grid, entry.first, i) = entry.second;
        }
        cout << R << ' ' << C << endl;
        for (int i = 0; i < R; ++i)
        {
            for (int j = 0; j < C; ++j)
                cout << grid[i][j];
            cout << endl;
        }
        cout << endl;
    }
}
