#include "ma.h"
#include <array>
#include <cmath>
#include <chrono>
#include <stack>
#include <sstream>

typedef chrono::high_resolution_clock Clock;

constexpr int DIRS = 8;
constexpr int W = 1;
array<int,DIRS> dx = {1,1,-1,-1,1,0,-1,0};
array<int,DIRS> dy = {1,-1,1,-1,0,1,0,-1};
array<Cost,DIRS> dcost = {14142,14142,14142,14142,10000,10000,10000,10000};

int num_discovered;
int num_expands;
double test_duration;

vector<Agent> agents;
ull takeMin[] = {0, 0};
Cost joint_cost, joint_bound;
vector<State> joint_solution;

void readMap()
{
    string doorIdToLoSymbol;
    string doorIdToHiSymbol;
    string doorIdToType;
    unordered_map<char,pair<ull,bool> > doorSymbolToId;
    int numAgents, numDoors;
    
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
        agent.start = agent.goal.pos = nullptr;
        agent.raw_grid.resize(agent.numRows);
        agent.grid.resize(agent.numRows);
        for (int i = 0; i < agent.numRows; ++i)
        {
            agent.grid[i].resize(agent.numCols);
            cin >> agent.raw_grid[i];
            assert(agent.raw_grid[i].length() == agent.numCols);
            for (int j = 0; j < agent.numCols; ++j)
            {
                PositionNode* pos = &agent.grid[i][j];
                pos->x = i;
                pos->y = j;
                pos->obst = false;
                pos->mask = 0;
                switch (agent.raw_grid[i][j])
                {
                    case '.':
                        break;
                    case '!':
                        assert(agent.start == nullptr);
                        agent.start = pos;
                        break;
                    case '@':
                        assert(agent.goal.pos == nullptr);
                        agent.goal.pos = pos;
                        break;
                    case '#':
                        pos->obst = true;
                        break;
                    default:
                        assert(doorSymbolToId.find(agent.raw_grid[i][j]) != doorSymbolToId.end());
                        tie(pos->mask, pos->late) = doorSymbolToId[agent.raw_grid[i][j]];
                }
            }
        }
        assert(agent.start != nullptr && agent.goal.pos != nullptr);
        assert(agent.start->mask == 0 && agent.goal.pos->mask == 0);
    }
}

inline Cost h(const PositionNode* const s1, const PositionNode* const s2)
{
    double dx = s1->x - s2->x;
    double dy = s1->y - s2->y;
    return sqrt(dx*dx+dy*dy)*10000;
}

inline Cost f(const State& s, const PositionNode* const focus)
{
    return s.getData().g + W*h(s.pos, focus);
}

void Agent::insert(const State& s)
{
    auto& iter = s.getData().iter;
    if (iter != open.cend())
    open.erase(iter);
    iter = open.emplace(f(s, start), s);
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

void testSolution(const vector<State>& starts, bool print)
{
    // if the solution attempts to use a door that's never opened, prune it
    ull joint_mask[] = {0, 0};
    for (const State& s : starts)
    {
        joint_mask[0] |= s.hist->mask[0];
        joint_mask[1] |= s.hist->mask[1];
    }
    if (takeMin[1] & joint_mask[1] & ~joint_mask[0])
        return;
    
    // cout << "testing candidate solution << endl;
    vector<State> going = starts;
    vector<State> cur(starts.size());
    multimap<Cost, int> events;
    for (int i = 0; i < starts.size(); ++i)
    {
        //cur[i].hist = nullptr;
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
        if (print)
        {
            cout << "Agent " << id << " reaches " << agents[id].raw_grid[cur[id].pos->x][cur[id].pos->y];
            cout << " at " << cur[id] << " at time = " << curTime/10000.0 << endl;
        }
        if (cur[id] != agents[id].goal && !cur[id].pos->late)
        {
            going[id] = cur[id].getData().bpCut;
            events.emplace(curTime + cur[id].getData().lenCut, id);
            if (print)
            {
                cout << "Agent " << id << "  leaves " << agents[id].raw_grid[cur[id].pos->x][cur[id].pos->y];
                cout << " at " << cur[id] << " at time = " << curTime/10000.0 << endl;
            }
            trigger |= cur[id].pos->mask & takeMin[0];
            ull safeToClose = ~takeMin[0];
            for (State& s : going)
                safeToClose &= ~s.hist->mask[0];
            trigger |= safeToClose;
        }
        for (int i = 0; i < starts.size(); ++i)
        if (cur[i] == going[i] && cur[i] != agents[i].goal && !(cur[i].pos->mask & ~trigger))
        {
            going[i] = cur[i].getData().bpCut;
            events.emplace(curTime + cur[i].getData().lenCut, i);
            if (print)
            {
                cout << "Agent " << i << "  leaves " << agents[i].raw_grid[cur[i].pos->x][cur[i].pos->y];
                cout << " at " << cur[i] << " at time = " << curTime/10000.0 << endl;
            }
        }
        /*if (going[id] != agents[id].goal && !going[id].pos->late && (going[id].pos->mask & ~trigger))
        {
            going[id] = going[id].getData().bpCut;
            ull safeToClose = ~takeMin[0];
            for (State& s : going)
                safeToClose &= ~s.hist->mask[0];
            trigger |= safeToClose;
            trigger |= cur[id].pos->mask & takeMin[0];
            //going[id] = cur[id];
        }
        for (int i = 0; i < starts.size(); ++i)
        if (cur[i] == going[i] || i == id)
        {
            Cost jump = 0;
            if (i == id && cur[i] != going[i])
                jump = cur[i].getData().lenCut;
            while (going[i] != agents[i].goal && !(going[i].pos->mask & ~trigger))
            {
                jump += going[i].getData().lenCut;
                going[i] = going[i].getData().bpCut;
            }
            if (jump > 0) // should 0-weight edges be allowed?
            {
                events.emplace(curTime + jump, i);
                if (print)
                {
                    cout << "Agent " << i << "  leaves " << agents[i].raw_grid[cur[i].pos->x][cur[i].pos->y];
                    cout << " at " << cur[i] << " at time = " << curTime/10000.0 << endl;
                }
            }
        }*/
    }
    /*State s = starts[0];
    while (s.getData().bp != s)
    {
        cout << s << endl;
        s = s.getData().bp;
    }
    cout << s << endl;*/
    for (int i = 0; i < starts.size(); ++i)
        if (cur[i] != agents[i].goal)
            return;
    if (joint_cost > curTime)
    {
        joint_cost = curTime;
        joint_solution = starts;
    }
}

void testAll(int pos)
{
    if (pos == agents.size())
    {
        vector<State> starts;
        for (Agent& agent : agents)
            starts.push_back(*agent.foundIt);
        return testSolution(starts, false);
    }
    if (agents[pos].foundIt == agents[pos].found.cend())
        return;
    testAll(pos+1);
    ++agents[pos].foundIt;
    testAll(pos);
    --agents[pos].foundIt;
}

HistoryNode* transition(HistoryNode* hist, PositionNode* pos)
{
    stack<pair<ull, bool> > st;
    ull posMins = pos->mask & takeMin[pos->late];
    while (posMins & (hist->mask[pos->late] | hist->mask[1])) // can ignore doors we've opened
    {
        ull change[2];
        change[0] = hist->mask[0] & ~hist->parent->mask[0];
        change[1] = hist->mask[1] & ~hist->parent->mask[1];
        if (change[0])
            st.emplace(change[0], false);
        else
            st.emplace(change[1], true);
        hist = hist->parent;
    }
    while (!st.empty())
    {
        ull mask = st.top().first;
        bool late = st.top().second;
        st.pop();
        if (late == pos->late || late) // can ignore doors we've opened
            mask &= ~posMins;
        hist = hist->getChild(mask, late);
    }
    return hist->getChild(pos->mask, pos->late);
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
        // cannot close a door that we plan to use later
        if (t.pos->late && (t.pos->mask & s.hist->mask[0] & ~takeMin[0]))
            continue;
        t.hist = transition(s.hist, t.pos);
        //critical section for updating g-value and inserting into the heap
        StateData& t_data = t.getData();
        if (t_data.g > s_data.g + dcost[d])
        {
            if (t_data.g == INFINITE)
            {
                num_discovered++;
                t_data.iter = open.cend();
            }
            State& prev = t_data.bpCut;
            t_data.g = s_data.g + dcost[d];
            t_data.lenCut = dcost[d];
            t_data.bp = prev = s;
            // prev is irrelevant if prev \cap takemin \subset t AND prev \cap takemax \subset s.bpCut.mask
            while (prev != goal
                && !(prev.pos->mask & takeMin[prev.pos->late] & ~(t.pos->mask * (prev.pos->late == t.pos->late/* || prev.pos->late*/)))
                && !(prev.pos->mask & ~takeMin[prev.pos->late] & ~prev.getData().bpCut.hist->mask[prev.pos->late]))
                t_data.lenCut += prev.getData().lenCut, prev = prev.getData().bpCut;
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
    for (Agent& agent : agents)
    {
        agent.fmin = 0;
        agent.open.clear();
        agent.found.clear();
        agent.historyRoot.reset(new HistoryNode);
        agent.historyRoot->parent = nullptr;
        agent.historyRoot->mask[0] = agent.historyRoot->mask[1] = 0;
        //agent.historyRoot->mask[agent.goal->late] = agent.goal->mask;
        agent.goal.hist = agent.historyRoot.get();
        StateData& goal_data = agent.goal.getData();
        goal_data.g = goal_data.lenCut = 0;
        goal_data.bpCut = goal_data.bp = agent.goal;
        goal_data.iter = agent.open.cend();
        agent.insert(agent.goal);
    }
}

void search()
{
    joint_bound = 0, joint_cost = INFINITE;
    while (agents.size() * joint_bound < joint_cost) // watch for overflow
    {
        joint_bound = INFINITE;
        for (Agent& agent : agents)
        if (!agent.open.empty())
        {
            //get a State to expand
            State s = agent.remove();
            num_expands++;
            if (s.pos == agent.start)
            {
                for (Agent& a : agents)
                    a.foundIt = a.found.cbegin();
                agent.found.push_back(s);
                agent.foundIt = agent.found.cend();
                --agent.foundIt;
                Clock::time_point t0 = Clock::now();
                testAll(0);
                Clock::time_point t1 = Clock::now();
                test_duration += chrono::duration<double, chrono::seconds::period>(t1-t0).count();
            }
            //cout << "expanding " << s << endl;
            agent.expand(s);
            if (!agent.open.empty())
            {
                agent.fmin = max(agent.fmin, agent.open.cbegin()->first);
                joint_bound = min(joint_bound, agent.fmin);
            }
        }
    }
}

int main(int argc, char** argv)
{
    FILE* fout = fopen("stats.csv","w");
    //load map
    readMap();

    //run planner
    test_duration = 0;
    Clock::time_point t0 = Clock::now();
    prepare();
    search();
    Clock::time_point t1 = Clock::now();
    
    //report joint plan
    testSolution(joint_solution, true);

    //report stats
    double dt =  chrono::duration<double, chrono::seconds::period>(t1-t0).count();
    cout << "results: Path Length=" << joint_cost/10000.0;
    cout << " Visited Nodes=" << num_discovered;
    cout << " Explored Nodes=" << num_expands;
    cout <<" Planning Time=" << dt << endl;
    cout <<" Joint Testing Time=" << test_duration << endl;
    fprintf(fout,"%f %f %d %f\n",W,dt,num_expands,joint_cost/10000.0);

    fclose(fout);
}
