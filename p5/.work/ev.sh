#!/bin/bash
cd /home/server/coding/competitions/QRT-Hackathon/p5
for f in tests/t*.in; do ( b=$(basename $f .in); env -u OUTPUT_PATH $1 < $f > .work/o_$b.$$ 2>/dev/null; python3 check.py $f .work/o_$b.$$ | tail -1 | awk -v n=$b '{print n, $2}'; rm -f .work/o_$b.$$ ) & done | sort | awk '{printf "%s ", $2; s+=$2; n++} END{printf " mean %.2f\n", s/n}'
wait
