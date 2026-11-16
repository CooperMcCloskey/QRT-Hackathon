from pathlib import Path
import re,base64,sys
s=re.search(r'"([A-Za-z0-9+/]+)"',Path('wl6k.inc').read_text()).group(1)
raw=base64.b64decode(s+'='*((-len(s))%4));sy=[];acc=bits=0
# The dictionary uses little-endian groups, unlike conventional base64.
a='ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
for c in s:
 acc|=a.index(c)<<bits;bits+=6
 while bits>=5:sy.append(acc&31);acc>>=5;bits-=5
prev=cur='';pp=0;i=0;words=[];bucket=0;opened=False
while i<len(sy):
 x=sy[i];i+=1
 if x<26:
  if opened:cur+=chr(97+x)
  continue
 if opened and cur:words.append((cur,bucket));prev=cur
 opened=False
 if x==31:
  if i==len(sy):break
  p=sy[i];i+=1
  if p==31:bucket+=1;prev='';pp=0;continue
 else:p=pp+{26:0,27:1,28:-1,29:2,30:-2}[x]
 if p<0 or p>len(prev):break
 cur=prev[:p];pp=p;opened=True
if opened and cur:words.append((cur,bucket))
for count in [1500,2500,3500,4000]:
 chosen=words[:count];sy=[]
 for b in range(4):
  prev='';pp=0
  for w,bk in chosen:
   if bk!=b:continue
   p=0
   while p<min(len(prev),len(w)) and prev[p]==w[p]:p+=1
   d=p-pp;code={0:26,1:27,-1:28,2:29,-2:30}.get(d)
   sy.extend([31,p] if code is None else [code]);sy.extend(ord(c)-97 for c in w[p:]);prev=w;pp=p
  sy.extend([31,31])
 acc=bits=0;out=''
 for x in sy:
  acc|=x<<bits;bits+=5
  while bits>=6:out+=a[acc&63];acc>>=6;bits-=6
 if bits:out+=a[(acc|(~0<<bits))&63]
 Path('wl_review_%d.inc'%count).write_text('static const char*WL="'+out+'";\n')
 print(count,len(out))
