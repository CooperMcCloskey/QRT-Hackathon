#!/usr/bin/env python3
"""check.py <input> <output>: Document Lego scorer (truth in <input>.truth sibling)."""
import sys,itertools
def bad(m):
    print("INVALID: "+m);sys.exit(1)
def main():
    inp,outp=sys.argv[1],sys.argv[2]
    L=open(inp).read().split('\n')
    N=int(L[0]);pcs=[[row.replace('_',' ') for row in L[1+i].split('|')] for i in range(N)]
    T=open(inp[:-3]+'.truth').read().split('\n')
    truth=[tuple(map(int,T[1+i].split())) for i in range(N)]
    real={}
    for i,(pg,r,c) in enumerate(truth):
        if pg<0: continue
        d=real.setdefault(pg,{})
        for a,row in enumerate(pcs[i]):
            for b,ch in enumerate(row):
                if ch!=' ': d[(r+a,c+b)]=ch
    total=sum(len(d) for d in real.values())
    mine={};occ={};seen=set();shifts={}
    for ln in open(outp).read().split('\n'):
        if not ln.strip(): continue
        t=ln.split()
        if len(t)!=4: bad("line not four integers: "+ln)
        try: p,pg,r,c=map(int,t)
        except: bad("non-integer: "+ln)
        if not (0<=p<N): bad("unknown piece id")
        if p in seen: continue
        o=occ.setdefault(pg,set())
        cells=[(r+a,c+b) for a,row in enumerate(pcs[p]) for b in range(len(row))]
        if any(x in o for x in cells): continue
        seen.add(p);o.update(cells)
        d=mine.setdefault(pg,{})
        for a,row in enumerate(pcs[p]):
            for b,ch in enumerate(row):
                if ch!=' ': d[(r+a,c+b)]=ch
        tp=truth[p]
        if tp[0]>=0: shifts.setdefault((pg,tp[0]),set()).add((tp[1]-r,tp[2]-c))
    mp=list(mine.keys());rp=list(real.keys())
    sc={}
    for (a,b),ss in shifts.items():
        best=0
        for (dr,dc) in ss:
            cnt=0;R=real[b]
            for (r,c),ch in mine[a].items():
                if R.get((r+dr,c+dc))==ch: cnt+=1
            best=max(best,cnt)
        sc[(a,b)]=best
    # Exact maximum-weight one-to-one matching, with dummy empty pages.
    # Hungarian algorithm: rows are real pages, columns are output pages.
    n=len(rp);m=len(mp)+n
    u=[0]*(n+1);v=[0]*(m+1);match=[0]*(m+1);way=[0]*(m+1)
    for i in range(1,n+1):
        match[0]=i;j0=0;dist=[float('inf')]*(m+1);used=[False]*(m+1)
        while True:
            used[j0]=True;i0=match[j0];delta=float('inf');j1=0
            for j in range(1,m+1):
                if used[j]:continue
                value=sc.get((mp[j-1],rp[i0-1]),0) if j<=len(mp) else 0
                cost=-value-u[i0]-v[j]
                if cost<dist[j]:dist[j]=cost;way[j]=j0
                if dist[j]<delta:delta=dist[j];j1=j
            for j in range(m+1):
                if used[j]:u[match[j]]+=delta;v[j]-=delta
                else:dist[j]-=delta
            j0=j1
            if match[j0]==0:break
        while True:
            j1=way[j0];match[j0]=match[j1];j0=j1
            if j0==0:break
    best=sum(sc.get((mp[j-1],rp[match[j]-1]),0) for j in range(1,len(mp)+1) if match[j])
    print("placed %d/%d chars on %d real pages (%d my pages)"%(best,total,len(rp),len(mp)))
    print("SCORE %.3f max"%(100.0*best/total))
main()
