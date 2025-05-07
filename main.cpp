#include <iostream>
#include "postfix2Nfa.h"
#include "nfa2Dfa.h"
#include "regex2Postfix.h"
#include "dfa2MiniDfa.h"
#include <string>
#include <map>

using namespace std;

int main() {
    vector<Edge> edges; 
    string regex;
    cin >> regex;
    string postfix = regex2Postfix(regex);
    Fragment nfa = postfix2Nfa(postfix, edges);
    reassignNodeIds(nfa, edges);
    // cout << "the regular expression is " << regex << endl;
    // cout << "the postfix is " << postfix << endl;
    // cout << "the start state of nfa is " << nfa.Start->id << endl;
    // cout << "the accept state of nfa is " << nfa.Accept->id << endl;
    // for (vector<Edge>::iterator it = edges.begin(); it != edges.end(); ++it) {
    //     cout << "edge from " << it->begin->id << " to " << it->end->id << " with label ";
    //     if (it->label == 0) {
    //         cout << "ε" << endl;
    //     }
    //     else {
    //         cout << it->label << endl;
    //     }
    // }
    // cout << "the number of edges is " << edges.size() << endl;
    // cout << endl;
    nfa2StateTable(edges);
    
    map<DfaNode*, map<char, DfaNode*> > dfa = nfa2Dfa(&nfa, edges);
    dfa2StateTable(dfa);

    // --- MiniDFA test code ---
    map < pair<int, int>, bool> distinguishable;
    initDisTable(dfa,distinguishable);
    splitAndMark(dfa,distinguishable);
    markDisPairs(dfa,distinguishable);
    vector<set<DfaNode*> > eClasses = equivalenceClasses(distinguishable, dfa);
    vector<MiniEdge*> miniDfa = buildMiniDfa(dfa, eClasses);
    cout << "MiniDFA transitions:" << endl;
    printMiniDfa(miniDfa);

    return 0;
}
