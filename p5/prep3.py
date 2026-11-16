import sys,math,re,collections
sys.path.insert(0,'/home/claude/p5')
from clean import paras
NW=int(sys.argv[1]); out=sys.argv[2]
wc=collections.Counter()
for p in paras('/home/claude/corpus/wt_train.txt'):
    p=''.join(ch for ch in p if ord(ch)<127)
    for w in re.findall(r'[A-Za-z]+',p): wc[w.lower()]+=1
rank={}
for i,(w,_) in enumerate(wc.most_common()): rank[w]=min(rank.get(w,1e9),i)
for i,l in enumerate(open('/home/claude/corpus/en50k.txt')):
    w=l.split()[0]
    if w.isalpha() and w.isascii(): rank[w]=min(rank.get(w,1e9),i*1.3)
for i,l in enumerate(open('/home/claude/corpus/g10k.txt')):
    w=l.strip()
    if w.isalpha(): rank[w]=min(rank.get(w,1e9),i*2.0+500)
SUF=['s','es','ed','d','ing','ly','er','est','al','ness']
def stems(w):
    o=[]
    for sf in SUF:
        if len(w)-len(sf)>=3 and w.endswith(sf):
            st=w[:-len(sf)]; o.append(st)
            if sf in('ed','ing','er','est'):
                o.append(st+'e')
                if len(st)>=4 and st[-1]==st[-2]: o.append(st[:-1])
            if st.endswith('i') and sf in('es','ed','er','est','ly','ness'): o.append(st[:-1]+'y')
    return o
order=sorted(rank,key=lambda w:rank[w])
chosen=[]
cs=set()
for w in order:
    if len(chosen)>=NW: break
    if len(w)==1 and w not in 'ais': continue
    if len(w)==2 and rank[w]>3000: continue
    if len(w)>20: continue
    if any(s in cs for s in stems(w)): continue
    chosen.append(w); cs.add(w)
for w in 'aist':
    if w not in cs: chosen.append(w); cs.add(w)
bounds=[300,1500,4000,10**9]
buckets=[[] for _ in bounds]
for i,w in enumerate(chosen):
    for b,lim in enumerate(bounds):
        if i<lim: buckets[b].append(w);break
syms=[]
for b in buckets:
    b.sort(); prev='';pp=0
    for w in b:
        p=0
        while p<len(prev) and p<len(w) and prev[p]==w[p]: p+=1
        d=p-pp
        code={0:26,1:27,-1:28,2:29,-2:30}.get(d)
        if code is None: syms+= [31,p]
        else: syms.append(code)
        syms+=[ord(ch)-97 for ch in w[p:]]
        prev=w;pp=p
    syms+=[31,31]
bits=[]
for s in syms:
    for j in range(5): bits.append(s>>j&1)
while len(bits)%6: bits.append(1)
AL='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
sb=''.join(AL[sum(bits[i+j]<<j for j in range(6))] for i in range(0,len(bits),6))
open(out,'w').write('static const char*WL="%s";\n'%sb)
print('words',len(chosen),'chars',len(sb),'per word',len(sb)/len(chosen))
def indict(w):
    return w in cs or any(s in cs for s in stems(w))
tot=hit=0
for p in paras('/home/claude/corpus/wt_valid.txt'):
    for w in re.findall(r'[A-Za-z]+',p):
        if w[0].isupper(): continue
        tot+=1; hit+=indict(w.lower())
print('coverage',hit/tot)
ex="Jigsaw puzzles began in the s when a London mapmaker glued maps to thin wooden boards and cut them along the borders of The first bicycles had no pedals at all riders sat on a wooden frame and pushed themselves Kites were invented in China more than two thousand years ago"
print([w for w in ex.split() if not indict(w.lower())])
