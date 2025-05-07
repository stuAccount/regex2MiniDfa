#include <iostream>
#include <vector>
#include <map>
#include <set>
#include "dfa2MiniDfa.h"
#include "postfix2Nfa.h"
#include "nfa2Dfa.h"

using namespace std;

struct MiniNode {
    int id;
    set<DfaNode*> dfaStates;
    MiniNode(int i, set<DfaNode*> d): id(i), dfaStates(d) {}
};

struct MiniEdge{
    MiniNode* begin;
    MiniNode* end;
    char label;
    MiniEdge(MiniNode* b, MiniNode* e, char l): begin(b), end(e), label(l){}
};

// Table-filling algorithm,  use the distinguishability table method

// make a 2d table for pair (i,j) while i < j
//default is false(indistinguishable)
void initDisTable (map<DfaNode*, map<char, DfaNode*> > dfaEdges,map < pair<int, int>, bool>& distinguishable) {
    set<int> dfaStatesNum;
    for (auto [fromState, to]: dfaEdges) {
        for (auto [symbol, toState]: to) {
            dfaStatesNum.insert(fromState->id);
            dfaStatesNum.insert(toState->id);
        }
    }
    // Initialize the table for all pairs (i, j) with i < j
    for (int i: dfaStatesNum) {
        for (int j: dfaStatesNum) {
            if (i < j) {
                distinguishable[{i, j}] = false;
            }
        }
    }
}

// Normalize the pair, always assure a is smaller than b
pair <int, int> normPair(int a, int b) {
    return a < b? make_pair(a, b) :make_pair(b,a);
}

void splitAndMark(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map < pair<int, int>, bool>& distinguishable) {
    set<int> nonAccept;
    set<int> accept;
    for (auto [fromState, to]: dfaEdges) {
        if (fromState->isAccept) {
            accept.insert(fromState->id);
        } else {
            nonAccept.insert(fromState->id);
        }
        for (auto [symbol, toState]: to) {
            if (toState->isAccept) {
                accept.insert(toState->id);
            } else {
                nonAccept.insert(toState->id);
            }
        }
    }
    for (auto [statePair,isDis]: distinguishable) {
        int i = statePair.first;
        int j = statePair.second;
        if ((nonAccept.count(i) && accept.count(j)) || (nonAccept.count(j) && accept.count(i))) {
            distinguishable[normPair(i,j)] = true;
        }
    }
    // Brief output
    cout << "[splitAndMark] Accept states: ";
    for (auto a : accept) cout << a << " ";
    cout << "\n[splitAndMark] Non-accept states: ";
    for (auto n : nonAccept) cout << n << " ";
    cout << endl;
}

/* Round 2...(iteratively):
For each pair (p, q) such that
on each transition to get a pair (m, n), 
if (m, n) is distinguishable(marked as T),
then mark (p, q) True
*/
void markDisPairs(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map < pair<int, int>, bool>& distinguishable) {
    set<char> allTransitions;
    map< pair<int, char>, int> transitionMap;

    for (auto [fromState, to]: dfaEdges) {
        for (auto [inputChar, toState]: to) {
            allTransitions.insert(inputChar);
            transitionMap[{fromState->id, inputChar}] = (toState->id);
        }
    }

    bool changed = true;
    int round = 0;
    while(changed) {
        changed = false;
        round++;
        cout << "\n[markDisPairs] === Round " << round << " ===" << endl;
        for (auto [statePair, isDis]: distinguishable) {
            int i = statePair.first;
            int j = statePair.second;
            if (i >= j) continue;
            if (distinguishable[normPair(i, j)] == true) continue;
            cout << "[markDisPairs] Checking pair (" << i << "," << j << "):" << endl;
            for (auto input: allTransitions) {
                int it = transitionMap.count({i, input}) ? transitionMap[{i, input}] : -1;
                int jt = transitionMap.count({j, input}) ? transitionMap[{j, input}] : -1;
                cout << "  On input '" << input << "': " << i << "->" << it << ", " << j << "->" << jt;
                if (it == -1 || jt == -1) {
                    cout << " (no transition for one or both states)" << endl;
                    continue;
                }
                cout << " | Checking (" << it << "," << jt << ")";
                if (distinguishable[normPair(it, jt)]) {
                    cout << " => Already marked distinguishable! Marking (" << i << "," << j << ") as distinguishable." << endl;
                    distinguishable[normPair(i, j)] = true;
                    changed = true;
                    break; // No need to check other inputs for this pair
                } else {
                    cout << " => Not distinguishable." << endl;
                }
            }
        }
        // Print current distinguishable table
        cout << "[markDisPairs] Distinguishable table after round " << round << ":" << endl;
        for (auto [statePair, isDis]: distinguishable) {
            cout << "  (" << statePair.first << "," << statePair.second << "): " << (isDis ? "T" : "F") << endl;
        }
    }
    // Brief output
    cout << "[markDisPairs] Table-filling complete after " << round << " rounds." << endl;
}

vector<set<DfaNode*> > equivalenceClasses(map < pair<int, int>, bool> distinguishable, map<DfaNode*, map<char, DfaNode*> > dfaEdges) {
    vector <set<int> > equivalenceClasses;
    set<int> isIndis;
    for (auto pairIJ: distinguishable) {
        int I = pairIJ.first.first;
        int J = pairIJ.first.second;
        if (distinguishable[normPair(I,J)] == false) {
            bool inserted = false;
            isIndis.insert(I);
            isIndis.insert(J);
            for (int i = 0; i < equivalenceClasses.size(); i++) {
                if (equivalenceClasses[i].count(I) || equivalenceClasses[i].count(J)) {
                    equivalenceClasses[i].insert(I);
                    equivalenceClasses[i].insert(J);
                    inserted = true;
                    break;
                }
            }
            if (inserted == false) {
                equivalenceClasses.push_back({I,J});
            }
        }
    }
    set<int> added;
    for (auto [pair,isDist]: distinguishable) {
        int I = pair.first;
        int J = pair.second;
        if (!isIndis.count(I) && !added.count(I)) {
            equivalenceClasses.push_back(set<int>{I});
            added.insert(I);
        }
        if (!isIndis.count(J) && !added.count(J)) {
            equivalenceClasses.push_back(set<int>{J});
            added.insert(J);
        }
    }
    vector<set<DfaNode*> > dfaStates;
    for (auto stateSet: equivalenceClasses) {
        set<DfaNode*> toBePushed;
        for (auto stateId: stateSet) {
            for (auto [fromState, to]: dfaEdges) {
                for (auto [symbol, toState]: to) {
                    if (stateSet.count(fromState->id)) {
                        toBePushed.insert(fromState); 
                    }
                    else if (stateSet.count(toState->id)) {
                        toBePushed.insert(toState);
                    }
                }
            }
        }
        dfaStates.push_back(toBePushed);
    }
    // Brief output
    cout << "[equivalenceClasses] Total classes: " << dfaStates.size() << endl;
    for (int i = 0; i < dfaStates.size(); ++i) {
        cout << "[equivalenceClasses] Class " << (i+1) << ": ";
        for (auto s : dfaStates[i]) cout << s->id << " ";
        cout << endl;
    }
    return dfaStates;
}

// MiniNode* belongTo(int dfaNodeId, vector<MiniNode*> miniNodes) {
//     // iteractively search inside miniNodes and find the miniNode whose dfaStates containing the dfaNode with id of dfaNodeId;
//     for (auto miniNode: miniNodes) {
//         for (auto dfaState: miniNode->dfaStates) {
//             if (dfaState->id == dfaNodeId) {
//                 cout << "[belongTo] DFA node " << dfaNodeId << " belongs to MiniNode " << miniNode->id << endl;
//                 return miniNode;
//             }
//         }
//     }
//     cout << "[belongTo] DFA node " << dfaNodeId << " not found in any MiniNode!\n";
//     return nullptr;
// }



// then start building a new miniDFA
vector<MiniEdge*> buildMiniDfa (map<DfaNode*, map<char, DfaNode*> > dfaEdges, vector<set<DfaNode* > >eClasses) {
    int nodeId = 1;
    vector<MiniNode*> miniNodes;
    for (int i = 0; i < eClasses.size(); i++) {
        MiniNode* newNode = new MiniNode(nodeId++, eClasses[i]);
        miniNodes.push_back(newNode);
        cout << "[MiniNode] " << newNode->id << ": ";
        for (auto s : eClasses[i]) cout << s->id << " ";
        cout << endl;
    }

    map<int,MiniNode*> belongTo;
    for (auto miniNode: miniNodes) {
        for (auto dfaState: miniNode->dfaStates) {
            belongTo[dfaState->id] = miniNode;
        }
    }

    map<pair<MiniNode*,char>, MiniNode*> miniEdgeMap;
    vector<MiniEdge*> miniEdges;
    for (auto [fromState, to]: dfaEdges) {
        for (auto [symbol, toState]: to) {
            cout << "[MiniEdge] " << belongTo[fromState->id]->id << " --" << symbol << "--> " << belongTo[toState->id]->id << endl;
            miniEdgeMap[{belongTo[fromState->id],symbol}] = belongTo[toState->id];
        }
    }
    for (auto [from, toState]: miniEdgeMap) {
        miniEdges.push_back(new MiniEdge(from.first, toState, from.second));
    }

    return miniEdges;
}


void printMiniDfa(const vector<MiniEdge*>& edges) {
    for (auto e : edges) {
        cout << "MiniNode " << e->begin->id << " --" << e->label << "--> " << e->end->id << endl;
    }
}