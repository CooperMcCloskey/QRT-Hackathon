#!/bin/bash
# usage: runall.sh binary [test glob]
S=/tmp/claude-1000/-home-server-coding-competitions/24a9292b-be07-4d02-81a2-d4da23a19ba4/scratchpad
B=$1; shift
for t in ${@:-tests/*.in}; do
  ( o=$(mktemp -p $S); e=$(mktemp -p $S); timeout 12 $B < $t > $o 2>$e; echo "$(basename $t) $(cat $e | tr '\n' ' ') $(python3 check.py $t $o | tail -1)"; rm -f $o $e ) &
done | sort
wait
