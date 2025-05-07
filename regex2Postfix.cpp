#include <iostream>
#include <string>
#include <stack>
#include "./regex2Postfix.h"

using namespace std;


bool isOperator(char c) {
    return (c == '.' || c == '*' || c == '|' || c == '(' || c == ')');
}

bool isOperand(char c) {
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

int getPrecedence(char op) {
    switch(op) {
        case '*': return 3;
        case '.': return 2;
        case '|': return 1;
        default: return 0;
    }
}

string insertConcat(const string& regex) {
    string newRegex;
    for (int i = 0; i < regex.length(); i++) {
        newRegex += regex[i];
        if (i+1 < regex.length()) {
            char curr = regex[i];
            char next = regex[i+1];
            if ((isOperand(curr) && (isOperand(next) || next == '(')) ||
                ((curr == '*') && (isOperand(next) || next == '(')) ||
                ((curr == ')') && (isOperand(next) || next == '('))) {
                    newRegex += '.';
            }
        }
    }
    return newRegex;
}

string regex2Postfix(const string& regex) {
    string infix = insertConcat(regex);
    string postfix;
    stack<char> opStack;
    for (int i = 0; i< infix.length(); i++) {
        char curr = infix[i];
        
        if (isOperand(curr)) {
            postfix += curr;
        }
        else if (curr == '(') {
            opStack.push(curr);
        }
        else if (curr == ')') {
            while (opStack.top() != '(') {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.pop();
        }
        else if (curr == '*' || curr == '|' || curr == '.') {
            while (!opStack.empty() && opStack.top() != '(' &&
                    getPrecedence(curr) <= getPrecedence(opStack.top())) {
                postfix += opStack.top();
                opStack.pop();
            }
            opStack.push(curr);
        }
    }
    while(!opStack.empty()) {
        postfix += opStack.top();
        opStack.pop();
    }
    return postfix;
}


