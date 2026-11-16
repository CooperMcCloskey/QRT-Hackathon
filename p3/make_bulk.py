#!/usr/bin/env python3
"""Build one console script using the existing Claude collector and checked C files."""
import json
from pathlib import Path
import subprocess
import tempfile
import os

ROOT = Path(__file__).resolve().parent
DESIGNS = [('P05',[.5]),('M05',[-.5]),('P10',[1.]),('M10',[-1.]),
           ('P05_R2',[.5]),('M05_R2',[-.5]),('ZERO',[0.]),
           ('N2_ZERO',[0.,0.]),('N2_LEFT',[.5,0.]),('N2_RIGHT',[0.,.5]),('N2_BOTH',[.5,.5])]


def main():
    template = (ROOT/'probe_v1.c').read_text()
    begin = template.index('float* probe(float t, int* result_count) {')
    end = template.index('\nint main()',begin)
    rounds=[]
    (ROOT/'.bin').mkdir(exist_ok=True)
    for identity, values in DESIGNS:
        body = '''float* probe(float t, int* result_count) {
    static float answer[%d] = {%s};
    *result_count = %d;
    printf("QRT_BULK_%s t=%%.9g x=%%.9g\\n", (double)t, (double)answer[0]);
    return answer;
}
''' % (len(values), ', '.join('%.8ff'%x for x in values),len(values),identity)
        source=template[:begin]+body+template[end:]
        path=ROOT/('probe_bulk_'+identity+'.c')
        if path.exists() and path.read_text()!=source: raise RuntimeError('Preserve '+str(path))
        path.write_text(source)
        binary=ROOT/'.bin'/('bulk_'+identity)
        built=subprocess.run(['cc','-std=c11','-O2','-Werror=return-type',str(path),'-o',str(binary)],capture_output=True,text=True)
        if built.returncode: raise RuntimeError(built.stderr)
        with tempfile.TemporaryDirectory(dir=str(ROOT/'.bin')) as directory:
            output=Path(directory)/'answer'
            r=subprocess.run([str(binary)],input='0\n',text=True,capture_output=True,env=dict(os.environ,OUTPUT_PATH=str(output)),timeout=2)
            if r.returncode or output.read_text()!=''.join('%.6f\n'%x for x in values): raise RuntimeError('Output check failed '+identity)
        rounds.append({'id':identity,'file':path.name,'values':values,'source':source})
    collector=(ROOT/'claude/collect.js').read_text()
    old="if (typeof copy === 'function') copy(text);"
    assert old in collector
    collector=collector.replace(old,'window.qrtBulkCollectorDone(out);')
    # Return the existing async collector promise so its errors can be caught.
    collector=collector.replace('(async () => {','return (async () => {',1)
    out='(function () {\nconst QRT_BULK_ROUNDS = '+json.dumps(rounds)+';\n'
    out+='const QRT_BULK_COLLECTOR = '+json.dumps(collector)+';\n'
    out+=(ROOT/'bulk_runner.js').read_text()+'\n})();\n'
    (ROOT/'bulk_console.js').write_text(out)
    print('Built bulk_console.js with {} compiled/output-checked C probes and the existing Claude collector.'.format(len(rounds)))


if __name__=='__main__': main()
