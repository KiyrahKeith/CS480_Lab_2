#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include "calculator.h"

int main() {
    printf("Welcome to the Calculator\n");

    char *input = NULL; //The expression input from the user
    int isValid; //Stores the result of getting the user input
    
    //Continue accepting input until the user enters the exit code
    while(1) {
        isValid = getInput(&input);
        //If the user 
        if(isValid == -1) {//User wants to exit
            printf("Thank you for using the calculator. Goodbye.\n");
            return 0;
        } else if(isValid == 0) {//An error has occurred
            continue;//Restart the loop to get new input
        } else {//The user entered a valid expression
            evaluateExpression(&input);
        }
    }
}

/*
* Get user input from the console
*
* Returns 1 if expression was valid
* Returns 0 if there was an error or expression was invalid
*/
int getInput(char** exp) {
    //Reset the expression pointer to ensure that the memory is ready to use
    free((*exp));
    (*exp) = (char *)malloc(INITIAL_CAPACITY*sizeof(char));
    if(exp == NULL) {
        printf("Memory allocation failed.\n");
        return 0;
    }
    printf("Enter the calculation. Type x to exit.\n");

    int length = 0;
    int capacity = INITIAL_CAPACITY;
    char ch;
    while((ch = getchar()) != '\n' && ch != EOF) {
        if(ch == 'x') {//If the user inputs the exit character
            return -1;
        }

        if(length + 1 >= capacity) {//If the user input is about to overflow the pointer
            capacity *= 2;//Expand the available capacity
            (*exp) = (char *)realloc((*exp), capacity * sizeof(char));
            if(exp == NULL) {
                printf("Memory allocation failed.\n");
                return 0;
            }
        }
        (*exp)[length] = ch;
        length++;
    }
    printf("User input: %s\n",(*exp));
    return 1;
}

double evaluateExpression(char** exp) {
    printf("Evaluate Expression: %s\n", (*exp));
    return 0;
}