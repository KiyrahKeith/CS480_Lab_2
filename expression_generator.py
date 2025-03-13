import csv
import sys 
import random
import math
import signal
import string

############### Define CSV reading and writing functions ###############
VALID_FILENAME = "Output/valid_expressions.csv"
INVALID_FILENAME = "Output/invalid_expressions.csv"
CHAR_FILENAME = "char_matrix.csv"
MAX_DOUBLE = sys.float_info.max # The maximum double value allowed

# Custom exception for the timeout
class TimeoutException(Exception):
    pass

# Signal handler function
def timeout_handler(signum, frame):
    raise TimeoutException("Evaluation timed out")

# Write expressions to a CSV file
# @param The path of the csv file to be written
# @param data The content of the file to be written to the csv
def writeCSV(fileName, data):
    with open(fileName, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(data)
    
    print(fileName, "successfully written.")

# Reads all content from a csv into an array
# @param The path of the csv file to be read.
# @return An array containing the contents of the entire csv
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
            return 'sin('
        case 'c':
            return 'cos('
        case 't':
            return 'tan('
        case 'o':
            return 'cot('
        case 'l':
            return 'log('
        case 'n':
            return 'ln('
        case _:
            return op

# Tests if an operator is a function that requires parenthesis after it (sin, cos, ln, etc). 
# @param op The operator to be converted
# @return True if operator is a function
#         Otherwise, returns False
def isFunction(op):
    match op:
        case 's' | 'c' | 't' | 'o' | 'l' | 'n':
            return True
        case _: # Default
            return False

# Selects a random character from the char_data table
# @param isValid A boolean representing whether the final expression must be valid (true) or invalid (false)
# @param char_data provides information about the available math characters and their properties
# @param rowNum The row number to determine whether the choice is valid or invalid. This tells either start, end, or previous char. 
# @param choiceRange A list of available index options based on the number of characters listed in char_data
# @param isEnd True if this character is the final char in the expression. These must be valid for both the previous row and also the ending condition
# @param context A list indicating the context status of the equation. 
#       [0]:    0 = not entering a number, 1 = a number but no period yet. 2 = a number with a period already.
#       [1]:    A string containing all previous parenthesis/brackets that are opened and not yet closed. This helps validate open and close parenthesis rules.
# @return The index of an appropriate randomized character in char_data
def randomizeChoice(isValid, char_data, rowNum, choiceRange, isEnd, context):
    choice = -1 # The index of the randomized choice
    tryCounter = 0 # Keeps track of how many times the loop has tried to choose a valid starting character
    if isValid: 
        while choice == -1: # Continue randomly selecting a choice until a valid one is found
            tryCounter += 1
            if tryCounter == 20: # If it's tried to find a valid character too many times and failed
                return -1
            else:
                choice = random.choice(choiceRange)


            if char_data[rowNum][choice] == '1': # If the random pick was a valid choice
                if char_data[0][choice].isdigit(): ############ DIGITS
                    # If the choice is a number, switch the context to indicate that a number is being entered. 
                    # This is used to verify proper use of decimal places
                    if context[0] == 0: # If this is the first number being entered
                        context[0] = 1

                elif char_data[0][choice] == '.': ############## PERIOD
                    # If the choice is a period, this is only valid if a number is being entered previously and there hasn't already been a period.
                    if context[0] == 2: # If the number already has a period in it
                        choice = -1
                        continue # Another period can't be used, so continue to pick a new choice
                    elif context[0] == 1: # If you're entering a number and this is the first period
                        context[0] = 2 # Update the context to include a period
                elif isFunction(char_data[0][choice]): #  If the choice is a function that requires parenthesis
                    # print("Function")
                    context[1] += '(' # Add another open parenthesis to the context. getOp will handle adding the actual ( to the expression.

                else: # If the char is anything but a digit, a period, or a function
                    context[0] = 0 # Reset the context to indicate that a number isn't being entered

                    # If the choice is a parenthesis/bracket ############# PARENTHESIS/BRACKET
                    if char_data[0][choice] == '(' or char_data[0][choice] == '{':
                        # print(F"Open bracket: {char_data[0][choice]}") # DEBUG
                        context[1] += char_data[0][choice] # Add the open parenthesis to the context

                    elif char_data[0][choice] == ')': # Closing parenthesis
                        # print(F"Closing parenthesis: {char_data[0][choice]}") # DEBUG
                        if len(context[1]) == 0: # If an open parenthesis hasn't already been added
                            choice = -1
                            continue # Then any closing bracket is invalid, so select a new random character
                        if context[1][-1] == '(': # if the last added bracket was a matching open parenthesis
                            # The closing parenthesis is valid, so remove its matching open brace
                            # print(f"Remove a parenthesis before: {context[1]}")
                            context[1] = context[1][:-1] # Remove the last character in the parenthesis list
                            # print(f"Remove a parenthesis after: {context[1]}")
                        else: # If the last added bracket was an opening curly bracket {
                            choice = -1
                            continue # Then any closing bracket is invalid, so select a new random character

                    elif char_data[0][choice] == '}': # Closing parenthesis
                        # print(F"Closing bracket: {char_data[0][choice]}") # DEBUG
                        if len(context[1]) == 0: # If an open parenthesis hasn't already been added
                            choice = -1
                            continue # Then any closing bracket is invalid, so select a new random character
                        if context[1][-1] == '{': # if the last added bracket was a matching open bracket
                            # The closing bracket is valid, so remove its matching open brace
                            # print(f"Remove a brace before: {context[1]}")
                            context[1] = context[1][:-1] # Remove the last character in the parenthesis list
                            # print(f"Remove a brace after: {context[1]}")
                        else: # If the last added bracket was an opening parenthesis (
                            choice = -1
                            continue # Then any closing bracket is invalid, so select a new random character    

                if isEnd: # If the character is an ending character, it has an extra condition
                    if char_data[2][choice] == '1': # If the char is a valid ending char
                        return choice
                    elif isFunction(char_data[0][choice]): # If you can't use this choice, and it was a function
                        context[1] = context[1][:-1] # Remove the last character in the parenthesis list
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
# @param choiceRange: The list of possible indices that can be randomly selected. Corresponds to the number of available characters.
# @return The randomly generated expression string
def generateExpression(isValid, char_data, length, choiceRange):
    expression = "" #Stores the full randomly generated expression
    op = '' # Stores each randomly generated token for the expression
    prevRow = 0 # The index of the row for the previous character
    context = [0, ""]

    # Select a starting character
    rowNum = 1 # row 1indicates whether a char can start the expression
    opIndex = randomizeChoice(isValid, char_data, 1, choiceRange,False, context)
    prevRow = opIndex + 2 # The column index + 2 equals the row index 
    op = char_data[0][opIndex]
    expression += getOp(op)

    
    # Select all of the middle characters
    while len(expression) < length-1:
        # print(f"#### Expression: {expression}      Context[1]: {context[1]}")
        opIndex = randomizeChoice(isValid, char_data, prevRow, choiceRange, False, context)
        if opIndex == -1: return -1 # Fail safe: if a valid choice could not be found for the next character, return the expression in the current state
        prevRow = opIndex + 2 # The column index + 2 equals the row index to account for the extra "start" and "end" rows
        op = char_data[0][opIndex]
        expression += getOp(op)
        
    # Select an ending character
    op = char_data[0][randomizeChoice(isValid, char_data, prevRow, choiceRange, True, context)]
    expression += getOp(op)

    # print(f"Initial expression: {expression}        Context[1]: {context[1]}") # DEBUG
    # After the full expression has been generated, use context[1] data to close any remaining parenthesis
    while len(context[1]) > 0: # If there are any opening brackets left that haven't been closed
        # print("Add an extra closing bracket")
        if context[1][-1] == '(':
            expression += ')'

        else: # If the last char is an opening curly bracket
            expression += '}'
        context[1] = context[1][:-1] # Remove the last character in the parenthesis list
    # print(f"Final expression:{expression}") # DEBUG

    return expression

# Generates a string of random characters that can be alphanumeric or symbols
# @param length The length of the random string
# @return Returns a string of random characters 
def generateRandomString(length): 
    allChoices = string.ascii_letters + string.digits + "!@#$%^&*()_+=|?><,.:;"
    resultLength = 0  # The length of the current string
    result = ""

    while resultLength < length:
        c = random.choice(allChoices)
        result += c
        resultLength += 1
    
    print(f"Random string:{result}")
    return result


# Evaluate an expression using Python's eval function. 
# The expression is manually modified to make the syntax temporarily suitable format for use in eval
# If eval takes more than the timeout value in seconds to evaluate, then the function immediately exits with an error code.
# @param expression The mathematical expression to be evaluated
# @param timeout The time (in seconds) to wait before exiting with a timeout
# @return The result of the expression evaluation. Returns nan if the evaluation times out.
def evaluate(expression, timeout=2):
    # Eval powers must be written as ** instead of ^
    convertedExp = expression.replace('^', '**')

    # Eval must use parenthesis () instead of curly brackets {}
    convertedExp = convertedExp.replace('{', '(')
    convertedExp = convertedExp.replace('}', ')')

    # Eval must use math.log and math.ln instead of log and ln. This is the same for standard trig functions (sin, cos, tan)
    convertedExp = convertedExp.replace('log', 'math.log')
    convertedExp = convertedExp.replace('ln', 'math.ln')
    convertedExp = convertedExp.replace('sin', 'math.sin')
    convertedExp = convertedExp.replace('cos', 'math.cos')
    convertedExp = convertedExp.replace('tan', 'math.tan')

    # Eval cannot calculate cot, so it must be mathematically replaced with an equivalent
    convertedExp = convertedExp.replace('cot', '1/math.tan')

    print(f"Converted Expression: {convertedExp}")
    signal.signal(signal.SIGALRM, timeout_handler)
    signal.alarm(timeout)  # Set the timeout to 'timeout' seconds
    
    # Try to evaluate the expression, but include a timeout
    try:
        result = eval(convertedExp)
        signal.alarm(0)  # Cancel the alarm if eval finishes before the timeout

        if abs(result) > MAX_DOUBLE: # If the result exceeds possible bounds for a double
            return float('nan')
        else: # If the result is within bounds
            return result
    except TimeoutException: # If the expression could not be evaluated due to timing out
        return float('nan') # Return nan
    except Exception as e:
        return float('nan') # For any other error, also return nan

    



############################ Generate a set of expressions ######################
expressions = [] # Stores the expressions to be output to the CSV file

char_data = readCSV(CHAR_FILENAME)
# Information about char_data:
# [0][0]: The number of available characters (aka the number of rows besides the column header row)
# [1][x]: Indicates whether the character at position x can start the expression
# [2][x]: Indicates whether the character at position x can end the expression
# [x][y]: 1 if character x can be followed validly by character y, 0 if it would be invalid

# The possible range of values is determined by the number of possible characters. 
# To obtain this value, it is stored in index [0][0] of the char_data array, but first the starting BOM character must be removed.
choiceRange = list(range(1, int(char_data[0][0].lstrip('\ufeff'))+1))

numExpressions = 0
longest = -1
while numExpressions < numValid: # Keep generating expressions until the requested number is created
    # print(f"Generate a new expression: # expressions {numExpressions}       # valid: {numValid}")
    newExpression = generateExpression(True, char_data, maxLength, choiceRange)
    if newExpression == -1: continue # If a new expression couldn't be successfully generated, try again
    # print(newExpression)
    try:
        expectedResult = evaluate(newExpression)
        if math.isnan(expectedResult): # If the expression could not be evaluated
            # print("Expression couldn't be successfully evaluated.")
            continue # Generate a new random expression without saving this one
        else: # If the expression could be successfully evaluated
            # print("-----SUCCESS-----")
            expressions.append([newExpression, expectedResult])
            #Keep track of the longest expression or result obtained
            longest = max(longest, maxLength, len(str(newExpression)) + len(str(expectedResult)) + 1)
            numExpressions += 1 # The expression was valid, so add it to the total count
    except Exception as e:
        # print(f"This expression isn't actually valid: {newExpression}")
        # print(f"Error code: {e}")
        continue # Go directly to the next loop to try to generate a new expression

# print(expressions)
writeCSV(VALID_FILENAME, expressions)

############# Generate Invalid Expression #################
expressions = [] # Reset all expression variables
numExpressions = 0
numMath = math.floor(numInvalid*0.9) # 90% of the expressions should be generated from within valid mathematical symbols
 # The remaining 10% of invalid expressions should be purely random characters (letters, symbols, etc)

while numExpressions < numInvalid: # Keep generating expressions until the requested number is created
    # print(f"Generate a new expression: # expressions {numExpressions}       # valid: {numValid}")
    if numExpressions < numMath:
        newExpression = generateExpression(False, char_data, maxLength, choiceRange)
    else: 
        newExpression = generateRandomString(maxLength)
    # print(newExpression)
    try:
        expectedResult = evaluate(newExpression)
        if math.isnan(expectedResult): # If the expression could not be evaluated
            expectedResult = "nan"
            expressions.append([newExpression, expectedResult])
            #Keep track of the longest expression or result obtained
            longest = max(longest, maxLength, len(str(newExpression)) + len(str(expectedResult)) + 1)
            numExpressions += 1 # The expression was valid, so add it to the total count
        else: # If the expression could be successfully evaluated
            # Expressions should only be saved if they are invalid
            continue # Generate a new random expression without saving this one
            
    except Exception as e:
        # If the expression had an error when evaluating, then that is fine to be saved as an invalid expression
        expectedResult = "nan"
        expressions.append([newExpression, expectedResult])
        #Keep track of the longest expression or result obtained
        longest = max(longest, maxLength, len(str(newExpression)) + len(str(expectedResult)) + 1)
        numExpressions += 1 # The expression was valid, so add it to the total count


# print(expressions)
writeCSV(INVALID_FILENAME, expressions)
print(f"{longest+1}")

