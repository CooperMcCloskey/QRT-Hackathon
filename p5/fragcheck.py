import sys,collections
# fragcheck.py in out : treat each output page as fragment, check internal consistency vs truth
L=open(sys.argv[1]).read().split('\n');N=int(L[0])
T=open(sys.argv[1][:-3]+'.truth').read().split('\n');truth=[tuple(map(int,T[1+i].split())) for i in range(N)]
pcs=[L[1+i] for i in range(N)]
fr=collections.defaultdict(list)
for ln in open(sys.argv[2]):
    t=ln.split()
    if len(t)==4: fr[int(t[1])].append((int(t[0]),int(t[2]),int(t[3])))
bad=0;tot=0;sizes=[]
for f,ps in fr.items():
    if len(ps)<2: continue
    tot+=1;sizes.append(len(ps))
    cnt=collections.Counter()
    for p,r,c in ps:
        tp=truth[p]; cnt[(tp[0],tp[1]-r,tp[2]-c)]+=1
    if len(cnt)>1 or list(cnt)[0][0]<0:
        bad+=1
        if len(sys.argv)>3: print('BAD frag',f,[(pcs[p],r,c,truth[p]) for p,r,c in ps])
print('multi frags',tot,'bad',bad,'sizes',sorted(sizes,reverse=True)[:15],'pieces in multi',sum(sizes),'of',N)
