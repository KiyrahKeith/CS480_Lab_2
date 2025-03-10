import csv
import sys 
import random

############### Define CSV reading and writing functions ###############
VALID_FILENAME = "valid_expressions.csv"
INVALID_FILENAME = "invalid_expressions.csv"
CHAR_FILENAME = "char_matrix.csv"

# Write expressions to a CSV file
def writeCSV(fileName, data):
    with open(fileName, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(data)
    
    print(fileName, "successfully written.")

def readCSV(fileName):
    data = [] # Stores the contents of the CSV file

    try:
        with open(fileName, mode='r', newline='', encoding='utf-8') as file:
            csv_reader = csv.reader(file)

            for row in csv_reader:
                data.append(row) # Read all csv data into the data variable
    except FileNotFoundError: # Handle error if the file could not be found
        print(f"Error: the file '{fileName}' could not be found.")
    except Exception as e: # Handle any other file reading error
        print(f"File reading error: {e}")

    return data

################## Get command line parameters ##########################
if len(sys.argv) != 4:
    print("Error: script must be run with 3 command line arguments: \n1. # valid expressions, \n2. # invalid expressions \n3. maximum length of a single expression.")
    sys.exit(1) 

try:
    numValid = int(sys.argv[1])
    if numValid <= 0:
        print("Error: Number of Valid expressions must be a positive integer.")
        sys.exit(1)

    numInvalid = int(sys.argv[2])
    if numInvalid <= 0:
        print("Error: Number of Invalid expressions must be a positive integer.")
        sys.exit(1)

    maxLength = int(sys.argv[3])
    if maxLength <= 0:
        print("Error: Maximum expression length must be a positive integer.")
        sys.exit(1)
except ValueError:
    print("All command line arguments must be integers.")
    sys.exit(1)
###########################################################################
# Tests if an operator is multi-digit (such as sin, cos, ln). 
# @param op The operator to be converted
# @return If op is multi-digit, getOp returns the full string of the operator. 
#         If op is single-digit (+, -, etc), returns op with no change
def getOp(op):
    match op:
        case 's':
            return 'sin'
        case 'c':
            return 'cos'
        case 't':
            return 'tan'
        case 'o':
            return 'cot'
        case 'l':
            return 'log'
        case 'n':
            return 'ln'
        case _:
            return op

# Selects a random character from the char_data table
# @param isValid A boolean representing whether the final expression must be valid (true) or invalid (false)
# @param char_data provides information about the available math characters and their properties
# @param rowNum The row number to determine whether the choice is valid or invalid. This tells either start, end, or previous char. 
# @param choiceRange A list of available index options based on the number of characters listed in char_data
# @param isEnd True if this character is the final char in the expression. These must be valid for both the previous row and also the ending condition
# @return The index of an appropriate randomized character in char_data
def randomizeChoice(isValid, char_data, rowNum, choiceRange, isEnd):
    choice = -1 # The index of the randomized choice
    if isValid: 
        while choice == -1: # Continue randomly selecting a choice until a valid one is found
            choice = random.choice(choiceRange)
            # print(f"Choice:{choice}, op:{char_data[rowNum][choice]}")
            if char_data[rowNum][choice] == '1': # If the random pick was a valid choice
                if isEnd: # If the character is an ending character, it has an extra condition
                    if char_data[2][choice] == '1': # If the char is a valid ending char
                        return choice
                else:
                    return choice
            choice = -1 # Reset choice
    else: # If the generated expression doesn't have to be valid, then any choice in the list is fine to use
        choice = random.choice(choiceRange)
        return choice 

# Generates a random mathematical expression
# @param isValid: determines whether the expression should be valid or invalid mathematically.
# @param char_data: provides information about the available math characters and their properties
# @param length: the number of characters in the expression 
# @return The randomly generated expression string
def generateExpression(isValid, char_data, length):
    # The possible range of values is determined by the number of possible characters. 
    # To obtain this value, it is stored in index [0][0] of the char_data array, but first the starting BOM character must be removed.
    choiceRange = range(1, int(char_data[0][0].lstrip('\ufeff'))+1)
    expression = "" #Stores the full randomly generated expression
    op = '' # Stores each randomly generated token for the expression
    prevRow = 0 # The index of the row for the previous character

    # Select a starting character
    # rowNum = 1 indicates whether a char can start the expression
    opIndex = randomizeChoice(isValid, char_data, 1, choiceRange,False)
    prevRow = opIndex + 2 # The column index + 2 equals the row index 
    op = char_data[0][opIndex]
    expression += getOp(op)

    # Select all of the middle characters
    while len(expression) < length-1:
        opIndex = randomizeChoice(isValid, char_data, prevRow, choiceRange, False)
        prevRow = opIndex + 2 # The column index + 2 equals the row index 
        op = char_data[0][opIndex]
        expression += getOp(op)
        
    # Select an ending character
    op = char_data[0][randomizeChoice(isValid, char_data, prevRow, choiceRange, True)]
    expression += getOp(op)

    # print(f"Final expression:{expression}")

    return expression

def evaluate(expression):
    # To evaluate the expression using Python's eval, powers must be written as ** instead of ^
    convertedExp = expression.replace('^', '**')
    return eval(convertedExp)


############################ Generate a set of expressions ######################
expressions = [] # Stores the expressions to be output to the CSV file

char_data = readCSV(CHAR_FILENAME)
# Information about char_data:
# [0][0]: The number of available characters (aka the number of rows besides the column header row)
# [1][x]: Indicates whether the character at position x can start the expression
# [2][x]: Indicates whether the character at position x can end the expression
# [x][y]: 1 if character x can be followed validly by character y, 0 if it would be invalid

numExpressions = 0
longest = -1
while numExpressions < numValid: # Keep generating expressions until the requested number is created
    newExpression = generateExpression(True, char_data, maxLength)
    try:
        expectedResult = evaluate(newExpression)
        expressions.append([newExpression, expectedResult])
        
        #Keep track of the longest expression or result obtained
        longest = max(longest, maxLength, len(str(expectedResult)))
        numExpressions += 1 # The expression was valid, so add it to the total count
    except Exception as e:
        # print(f"This expression isn't actually valid: {newExpression}")
        continue # Go directly to the next loop to try to generate a new expression

# print(expressions)
writeCSV(VALID_FILENAME, expressions)
print(f"{longest}")

