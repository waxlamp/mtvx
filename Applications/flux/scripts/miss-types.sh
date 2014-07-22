#!/bin/sh

# Process arguments.
config="$1"
shift

trace="$1"
shift

bsfile="$1"
shift

# Get the working directory.
pwd=`dirname \`pwd\`/$0`

if [ -z "$config" -o -z "$trace" -o -z "$bsfile" ]; then
    echo "usage: miss-types.sh <cache-configuration> <trace-file> <blockstream-file>" >/dev/stderr
    exit 1
fi

# Run the gen-caches.py script - capture the output into three
# filenames.
names=`$pwd/gen-caches.py $config`
if [ $? -ne "0" ]; then
    echo "error: couldn't generate cache config xml files" >/dev/stderr
    exit 1
fi

name_array=$(echo $names | awk -F";" '{print $1,$2,$3,$4,$5}')
set -- $name_array
capacity_cache=$1
associative_cache=$2
real_cache=$3
blocksize=$4
num_levels=$5

names="$capacity_cache $associative_cache $real_cache"

# Run the simulator on the trace for all three cache configurations.
$pwd/credit -p 0 -w -1 -d hit-level -c $capacity_cache -c $associative_cache -c $real_cache -t $trace --blockstream-file $bsfile --num-streams 333
if [ $? -ne "0" ]; then
    echo "error: couldn't run cache simulator" >/dev/stderr
    rm $names
    exit 1
fi

# Compare the hit-level data to identify the miss types.
$pwd/misstypes -t $trace -l hit-level.c0.dat -l hit-level.c1.dat -l hit-level.c2.dat -b $blocksize -m $num_levels

rm $names
