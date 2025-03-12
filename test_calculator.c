#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "calculator.h"

//#define MAX_EXPECTED_RESULT 100
#define ACCURACY 3 //The number of rounding digits of accuracy that must be met for an expression result to be classified as "equal"

#define VALID_EXPRESSIONS "valid_expressions.csv"
#define INVALID_EXPRESSIONS "invalid_expressions.csv"
#define VALID_EXPRESSIONS_OUTPUT "failed_valid_expressions.csv"
#define INVALID_EXPRESSIONS_OUTPUT "failed_invalid_expressions.csv"

/**
Rounds a double value to a certain number of precision. If the value is NaN, then NaN is returned instead.
@param value The double number to be rounded
@param precision The number of digits that value will be rounded to
@return The rounded version of value or NaN*/
double roundValue(double value, int precision) {
    if(isnan(value)){//If the value is already not a number (NaN), do not try to round it
        return value;
    }
    double precisionFactor = pow(10.0, precision);
    return round(value*precisionFactor) / precisionFactor;
}

/**
Compares the results of a sample expression to the result obtained by the evaluateExpression method of calculator.c
Numerical accuracy for tolerance of how closely expressions must match uses the ACCURACY value define at the top. 
ACCURACY determines the number of decimal points required that must match for two values to be considered "equal". 

@param expression The mathematical expression to be tested
@param expectedResult The value of the result that should be obtained from the test expression.
@param actualResult The result that is obtained from the calculator. It is a pointer so that it can be passed back to the calling function
@return true if the results match, and returns false otherwise. 
*/
bool compareExpression(char* expression, double expectedResult, double* calculatorResult) {
    //printf("Compare - Expression: %s\n", expression);
    expectedResult = roundValue(expectedResult, ACCURACY); //Round the expected result to a certain number of decimal places.

    //Suppress calculator print output temporarily. This will prevent it from returning error messages for invalid expressions.
    freopen("/dev/null", "w", stdout); 

    *calculatorResult = evaluateExpression(&expression);

    //Stop suppressing terminal output
    freopen("/dev/tty", "w", stdout);
    

    *calculatorResult = roundValue(*calculatorResult, ACCURACY);
    //printf("Actual Result: %f, status: %i\n", calculatorResult, expectedResult == calculatorResult);

    //Returns true if either the expected and actual results match numerically
    //or returns true if both actual and expected are nan
    return expectedResult == *calculatorResult || (isnan(expectedResult) && isnan(*calculatorResult));
}

/**
Reads a single csv file and compares every expression to the expression in the CSV file.
Once all expression are tested, the successful expression statistics are printed to the screen.
@param fileName The path of the CSV file to be read
@param outputFileName The path of the CSV file to be output that contains non-matching expressions.
@param title The title to be output to the screen describing what the statistics represent
@param maxLength The longest expected line to be read from the CSV file. 

@return 0 if expression evaluation was successful. Returns 1 if any errors occured. 
*/
int testExpressions(char* fileName, char* outputFileName, char* title, int maxLength) {
    FILE *file;
    char line[maxLength];
    char expression[maxLength];
    char expected_result[maxLength];
    double calculator_result = nan(""); // Stores the result from the calculator
    bool isMatching;//Stores the comparison result of each expression test. True if the expression matches, false otherwise.
    int numMatching = 0;//The number of expressions that match the expected result
    int numNotMatching = 0;//The number of expressions that do not match the expected result

    // Open the expressions file in read-only mode
    file = fopen(fileName, "r");
    if (file == NULL) { //Test for an error in opening the file.
        perror("Error opening file");
        return 1;
    }

    // Check for BOM (Byte Order Mark) at the start of the file (UTF-8 BOM is 0xEF 0xBB 0xBF)
    // Reference: https://en.wikipedia.org/wiki/Byte_order_mark
    unsigned char bom[3];
    size_t bytesRead = fread(bom, 1, 3, file);  // Read the first 3 bytes
    if (bytesRead == 3 && bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF) {
        // BOM found, move the file pointer 3 bytes forward to skip the BOM
        fseek(file, 3, SEEK_SET);
    } else {
        // No BOM found, reset the file pointer to the beginning
        fseek(file, 0, SEEK_SET);
    }

    FILE *outputFile = fopen(outputFileName, "w");
    if(outputFile == NULL) {
        perror("Error opening output file");
        return 1;
    }
    fputs("Expression,Calculator Result,Actual Result\n", outputFile); // Add the header row to the output file to improve readability

    while (fgets(line, sizeof(line), file)) {
        //printf("Line: %s\n", line);
        line[strcspn(line, "\n")] = '\0'; //Replace any newline characters with termination characters if applicable

        // Skip empty lines or lines with only whitespace
        if (strlen(line) == 0 || strlen(line) == 1 || strspn(line, " \t") == strlen(line)) {
            continue;  // Skip the empty or whitespace-only line
        }

        // Split the line into two parts (expression and expected_result)
        // strtok will split the line by commas
        char *token = strtok(line, ",");
        int tokenLength = strlen(token);
        
        //If the expression read from the CSV is too long, return an error. 
        if(tokenLength >= maxLength) {
            printf("Error: expression read from CSV is too long. Expression length: %i\n", tokenLength);
        }
        if (token != NULL) {
            strncpy(expression, token, tokenLength);
            expression[tokenLength] = '\0';  // Ensure null-termination
        }
        token = strtok(NULL, ",");
        tokenLength = strlen(token);
        
        //If the expected_result read from the CSV is too long, return an error. 
        if(tokenLength >= maxLength) {
            printf("Error: expected result read from CSV is too long. Result length: %i\n", tokenLength);
        }
        if (token != NULL) {
            strncpy(expected_result, token, tokenLength);
            expected_result[tokenLength] = '\0';  // Ensure null-termination
        }
        char* resultEndPtr;//Used in the conversion of expected_result from string to double

        bool isMatching = false;
        if(expected_result == "nan") { // Compare invalid expressions
            if isnan(calculator_result) { // If it was expected to be nan and it is nan
                isMatch = true; // Then that's a valid outcome
            } else {
                isMatching = false; //If it was expected to be nan but the calculator result isn't nan, that means it's invalid
            }
        } else { // Compare valid expressions
            bool isMatching = compareExpression(expression, strtod(expected_result, &resultEndPtr), &calculator_result);
        }
        
        if(isMatching) numMatching++;
        else {//If the expression expected result did not match the calculator's results
            fprintf(outputFile, "%s, %f, %s", expression, calculator_result, expected_result); //Append the bad expression to the end of the output file so that it can reviewed later
            numNotMatching++; //Increase the number of expressions that evaluated incorrectly
        }
    }
    fclose(file); //Close the file after it has been read

    printf("%s\n", title);
    printf("Passed tests: %.2f%% \n", (double)numMatching/(numMatching+numNotMatching) * 100);
    printf("# Correct Evaluations: %i\n", numMatching);
    printf("# Incorrect Evaluations: %i\n", numNotMatching);
    return 0;
}

/**
The entry point of the calculator testing program. This program tests valid and invalid expressions by comparing previously-generated
CSV files of expressions and expected results to the numerical output of calculator.c

@param argc The number of command line parameters provided. This should be 4 in this test program. 
@param argv An array of command line parameters provided. This should include:
    1. the number of valid samples
    2. the number of invalid samples
    3. the maximum length of a single line in either CSV file.

@return 0 if the program exits without error. Returns 1 if an error occurs.*/
int main(int argc, char *argv[]) {
    if(argc != 4) {//Test if there are 3 command line arguments provided.
        printf("Error: Please provide three command line arguments - \n 1. # valid samples \n 2. # invalid samples \n 3. maximum length of a single expression.\n");
        return 1;
    }

    int n_valid = atoi(argv[1]);
    if(n_valid <= 0) {
        printf("Error: # valid samples must be an integer greater than 0.\n");
        return 1;
    }

    int n_invalid = atoi(argv[2]);
    if(n_invalid <= 0) {
        printf("Error: # invalid samples must be an integer greater than 0.\n");
        return 1;
    }

    int maxLength = atoi(argv[3]);
    if(maxLength <= 0) {
        printf("Error: maximum expression length must be an integer greater than 0.\n");
        return 1;
    }

    // Before testing valid expressions, check if the output file already exists and delete it
    if (access(VALID_EXPRESSIONS_OUTPUT, F_OK) == 0) {
        // If the file exists, delete it
        if (remove(VALID_EXPRESSIONS_OUTPUT) != 0) {
            perror("Error deleting file");
        }
    }

    // Test the valid expressions
    if(testExpressions(VALID_EXPRESSIONS, VALID_EXPRESSIONS_OUTPUT, "*********Valid Expressions*********", maxLength)) {
        printf("Error testing valid expressions.\n");
        return 1;
    }

    printf("\n");

    // Test the invalid expressions
    if(testExpressions(INVALID_EXPRESSIONS, INVALID_EXPRESSIONS_OUTPUT, "*********Invalid Expressions*********", maxLength)) {
        printf("Error testing invalid expressions.\n");
        return 1;
    }
}