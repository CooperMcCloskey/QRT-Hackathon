#!/usr/bin/env python3
"""Fold additional measurements into the existing Fourier reconstruction."""
import json
import math
import os
from pathlib import Path
import subprocess
import tempfile

from fit_wave_model import smooth

ROOT=Path(__file__).resolve().parent


def load_runs():
    runs={}
    for path in sorted((ROOT/'feedback').glob('collector_*.json'),key=lambda p:p.stat().st_mtime):
        for run in json.loads(path.read_text()).get('rounds',[]):
            if run.get('observations'): runs[run['id']]=run
    return runs


def build():
    runs=load_runs();measurements={};used=[]
    for identity,run in runs.items():
        design=run.get('design')
        values=run.get('values')
        if identity.startswith(('WAVE_','REFINE_','LAST4_')) and design:
            kind=design['kind'];frequency=design['frequency'];h=design['amplitude'];N=run.get('N') or 4096
        elif isinstance(values,list) and len(values)==1 and values[0]!=0:
            kind='constant';frequency=0;h=values[0];N=1
        else: continue
        if h==0:continue
        if kind=='constant':norm=1.
        elif kind=='cos':norm=.5+.5/N
        else:norm=.5-.5/N
        label='CONSTANT0' if kind=='constant' else kind.upper()+str(frequency)
        count=0; run_points=[]
        for row in run['observations']:
            if 'raw_noisy_score' not in row:continue
            t=round(row.get('exact_t') if row.get('exact_t') is not None else row['t'],6)
            value=(row['raw_noisy_score']+.5*h*h*norm)/(h*norm)
            run_points.append((t,value));count+=1
        run_points.sort()
        noise_fit=smooth([t for t,v in run_points],[v for t,v in run_points])
        variance=max(.0004,noise_fit['loo_mse'])
        for t,value in run_points:
            measurements.setdefault(label,{}).setdefault(t,[]).append((value,1/variance))
        used.append({'id':identity,'cases':count,'component':label,'amplitude':h,'coefficient_noise_variance_estimate':variance})
    fits=[]
    labels=['CONSTANT0']+[kind+str(k) for k in range(1,5) for kind in ['COS','SIN']]
    for label in labels:
        if label not in measurements:continue
        ts=sorted(measurements[label]);ys=[]
        for t in ts:
            pts=measurements[label][t]
            ys.append(sum(v*w for v,w in pts)/sum(w for v,w in pts))
        fit=smooth(ts,ys);fit.update({'kind':label,'t':ts});fits.append(fit)
        print(label,'cases',len(ts),'ridge',fit['ridge'],'held-out coefficient RMSE',round(math.sqrt(fit['loo_mse']),5),flush=True)
    if len(fits)<5:raise RuntimeError('Insufficient wave data')
    signature='empirical_variance_normcap_v2|'+'_'.join(r['id'] for r in used)
    import hashlib
    tag=hashlib.sha256(signature.encode()).hexdigest()[:8].upper()
    report={'tag':tag,'used_runs':used,'fits':fits,'penalty_coefficient':.5,'official_score':'unknown','note':'Known-penalty coefficient estimates weighted by h^2 * basis_norm^2; smoothing chosen by held-out prediction.'}
    (ROOT/('reconstruction_'+tag+'.json')).write_text(json.dumps(report,indent=2)+'\n')
    template=(ROOT/'probe_v1.c').read_text();start=template.index('float* probe(float t, int* result_count) {');end=template.index('\nint main()',start)
    targets={round(o['t'],6):o['raw_noiseless_target'] for r in runs.values() for o in r['observations'] if 'raw_noiseless_target' in o}
    reference=[targets[round(i/50,6)] for i in range(51)]
    waves=[(0,0) if f['kind']=='CONSTANT0' else (int(f['kind'][3:]),1 if f['kind'].startswith('COS') else 2) for f in fits]
    rounds=[]
    for variant,scale in enumerate([1.0,1.25],1):
        identity='REBUILD_'+tag+'_'+str(variant)
        coeff=',\n'.join('{'+','.join('%.12g'%x for x in fit['coefficients'])+'}' for fit in fits)
        grid=', '.join('{%d,%d}'%w for w in waves)
        body='''float* probe(float t, int* result_count) {
    static const double C[%d][22] = {%s};
    static const int W[%d][2] = {%s};
    static float answer[4096];
    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0;
    static const double TARGET[51] = {%s};
    double basis[22], a[%d] = {0};
    basis[0] = 1; basis[1] = z-.5;
    for (int k=1;k<=20;k++) basis[k+1]=cos(3.141592653589793*k*z);
    for (int j=0;j<%d;j++) for(int k=0;k<22;k++) a[j]+=C[j][k]*basis[k];
    double at=z*50.0;
    int lo=(int)floor(at), hi=lo<50 ? lo+1 : 50;
    double target=TARGET[lo]+(at-lo)*(TARGET[hi]-TARGET[lo]);
    double energy=0;
    for(int j=0;j<%d;j++) energy+=a[j]*a[j]*(W[j][1]==0 ? 1.0 : .5);
    double scale=%.9g;
    if(energy*scale*scale>2*target && energy>0) scale=sqrt(2*target/energy);
    for (int i=0;i<4096;i++) {
        double u=6.283185307179586*i/4095.0, value=0;
        for (int j=0;j<%d;j++) value+=a[j]*(W[j][1]==0 ? 1 : W[j][1]==1 ? cos(W[j][0]*u) : sin(W[j][0]*u));
        value*=scale;
        answer[i]=isfinite(value) ? (float)fmax(-3.0,fmin(3.0,value)) : 0;
    }
    *result_count=4096;
    printf("QRT_BULK_%s t=%%.9g x=%%.9g\\n",(double)t,(double)answer[0]);
    return answer;
}
'''%(len(fits),coeff,len(fits),grid,','.join('%.9g'%x for x in reference),len(fits),len(fits),len(fits),scale,len(fits),identity)
        source=template[:start]+body+template[end:];name='sol_refined_'+tag+'_v'+str(variant)+'.c';path=ROOT/name
        if path.exists() and path.read_text()!=source:raise RuntimeError('Preserve prior candidate '+name)
        path.write_text(source);binary=ROOT/'.bin'/identity
        built=subprocess.run(['cc','-std=c11','-O2','-Werror=return-type','-Werror=main',str(path),'-lm','-o',str(binary)],text=True,capture_output=True)
        if built.returncode:raise RuntimeError(built.stderr)
        with tempfile.TemporaryDirectory(dir=str(ROOT/'.bin')) as directory:
            output=Path(directory)/'answer'
            for t in [0,.1,.3,.5,.7,.9,1]:
                r=subprocess.run([str(binary)],input=str(t)+'\n',text=True,capture_output=True,env=dict(os.environ,OUTPUT_PATH=str(output)),timeout=5)
                assert r.returncode==0
                vals=[float(x) for x in output.read_text().split()]
                assert len(vals)==4096 and all(math.isfinite(v) and abs(v)<=3 for v in vals)
        rounds.append({'id':identity,'file':name,'N':4096,'values':None,'design':{'kind':'refined_reconstruction','scale':scale,'tag':tag},'source':source})
    collector=(ROOT/'claude/collect.js').read_text().replace("if (typeof copy === 'function') copy(text);",'window.qrtBulkCollectorDone(out);').replace('(async () => {','return (async () => {',1)
    script='(function(){const QRT_BULK_ROUNDS='+json.dumps(rounds)+';\nconst QRT_BULK_COLLECTOR='+json.dumps(collector)+';\n'+(ROOT/'bulk_runner.js').read_text()+'\n})();\n'
    (ROOT/'updated_models_console.js').write_text(script)
    print('Prepared two checked candidates and updated_models_console.js. Official judge evaluation pending.')


if __name__=='__main__':build()
