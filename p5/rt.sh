#!/bin/bash
# rt.sh <binary> [tests...] : run + score
bin=$1; shift; tot=0; n=0
for t in "${@:-tests/t*.in}"; do for f in $t; do
  s=$(date +%s.%N); env -u OUTPUT_PATH ./$bin < $f > /tmp/out_$$.txt 2>/tmp/err_$$.txt; e=$(date +%s.%N)
  sc=$(python3 check.py $f /tmp/out_$$.txt | tail -1 | awk '{print $2}')
  printf "%s %s %.2fs\n" "$f" "$sc" $(echo "$e - $s" | bc)
done; done
