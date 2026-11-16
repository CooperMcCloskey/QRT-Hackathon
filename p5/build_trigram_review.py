from pathlib import Path
import subprocess,sys
s=Path('core_confidence_review.cpp').read_text()
a=s.index('    static double c1[K]');b=s.index('    auto isV',a);s=s[:a]+s[b:]
s=s.replace('double wt = BW[min(bk[wi], 5)];','').replace('double ww = f ? wt * 0.2 : wt;','')
a=s.index('            int q[40]');b=s.index('\n        }\n    }',a);s=s[:a]+s[b:]
a=s.index('    double T = 0, ST = 0;');b=s.index('\n}\ninline bool ends',a)
s=s[:a]+'''
    static const float Q[16]={0,.25,.5,.75,1,1.25,1.5,1.75,2,2.5,3,3.5,4,5,6.5,9};
    unsigned acc=0;int bits=0;vector<float> vals;
    for(const char* p=TG_DATA;*p && *p!='=';p++) {
        acc=(acc<<6)|dec64(*p);bits+=6;
        if(bits>=8) {bits-=8;int b=(acc>>bits)&255;vals.push_back(-Q[b&15]);vals.push_back(-Q[b>>4]);}
    }
    int i=0;
    for(int a=0;a<27;a++)lp1[a]=vals[i++];
    for(int a=0;a<27;a++)for(int b=0;b<27;b++)lp2[a][b]=vals[i++];
    for(int a=0;a<27;a++)for(int b=0;b<27;b++)for(int c=0;c<27;c++)lp3[a][b][c]=vals[i++];
'''+s[b:]
s=s.replace('WJ = 0.5f;', 'WJ = '+sys.argv[1]+'f;')
pos=s.index('void initLM()');s=s[:pos]+Path('trigram_data_review.inc').read_text()+s[pos:]
s=s.replace('for (int gi = 0; gi < NFr; gi++) {','for (int gi = 0; gi < NFr; gi++) {\n                if ((gi&31)==0 && elapsed()>TLIM) goto done_region;')
s=s.replace('    if (merges) { fr[f].ps', 'done_region:\n    if (merges) { fr[f].ps')
core='core_trigram_'+sys.argv[1]+'.cpp';out='sol_trigram_'+sys.argv[1]+'.cpp';Path(core).write_text(s)
subprocess.run(['python3','build.py',core,out,'wl6k.inc','calib_lite.inc'],check=True)
