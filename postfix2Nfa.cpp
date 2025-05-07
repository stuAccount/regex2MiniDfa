#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <queue>
#include "./regex2Postfix.h"

using namespace std;

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

Fragment createFrag(char l, int& nodeId, vector<Edge>& edges) {
    Node* start = new Node(nodeId++);
    Node* accept = new Node(nodeId++);
    edges.push_back(Edge(start, accept, l));
    return Fragment(start, accept);
}

Fragment concatFrag(Fragment &f1, Fragment &f2, vector<Edge>& edges) {
    Node* newStart = f1.Start;
    Node* newAccept = f2.Accept;
    /* merge f1.accept and f2.start, 
    change all the edges which begin node is f2.Start to f1.Accept
    */
    for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
        if (it->begin->id == f2.Start->id) {
            it->begin = f1.Accept;
        }
    }
    return Fragment(newStart,newAccept);
}

Fragment unionFrag(Fragment &f1, Fragment &f2, int& nodeId, vector<Edge>& edges) {
    Node* newStart = new Node(nodeId++);
    Node* newAccept = new Node(nodeId++);
    edges.push_back(Edge(newStart, f1.Start, 0));
    edges.push_back(Edge(newStart, f2.Start, 0));
    edges.push_back(Edge(f1.Accept, newAccept, 0));
    edges.push_back(Edge(f2.Accept, newAccept, 0));
    return Fragment(newStart, newAccept);
}

Fragment KleeneFrag(Fragment& f, int& nodeId, vector<Edge>& edges) {
    Node* newStart = new Node(nodeId++);
    Node* newAccept = new Node(nodeId++);
    edges.push_back(Edge(newStart, f.Start, 0));
    edges.push_back(Edge(newStart, newAccept, 0));
    edges.push_back(Edge(f.Accept, newAccept, 0));
    edges.push_back(Edge(f.Accept, f.Start, 0));
    return Fragment(newStart, newAccept);
}

Fragment postfix2Nfa(string postfix, vector<Edge>& edges) {
    stack<Fragment> fragStack;
    int nodeId = 0;
    for (int i = 0; i < postfix.length(); i++) {
        char curr = postfix[i];
        if (isOperand(curr)) {
            fragStack.push(createFrag(curr, nodeId, edges));
        }
        else if (curr == '.') {
            Fragment f2 = fragStack.top();
            fragStack.pop();
            Fragment f1 = fragStack.top();
            fragStack.pop();
            fragStack.push(concatFrag(f1,f2,edges));
        }
        else if (curr == '|') {
            Fragment f1 = fragStack.top();
            fragStack.pop();
            Fragment f2 = fragStack.top();
            fragStack.pop();
            fragStack.push(unionFrag(f1,f2,nodeId, edges));
        }
        else if (curr == '*') {
            Fragment f = fragStack.top();
            fragStack.pop();
            fragStack.push(KleeneFrag(f, nodeId, edges));
        }
    }
    Fragment nfa = fragStack.top();
    return nfa;
}

void reassignNodeIds(Fragment &nfa, vector<Edge>& edges) {
    vector<Node*> allNodes;
    set<Node*> visited;
    queue<Node*> queue;
    queue.push(nfa.Start);

    while(!queue.empty()) {
        Node* curr = queue.front();
        queue.pop();
        if (visited.count(curr)) {
            continue;
        }
        visited.insert(curr);
        allNodes.push_back(curr);
        for(auto e: edges) {
            if (e.begin->id == curr->id && !visited.count(e.end)) {
                queue.push(e.end);
            }
        }
    }    
    
    int nodeId = 1;
    for (auto n: allNodes) {
        n->id = nodeId++;
    }
}

void nfa2StateTable(vector<Edge>& edges) {
    map< int, map < char, set<int> > > stateTable;
    // state, inputChar, toState
    for (auto e: edges) {
        stateTable[e.begin->id][e.label].insert(e.end->id);
    }
    for (auto s: stateTable) {
        cout << s.first;
        for (const auto& [inputChar, toStates] : s.second) {
            cout << "-->" << inputChar << "-->";
            for (auto state: toStates) {
                cout << state << " ";
            } 
        }
        cout << endl;
    }
}

