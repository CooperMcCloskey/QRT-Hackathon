import json,math,csv,sys
from pathlib import Path
names=sys.argv[1:] or ['finSolve.cpp','finSolve_optimized.cpp']
records=[json.loads(x) for x in Path('scores.jsonl').read_text().splitlines()]
runs={s:next(r for r in reversed(records) if r.get('sol')==s) for s in names}
common=sorted(set.intersection(*(set(r['tests']) for r in runs.values())))
groups={'all':common,'original':[t for t in common if (Path('tests')/(t+'.in')).exists()], 'independent':[t for t in common if (Path('tests_astra')/(t+'.in')).exists()], 'transforms':[t for t in common if t.startswith('case')], 'fresh':[t for t in common if t.startswith('fresh')]}
refs={t:min(runs[s]['tests'][t]['v'] for s in names if runs[s]['tests'][t]['ok']) for t in common}
def score(s,t):
    a=runs[s]['tests'][t]
    return 100*min(1,refs[t]/a['v'])**2*(.5+.5*math.exp(-max(0,a['s']-1)/5)) if a['ok'] else 0
with open('benchmark_optimized.csv','w') as f:
    w=csv.writer(f);w.writerow(['test','solution','valid','length','seconds','local_reference','proxy_score'])
    for t in common:
        for s in names:
            a=runs[s]['tests'][t];w.writerow([t,s,a['ok'],a['v'],a['s'],refs[t],score(s,t)])
for group,ts in groups.items():
    if ts: print(group,len(ts),*[s+': '+str(round(sum(score(s,t) for t in ts)/len(ts),3)) for s in names])
