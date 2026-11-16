from pathlib import Path
import subprocess,sys
s=Path('core_confidence_review.cpp').read_text();pos=s.index('struct HS');s=s[:pos]+Path('word_ngram_helper.inc').read_text()+s[pos:]
s=s.replace('void initLM() {','void initLM() {\n    ngStart();')
s=s.replace('double ww = f ? wt * 0.2 : wt;','double ww = f ? wt * 0.2 : wt; ngWord(s,ww);')
s=s.replace('    initLM();','    initLM(); ngFinish();')
s=s.replace('WJ = 0.5f;','WJ = 0.0f;')
s=s.replace('gs += runCheck2(buf, cc, c2, R.E);','gs += runCheck2(buf, cc, c2, R.E); if(b)gs += '+sys.argv[1]+'f*ngSeam(buf,cc,c2);if(R1)gs += '+sys.argv[1]+'f*ngSeam(buf,c2,CW);')
s=s.replace('for (int gi = 0; gi < NFr; gi++) {','for (int gi = 0; gi < NFr; gi++) {\n                if ((gi&31)==0 && elapsed()>TLIM) goto done_region;')
s=s.replace('    if (merges) { fr[f].ps','done_region:\n    if (merges) { fr[f].ps')
core='core_word_ngram_'+sys.argv[1]+'.cpp';out='sol_word_ngram_'+sys.argv[1]+'.cpp';Path(core).write_text(s)
subprocess.run(['python3','build.py',core,out,'wl6k.inc','calib_lite.inc'],check=True)
