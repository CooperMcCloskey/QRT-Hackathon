import sys
L=open(sys.argv[1]).read().split('\n');N=int(L[0]);pcs=[[r.replace('_',' ') for r in L[1+i].split('|')] for i in range(N)]
pages={}
for ln in open(sys.argv[2]):
    t=ln.split()
    if len(t)!=4: continue
    p,pg,r,c=map(int,t)
    g=pages.setdefault(pg,{})
    for a,row in enumerate(pcs[p]):
        for b,ch in enumerate(row): g[(r+a,c+b)]=ch
for pg,g in pages.items():
    R=max(k[0] for k in g)+1;C=max(k[1] for k in g)+1
    print('--- page',pg,R,'x',C)
    for r in range(R): print('|'+''.join(g.get((r,c),'#') for c in range(C))+'|')
