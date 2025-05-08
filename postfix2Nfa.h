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

Fragment createFrag(char l, int& nodeId, std::vector<Edge>& edges);

Fragment concatFrag(Fragment& f1, Fragment& f2, std::vector<Edge>& edges);

Fragment unionFrag(Fragment& f1, Fragment& f2, int& nodeId, std::vector<Edge>& edges);

Fragment KleeneFrag(Fragment& f, int& nodeId, std::vector<Edge>& edges);

Fragment postfix2Nfa(std::string postfix, std::vector<Edge>& edges);

void reassignNodeIds(Fragment& nfa, std::vector<Edge>& edges);

void nfa2StateTable(std::vector<Edge>& edges);

#endif