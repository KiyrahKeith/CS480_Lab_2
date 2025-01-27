#include <stdio.h>
#include <float.h>
#include <stdbool.h>
#include <string.h>
#include "stack.h"

// Operand Stack --------------------------------------

void initOperand(Operands* s) {
    s->top = -1; 
}

int pushOperand(Operands* s, double value) {
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

void printOperand(Operands* s) {
    printf("Operands: ");
    for(int i = 0; i <= s->top; i++) {
        printf("%f > ", s->items[i]);
    }
    printf("\n");
}

// Operator Stack --------------------------------------


void initOperator(Operators* s) {
    s->top = -1; 
}

int pushOperator(Operators* s, char value) {
    if(s->top >= MAX) {
        printf("Overflow Error: Too many operands.\n");
        return 1;
    } 

    s->items[++(s->top)] = value; //Duplicate the string to store it in the stack
    return 0;
}

char popOperator(Operators* s) {
    if(s->top == -1) {
        printf("Error: Cannot pop from an empty stack.\n");
        return '\0';
    }

    return s->items[(s->top)--];
}

char peekOperator(Operators* s) {
    if(s->top == -1) {
        return '\0';
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

void printOperator(Operators* s) {
    printf("Operators: ");
    for(int i = 0; i <= s->top; i++) {
        printf("%c > ", s->items[i]);
    }
    printf("\n");
}