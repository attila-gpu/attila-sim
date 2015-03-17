#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
import re

output = sys.stdout

if len(sys.argv) < 3:
    print ("Usage:", sys.argv[0], "cycle_count_file1", "cycle_count_file2", "[start_frame]")
    sys.exit(-1)

cycle_count_file1 = sys.argv[1]
cycle_count_file2 = sys.argv[2]

if len(sys.argv) > 3:
    start_frame = int(sys.argv[3])
else:
    start_frame = 0

# Open input files

cycle_count1 = open(cycle_count_file1, 'r')
cycle_count2 = open(cycle_count_file2, 'r')

header_str1 = cycle_count1.readline()[:-1]
header_str2 = cycle_count2.readline()[:-1]

if header_str1 != 'CycleCounter':
    print("Not recognized header in " + cycle_count_file1 + ". \'CycleCounter\' was expected.")
    sys.exit(-1)

if header_str2 != "CycleCounter":
    print("Not recognized header in " + cycle_count_file2 + ". \'CycleCounter\' was expected.")
    sys.exit(-1)

#header_count = len(header_str.split(";")) - 1

if start_frame > 0:
    # current_frame = (start_frame - 1)
    current_frame = start_frame
else:
    current_frame = 0

skipped_cycles1 = 0
skipped_cycles2 = 0

line1 = cycle_count1.readline()
line2 = cycle_count2.readline()

while line1 and line2:

    cycles1 = float(line1)
    cycles2 = float(line2)

    speedup = 0.0
    equal = 1 

    if start_frame != 0 and current_frame == (start_frame - 1):
       skipped_cycles1 = cycles1
       skipped_cycles2 = cycles2

    else:
        if (start_frame != 0) and (current_frame == start_frame):
            if (cycles1 - skipped_cycles1) == (cycles2 - skipped_cycles2):
                speedup = 1.0
                equal = 1 
            else:
                speedup = (cycles1 - skipped_cycles1)/(cycles2 - skipped_cycles2);
                equal = 0

        else:
            if cycles1 == cycles2:
                speedup = 1.0;
                equal = 1 
            else:
                speedup = cycles1/cycles2;
                equal = 0

        if equal:
            print("[%d] %0.5fx (=) " % (current_frame, 1.0),)
        else:
            print("[%d] %0.5fx " % (current_frame, speedup),)

    line1 = cycle_count1.readline()
    line2 = cycle_count2.readline()

    current_frame = current_frame + 1

if line1 != line2:
    print("Different number of cycle counter rows in " + cycle_count_file1 + " and " + cycle_count_file2 + ".")
    sys.exit(-1)
      
#    items = (line[:-1]).split(";")
#    cycles = items[0].split("..")
#    cycle_count = int(cycles[1]) - int(cycles[0]) + 1
#    items = items[1:] # discard cycles field

#    i = 0
#    while i < merge_count:
#        i = i + 1
#        j = 0
#        for item in items:
#            try:
#                accum[j] += int(item)
#                accum_float[j] += float(item)
#            except ValueError:
#                accum_float[j] += float(item)
#                is_float[j] = True # This accumulation will be considered as float
#            j = j + 1
#        line = input.readline()
#        if not line:
#            break
#        items = (line[:-1]).split(";")
#        items = items[1:]
#
#    row_str = str(cycles[0]) + ".." + str(int(cycles[0]) + (i * cycle_count - 1))
#
#    pos = 0
#    while pos < header_count:
#        if is_float[pos]:
#            # row_str = row_str + ";" + str(accum_float[pos] / i) # Assume floats are AVGs
#            row_str = row_str + ";" + str(accum_float[pos]) # Float are not AVGs
#        else:
#            row_str = row_str + ";" + str(accum[pos])
#        pos = pos + 1
#
#    # Clean intermediate accumulator vectors
#    accum = [0] * header_count 
#    accum_float = [0.0] * header_count
#    is_float = [False] * header_count
#
#    print row_str

cycle_count1.close()
cycle_count2.close()

