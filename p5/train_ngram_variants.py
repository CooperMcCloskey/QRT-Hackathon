"""Train only public WikiText-2 TRAIN. No local puzzle inputs or truth are read."""
from collections import Counter
from pathlib import Path
import re,math,base64,sys
from clean import paras
src=Path('corpus_train_ngram.txt')
cs=[None]+[Counter() for _ in range(5)]
for paragraph in paras(str(src)):
    text=' '+re.sub('[^a-z]+',' ',paragraph.lower()).strip()+' '
    for n in range(1,6):cs[n].update(text[i:i+n] for i in range(len(text)-n+1))
print('counts',*[len(c) for c in cs[1:]],flush=True)
base=[0,0,27,27+729,27+729+19683,27+729+19683+531441]
total=sum(cs[1].values())
def prob(s):
    n=len(s)
    if n==1:return (cs[1][s]+1)/(total+27)
    low=prob(s[1:])
    return (cs[n][s]+8*low)/(cs[n-1][s[:-1]]+8)
def key(s):
    x=0
    for c in s:x=x*27+(ord(c)-97 if c!=' ' else 26)
    return base[len(s)]+x
for name,limits in [('large',[27,729,7000,8000,7000]),('small',[27,729,2400,800,600]),('medium',[27,729,4000,3500,2500]),('compact',[27,729,3000,2200,1800])]:
    entries=[]
    for n in range(1,6):
        for s,c in cs[n].most_common(limits[n-1]):
            q=max(0,min(255,round(-math.log(max(prob(s),1e-10))*16)))
            entries.append((key(s),q))
    entries.sort();raw=bytearray();prev=0
    for k,q in entries:
        d=k-prev;prev=k
        while d>=128:raw.append((d&127)|128);d>>=7
        raw.append(d);raw.append(q)
    enc=base64.b64encode(raw).decode()
    Path('ngram_'+name+'_data.inc').write_text('// WikiText-2 TRAIN, quantized conditional character probabilities.\nstatic const char* NGRAM_DATA=\n'+''.join('"'+enc[i:i+120]+'"\n' for i in range(0,len(enc),120))+';\n')
    print(name,len(entries),len(enc),flush=True)
