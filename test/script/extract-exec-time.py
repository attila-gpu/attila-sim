#!/usr/bin/env python
# -*- coding: UTF-8 -*-

import sys
import os
import os.path

# 1st param: stats-file.csv.gz  [working dir] (optional)

if len(sys.argv) > 4:
    print("Usage: " + sys.argv[0] + " stats-file.csv.gz [working dir]")
    sys.exit(0)

inputPath = sys.argv[1]

if len(sys.argv) > 2:
    workdir = sys.argv[2] + '/'
else:
    workdir = "./"

filter_keys = \
'^CycleCounter'

filter_keys_prev = \
'Cycles\n\
^CycleCounter'

temp_file_name = "xxx.xxx.key"
temp_file = open(temp_file_name, 'w')
temp_file.write(filter_keys)
temp_file.close()

temp_file_name_prev = "prev.xxx.xxx.key"
temp_file_prev = open(temp_file_name_prev, 'w')
temp_file_prev.write(filter_keys_prev)
temp_file_prev.close()

command = 'zcat ' + inputPath + ' | ' + workdir + 'filter-cols.py - - @' + temp_file_name
os.system(command)

os.remove(temp_file_name_prev)
os.remove(temp_file_name)
