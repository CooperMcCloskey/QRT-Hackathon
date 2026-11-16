from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
a=s.index('float wrapJ(');b=s.index('const int CH =',a);s=s[:a]+s[b:]
# Preserve quoted literals byte-for-byte while removing comments and redundant space.
pat=r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|//[^\n]*|/\*[\s\S]*?\*/'
s=re.sub(pat,lambda m:m.group(1) or '',s)
parts=re.split(r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')',s)
for i in range(0,len(parts),2):
 p=parts[i]
 p=re.sub('[ \\t]+',' ',p)
 p=re.sub(r' *([{}();,\[\]]) *',r'\1',p)
 p=re.sub(r'\n +','\n',p)
 parts[i]=p
s=''.join(parts)
s='\n'.join(l.strip() for l in s.splitlines() if l.strip())+'\n'
Path(sys.argv[2]).write_text(s)
print(sys.argv[2],len(s.encode()))
