#include <iostream>
#include <vector>
#include <map>
#include <set>
#include "dfa2MiniDfa.h"
#include "postfix2Nfa.h"
#include "nfa2Dfa.h"

using namespace std;

// Initialize distinguishability table for all pairs (i, j) with i < j
void initDisTable(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map<pair<int, int>, bool>& distinguishable) {
    set<int> dfaStatesNum;
    for (auto [fromState, to] : dfaEdges) {
        for (auto [symbol, toState] : to) {
            dfaStatesNum.insert(fromState->id);
            dfaStatesNum.insert(toState->id);
        }
    }
    for (int i : dfaStatesNum) {
        for (int j : dfaStatesNum) {
            if (i < j) {
                distinguishable[{i, j}] = false;
            }
        }
    }
}

// Ensure pair is ordered (a < b)
pair<int, int> normPair(int a, int b) {
    return a < b ? make_pair(a, b) : make_pair(b, a);
}

void splitAndMark(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map<pair<int, int>, bool>& distinguishable) {
    set<int> nonAccept;
    set<int> accept;
    for (auto [fromState, to] : dfaEdges) {
        if (fromState->isAccept) {
            accept.insert(fromState->id);
        } else {
            nonAccept.insert(fromState->id);
        }
        for (auto [symbol, toState] : to) {
            if (toState->isAccept) {
                accept.insert(toState->id);
            } else {
                nonAccept.insert(toState->id);
            }
        }
    }
    for (auto [statePair, isDis] : distinguishable) {
        int i = statePair.first;
        int j = statePair.second;
        if ((nonAccept.count(i) && accept.count(j)) || (nonAccept.count(j) && accept.count(i))) {
            distinguishable[normPair(i, j)] = true;
        }
    }
    cout << "[splitAndMark] Accept states: ";
    for (auto a : accept) cout << a << " ";
    cout << "\n[splitAndMark] Non-accept states: ";
    for (auto n : nonAccept) cout << n << " ";
    cout << endl;
}

// Iteratively mark distinguishable pairs
void markDisPairs(map<DfaNode*, map<char, DfaNode*> > dfaEdges, map<pair<int, int>, bool>& distinguishable) {
    set<char> allTransitions;
    map<pair<int, char>, int> transitionMap;
    for (auto [fromState, to] : dfaEdges) {
        for (auto [inputChar, toState] : to) {
            allTransitions.insert(inputChar);
            transitionMap[{fromState->id, inputChar}] = toState->id;
        }
    }
    bool changed = true;
    int round = 0;
    while (changed) {
        changed = false;
        round++;
        cout << "\n[markDisPairs] === Round " << round << " ===" << endl;
        for (auto [statePair, isDis] : distinguishable) {
            int i = statePair.first;
            int j = statePair.second;
            if (i >= j) continue;
            if (distinguishable[normPair(i, j)] == true) continue;
            cout << "[markDisPairs] Checking pair (" << i << "," << j << "):" << endl;
            for (auto input : allTransitions) {
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
                    break;
                } else {
                    cout << " => Not distinguishable." << endl;
                }
            }
        }
        cout << "[markDisPairs] Distinguishable table after round " << round << ":" << endl;
        for (auto [statePair, isDis] : distinguishable) {
            cout << "  (" << statePair.first << "," << statePair.second << "): " << (isDis ? "T" : "F") << endl;
        }
    }
    cout << "[markDisPairs] Table-filling complete after " << round << " rounds." << endl;
}

vector<set<DfaNode*>> equivalenceClasses(map<pair<int, int>, bool> distinguishable, map<DfaNode*, map<char, DfaNode*>> dfaEdges) {
    DfaNode* deadState = nullptr;
    for (auto [fromState, to] : dfaEdges) {
        if (fromState->nfaStates.empty()) {
            deadState = fromState;
            break;
        }
    }
    map<int, DfaNode*> idToNode;
    for (auto [fromState, to] : dfaEdges) {
        idToNode[fromState->id] = fromState;
        for (auto [symbol, toState] : to) {
            idToNode[toState->id] = toState;
        }
    }
    vector<set<int>> eqClasses;
    set<int> isIndis;
    for (auto pairIJ : distinguishable) {
        int I = pairIJ.first.first;
        int J = pairIJ.first.second;
        if (!distinguishable[normPair(I, J)]) {
            bool inserted = false;
            isIndis.insert(I);
            isIndis.insert(J);
            for (auto& eq : eqClasses) {
                if (eq.count(I) || eq.count(J)) {
                    eq.insert(I);
                    eq.insert(J);
                    inserted = true;
                    break;
                }
            }
            if (!inserted) {
                eqClasses.push_back({I, J});
            }
        }
    }
    set<int> added;
    for (auto [pair, isDist] : distinguishable) {
        int I = pair.first;
        int J = pair.second;
        if (!isIndis.count(I) && !added.count(I)) {
            eqClasses.push_back({I});
            added.insert(I);
        }
        if (!isIndis.count(J) && !added.count(J)) {
            eqClasses.push_back({J});
            added.insert(J);
        }
    }
    vector<set<DfaNode*>> dfaStates;
    for (auto& stateSet : eqClasses) {
        set<DfaNode*> toBePushed;
        for (int stateId : stateSet) {
            if (idToNode.count(stateId)) {
                if (deadState && idToNode[stateId] == deadState) continue;
                toBePushed.insert(idToNode[stateId]);
            }
        }
        if (!toBePushed.empty())
            dfaStates.push_back(toBePushed);
    }
    cout << "[equivalenceClasses] Total classes: " << dfaStates.size() << endl;
    for (int i = 0; i < dfaStates.size(); ++i) {
        cout << "[equivalenceClasses] Class " << (i + 1) << ": ";
        for (auto s : dfaStates[i]) cout << s->id << " ";
        cout << endl;
    }
    return dfaStates;
}

vector<MiniEdge*> buildMiniDfa(map<DfaNode*, map<char, DfaNode*>> dfaEdges, vector<set<DfaNode*>> eClasses) {
    int startClassIdx = -1;
    for (int i = 0; i < eClasses.size(); ++i) {
        for (auto s : eClasses[i]) {
            if (s->id == 1) {
                startClassIdx = i;
                break;
            }
        }
        if (startClassIdx != -1) break;
    }
    vector<set<DfaNode*>> orderedClasses;
    if (startClassIdx != -1) {
        orderedClasses.push_back(eClasses[startClassIdx]);
        for (int i = 0; i < eClasses.size(); ++i) {
            if (i != startClassIdx) orderedClasses.push_back(eClasses[i]);
        }
    } else {
        orderedClasses = eClasses;
    }
    int nodeId = 1;
    vector<MiniNode*> miniNodes;
    for (auto& cls : orderedClasses) {
        MiniNode* newNode = new MiniNode(nodeId++, cls);
        miniNodes.push_back(newNode);
    }
    map<int, MiniNode*> belongTo;
    for (auto miniNode : miniNodes) {
        for (auto dfaState : miniNode->dfaStates) {
            belongTo[dfaState->id] = miniNode;
        }
    }
    map<pair<MiniNode*, char>, MiniNode*> miniEdgeMap;
    vector<MiniEdge*> miniEdges;
    for (auto [fromState, to] : dfaEdges) {
        if (fromState->nfaStates.empty()) continue;
        for (auto [symbol, toState] : to) {
            if (toState->nfaStates.empty()) continue;
            miniEdgeMap[{belongTo[fromState->id], symbol}] = belongTo[toState->id];
        }
    }
    for (auto [from, toState] : miniEdgeMap) {
        miniEdges.push_back(new MiniEdge(from.first, toState, from.second));
    }
    return miniEdges;
}

void printMiniDfa(const vector<MiniEdge*>& edges) {
    for (auto e : edges) {
        cout << "MiniNode " << e->begin->id << " --" << e->label << "--> " << e->end->id << endl;
    }
}
