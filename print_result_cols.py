
import sys
import re
from tabulate import tabulate
import numpy as np

col = int(sys.argv[1])
cols = []

title = ''

headers = []

for f in sys.argv:
    if f.endswith('.data'):
        
        headers.append(f[8:-5])
        column = []
        fi = open(f, 'r')
        first = True
        for line in fi:
            values = line.split(' ')
            if first:
                title = values[col]
                first = False
            else:
                column.append(float(values[col].strip()))
        cols.append(column)

arr = np.array(cols)
arr = arr.transpose()

print("-----")
print(title)
print("-----")

print(tabulate(arr,headers))
