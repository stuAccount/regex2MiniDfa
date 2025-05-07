#ifndef POSTFIX2NFA_H
#define POSTFIX2NFA_H

#include <vector>
#include <string>

struct Node {
    int id;
    Node(int i) : id(i) {}
};

struct Edge{
    Node* begin;
    Node* end;
    char label; 
    Edge(Node* b, Node* e, char l): begin(b), end(e), label(l) {}
};

struct Fragment {
    Node* Start;
    Node* Accept;
    Fragment(Node* s, Node* a): Start(s), Accept(a) {}
};

// Create an NFA fragment for a single character
Fragment createFrag(char l, int& nodeId, std::vector<Edge>& edges);

// Concatenate two NFA fragments
Fragment concatFrag(Fragment& f1, Fragment& f2, std::vector<Edge>& edges);

// Union (|) two NFA fragments
Fragment unionFrag(Fragment& f1, Fragment& f2, int& nodeId, std::vector<Edge>& edges);

// Apply Kleene star to an NFA fragment
Fragment KleeneFrag(Fragment& f, int& nodeId, std::vector<Edge>& edges);

// Convert postfix regex to NFA fragment
Fragment postfix2Nfa(std::string postfix, std::vector<Edge>& edges);

// Reassign node IDs in the NFA
void reassignNodeIds(Fragment& nfa, std::vector<Edge>& edges);

// Print NFA as a state table
void nfa2StateTable(std::vector<Edge>& edges);

#endif