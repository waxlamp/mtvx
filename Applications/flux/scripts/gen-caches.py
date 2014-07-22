#!/usr/bin/python

import string
import sys
import tempfile

class LevelData:
    def __init__(self, numblocks, assoc, write_policy):
        self.numblocks = numblocks
        self.assoc = assoc
        self.write_policy = write_policy

def die(msg):
    print(msg, file=sys.stderr)
    sys.exit(1)

def xml_header():
    return bytes('<?xml version="1.0" encoding="ISO-8859-1" ?>', 'UTF-8')

def newline():
    return bytes('\n', 'UTF-8')

def root_element_open(blocksize, write_miss_policy, replacement_policy):
    return bytes('<Cache blocksize="%s" write_miss_policy="%s" replacement_policy="%s">' % (blocksize, write_miss_policy, replacement_policy), 'UTF-8')

def level_element(numblocks, associativity, write_policy):
    return bytes('  <CacheLevel num_blocks="%s" associativity="%s" write_policy="%s" />' % (numblocks, associativity, write_policy), 'UTF-8')

def root_element_close():
    return bytes('</Cache>', 'UTF-8')

if len(sys.argv) < 2:
    print("usage: gen-caches.py <filename>", file=sys.stderr)
    sys.exit(1)

# The input is either stdin or a named file.
f = None
if sys.argv[1] == '-':
    f = sys.stdin
else:
    try:
        f = open(sys.argv[1])
    except IOError:
        die("error: can't open file '%s' for reading." % sys.argv[1])

# Initialize the data store.
data = {}
data['levels'] = []

# Read the data.
for line in f:
    tokens = line.split()
    if len(tokens) == 0:
        continue

    if tokens[0] == 'blocksize':
        if len(tokens) != 2:
            die("fatal error: 'blocksize' line should contain exactly two tokens, has %d" % len(tokens))

        try:
            data['blocksize'] = int(tokens[1])
        except ValueError:
            die("fatal error: could not read blocksize value")
    elif tokens[0] == 'write_miss_policy':
        if len(tokens) != 2:
            die("fatal error: 'write_miss_policy' line should contain exactly two tokens, has %d" % len(tokens))

        data['write_miss_policy'] = tokens[1]
    elif tokens[0] == 'replacement_policy':
        if len(tokens) != 2:
            die("fatal error: 'replacement_policy' line should contain exactly two tokens, has %d" % len(tokens))

        data['replacement_policy'] = tokens[1]
    elif tokens[0] == 'level':
        if len(tokens) != 4:
            die("fatal error: 'level' line should contain exactly four tokens, has %d" % len(tokens))

        try:
            data['levels'].append(LevelData(numblocks=int(tokens[1]), assoc=int(tokens[2]), write_policy=tokens[3]))
        except ValueError:
            die("fatal error: 'level' line contains non-numeric data in its first two argument tokens")
    elif tokens[0][0] == "#":
        continue
    else:
        print("fatal error: unexpected keyword '%s'" % (tokens[0]), file=sys.stderr)
        sys.exit(1)
    
# Validate the data store.
ok = True
if len(data['levels']) == 0:
    print("no cache levels specified", file=sys.stderr)
    ok = False
if 'blocksize' not in data:
    print("no block size specified", file=sys.stderr)
    ok = False
if 'write_miss_policy' not in data:
    print("no write miss policy specified", file=sys.stderr)
    ok = False
if 'replacement_policy' not in data:
    print("no replacement policy specified", file=sys.stderr)
    ok = False

if not ok:
    die("fatal error: underspecified cache configuration")

# Begin outputting the cache configuration files.
#
# Create temp file names for them.  There are three files: 1. the
# "capacity" cache, which uses OPT for the replacement policy and is
# fully associative, but has the specified level sizes; 2. the
# "associative" cache, which uses OPT for the replacment policy and
# uses the specified level sizes and associativities; and 3. the
# "real" cache, which uses the specified replacment policy POL, the
# specified associativies, and the specified levels.
capacity_cache = tempfile.NamedTemporaryFile(delete=False)
associative_cache = tempfile.NamedTemporaryFile(delete=False)
real_cache = tempfile.NamedTemporaryFile(delete=False)

# Capture the filenames (for use by the calling shell script)
names = [capacity_cache.name, associative_cache.name, real_cache.name]

# Construct the common preamble to all the files.
top = xml_header() + newline()*2

# Write out the preamble and the cache element opening tag.
capacity_cache.write(top + root_element_open(data['blocksize'], data['write_miss_policy'], "OPT") + newline())
associative_cache.write(top + root_element_open(data['blocksize'], data['write_miss_policy'], "OPT") + newline())
real_cache.write(top + root_element_open(data['blocksize'], data['write_miss_policy'], data['replacement_policy']) + newline())

# Write out the level elements.
for L in data['levels']:
    capacity_cache.write(level_element(L.numblocks, 1, L.write_policy) + newline())
    associative_cache.write(level_element(L.numblocks, L.assoc, L.write_policy) + newline())
    real_cache.write(level_element(L.numblocks, L.assoc, L.write_policy) + newline())

# Write out the closing root tag.
capacity_cache.write(root_element_close() + newline())
associative_cache.write(root_element_close() + newline())
real_cache.write(root_element_close() + newline())

# Close the temp files.
capacity_cache.close()
associative_cache.close()
real_cache.close()

# Print out the file names so the calling script can use them and
# clean them up.
print(" ".join(names) + " %d %d" % (data['blocksize'], len(data['levels'])))
