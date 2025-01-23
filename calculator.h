#ifndef calculator_h
#define calculator_h

#define INITIAL_CAPACITY 20 //The initial number of characters of the user-input expression

int getInput(char** exp);
double evaluateExpression(char** exp);

#endif