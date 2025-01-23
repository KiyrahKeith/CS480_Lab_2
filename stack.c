#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#define MAX 100 // The maximum number of operands and/or operators valid in the expression

// Operand Stack --------------------------------------
typedef struct{
    double items[MAX];
    int top;
} Operands;

void initOperand(Operands* s) {
    s->top = -1; 
}

int pushOperand(Operands* s, double value) {
    s->top++;
    if(s->top >= MAX) {
        printf("Overflow Error: Too many operands.\n");
        return 1;
    } 

    s->items[++(s->top)] = value;
    return 0;
}

double popOperand(Operands* s) {
    if(s->top == -1) {
        printf("Error: Cannot pop from an empty stack.\n");
        return DBL_MIN;
    }

    return s->items[(s->top)--];
}

double peekOperand(Operands* s) {
    if(s->top == -1) {
        return DBL_MIN;
    }
    return s->items[s->top];
}

bool isEmptyOperand(Operands* s) {
    if(s->top == -1) {
        return true;
    } else {
        return false;
    }
}


// Operator Stack --------------------------------------
typedef struct{
    char* items[MAX];
    int top;
} Operators;

void initOperator(Operators* s) {
    s->top = -1; 
}

int pushOperator(Operators* s, char* value) {
    s->top++;
    if(s->top >= MAX) {
        printf("Overflow Error: Too many operands.\n");
        return 1;
    } 

    s->items[++(s->top)] = strdup(value); //Duplicate the string to store it in the stack
    return 0;
}

char* popOperator(Operators* s) {
    if(s->top == -1) {
        printf("Error: Cannot pop from an empty stack.\n");
        return NULL;
    }

    return s->items[(s->top)--];
}

char* peekOperator(Operators* s) {
    if(s->top == -1) {
        return NULL;
    }
    return s->items[s->top];
}

bool isEmptyOperator(Operators* s) {
    if(s->top == -1) {
        return true;
    } else {
        return false;
    }
}