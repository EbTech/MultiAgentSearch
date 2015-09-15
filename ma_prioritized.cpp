#include "ma_prioritized.h"
#include <array>
#include <cmath>
#include <chrono>
#include <stack>
#include <sstream>
#include <algorithm>

typedef chrono::high_resolution_clock Clock;

constexpr int DIRS = 8;
constexpr int W = 1;
array<int,DIRS> dx = {1,1,-1,-1,1,0,-1,0};
array<int,DIRS> dy = {1,-1,1,-1,0,1,0,-1};
array<Cost,DIRS> dcost = {14142,14142,14142,14142,10000,10000,10000,10000};

int num_discovered;
int num_expands;

vector<Agent> agents;
ull takeMin[] = {0, 0};
Cost joint_cost;
vector<int> openerAgent;
vector<int> openerPos;
int numDoors;

void readMap()
{
    string doorIdToLoSymbol;
    string doorIdToHiSymbol;
    string doorIdToType;
    unordered_map<char,pair<ull,bool> > doorSymbolToId;
    int numAgents;//, numDoors;
    
    cin >> doorIdToLoSymbol >> doorIdToHiSymbol >> doorIdToType;
    numDoors = doorIdToType.length();
    assert(numDoors <= 64);
    assert(numDoors == doorIdToLoSymbol.length());
    assert(numDoors == doorIdToHiSymbol.length());
    //doorSymbolToId.clear();
    for (int i = 0; i < numDoors; ++i)
    {
        ull doorMask = 1ull << i;
        assert(doorSymbolToId.find(doorIdToLoSymbol[i]) == doorSymbolToId.end());
        assert(doorSymbolToId.find(doorIdToHiSymbol[i]) == doorSymbolToId.end());
        assert(doorIdToType[i] == 'O' || doorIdToType[i] == 'C');
        doorSymbolToId[doorIdToLoSymbol[i]] = make_pair(doorMask, false);
        doorSymbolToId[doorIdToHiSymbol[i]] = make_pair(doorMask, true);
        if (doorIdToType[i] == 'O')
            takeMin[0] |= doorMask;
        takeMin[1] |= doorMask;
    }
    cin >> numAgents;
    agents.resize(numAgents);
    for (Agent& agent : agents)
    {
        cin >> agent.numRows >> agent.numCols;
        agent.goalPos = agent.start.pos = nullptr;
        agent.raw_grid.resize(agent.numRows);
        agent.grid.resize(agent.numRows);
        agent.hDist.clear();
        agent.canOpen = 0;
        for (int i = 0; i < agent.numRows; ++i)
        {
            agent.grid[i].resize(agent.numCols);
            cin >> agent.raw_grid[i];
            assert(agent.raw_grid[i].length() == agent.numCols);
            for (int j = 0; j < agent.numCols; ++j)
            {
                PositionNode* pos = &agent.grid[i][j];
                agent.hDist[pos] = INFINITE;
                pos->x = i;
                pos->y = j;
                pos->obst = false;
                pos->mask = 0;
                switch (agent.raw_grid[i][j])
                {
                    case '.':
                        break;
                    case '!':
                        assert(agent.start.pos == nullptr);
                        agent.start.pos = pos;
                        break;
                    case '@':
                        assert(agent.goalPos == nullptr);
                        agent.goalPos = pos;
                        break;
                    case '#':
                        pos->obst = true;
                        break;
                    default:
                        assert(doorSymbolToId.find(agent.raw_grid[i][j]) != doorSymbolToId.end());
                        tie(pos->mask, pos->late) = doorSymbolToId[agent.raw_grid[i][j]];
                        if (!pos->late)
                            agent.canOpen |= pos->mask & takeMin[0];
                }
            }
        }
        assert(agent.goalPos != nullptr && agent.start.pos != nullptr);
        assert(agent.goalPos->mask == 0 && agent.start.pos->mask == 0);
        // compute relaxed distance estimates for the heuristic
        unordered_set<PositionNode*> visited;
        queue<PositionNode*> Q;
        agent.hDist[agent.goalPos] = 0;
        Q.push(agent.goalPos);
        visited.insert(agent.goalPos);
        while (!Q.empty())
        {
            PositionNode* curPos = Q.front(); Q.pop();
            visited.insert(curPos);
            for (int d = 0; d < DIRS; ++d)
            {
                int xx = curPos->x + dx[d];
                int yy = curPos->y + dy[d];
                if (0 <= xx && xx < agent.numRows && 0 <= yy && yy < agent.numCols)
                {
                    PositionNode* nextPos = &agent.grid[xx][yy];
                    if (!nextPos->obst && visited.find(nextPos) == visited.end())
                    {
                        agent.hDist[nextPos] = min(agent.hDist[nextPos], agent.hDist[curPos] + dcost[d]);
                        Q.push(nextPos);
                        visited.insert(nextPos);
                    }
                }
            }
        }
    }
}

inline Cost h(const PositionNode* const s1, const PositionNode* const s2)
{
    double dx = s1->x - s2->x;
    double dy = s1->y - s2->y;
    return sqrt(dx*dx+dy*dy)*10000;
}

inline Cost f(const State& s, Agent& agent)
{
    return s.getData().g + W*agent.hDist[s.pos];
}

/*inline Cost f(const State& s, const PositionNode* const focus)
{
    return s.getData().g + W*h(s.pos, focus);
}*/

void Agent::insert(const State& s)
{
    auto& iter = s.getData().iter;
    if (iter != open.cend())
    open.erase(iter);
    iter = open.emplace(f(s, *this), s);
}

State Agent::remove()
{
    State s = open.cbegin()->second;
    auto& iter = s.getData().iter;
    open.erase(iter);
    iter = open.cend();
    return s;
}
/*STUFF FOR PRINTING THE QUEUE
    cout << "  queue={";
    for(multimap<int,State*>::iterator it=m.begin(); it!=m.end(); it++)
        cout << it->second << "|" << it->first << " ";
    cout << endl;
*/

bool testSolution(bool print)
{
    // cout << "testing candidate solution << endl;
    vector<int> going(agents.size(), 0);
    vector<int> cur(agents.size(), -1);
    multimap<Cost, int> events;
    for (int i = 0; i < agents.size(); ++i)
    {
        events.emplace(0, i);
    }
    Cost curTime = 0;
    ull trigger = 0;
    while (!events.empty())
    {
        auto it = events.cbegin();
        curTime = it->first;
        int id = it->second;
        events.erase(it);
        cur[id] = going[id];
        PositionNode* pos = agents[id].solution[cur[id]];
        if (print)
        {
            cout << "Agent " << id << " reaches " << agents[id].raw_grid[pos->x][pos->y];
            cout << " at " << *pos << " at time = " << curTime/10000.0 << endl;
        }
        if (cur[id] != agents[id].solution.size()-1 && !pos->late)
        {
            ++going[id];
            events.emplace(curTime + h(pos, agents[id].solution[going[id]]), id);
            if (print)
            {
                cout << "Agent " << id << "  leaves " << agents[id].raw_grid[pos->x][pos->y];
                cout << " at " << *pos << " at time = " << curTime/10000.0 << endl;
            }
            trigger |= pos->mask & takeMin[0];
            ull safeToClose = ~takeMin[0];
            for (int a = 0; a < agents.size(); ++a)
            for (int j = 0; j < numDoors; ++j)
            if (going[a] <= agents[a].closeUsePos[j])
                safeToClose &= ~(1ull << j);
            trigger |= safeToClose;
        }
        for (int i = 0; i < agents.size(); ++i)
        {
            pos = agents[i].solution[cur[i]];
            if (cur[i] == going[i] && cur[i] != agents[i].solution.size()-1 && !(pos->mask & ~trigger))
            {
                ++going[i];
                events.emplace(curTime + h(pos, agents[i].solution[going[i]]), i);
                if (print)
                {
                    cout << "Agent " << i << "  leaves " << agents[i].raw_grid[pos->x][pos->y];
                    cout << " at " << *pos << " at time = " << curTime/10000.0 << endl;
                }
            }
        }
    }
    for (int i = 0; i < agents.size(); ++i)
    {
        if (cur[i] != agents[i].solution.size()-1)
            return false;
    }
    if (joint_cost > curTime)
    {
        joint_cost = curTime;
    }
    return true;
}

HistoryNode* transition(HistoryNode* hist, PositionNode* pos)
{
    // C-door uses, which are the only takeMax, need not be remembered
    ull posMins = pos->mask & takeMin[pos->late];
    return hist->getChild(posMins, pos->late);
}

bool resolve(PositionNode* pos, HistoryNode*& hist);
// move agents[agentID] at least as far forward as solPos
bool resolve(int agentID, int solPos, HistoryNode*& hist)
{
    if (agents[agentID].stuck && hist->jointProgress[agentID] < solPos)
        return false;
    agents[agentID].stuck = true;
    while (hist->jointProgress[agentID] < solPos)
    {
        PositionNode* pos = agents[agentID].solution[++hist->jointProgress[agentID]];
        if (!resolve(pos, hist))
        {
            agents[agentID].stuck = false;
            return false;
        }
    }
    agents[agentID].stuck = false;
    return true;
}
// append pos to hist, but only after successfully traversing pos
bool resolve(PositionNode* pos, HistoryNode*& hist)
{
    // we must not enter a closed door
    if (!pos->late && (pos->mask & ~takeMin[0] & hist->mask[1]))
        return false;
    if (pos->late)
    for (int i = 0; i < numDoors; ++i)
    {
        // if we're about to close a door, let everyone through first
        if ((1ull<<i) & pos->mask & ~takeMin[0])
        for (int a = 0; a < agents.size(); ++a)
            if (!resolve(a, agents[a].closeUsePos[i], hist))
                return false;
        // if we're about to enter a door, open it first
        if ((1ull<<i) & pos->mask & takeMin[0] & ~hist->mask[0])
            if (openerAgent[i] == -1 || !resolve(openerAgent[i], openerPos[i], hist))
                return false;
    }
    hist = transition(hist, pos);
    return true;
}

void Agent::expand(State& s)
{
    StateData& s_data = s.getData();
    assert(!s_data.closed);
    s_data.closed = true;
    for (int d = 0; d < DIRS; ++d)
    {
        int newX = s.pos->x + dx[d];
        int newY = s.pos->y + dy[d];
        // cannot escape the grid boundaries
        if (newX<0 || newX==numRows || newY<0 || newY==numCols)
            continue;
        State t;
        t.pos = &grid[newX][newY];
        // cannot pass through obstacles
        if (t.pos->obst)
            continue;
        // try to move all the agents jointly in a consistent manner
        t.hist = s.hist;
        if (!resolve(t.pos, t.hist))
            continue;
        // critical section for updating g-value and inserting into the heap
        StateData& t_data = t.getData();
        if (t_data.g > s_data.g + dcost[d])
        {
            if (t_data.g == INFINITE)
            {
                num_discovered++;
                t_data.iter = open.cend();
            }
            t_data.g = s_data.g + dcost[d];
            t_data.bp = s;
            if (!t_data.closed)
                insert(t);
        }
        //cout << "done adding successor" << endl;
    }
}

void prepare()
{
    num_expands = 0;
    num_discovered = 1;
    openerPos = openerAgent = vector<int>(numDoors, -1);
    for (Agent& agent : agents)
    {
        agent.open.clear();
        agent.solution.clear();
        agent.stuck = false;
        agent.closeUsePos = vector<int>(numDoors, -1);
        agent.historyRoot.reset(new HistoryNode);
        agent.historyRoot->parent = nullptr;
        agent.historyRoot->mask[0] = agent.historyRoot->mask[1] = 0;
        agent.historyRoot->jointProgress = vector<int>(agents.size(), 0);
        //agent.historyRoot->mask[agent.start->late] = agent.start->mask;
        agent.start.hist = agent.historyRoot.get();
        StateData& start_data = agent.start.getData();
        start_data.g = 0;
        start_data.bp = agent.start;
        start_data.iter = agent.open.cend();
        agent.insert(agent.start);
    }
}

void search()
{
    joint_cost = INFINITE;
    for (int a = 0; a < agents.size(); ++a)
    {
        Agent& agent = agents[a];
        for (int i = a+1; i < agents.size(); ++i)
        {
            // optimistically open doors using the remaining agents
            agent.historyRoot->mask[0] |= agents[i].canOpen;
        }
        while (!agent.open.empty())
        {
            // get a State to expand
            State s = agent.remove();
            num_expands++;
            if (s.pos == agent.goalPos)
            {
                bool done = true;
                HistoryNode* goalHist = s.hist;
                for (int i = 0; i < a; ++i)
                {
                    done &= resolve(i, agents[i].solution.size()-1, goalHist);
                }
                if (done)
                {
                    while (s != agent.start)
                    {
                        agent.solution.push_back(s.pos);
                        s = s.getData().bp;
                    }
                    agent.solution.push_back(s.pos);
                    reverse(agent.solution.begin(), agent.solution.end());
                    for (int i = 0; i < agent.solution.size(); ++i)
                    {
                        PositionNode* pos = agent.solution[i];
                        if (!pos->late)
                        for (int j = 0; j < numDoors; ++j)
                        {
                            if ((1ull<<j) & pos->mask & ~takeMin[0])
                                agent.closeUsePos[j] = i;
                            if ((1ull<<j) & pos->mask & takeMin[0])
                            if (openerAgent[j] == -1 || openerPos[j] > i)
                                openerAgent[j] = a, openerPos[j] = i;
                        }
                    }
                    break;
                }
            }
            //cout << "expanding " << s << endl;
            agent.expand(s);
        }
        if (agent.solution.empty())
        {
            cout << "Agent " << a << " FAILED to find a path consistent with its predecessors." << endl;
            break;
        }
    }
}

int main(int argc, char** argv)
{
    FILE* fout = fopen("stats.csv","w");
    //load map
    readMap();

    //run planner
    Clock::time_point t0 = Clock::now();
    prepare();
    search();
    Clock::time_point t1 = Clock::now();
    
    //report joint plan
    if (!agents.back().solution.empty())
    {
        cout << "ALL " << agents.size() << " agents found consistent plans!" << endl;
        assert(testSolution(true));
    }

    //report stats
    double dt =  chrono::duration<double, chrono::seconds::period>(t1-t0).count();
    cout << "results: Path Length=" << joint_cost/10000.0;
    cout << " Visited Nodes=" << num_discovered;
    cout << " Explored Nodes=" << num_expands;
    cout << " Planning Time=" << dt << endl;
    fprintf(fout,"%f %f %d %f\n",W,dt,num_expands,joint_cost/10000.0);

    fclose(fout);
}
