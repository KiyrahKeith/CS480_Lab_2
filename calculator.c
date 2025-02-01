#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
#include "stack.h"
#include "calculator.h"

int main() {
    printf("Welcome to the Calculator\n");

    char *input = NULL; //The expression input from the user
    int inputLength; //Stores the result of getting the user input
    
    //Continue accepting input until the user enters the exit code
    while(1) {
        inputLength = getInput(&input);
        if(inputLength == -2) {//User typed 'x' and wants to exit
            printf("Thank you for using the calculator.\n");
            return 0;

        } else if(inputLength == -1) {//An error has occurred from the user input
            continue;//Restart the loop to get new input

        } else {//The user input was successfully received
            double result = evaluateExpression(&input);
            if(!isnan(result)) {
                //It is using %.*g to print up to 15 digits after the decimal point, but also truncating excess 0's. 
                printf("Result: %.*g\n", 15, result);
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

        //ignore spaces and tabs
        if(ch == ' ' || ch == '\t') {
            continue;
        }

        if(length + 1 >= capacity) {//If the user input is about to overflow the pointer
            capacity *= 2;//Expand the available capacity
            (*exp) = (char *)realloc((*exp), capacity * sizeof(char));
            if(exp == NULL) {
                printf("Memory allocation failed.\n");
                return -1;
            }
        }
        (*exp)[length] = tolower(ch);
        length++;
    }
    (*exp)[length] = '\0';//Add the escape character to the end
    return length;
}


/*
Evaluate a string of infix mathematical expression
Returns a double of the expression result
Returns NAN if an error has occurred or input was invalid
*/
double evaluateExpression(char** input) {
    Operators operators;
    initOperator(&operators);

    Operators braceStack;
    initOperator(&operators);//Stores () and {} to ensure the pairs match correctly

    Operands operands;
    initOperand(&operands);

    //Variables to keep track of current location in arrays
    int inputIndex = 0;

    while((*input)[inputIndex] != '\0') {
        char token = (*input)[inputIndex];
        if(isNumber(token)) {
            double value = findNumber((*input), &inputIndex);
            if(isnan(value)) {
                return NAN;
            }
            pushOperand(&operands, value);
        } else if(isOperator(token)) {
            char op = '\0';//Stores the current operator
            if(token == '-') {
                //If the previous token is an operator or the start of the expression or an opening brace, use unary minus
                if(inputIndex == 0 || isOperator((*input)[inputIndex-1]) || (*input)[inputIndex-1] == '(' || (*input)[inputIndex-1] == '{') {
                    //A unary minus cannot be the end of the expression or followed by a closing bracket
                    //A unary minus cannot be followed by a binary operator
                    if((*input)[inputIndex+1] == '\0' || 
                        (*input)[inputIndex+1] == ')' || 
                        (*input)[inputIndex+1] == '}' ||
                        (isOperator((*input)[inputIndex+1]) && !isUnary((*input)[inputIndex+1]))) {
                            printf("Error: Improper use of minus.\n");
                            return NAN;
                        }
                    op = 'm';
                    inputIndex++;//Move to the next character
                } else {//Otherwise, use binary minus
                    op = findOperator(*input, &inputIndex);
                }
                
            } else {
                op = findOperator(*input, &inputIndex);
            }

            if(op == '\0') {
                printf("Error: Invalid Operation\n");
                return NAN;
            }
            while(operators.top >= 0 && 
                    peekOperator(&operators) != '(' && 
                    peekOperator(&operators) != '{' && 
                    precedence(peekOperator(&operators)) >= precedence(op)
            ) {
                char stackOp = popOperator(&operators);
                if(isUnary(stackOp) || stackOp == 'm') {
                    double a = popOperand(&operands);
                    pushOperand(&operands, evaluateOp(stackOp, a, 0));
                } else {
                    double b = popOperand(&operands);
                    double a = popOperand(&operands);
                    pushOperand(&operands, evaluateOp(stackOp, a, b));
                }
            }

            pushOperator(&operators, op);
        } else if(token == '(' || token == '{') {
            //An open bracket can't be the last character in an expression
            //and an open brack can't be immediately followed by a closing bracket
            if((*input)[inputIndex+1] == '\0' ||
                (*input)[inputIndex+1] == ')' ||
                (*input)[inputIndex+1] == '}') {
                printf("Error: Invalid use of brackets.\n");
                return NAN;
            }
            pushOperator(&operators, token);
            pushOperator(&braceStack, token);
            inputIndex++;
        } else if(token == ')' || token == '}') {
            //If the opening and closing brackets don't match in type, return an error
            if((token == ')' && peekOperator(&braceStack) != '(') ||
                (token == '}' && peekOperator(&braceStack) != '{')) {
                printf("Error: opening and closing brackets must match.\n");
                return NAN;
            }
            while(operators.top >= 0 && peekOperator(&operators) != '(' && peekOperator(&operators) != '{') {
                char stackOp = popOperator(&operators);
                double result = NAN;
                if(isUnary(stackOp) || stackOp == 'm') {
                    double a = popOperand(&operands);
                    result = evaluateOp(stackOp, a, 0);
                    if(isnan(result)) {
                        return NAN;
                    } else {
                        pushOperand(&operands, result);
                    }
                } else {
                    double b = popOperand(&operands);
                    double a = popOperand(&operands);
                    result = evaluateOp(stackOp, a, b);
                    if(isnan(result)) {
                        printf("Error: Invalid operation.\n");
                        return NAN;
                    } else {
                        pushOperand(&operands, result);
                    }
                }
            }
            popOperator(&operators); //Pop the '(' or '{'
            popOperator(&braceStack);
            inputIndex++;
        } else {
            printf("Error: Invalid input\n");
            return NAN;
        }
    }

    while (operators.top >= 0) {
        char op = popOperator(&operators);
        double result = NAN;
        if(isUnary(op) || op == 'm') {
            double a = popOperand(&operands);
            result = evaluateOp(op, a, 0);
            if(isnan(result)) {
                printf("Error: Invalid operation.\n");
                return NAN;
            } else {
                pushOperand(&operands, result);
            }
        } else {
            double b = popOperand(&operands);
            double a = popOperand(&operands);
            result = evaluateOp(op, a, b);
            if(isnan(result)) {
                printf("Error: Invalid operation.\n");
                return NAN;
            } else {
                pushOperand(&operands, result);
            }
        }
    }

    double result = popOperand(&operands); //Get the result from the last operand on the stack
    return result;
}

bool isNumber(char value) {
    if(value == '.' || isdigit(value)) return true;
    else return false;
}

/*
Parses through the expression to find the full decimal number starting at index i.
After the function executes, i will be moved to the next valid index after the number.
Returns NAN if the number is invalid
*/
double findNumber(char* exp, int* i) {
    double result = 0;
    bool isDecimal = false;
    double divisor = 1.0;
    int sigDigits = 0; //Keeps track of the number of significant digits found. 
    //This number can't be over 15 because the calculations will lose precision for doubles.

    //Source for converting from character to int using - '0'
    //https://www.geeksforgeeks.org/c-program-for-char-to-int-conversion/

    //Keep looking through the expression until a non-number is found
    while (isNumber(exp[*i])) {
        if(exp[*i] == '.') {
            //If the decimal flag has already been tripped and there's another decimal point, this is invalid
            if(isDecimal) {
                printf("Error: Numbers cannot contain multiple decimal points.\n");
                return NAN;
            } 

            //A period cannot be the final character in the expression
            //and a period must be followed by a digit
            if(exp[*i+1] == '\0' || !isdigit(exp[*i+1])) {
                printf("Error: Numbers cannot end in a period.\n");
                return NAN;
            }

            isDecimal = true;//Flip the flag to indicate we are adding the decimal portion
        } else {
            sigDigits++;
            if(sigDigits > 15) {
                printf("Error: Number contains too many significant digits.\n");
                return NAN;
            }
            if(isDecimal) {
                divisor *= 10; //Shift the divisor to the next decimal place
                result += (double)(exp[*i] - '0') / divisor;
            } else {
                //If multiplying by 10 will cause the number to exceed maximum
                if(result > DBL_MAX/10) {
                    printf("Error: Number is too large.\n");
                    return NAN;
                }
                result *= 10;
                //If adding the additional digit will cause the number to exceed maximum
                if(result > DBL_MAX - (exp[*i] - '0')) {
                    printf("Error: Number is too large.\n");
                    return NAN;
                }
                result += (exp[*i] - '0');
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
bool isOperator(char op) {
    if(op == '+' || op == '-' ||
        op == '*' || op == '/' ||
        op == '^' || op == 's' || 
        op == 'c' || op == 't' || 
        op == 'o' || 
        op == 'l' || op == 'n') {
            return true;
        } else {
            return false;
        }
}

/*
Checks if an operator is unary or binary. 
Examples of unary operators are: sin, cos, tan, ln, and log
Returns true if an operator is unary, returns false otherwise.*/
bool isUnary(char op) {
    if(op == 's' || 
        op == 'c' || op == 't' || 
        op == 'o' || 
        op == 'l' || op == 'n') {
            return true;
        } else {
            return false;
        }
}

/*
Parses through the expression to find the full operator. 
After the function executes, i will be moved to the next valid index after the number.
Returns '\0' if the operator is invalid or if the preceding or next characters are not valid
*/
char findOperator(char* exp, int* i) {
    //Both a binary operator or unary operator must have at least one token after it
    if(exp[*i+1] == '\0') {
        return '\0';
    }
    //Check for any of the single-character operators
    if(!isUnary(exp[*i])) {//If it is a binary, single digit operator
        //Binary operators must be followed by a digit, an open brace, or a unary operator to be valid
        if(isdigit(exp[(*i)+1]) || exp[(*i)+1] == '(' || exp[(*i)+1] == '{' || isUnary(exp[(*i)+1]) || exp[(*i)+1] == '-') {
            //Operators must be preceded by a digit or closing brace to be valid
            if(isdigit(exp[(*i)-1]) || exp[(*i)-1] == ')' || exp[(*i)-1] == '}') {
                return exp[(*i)++];//Return the operator and move to the next index
            } else {
                return '\0'; //The preceding character was invalid
            }
            
        } else {
            return '\0';//The following character was invalid
        }
    }

    //Unary operators must be the first thing in the expression, preceded by a binary operator, or preceded by an opening brace
    if((*i) != 0) { //If the unary operator isn't the first character
        if((isOperator(exp[*i-1]) && isUnary(exp[*i-1])) || 
            exp[*i-1] == '}' || exp[*i-1] == ')' ||
            isdigit(exp[*i-1])) {
            return '\0';
        }
    }
    char result = '\0';//Stores the single-digit operator that will be returned if it is valid.


    switch(exp[*i]) {
        case 's': //sin
            if(exp[*i+1] == 'i' && exp[*i+2] == 'n') {//Verify that s was the start of the correct "sin" input
                *i += 3;//Move to the end of sin
                result = 's';
            } else {
                return '\0';
            }
            break;
        case 'c': //cos
            if(exp[*i+1] == 'o' && exp[*i+2] == 's') {//Verify that c was the start of the correct "cos" input
                *i += 3;//Move to the end of cos
                result = 'c';
            } else {
                if(exp[*i+1] == 'o' && exp[*i+2] == 't') {//Verify that c was the start of the correct "cot" input
                *i += 3;//Move to the end of cot
                result = 'o';//The single-digit symbol for cot is 'o'
                } else {
                    return '\0';
                }
            }
            break;
        case 't': //tan
            if(exp[*i+1] == 'a' && exp[*i+2] == 'n') {//Verify that t was the start of the correct "tan" input
                *i += 3;//Move to the end of tan
                result = 't';
            } else {
                return '\0';
            }
            break;
        case 'l': //ln and log
            if(exp[*i+1] == 'n') { //Check if the input is lin
                *i += 2; //Move to the end of ln
                result =  'n'; //n character represents ln
            } else if(exp[*i+1] == 'o' && exp[*i+2] == 'g') {
                *i += 3; //Move to the end of log
                result = 'l'; //l character represents log_10
            } else {
                return '\0'; // Return an error if the l was not part of "ln" or "log"
            }
            break;
        
        default: return '\0';
    }

    //By this point, i will already be placed at the next token in the expression
    //Operators must be followed by a digit or an open brace to be valid
    if(!isdigit(exp[(*i)]) && exp[(*i)] != '(' && exp[(*i)] != '(') {
        return '\0';
    }

    return result;//Return the single digit operator if valid and return '\0' if not valid.
}

/*
Get the order of operations precedence for each operator
*/
int precedence(char op) {
    if(op == '(' || op == ')' || op == '{' || op == '}') return 6;
    if(op == 's' || op == 'c' || op == 't'  || op == 'o' || op == 'n' || op == 'l') return 5;
    if(op == 'm') return 4;
    if(op == '^') return 3;
    if(op == '*' || op == '/') return 2; //Unary minus is treated as (-1*) so it has the same precedence as multiplication
    if(op == '+' || op == '-') return 1;
    return 0;
}

/*
Performs the requested operation on one or two operands. 
For unary operators, it completes the calculation using the a parameter.
Returns the double result of the operation. 
Returns NAN if the operation is invalid.
 */
double evaluateOp(char op, double a, double b) {
    double result;
    switch(op) {
        case '+': 
            if(a > DBL_MAX - b) {//Check if the addition will overflow the double
                return NAN;
            } else {
                return a+b;
            }
        case '-': 
            if(a > NAN + b) {//Check if the subtraction will overflow the negative double
                return NAN;
            } else {
                return a-b;
            }
        case '*': 
            if(abs(a) > DBL_MAX/abs(b)) {//Check if the multiplication with overflow double
                return NAN;
            } else {
                return a*b;
            }
        case '/': 
            if (b == 0) {
                return NAN;
            } else {
                return a/b;
            }
        case '^': 
            result = pow(a, b);
            if(isinf(result)) {//Check for exponent overflow.
                return NAN;
            } else {
                return result;
            }
        case 's': return sin(a);
        case 'c': return cos(a);
        case 't': return tan(a);
        case 'o': 
            if(tan(a) == 0) {//Check if there will be a division by 0 error
                return NAN;
            } else {
                return 1.0/tan(a);
            }
        case 'n': 
            //Natural logarithm is only defined for positive numbers.
            if (a > 0) {
                return log(a);
            } else {
                return NAN;
            }
        case 'l':
            //Logarithm is only defined for positive numbers.
            if (a > 0) {
                return log10(a);
            } else {
                return NAN;
            }
        case 'm': return -1*a; //Unary minus, negate one operand
        default: return NAN;
    }
}
