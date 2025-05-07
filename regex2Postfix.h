#ifndef REGEX_CONVERTER_H
#define REGEX_CONVERTER_H

bool isOperand(char c);
bool isOperator(char c);
std::string regex2Postfix(const std::string& regex);

#endif