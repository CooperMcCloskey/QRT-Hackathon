#!/usr/bin/env python3
"""Fit a general smooth function of t and array position from paired wave runs."""
import json
import math
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parent
ORDER=20


def features(t):
    return [1.0,t-.5]+[math.cos(math.pi*k*t) for k in range(1,ORDER+1)]


def solve_many(matrix,rhs):
    n=len(matrix);q=len(rhs[0]);work=[list(a)+list(b) for a,b in zip(matrix,rhs)]
    for col in range(n):
        pivot=max(range(col,n),key=lambda r:abs(work[r][col]))
        if abs(work[pivot][col])<1e-12: raise ValueError('Singular fit')
        work[col],work[pivot]=work[pivot],work[col]
        scale=work[col][col]
        for j in range(col,n+q): work[col][j]/=scale
        for r in range(n):
            if r==col: continue
            scale=work[r][col]
            for j in range(col,n+q): work[r][j]-=scale*work[col][j]
    return [r[n:] for r in work]


def smooth(ts,ys):
    X=[features(t) for t in ts];n=len(X);p=len(X[0]);best=None
    for ridge in [.001,.003,.01,.03,.1,.3,1.,3.,10.,30.,100.,300.]:
        A=[[sum(x[i]*x[j] for x in X) for j in range(p)] for i in range(p)]
        for j in range(p): A[j][j]+=ridge*((j-1)/4.)**4 if j>=2 else 1e-7
        inverse=solve_many(A,[[float(i==j) for j in range(p)] for i in range(p)])
        xy=[sum(x[j]*y for x,y in zip(X,ys)) for j in range(p)]
        coef=[sum(row[j]*xy[j] for j in range(p)) for row in inverse]
        prediction=[sum(a*b for a,b in zip(x,coef)) for x in X]
        leverages=[sum(x[i]*sum(inverse[i][j]*x[j] for j in range(p)) for i in range(p)) for x in X]
        cv=sum(((y-yhat)/max(.02,1-h))**2 for y,yhat,h in zip(ys,prediction,leverages))/n
        if best is None or cv<best['loo_mse']:
            best={'coefficients':coef,'ridge':ridge,'loo_mse':cv,'fitted':prediction,'raw':ys}
    return best


def main():
    runs={}
    for path in sorted((ROOT/'feedback').glob('collector_*.json')):
        data=json.loads(path.read_text())
        for r in data.get('rounds',[]):
            if len(r.get('observations',[]))==51:
                runs[r['id']]={round(o['t'],5):o for o in r['observations'] if 'raw_noisy_score' in o}
    ts=sorted(runs['WAVE_CONSTANT0_P'])
    kinds=['CONSTANT0','COS1','SIN1','COS2','SIN2'];series=[];curvatures=[]
    for kind in kinds:
        ys=[]
        for t in ts:
            yp=runs['WAVE_'+kind+'_P'][t]['raw_noisy_score'];ym=runs['WAVE_'+kind+'_M'][t]['raw_noisy_score']
            norm=1. if kind=='CONSTANT0' else .5
            value=(yp-ym)/norm
            if kind=='CONSTANT0':
                # Independent earlier scalar probes also estimate the constant component.
                numerator=.5*(yp-ym);denominator=.5
                for label,x in [('P05',.5),('M05',-.5),('P10',1.),('M10',-1.),('P05_R2',.5),('M05_R2',-.5)]:
                    if label in runs and t in runs[label]:
                        numerator+=x*(runs[label][t]['raw_noisy_score']+.5*x*x)
                        denominator+=x*x
                value=numerator/denominator
            ys.append(value)
            curvatures.append(-(yp+ym)/(.5*norm))
        fit=smooth(ts,ys);fit['kind']=kind;series.append(fit)
        print(kind,'ridge',fit['ridge'],'held-out coefficient RMSE',round(math.sqrt(fit['loo_mse']),5))
    curvature=sorted(curvatures)[len(curvatures)//2]
    model={'basis':'[1,t-0.5,cos(pi*t),...,cos(20*pi*t)]','t_order':ORDER,'spatial_basis':kinds,'series':series,'quadratic_penalty':curvature,'note':'General fitted model; no per-test answer lookup; official judge score unknown.'}
    (ROOT/'wave_model.json').write_text(json.dumps(model,indent=2)+'\n')
    template=(ROOT/'probe_v1.c').read_text();start=template.index('float* probe(float t, int* result_count) {');end=template.index('\nint main()',start)
    rounds=[]
    for index,factor in enumerate([1.,.8,1.2],1):
        identity='MODEL'+str(index)
        arrays=',\n'.join('        {'+', '.join('%.12g'%v for v in fit['coefficients'])+'}' for fit in series)
        body='''float* probe(float t, int* result_count) {
    static const double C[5][22] = {
%s
    };
    static float answer[4096];
    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0.0;
    double basis[22], a[5] = {0};
    basis[0] = 1.0; basis[1] = z - 0.5;
    for (int k = 1; k <= 20; ++k) basis[k+1] = cos(3.141592653589793 * k * z);
    for (int j = 0; j < 5; ++j)
        for (int k = 0; k < 22; ++k) a[j] += C[j][k] * basis[k];
    for (int i = 0; i < 4096; ++i) {
        double u = 6.283185307179586 * i / 4095.0;
        double value = (a[0] + a[1]*cos(u) + a[2]*sin(u) + a[3]*cos(2*u) + a[4]*sin(2*u)) * %.12g;
        answer[i] = isfinite(value) ? (float)fmax(-2.0, fmin(2.0, value)) : 0.0f;
    }
    *result_count = 4096;
    printf("QRT_BULK_%s t=%%.9g x=%%.9g\\n", (double)t, (double)answer[0]);
    return answer;
}
'''%(arrays,factor/(2*curvature),identity)
        source=template[:start]+body+template[end:];path=ROOT/('sol_wave_v'+str(index)+'.c')
        if path.exists() and path.read_text()!=source: raise RuntimeError('Preserve '+str(path))
        path.write_text(source)
        binary=ROOT/'.bin'/('wave_model'+str(index))
        build=subprocess.run(['cc','-std=c11','-O2','-Werror=return-type',str(path),'-lm','-o',str(binary)],text=True,capture_output=True)
        if build.returncode: raise RuntimeError(build.stderr)
        with tempfile.TemporaryDirectory(dir=str(ROOT/'.bin')) as directory:
            output=Path(directory)/'answer'
            for t in [0,.01,.2,.5,.8,.99,1]:
                r=subprocess.run([str(binary)],input=str(t)+'\n',text=True,capture_output=True,env=dict(os.environ,OUTPUT_PATH=str(output)),timeout=2)
                assert r.returncode==0
                vals=[float(x) for x in output.read_text().split()]
                assert len(vals)==4096 and all(math.isfinite(x) and abs(x)<=2 for x in vals)
        rounds.append({'id':identity,'file':path.name,'values':None,'N':4096,'design':{'kind':'fitted_model','factor':factor},'source':source})
    collector=(ROOT/'claude/collect.js').read_text().replace("if (typeof copy === 'function') copy(text);",'window.qrtBulkCollectorDone(out);').replace('(async () => {','return (async () => {',1)
    script='(function(){const QRT_BULK_ROUNDS='+json.dumps(rounds)+';\nconst QRT_BULK_COLLECTOR='+json.dumps(collector)+';\n'+(ROOT/'bulk_runner.js').read_text()+'\n})();\n'
    (ROOT/'models_console.js').write_text(script)
    print('Prepared 3 compiled general-function candidates and models_console.js. Judge validation pending.')


if __name__=='__main__':main()
