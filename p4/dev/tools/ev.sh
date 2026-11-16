#!/bin/bash
# usage: ev.sh binary tag  (env passes through); prints per-test L and total
S=/tmp/claude-1000/-home-server-coding-competitions/24a9292b-be07-4d02-81a2-d4da23a19ba4/scratchpad
B=$1; TAG=$2
ls ${TESTS:-tests/*.in} | xargs -P 6 -I{} bash -c 'o=$(mktemp -p '$S'); e=$(mktemp -p '$S'); '$B' < {} > $o 2>$e; r=$(python3 check.py {} $o | tail -1); echo "$(basename {} .in) $(echo $r | awk "{print \$2}") $(grep -o "t=[0-9.]*" $e)"; rm -f $o $e' | sort > $S/res_$TAG.txt
cat $S/res_$TAG.txt | awk '{printf "%s:%s ", substr($1,1,2), $2} END{print ""}'
