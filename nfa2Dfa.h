#ifndef NFA2DFA_H
#define NFA2DFA_H

#include <set>
#include <vector>
#include <map>

// Forward declarations for Node and Edge
struct Node;
struct Edge;
struct DfaNode {
    int id;
    std::set<Node*> nfaStates;
    bool isAccept = false;
    DfaNode(int i, std::set<Node*> n): id(i), nfaStates(n) {}
};

std::set<Node*> epsilonClosure(Node* node, const std::vector<Edge>& edges);
std::map<DfaNode*, std::map<char, DfaNode*> > nfa2Dfa(Fragment* nfa, const std::vector<Edge>& edges);
void dfa2StateTable(std::map<DfaNode*, std::map<char, DfaNode*> > dfaEdges);

#endif