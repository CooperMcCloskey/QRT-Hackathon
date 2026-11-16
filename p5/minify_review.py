from pathlib import Path
import re,sys
s=Path(sys.argv[1]).read_text()
# Preserve every string/character literal and preprocessor line verbatim.
pat=r'("(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\')|//[^\n]*|/\*[\s\S]*?\*/'
s=re.sub(pat,lambda m:m.group(1) or '',s)
lines=[]
for line in s.splitlines():
 if line.lstrip().startswith('#'):lines.append(line.strip());continue
 line=re.sub(r'\s+',' ',line).strip()
 if line:lines.append(line)
s='\n'.join(lines)+'\n'
Path(sys.argv[2]).write_text(s)
print(sys.argv[2],len(s.encode()))
