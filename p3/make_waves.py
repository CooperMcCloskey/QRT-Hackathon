#!/usr/bin/env python3
"""Paired wave probes, using the working manual-panel collection workflow."""
import json
import math
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parent
DESIGNS=[('constant',0),('cos',1),('sin',1),('cos',2),('sin',2)]
SIGNS=[1,-1]
AMPLITUDE=.5
PREFIX='WAVE_'
SCRIPT_FILE='waves_console.js'
DESIGN_FILE='wave_designs.json'


def main():
    template=(ROOT/'probe_v1.c').read_text()
    start=template.index('float* probe(float t, int* result_count) {')
    end=template.index('\nint main()',start)
    rounds=[]
    for kind,frequency in DESIGNS:
        for sign in SIGNS:
            identity=PREFIX+kind.upper()+str(frequency)+('_P' if sign==1 else '_M')
            amplitude=sign*AMPLITUDE
            expression='1.0' if kind=='constant' else '%s(6.283185307179586 * %d * u)'%(kind,frequency)
            body='''float* probe(float t, int* result_count) {
    static float answer[4096];
    for (int i = 0; i < 4096; ++i) {
        double u = (double)i / 4095.0;
        answer[i] = (float)(%.8f * %s);
    }
    *result_count = 4096;
    printf("QRT_BULK_%s t=%%.9g x=%%.9g\\n", (double)t, (double)answer[0]);
    return answer;
}
'''%(amplitude,expression,identity)
            source=template[:start]+body+template[end:]
            path=ROOT/('probe_'+identity+'.c')
            if path.exists() and path.read_text()!=source: raise RuntimeError('Preserve '+str(path))
            path.write_text(source)
            binary=ROOT/'.bin'/identity
            r=subprocess.run(['cc','-std=c11','-O2','-Werror=return-type',str(path),'-lm','-o',str(binary)],capture_output=True,text=True)
            if r.returncode: raise RuntimeError(r.stderr)
            with tempfile.TemporaryDirectory(dir=str(ROOT/'.bin')) as directory:
                output=Path(directory)/'answer'
                r=subprocess.run([str(binary)],input='0\n',text=True,capture_output=True,env=dict(os.environ,OUTPUT_PATH=str(output)),timeout=2)
                assert r.returncode==0
                values=[float(x) for x in output.read_text().split()]
                assert len(values)==4096 and all(math.isfinite(x) and abs(x)<=AMPLITUDE+.000001 for x in values)
            rounds.append({'id':identity,'file':path.name,'N':4096,'values':None,'design':{'kind':kind,'frequency':frequency,'amplitude':amplitude,'grid':'i/(N-1)'},'source':source})
    collector=(ROOT/'claude/collect.js').read_text().replace("if (typeof copy === 'function') copy(text);",'window.qrtBulkCollectorDone(out);').replace('(async () => {','return (async () => {',1)
    script='(function(){\nconst QRT_BULK_ROUNDS='+json.dumps(rounds)+';\nconst QRT_BULK_COLLECTOR='+json.dumps(collector)+';\n'+(ROOT/'bulk_runner.js').read_text()+'\n})();\n'
    (ROOT/SCRIPT_FILE).write_text(script)
    (ROOT/DESIGN_FILE).write_text(json.dumps([{k:v for k,v in r.items() if k!='source'} for r in rounds],indent=2)+'\n')
    print('Prepared {} compiled wave probes, amplitude {}, N=4096; manual-panel workflow preserved.'.format(len(rounds),AMPLITUDE))


if __name__=='__main__': main()
