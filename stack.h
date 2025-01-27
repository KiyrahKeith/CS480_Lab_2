#ifndef stack_h
#define stack_h
#include <stdbool.h>

#define MAX 100 // The maximum number of operands and/or operators valid in the expression

// Operand Stack --------------------------------------
typedef struct{
    double items[MAX];
    int top;
} Operands;

void initOperand(Operands* s);
int pushOperand(Operands* s, double value);
double popOperand(Operands* s);
double peekOperand(Operands* s);
bool isEmptyOperand(Operands* s);
void printOperand(Operands* s);

// Operator Stack --------------------------------------
typedef struct{
    char items[MAX];
    int top;
} Operators;

void initOperator(Operators* s);
int pushOperator(Operators* s, char value);
char popOperator(Operators* s);
char peekOperator(Operators* s);
bool isEmptyOperator(Operators* s);
void printOperand(Operands* s);
void printOperator(Operators* s);

#endif