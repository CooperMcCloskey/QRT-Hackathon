import json,math,sys
from pathlib import Path
solutions=sys.argv[1:]
runs=[json.loads(x) for x in Path('scores.jsonl').read_text().splitlines()]
latest={s:next(r for r in reversed(runs) if r.get('sol')==s) for s in solutions}
common=set.intersection(*(set(r['tests']) for r in latest.values()))
refs={t:min(latest[s]['tests'][t]['v'] for s in solutions if latest[s]['tests'][t]['ok']) for t in common if any(latest[s]['tests'][t]['ok'] for s in solutions)}
for s,r in latest.items():
    ts=[r['tests'][t] for t in common]
    score=sum(100*min(1,refs[t]/r['tests'][t]['v'])**2*(.5+.5*math.exp(-max(0,r['tests'][t]['s']-1)/5)) if r['tests'][t]['ok'] else 0 for t in common)/len(common)
    print(s,'cases',len(ts),'valid',sum(x['ok'] for x in ts),'local proxy',round(score,3),'max seconds',max(x['s'] for x in ts))
    if s!=solutions[0]:
        counts=[0,0,0]
        for t in common:
            a=latest[solutions[0]]['tests'][t];b=r['tests'][t]
            if a['ok'] and b['ok']: counts[0 if b['v']<a['v'] else 1 if b['v']==a['v'] else 2]+=1
        print('shorter/equal/longer',counts)
