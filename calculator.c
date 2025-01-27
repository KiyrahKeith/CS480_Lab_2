#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#include "stack.h"
#include "calculator.h"

int main() {
    printf("Welcome to the Calculator\n");

    char *input = NULL; //The expression input from the user
    int inputLength; //Stores the result of getting the user input
    
    //Continue accepting input until the user enters the exit code
    while(1) {
        inputLength = getInput(&input);
        //If the user 
        if(inputLength == -2) {//User wants to exit
            printf("Thank you for using the calculator. Goodbye.\n");
            return 0;
        } else if(inputLength == -1) {//An error has occurred
            continue;//Restart the loop to get new input
        } else {//The user input was successfully received
            //Create a string array to store the postfix version of the user input
            char* postFix = (char *)malloc(inputLength*sizeof(char));
            if(convertToPostFix(&input, &postFix) == -1) {
                continue; //The input was invalid. Accept new input again.
            } else {
                evaluateExpression(&input);
            }
        }
    }
}

/*
* Get user input from the console
* exp: A pointer containing a dynamically-allocated char array to store the user input
* Returns expression length if expression was valid. Example: "2+5" would return 4 (3 characters and a return char)
* Returns -1 if there was an error or expression was invalid
* Returns -2 if the user wants to exit
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
            return -2;
        }

        if(length + 1 >= capacity) {//If the user input is about to overflow the pointer
            capacity *= 2;//Expand the available capacity
            (*exp) = (char *)realloc((*exp), capacity * sizeof(char));
            if(exp == NULL) {
                printf("Memory allocation failed.\n");
                return -1;
            }
        }
        (*exp)[length] = ch;
        length++;
    }
    (*exp)[length] = '\0';//Add the escape character to the end
    printf("User input: %s\n",(*exp));
    return length;
}

double evaluateExpression(char** exp) {
    printf("Evaluate Expression: %s\n", (*exp));
    return 0;
}

/*
Convert a standard infix expression to postFix
Returns -1 if an error has occurred or input was invalid
*/
int convertToPostFix(char** input, char** postFix) {
    Operators operators;
    initOperator(&operators);

    Operands operands;
    initOperand(&operands);

    //Variables to keep track of current location in arrays
    int postFixIndex = 0;
    int inputIndex = 0;

    while((*input)[inputIndex] != '\0') {
        char token = (*input)[inputIndex];
        if(isNumber(token)) {
            double value = findNumber((*input), &inputIndex);
            pushOperand(&operands, value);
            //printf("The number found was: %lf\n", value);
        } else if(isOperator(token)) {
            char op = findOperator((*input), &inputIndex);
            if(op == '\0') {
                printf("Invalid Operator\n");
                return -1;
            }
            pushOperator(&operators, op);
            //printf("The operator found was: %c\n", op);
        } else {
            printf("Invalid input\n");
            return -1;
        }
    }
    
    printOperand(&operands);
    printOperator(&operators);
    return 0;
}

bool isNumber(char value) {
    if(value == '.' || isdigit(value)) return true;
    else return false;
}

/*
Parses through the expression to find the full decimal number starting at index i.
After the function executes, i will be moved to the next valid index after the number.
*/
double findNumber(char* exp, int* i) {
    double result = 0.0;
    bool isDecimal = false;
    double divisor = 1.0;

    //Source for converting from character to int using - '0'
    //https://www.geeksforgeeks.org/c-program-for-char-to-int-conversion/

    //Keep looking through the expression until a non-number is found
    while (isNumber(exp[*i])) {
        if(exp[*i] == '.') {
            isDecimal = true;//Flip the flag to indicate we are adding the decimal portion
        } else {
            if(isDecimal) {
                divisor *= 10; //Shift the divisor to the next decimal place
                result += (double)(exp[*i] - '0') / divisor;
            } else {
                result = result * 10 + (exp[*i] - '0');
            }
        }
        (*i)++;//Move to the next character
    }

    return result;
}

/*
Checks if a character (value) is the first character of any of the available operators
Returns true if value does begin an operator
Returns false if value is not an operator
*/
bool isOperator(char value) {
    if(value == '+' || value == '-' ||
        value == '*' || value == '/' ||
        value == '^' || value == 's' || 
        value == 'c' || value == 't' || 
        value == 'l') {
            return true;
        } else {
            return false;
        }
}

/*
Parses through the expression to find the full operator. 
After the function executes, i will be moved to the next valid index after the number.
Returns '\0' if the operator is invalid
*/
char findOperator(char* exp, int* i) {
    //Check for any of the single-character operators
    if(exp[*i] == '+' || exp[*i] == '-' || exp[*i] == '*' || exp[*i] == '/' || exp[*i] == '^') {
        return exp[(*i)++];//Return the operator and move to the next index
    } 

    if(exp[*i] == 's') {
        if(exp[*i+1] == 'i' && exp[*i+2] == 'n') {//Verify that s was the start of the correct "sin" input
            *i += 3;//Move to the end of sin
            return 's';
        } else {
            return '\0';
        }
    }

    if(exp[*i] == 'c') {
        if(exp[*i+1] == 'o' && exp[*i+2] == 's') {//Verify that c was the start of the correct "cos" input
            *i += 3;//Move to the end of cos
            return 'c';
        } else {
            return '\0';
        }
    }

    if(exp[*i] == 't') {
        if(exp[*i+1] == 'a' && exp[*i+2] == 'n') {//Verify that t was the start of the correct "tan" input
            *i += 3;//Move to the end of tan
            return 't';
        } else {
            return '\0';
        }
    }

    if(exp[*i] == 'l') {
        if(exp[*i+1] == 'n') { //Check if the input is lin
            *i += 2; //Move to the end of ln
            return 'n'; //n character represents ln
        } else if(exp[*i+1] == 'o' && exp[*i+2] == 'g') {
            *i += 3; //Move to the end of log
            return 'l'; //l character represents log_10
        } else {
            return '\0'; // Return an error if the l was not part of "ln" or "log"
        }
    }

    return '\0';//No valid operator was found
}

/*
Get the order of operations precedence for each operator
*/
int precedence(char op) {
    if(op == '(' || op == ')') return 6;
    if(op == '{' || op == '}') return 5;
    if(op == '^') return 4;
    if(op == 's' || op == 'c' || op == 't' || op == 'n' || op == 'l') return 3;
    if(op == '*' || op == '/') return 2;
    if(op == '+' || op == '-') return 1;
    return 0;
}
