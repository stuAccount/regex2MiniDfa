#ifndef DFA2MINIDFA_H
#define DFA2MINIDFA_H

#include <set>
#include <map>
#include <vector>
using namespace std;

struct DfaNode; // Forward declaration

struct MiniNode {
    int id;
    std::set<DfaNode*> dfaStates;
    MiniNode(int i, std::set<DfaNode*> d): id(i), dfaStates(d) {}
};

struct MiniEdge{
    MiniNode* begin;
    MiniNode* end;
    char label;
    MiniEdge(MiniNode* b, MiniNode* e, char l): begin(b), end(e), label(l){}
};

void initDisTable(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map< pair<int, int>, bool>& distinguishable);
pair<int, int> normPair(int a, int b);
void splitAndMark(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map < pair<int, int>, bool>& distinguishable);
void markDisPairs(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map< pair<int, int>, bool>& distinguishable);
vector<set<DfaNode*> > equivalenceClasses(map< pair<int, int>, bool> distinguishable, map<DfaNode*, map<char, DfaNode*> > dfaEdges);
MiniNode* belongTo(int dfaNodeId, vector<MiniNode*> miniNodes);
vector<MiniEdge*> buildMiniDfa(map<DfaNode*, map<char, DfaNode*> > dfaEdges, vector<set<DfaNode*> > eClasses);
void printMiniDfa(const vector<MiniEdge*>& edges);

#endif