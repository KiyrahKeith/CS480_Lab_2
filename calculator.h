#ifndef calculator_h
#define calculator_h

#include <stdbool.h>

#define INITIAL_CAPACITY 20 //The initial number of characters of the user-input expression

int getInput(char** exp);
double evaluateExpression(char** exp);
bool isNumber(char value);
double findNumber(char* exp, int* i);
bool isOperator(char op);
bool isUnary(char op);
char findOperator(char* exp, int* i);
int convertToPostFix(char** input, char** postFix);
double evaluateOp(char op, double a, double b);
int precedence(char op);


#endif