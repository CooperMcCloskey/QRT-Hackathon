from pathlib import Path
import sys
size=sys.argv[1] if len(sys.argv)>1 else 'large'
weight=sys.argv[2] if len(sys.argv)>2 else '1.0'
s=Path('core_confidence_review.cpp').read_text()
pos=s.index('struct HS')
s=s[:pos]+Path('ngram_'+size+'_data.inc').read_text()+Path('ngram_helper.inc').read_text()+s[pos:]
s=s.replace('    initLM();','    initLM(); initNgram();')
# Dictionary remains as independent lexical evidence; the new model replaces
# the dictionary-derived trigram term, retaining all punctuation/case rules.
s=s.replace('float LINK_T = 6.0f, LINK_D = 3.0f, WJ = 0.5f;','float LINK_T = 6.0f, LINK_D = 3.0f, WJ = 0.0f;')
s=s.replace('gs += runCheck2(buf, cc, c2, R.E);', 'gs += runCheck2(buf, cc, c2, R.E); if(b) gs += '+weight+'f*ngSeam(buf,cc,c2); if(R1) gs += '+weight+'f*ngSeam(buf,c2,CW);')
# Long n-gram evaluation must not overrun the deadline in a single scan.
s=s.replace('for (int gi = 0; gi < NFr; gi++) {','for (int gi = 0; gi < NFr; gi++) {\n                if ((gi&31)==0 && elapsed()>TLIM) goto done_region;')
s=s.replace('    if (merges) { fr[f].ps', 'done_region:\n    if (merges) { fr[f].ps')
core=Path('core_ngram_'+size+'.cpp');core.write_text(s)
import subprocess
subprocess.run(['python3','build.py',str(core),'sol_ngram_'+size+'.cpp','wl6k.inc','calib_lite.inc'],check=True)
