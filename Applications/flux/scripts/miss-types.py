#!/usr/bin/python

import string
import sys

if len(sys.argv) != 3:
    sys.stderr.write("usage: miss-types.py <blocksize> <miss-level>\n")
    sys.exit(1)

# Grab command line arguments.
blocksize = int(sys.argv[1])
misslevel = int(sys.argv[2])

# Create a set of unique block references
compulsory = set()

# Create a table from numbers to miss type labels.
types = {1 : "capacity",
         2 : "mapping",
         3 : "replacement"}

try:
    for line in sys.stdin:
        # Split the line into fields.
        fields = line.strip().split()

        # If there are not four fields, quit.
        if len(fields) != 4:
            sys.stderr.write("error: input line has too many fields\n")
            sys.exit(1)

        # The first field is the address of the reference.
        addr = int(fields[0])

        # By default, the miss type is "hit".
        misstype = "hit"

        # Determine if the reference is a compulsory miss.
        block = addr / blocksize
        if block not in compulsory:
            # The block is previously unseen, so this is a compulsory
            # miss.
            compulsory.add(block)
            misstype = "compulsory"
        else:
            # The block has been seen before, so its miss status
            # depends on which (if any) of the three simulations it
            # misses for.
            for i in [1,2,3]:
                if int(fields[i]) == misslevel:
                    misstype = types[i]
                    break

        # Print the result.
        print(addr, misstype)

except StopIteration:
    pass
