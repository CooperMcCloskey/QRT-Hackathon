from pathlib import Path
from collections import Counter
from clean import paras
import re,math,base64,heapq,pickle
cache=Path('ngram_train_counts_review.pkl')
if cache.exists():cs=pickle.loads(cache.read_bytes())
else:
 cs=[None]+[Counter() for _ in range(5)]
 for paragraph in paras('corpus_train_ngram.txt'):
  text=' '+re.sub('[^a-z]+',' ',paragraph.lower()).strip()+' '
  for n in range(1,6):cs[n].update(text[i:i+n] for i in range(len(text)-n+1))
 cache.write_bytes(pickle.dumps(cs))
base=[0,0,27,756,20439,551880];total=sum(cs[1].values())
def prob(s):
 if len(s)==1:return (cs[1][s]+1)/(total+27)
 return (cs[len(s)][s]+8*prob(s[1:]))/(cs[len(s)-1][s[:-1]]+8)
def key(s):
 x=0
 for c in s:x=x*27+(ord(c)-97 if c!=' ' else 26)
 return base[len(s)]+x
for name,limits,mode in [('selected',[27,729,2000,900,900],0),('selected2',[27,729,1500,1200,1100],0),('selected3',[27,729,2000,900,900],1)]:
 chosen={};entries=[]
 for n in range(1,6):
  scored=[]
  for s,c in cs[n].items():
   p=prob(s);q=prob(s[1:]) if n>1 else p
   if n>2:
    suffix=s[1:]
    while suffix not in chosen:suffix=suffix[1:]
    q=chosen[suffix]
   gain=c if n<3 else (abs(c-(cs[n-1][s[:-1]]+8)*q)*abs(math.log(p/q)) if mode==0 else c*abs(math.log(p/q)))
   scored.append((gain,s,p))
  for _,s,p in heapq.nlargest(limits[n-1],scored):
   chosen[s]=p;entries.append((key(s),max(0,min(254,round(-math.log(max(p,1e-10))*16)))))
 entries.sort();raw=bytearray();prev=0
 for k,q in entries:
  d=k-prev;prev=k
  while d>=128:raw.append((d&127)|128);d>>=7
  raw.extend([d,q])
 enc=base64.b64encode(raw).decode()
 Path('ngram_'+name+'_data.inc').write_text('static const char* NGRAM_DATA=\n'+''.join('"'+enc[i:i+120]+'"\n' for i in range(0,len(enc),120))+';\n')
 print(name,len(entries),len(enc),flush=True)
