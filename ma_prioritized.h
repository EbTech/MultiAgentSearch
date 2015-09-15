// #define NDEBUG
#include <cassert>
#include <memory>
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>

typedef unsigned long long ull;
typedef int Cost;
using namespace std;

constexpr Cost INFINITE = 100000000;
/*inline bool SUBSET(ull a, ullb)
{
    return (a & b) == a;
}*/

class PositionNode;
class HistoryNode;
class State;
class StateData;
class Agent;

class State
{
  public:
    PositionNode* pos;
    HistoryNode* hist;
    StateData& getData() const;
    bool isGoal() const;
    bool operator==(const State& other) const
    {
        return pos == other.pos && hist == other.hist;
    }
    bool operator!=(const State& other) const
    {
        return !(*this == other);
    }
};

class PositionNode
{
  public:
    int x;
    int y;
    bool obst;
    bool late;
    ull mask;
};

class StateData
{
  public:
    multimap<Cost,State>::const_iterator iter;
    Cost g;
    State bp;
    bool closed;
    StateData() : g(INFINITE), closed(false) {}
};

class HistoryNode
{
  public:
    // TODO: implement shortcuts like so: unordered_map<ull, HistoryNode*> shortcut[2];
    HistoryNode* parent;
    unordered_map<ull, unique_ptr<HistoryNode> > child[2];
    unordered_map<PositionNode*, StateData> data;
    ull mask[2];
    vector<int> jointProgress;
    // "dumb" history append, without reordering
    HistoryNode* getChild(ull key, bool late)
    {
        key &= ~mask[late];
        if (!key)
            return this;
        if (parent != nullptr)
        {
            ull changeMask = mask[late] & ~parent->mask[late];
            if (changeMask)
                return parent->getChild(key | changeMask, late);
        }
        unique_ptr<HistoryNode>& theChild = child[late][key];
        if (theChild == nullptr)
        {
            theChild.reset(new HistoryNode);
            theChild->parent = this;
            theChild->mask[0] = mask[0];
            theChild->mask[1] = mask[1];
            theChild->mask[late] |= key;
            theChild->jointProgress = jointProgress;
        }
        return theChild.get();
    }
};

ostream& operator<<(ostream& os, const PositionNode& pos)
{
    os << "(" << pos.x << ", " << pos.y << ")";
    return os;
}
ostream& operator<<(ostream& os, const HistoryNode& hist)
{
    os << hist.mask[0] << " " << hist.mask[1];
    return os;
}
ostream& operator<<(ostream& os, const State& s)
{
    os << *s.pos << " " << *s.hist;
    return os;
}

class Agent
{
  public:
    // auxiliary data for greedy
    ull canOpen;
    bool stuck;
    vector<PositionNode*> solution;
    vector<int> closeUsePos;
    
    
    unique_ptr<HistoryNode> historyRoot;
    multimap<Cost,State> open;
    unordered_map<PositionNode*, Cost> hDist;
    vector<vector<PositionNode> > grid;
    vector<string> raw_grid;
    PositionNode* goalPos;
    State start;
    int numRows;
    int numCols;
    
    void insert(const State& s);
    State remove();
    void expand(State& s);
};

StateData& State::getData() const { return hist->data[pos]; }

