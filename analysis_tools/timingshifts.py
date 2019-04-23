#!/usr/bin/env python

import os
import sys

inputFile = sys.argv[1]
mode = sys.argv[2]
time_offset = int(round(float(sys.argv[3])))
outputFile = sys.argv[4]

time_idx = 0
if mode == 'chamber':
    time_idx = 5
elif mode == 'hodo':
    time_idx = 5
elif mode == 'trigger':
    time_idx = 6

lines = [line.strip() for line in open(inputFile).readlines()]

fout = open(outputFile, 'w')
fout.write(lines[0] + '\n')
for line in lines[1:]:
    vals = line.split()
    try:
        time_original = int(vals[time_idx])
        fout.write('\t'.join(vals[:time_idx]) + '\t' + str(time_original + time_offset) + '\t' + '\t'.join(vals[time_idx+1:]) + '\n')
    except:
        fout.write(line + '\n')
fout.close()

