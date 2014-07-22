#!/bin/sh

# Create a tempfile and save its name in a list.
gettmp()
{
    filename=`mktemp /tmp/linemap.XXXX`
    echo $filename
}

# Cleanup code.
cleanup()
{
    rm -f $addrfile $linefile
}

# Grab command line parameters.
exe="$1"

if [[ -z $exe ]]; then
    echo "usage: linemap.sh <executable>" >&2
    exit 1
fi

# Generate tempfiles for the two intermediate computations.
addrfile=`gettmp`
linefile=`gettmp`

# Remove tempfiles on exit/interruption.
trap "cleanup" INT TERM EXIT

# Get the addresses associated with line number information from the
# executable's symtable (awk numbers the list, for the join command
# later).
objdump --dwarf=decodedline $exe | grep 0x | awk '{print NR,$3}' >$addrfile || exit 1

# Run the address list (without the record number) through the
# addr2line program to generate a corresponding list of line number
# references (awk numbers the list, for the join command later).
cat $addrfile | awk '{print $2}' | addr2line -e $exe | awk '{print NR,$0}' >$linefile || exit 1

# Join the two files, tossing out the indexing record.
join $addrfile $linefile | awk '{$1=""; print}' || exit 1
