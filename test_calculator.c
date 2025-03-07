#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXPECTED_RESULT 100

#define VALID_EXPRESSIONS "valid_expressions.csv"
#define INVALID_EXPRESSIONS "invalid_expressions.csv"
#define VALID_EXPRESSIONS_OUTPUT "failed_valid_expressions.csv"
#define INVALID_EXPRESSIONS_OUTPUT "failed_invalid_expressions.csv"

int testExpressions(char* fileName, char* outputFileName, char* title, int maxLength) {
    FILE *file;
    char line[maxLength];
    char expression[maxLength];
    char expected_result[MAX_EXPECTED_RESULT];

    // Open the expressions file in read-only mode
    file = fopen(fileName, "r");
    if (file == NULL) { //Test for an error in opening the file.
        perror("Error opening file");
        return 1;
    }

    while (fgets(line, sizeof(line), file)) {
        // Split the line into two parts (expression and expected_result)
        // strtok will split the line by commas
        char *token = strtok(line, ",");
        if (token != NULL) {
            strncpy(expression, token, maxLength);
            expression[maxLength - 1] = '\0';  // Ensure null-termination
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            strncpy(expected_result, token, MAX_EXPECTED_RESULT);
            expected_result[MAX_EXPECTED_RESULT - 1] = '\0';  // Ensure null-termination
        }
        printf("Expression: %s, Expected Result: %s\n", expression, expected_result);
    }

    fclose(file);
    return 0;
}

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

    if(testExpressions(VALID_EXPRESSIONS, VALID_EXPRESSIONS_OUTPUT, "Valid Expressions", maxLength)) {
        printf("Error testing valid expressions.\n");
        return 1;
    }
}