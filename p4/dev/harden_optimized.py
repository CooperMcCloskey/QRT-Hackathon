from pathlib import Path
import sys
s=Path(sys.argv[1]).read_text()
# Stop only on a rigorous geometric/visit-count certificate.
pos=s.index('    int bestL=1<<30;',s.index('int main()'))
s=s[:pos]+'''    int rlo=R,rhi=0,clo=C,chi=0;
    for(int p:pearls) { rlo=min(rlo,p/C);rhi=max(rhi,p/C);clo=min(clo,p%C);chi=max(chi,p%C); }
    const int lower=max(4,max(2*(rhi-rlo+chi-clo),(nPearls+1)/2*2));
'''+s[pos:]
s=s.replace('        int fails=0,steps=0;', '        int fails=0,steps=0;\n        if(lenOf(cur)==lower) tend=elapsed();')
s=s.replace('                continue;\n            }\n            int e0;', '                if(lenOf(cur)==lower) break;\n                continue;\n            }\n            int e0;')
s=s.replace('            if(ok) fails=0; else fails++;', '            if(ok) { fails=0; if(lenOf(cur)==lower) break; } else fails++;')
# The initial loop can already satisfy all pearls.
s=s.replace('    sa::setState(start);\n    sa::run(b,bl,0.25', '    if(validLoop(start)) { b=start; bl=lenOf(start); }\n    sa::setState(start);\n    sa::run(b,bl,0.25')
Path('finSolve_optimized.cpp').write_text(s)
