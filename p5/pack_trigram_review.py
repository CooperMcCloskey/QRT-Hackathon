from pathlib import Path
import re,base64,collections
s=Path('trigram_data_review.inc').read_text();data=base64.b64decode(''.join(re.findall(r'"([A-Za-z0-9+/=]+)"',s)))
out=bytearray(len(data).to_bytes(2,'little'));hist=collections.defaultdict(list);pos=0
while pos<len(data):
 flagpos=len(out);out.append(0);flag=0
 for bit in range(8):
  if pos>=len(data):break
  key=data[pos:pos+3];best=0;off=0
  for old in reversed(hist[key][-100:]):
   if pos-old>2048:break
   n=3
   while n<34 and pos+n<len(data) and data[old+n]==data[pos+n]:n+=1
   if n>best:best=n;off=pos-old
  if best>=3:
   flag|=1<<bit;v=((off-1)<<5)|(best-3);out+=v.to_bytes(2,'little');n=best
  else:out.append(data[pos]);n=1
  for i in range(pos,pos+n):hist[data[i:i+3]].append(i)
  pos+=n
 out[flagpos]=flag
enc=base64.b64encode(out).decode()
Path('trigram_packed_review.inc').write_text('static const char*TG_DATA=\n'+''.join('"'+enc[i:i+120]+'"\n' for i in range(0,len(enc),120))+';\n')
print('raw',len(data),'lz',len(out),'source',len(enc))
