#!/bin/sh

echo
echo "Tracing ls..."
echo

./dynamorio/bin64/drrun -root dynamorio -c libqbinstr.so -- ls
