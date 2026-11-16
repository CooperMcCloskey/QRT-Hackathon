from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
a=s.index('    static double c1[K]');b=s.index('    auto isV',a);s=s[:a]+s[b:]
s=s.replace('double wt = BW[min(bk[wi], 5)];','')
s=s.replace('double ww = f ? wt * 0.2 : wt;','')
a=s.index('            int q[40]');b=s.index('\n        }\n    }',a);s=s[:a]+s[b:]
a=s.index('    double T = 0, ST = 0;');b=s.index('\n}\ninline bool ends',a);s=s[:a]+s[b:]
a=s.index('    int cb = cls(b), cx = cls(x); float g = 0;');b=s.index('    if (isUp(x)',a);s=s[:a]+'    float g = 0;\n'+s[b:]
Path(sys.argv[2]).write_text(s)
