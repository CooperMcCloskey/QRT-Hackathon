from collections import Counter
from pathlib import Path
from clean import paras
import re,math,base64,itertools
cs=[None]+[Counter() for _ in range(3)]
for p in paras('corpus_train_ngram.txt'):
 text=' '+re.sub('[^a-z]+',' ',p.lower()).strip()+' '
 for n in range(1,4):cs[n].update(text[i:i+n] for i in range(len(text)-n+1))
levels=[0,.25,.5,.75,1,1.25,1.5,1.75,2,2.5,3,3.5,4,5,6.5,9]
total=sum(cs[1].values());alpha='abcdefghijklmnopqrstuvwxyz '
def prob(s):
 if len(s)==1:return (cs[1][s]+1)/(total+27)
 return (cs[len(s)][s]+8*prob(s[1:]))/(cs[len(s)-1][s[:-1]]+8)
q=[]
for n in range(1,4):
 for t in itertools.product(alpha,repeat=n):
  lp=-math.log(prob(''.join(t)));q.append(min(range(16),key=lambda i:abs(lp-levels[i])))
if len(q)%2:q.append(0)
b=bytes(q[i]|q[i+1]<<4 for i in range(0,len(q),2));enc=base64.b64encode(b).decode()
Path('trigram_data_review.inc').write_text('static const char*TG_DATA=\n'+''.join('"'+enc[i:i+120]+'"\n' for i in range(0,len(enc),120))+';\n')
print(len(enc))
