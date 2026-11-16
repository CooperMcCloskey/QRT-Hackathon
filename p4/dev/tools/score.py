import json, sys, os, math
S = "/tmp/claude-1000/-home-server-coding-competitions/24a9292b-be07-4d02-81a2-d4da23a19ba4/scratchpad"
ref_path = S + "/refs.json"
refs = json.load(open(ref_path)) if os.path.exists(ref_path) else {}
runs = {}
for tag in sys.argv[1:]:
    d = {}
    for line in open(f"{S}/res_{tag}.txt"):
        p = line.split()
        if len(p) >= 2 and p[1].isdigit():
            d[p[0]] = int(p[1])
            if p[0] not in refs or int(p[1]) < refs[p[0]]:
                refs[p[0]] = int(p[1])
        else:
            d[p[0]] = None
    runs[tag] = d
json.dump(refs, open(ref_path, "w"))
for tag, d in runs.items():
    tot = 0
    for k in refs:
        if k in d and d[k]:
            tot += min(1, refs[k] / d[k]) ** 2
    print(f"{tag:12s} quality-sum {tot:7.2f} / {len(refs)}")
