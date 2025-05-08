#include <iostream>
#include <string>
#include "./postfix2Nfa.h"
#include <set>
#include <queue>
#include <vector>
#include <map>

using namespace std;

struct DfaNode {
    int id;
    set<Node*> nfaStates;
    bool isAccept = false;
    DfaNode(int i, set<Node*> n): id(i), nfaStates(n) {}
};

set<Node*> epsilonClosure(set<Node*> nodes, const vector<Edge>& edges) {
    queue<Node*> q;
    for (auto node : nodes) {
        q.push(node);
    }
    set<Node*> closure;
    for (auto node : nodes) {
        closure.insert(node);
    }
    while (!q.empty()) {
        Node* curr = q.front();
        q.pop();
        for (auto e : edges) {
            if (e.begin->id == curr->id && e.label == 0 && !closure.count(e.end)) {
                closure.insert(e.end);
                q.push(e.end);
            }
        }
    }
    return closure;
}

map<DfaNode*, map<char, DfaNode*>> nfa2Dfa(Fragment* nfa, const vector<Edge>& edges) {
    int nodeId = 1;
    queue<DfaNode*> q;
    set<Node*> startSet;
    startSet.insert(nfa->Start);
    set<Node*> startClosure = epsilonClosure(startSet, edges);
    DfaNode* startDfa = new DfaNode(nodeId++, startClosure);
    q.push(startDfa);
    map<DfaNode*, map<char, DfaNode*>> newEdges;
    map<set<Node*>, DfaNode*> stateMap;
    stateMap[startClosure] = startDfa;
    for (auto s : startClosure) {
        if (s->id == nfa->Accept->id) {
            startDfa->isAccept = true;
            cout << startDfa->id << " is accept state" << endl;
            break;
        }
    }
    set<char> uniqueLabels;
    for (auto e : edges) {
        if (e.label != 0) {
            uniqueLabels.insert(e.label);
        }
    }
    while (!q.empty()) {
        DfaNode* curr = q.front();
        q.pop();
        map<char, set<Node*>> transitionsMap;
        DfaNode* targetDfa;
        for (auto element : curr->nfaStates) {
            for (auto e : edges) {
                if (element->id == e.begin->id && uniqueLabels.count(e.label)) {
                    transitionsMap[e.label].insert(e.end);
                }
            }
        }
        for (auto inputChar : uniqueLabels) {
            if (transitionsMap.count(inputChar)) {
                set<Node*> closure = epsilonClosure(transitionsMap[inputChar], edges);
                if (stateMap.count(closure) == 0) {
                    targetDfa = new DfaNode(nodeId++, closure);
                    for (auto s : targetDfa->nfaStates) {
                        if (s->id == nfa->Accept->id) {
                            targetDfa->isAccept = true;
                            cout << targetDfa->id << " is accept state" << endl;
                            break;
                        }
                    }
                    stateMap[closure] = targetDfa;
                    q.push(targetDfa);
                } else {
                    targetDfa = stateMap[closure];
                }
                newEdges[curr][inputChar] = targetDfa;
            }
        }
    }
    DfaNode* deadState = new DfaNode(nodeId++, {});
    deadState->isAccept = false;
    for (char c : uniqueLabels) {
        newEdges[deadState][c] = deadState;
    }
    for (auto [state, transitions] : newEdges) {
        for (char c : uniqueLabels) {
            if (newEdges[state].count(c) == 0) {
                newEdges[state][c] = deadState;
            }
        }
    }
    return newEdges;
}

void dfa2StateTable(map<DfaNode*, map<char, DfaNode*>> dfaEdges) {
    for (auto s : dfaEdges) {
        for (const auto& [inputChar, toState] : s.second) {
            cout << s.first->id;
            cout << "--" << inputChar << "-->";
            cout << toState->id << endl;
        }
    }
}

