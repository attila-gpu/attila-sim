#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
import re

if len(sys.argv) < 4:
    print("Usage: " + sys.argv[0] + " input-path output-path @columns-path|regexp [accept(default)|reject]")
    print("Use '-' instead of input-path to use standard input as input-path, the same for output-path")
    sys.exit(0)


inputPath = sys.argv[1]
outputPath = sys.argv[2]
columnsPath = sys.argv[3]
function = "accept"
if len(sys.argv) > 4:
    function = sys.argv[4]
   
separator = r';'

# Open input and output tables

if inputPath == '-':
    input = sys.stdin
else:
    input = open(inputPath, 'r')
    
if outputPath == '-':
    output = sys.stdout
else:
    output = open(outputPath, 'w')

# Read selection columns patterns

selectedCols = []

if columnsPath[0] == '@':
    selectedColsFile = open(columnsPath[1:len(columnsPath)], 'r')
    for line in selectedColsFile:
        line = line.strip()
        if len(line) > 0 and line[0] != '#':
            selectedCols.append(re.compile(line))
    selectedColsFile.close()
else:
    selectedCols.append(re.compile(columnsPath))

# Retrieve the valid column indices from input table

line = input.readline()[:-1]
inputColNames = re.split(separator, line)

validCols = []

for index, name in enumerate(inputColNames):
    name = name.strip()
    
    if len(name) > 0:
        if function == "accept":
            for pattern in selectedCols:
                if pattern.match(name):
                    validCols.append(index)
                    break
        else:
           addColumnName = True
           for pattern in selectedCols:
               if pattern.match(name):
                   addColumnName = False
                   break
           if addColumnName:
               validCols.append(index)
        
# Generate output header

for i, index in enumerate(validCols):
    name = inputColNames[index]
    if i < len(validCols) - 1:
        name = name + ';'
    output.write(name)

output.write('\n')

# Generate body

for line in input:
    line = line[:-1]
    rowValues = re.split(separator, line)
    numEmptyValues = len(inputColNames) - len(rowValues)
    rowValues = rowValues + ['' for i in range(numEmptyValues)]

    for i, index in enumerate(validCols):
        name = rowValues[index]
        # name = rowValues[index-1]
        if i < len(validCols) - 1:
            name = name + ';'    
        output.write(name)
        
    output.write('\n')
    
# Close tables

if inputPath != '-':
    input.close()

if outputPath != '-':
    output.close()

