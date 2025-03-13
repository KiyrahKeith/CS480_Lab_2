#!/bin/bash

# Define three variables
numValid="10"
numInvalid="10"
eqLength="25"

# Run Python script and capture output
python_output=$(python3 expression_generator.py "$numValid" "$numInvalid" "$eqLength" | tail -n 1)

last_two="${python_output: -2}"

# Check if the Python script was successful and returned a value
if [ "$last_two" != " 0" ]; then
    echo "Python script executed successfully, output: $python_output"
    
    # Now use the Python output to compile and run the C script
    gcc -o calc_tester *.c
    if [ $? -eq 0 ]; then
        echo "C program compiled successfully."
        
        # Run the C program and pass var1, var2, and Python output as arguments
        ./calc_tester "$numValid" "$numInvalid" "$python_output"
    else
        echo "C program compilation failed."
    fi
else
    echo $python_output
    echo "Python script failed."
fi
