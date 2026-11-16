import sys,collections
L=open(sys.argv[1]).read().split('\n');N=int(L[0])
T=open(sys.argv[1][:-3]+'.truth').read().split('\n');truth=[tuple(map(int,T[1+i].split())) for i in range(N)]
pcs=[[r.replace('_',' ') for r in L[1+i].split('|')] for i in range(N)]
fr=collections.OrderedDict()
for ln in open(sys.argv[2]):
    t=ln.split()
    if len(t)==4: fr.setdefault(int(t[1]),[]).append((int(t[0]),int(t[2]),int(t[3])))
k=0
for f,ps in fr.items():
    if len(ps)<3: continue
    cnt=collections.Counter((truth[p][0],truth[p][1]-r,truth[p][2]-c) for p,r,c in ps)
    maj=cnt.most_common(1)[0][0]
    r0=min(r for p,r,c in ps);c0=min(c for p,r,c in ps)
    g={}
    print('--- region',f,'pieces',len(ps),'majority',cnt.most_common(1)[0][1])
    for i,(p,r,c) in enumerate(ps):
        ok=(truth[p][0],truth[p][1]-r,truth[p][2]-c)==maj
        for a,row in enumerate(pcs[p]):
            for b,ch in enumerate(row): g[(r-r0+a,c-c0+b)]=ch if ok else ch.upper() if ch!=' ' else '~'
        print('   step',i,L[1+p],(r-r0,c-c0),'OK' if ok else 'WRONG true=%s'%(truth[p],))
    R=max(x[0] for x in g)+1;C=max(x[1] for x in g)+1
    for r in range(R): print('   |'+''.join(g.get((r,c),'#') for c in range(C))+'|')
    k+=1
    if k>=int(sys.argv[3]): break
