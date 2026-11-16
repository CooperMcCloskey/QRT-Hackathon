#!/usr/bin/env python3
"""Clipboard driver for controlled judge experiments. Python 3.8+, stdlib only."""
import argparse
import collections
import json
import math
import os
from pathlib import Path
import subprocess
import tempfile
import time
import uuid

ROOT = Path(__file__).resolve().parent
PLAN = [
    ('zero', [0.0]), ('plus', [1.0]), ('minus', [-1.0]),
    ('half', [0.5]), ('plus_repeat', [1.0]), ('minus_repeat', [-1.0]),
    ('two_zero', [0.0, 0.0]), ('left', [0.5, 0.0]),
    ('right', [0.0, 0.5]), ('both', [0.5, 0.5]),
]


def make_source(name, values):
    original = (ROOT / 'probe_v1.c').read_text()
    start = original.index('float* probe(float t, int* result_count) {')
    end = original.index('\nint main()', start)
    literals = ', '.join('{:.9f}f'.format(x) for x in values)
    body = '''float* probe(float t, int* result_count) {
    static float answer[%d] = {%s};
    *result_count = %d;
    printf("QRT_DRIVER round=%s t=%%.9g\\n", (double)t);
    return answer;
}
''' % (len(values), literals, len(values), name)
    return original[:start] + body + original[end:]


def prepare():
    rounds = []
    (ROOT / '.bin').mkdir(exist_ok=True)
    for index, (name, values) in enumerate(PLAN, 1):
        identity = 'auto_{:02d}_{}'.format(index, name)
        path = ROOT / ('probe_' + identity + '.c')
        source = make_source(identity, values)
        if path.exists() and path.read_text() != source:
            raise RuntimeError('Refusing to overwrite a different experiment: ' + str(path))
        if not path.exists():
            path.write_text(source)
        binary = ROOT / '.bin' / identity
        built = subprocess.run(['cc', '-std=c11', '-O2', '-Werror=return-type', '-Werror=main', str(path), '-o', str(binary)],
                               capture_output=True, text=True)
        if built.returncode:
            raise RuntimeError(built.stderr)
        with tempfile.TemporaryDirectory(dir=str(ROOT / '.bin')) as directory:
            output = Path(directory) / 'answer'
            ran = subprocess.run([str(binary)], input='0\n', text=True, capture_output=True,
                                 env=dict(os.environ, OUTPUT_PATH=str(output)), timeout=2)
            expected = ''.join('{:.6f}\n'.format(x) for x in values)
            if ran.returncode or not output.exists() or output.read_text() != expected:
                raise RuntimeError('Local format check failed for ' + identity)
        rounds.append({'id': identity, 'file': path.name, 'values': values, 'source': source})
    return rounds


def clipboard_write(text):
    completed = subprocess.run(['pbcopy'], input=text, text=True)
    if completed.returncode:
        raise RuntimeError('Clipboard copy failed. Run this command in your normal Mac terminal.')


def clipboard_read():
    return subprocess.check_output(['pbpaste'], text=True)


def bundle(rounds, mode, token):
    config = {'rounds': rounds, 'mode': mode, 'token': token}
    return '(function () {\nconst QRT_CONFIG = ' + json.dumps(config) + ';\n' + (ROOT / 'browser_driver.js').read_text() + '\n})();\n'


def solve(matrix, vector):
    size = len(vector)
    work = [list(row) + [value] for row, value in zip(matrix, vector)]
    for column in range(size):
        pivot = max(range(column, size), key=lambda r: abs(work[r][column]))
        if abs(work[pivot][column]) < 1e-12:
            return None
        work[column], work[pivot] = work[pivot], work[column]
        scale = work[column][column]
        work[column] = [x / scale for x in work[column]]
        for row in range(size):
            if row == column:
                continue
            scale = work[row][column]
            work[row] = [a - scale*b for a, b in zip(work[row], work[column])]
    return [work[i][-1] for i in range(size)]


def quadratic_fit(points):
    if len(set(x for x, _ in points)) < 3:
        return None
    features = [[1.0, x, x*x] for x, _ in points]
    matrix = [[sum(f[i]*f[j] for f in features) for j in range(3)] for i in range(3)]
    target = [sum(f[i]*y for f, (_, y) in zip(features, points)) for i in range(3)]
    fit = solve(matrix, target)
    if fit is None:
        return None
    c, a, d = fit
    residuals = [y - (c+a*x+d*x*x) for x, y in points]
    repeats = collections.defaultdict(list)
    for x, y in points:
        repeats[x].append(y)
    dof = sum(len(ys)-1 for ys in repeats.values())
    variance = (sum(sum((y-sum(ys)/len(ys))**2 for y in ys) for ys in repeats.values()) / dof) if dof else None
    stationary = -a/(2*d) if d < -1e-9 else None
    return {'intercept_estimate': c, 'linear_estimate': a, 'quadratic_estimate': d,
            'stationary_x_estimate': stationary,
            'within_explored_interval': stationary is not None and min(repeats) <= stationary <= max(repeats),
            'residual_rms': math.sqrt(sum(r*r for r in residuals)/len(points)),
            'repeat_noise_sd_estimate': math.sqrt(variance) if variance is not None else None,
            'observations': len(points), 'model_status': 'quadratic hypothesis, not a proved optimum'}


def analyse(saved, rounds):
    by_id = {r['id']: r for r in rounds}
    scalar = collections.defaultdict(list)
    targets = collections.defaultdict(list)
    shape = collections.defaultdict(dict)
    issues = []
    for result in saved:
        design = by_id[result['id']]
        values = design['values']
        for row in result['observations']:
            if row.get('error'):
                issues.append({'round': result['id'], 'case': row.get('test_case'), 'error': row['error']})
                continue
            if row['N'] != len(values):
                issues.append({'round': result['id'], 'case': row.get('test_case'), 'error': 'Array length does not match this probe; possible stale feedback'})
                continue
            # Displayed t can be rounded; case identity keeps such inputs separate.
            key = (str(row.get('test_case', 'unknown')), row['t'])
            if len(values) == 1:
                scalar[key].append((values[0], row['raw_noisy_score']))
                if 'raw_noiseless_target' in row:
                    targets[key].append(row['raw_noiseless_target'])
            else:
                shape[key][tuple(values)] = row['raw_noisy_score']
    estimates = []
    for (case, t), points in sorted(scalar.items()):
        fit = quadratic_fit(points)
        if fit:
            fit.update({'test_case': case, 't_displayed': t})
            references = targets[(case, t)]
            if references:
                reference = sum(references)/len(references)
                fit['noiseless_target_reference'] = reference
                fit['target_reference_range'] = [min(references), max(references)]
                d = fit['quadratic_estimate']
                if d < -1e-9:
                    model_peak = fit['intercept_estimate'] - fit['linear_estimate']**2/(4*d)
                    fit['model_peak_minus_target'] = model_peak - reference
                    fit['target_check_assumption'] = 'The reported target is the attainable unconstrained maximum for this N; not yet established.'
            estimates.append(fit)
    interactions = []
    required = [(0.0, 0.0), (0.5, 0.0), (0.0, 0.5), (0.5, 0.5)]
    for (case, t), values in sorted(shape.items()):
        if all(key in values for key in required):
            contrast = values[(0.5, 0.5)] - values[(0.5, 0.0)] - values[(0.0, 0.5)] + values[(0.0, 0.0)]
            interactions.append({'test_case': case, 't_displayed': t, 'noisy_interaction_contrast': contrast})
    return {'scalar_model_estimates': estimates, 'array_interaction_estimates': interactions,
            'issues': issues, 'note': 'Model estimates from judge observations; no official score or optimality claim.'}


def get_response(config_rounds, mode):
    token = uuid.uuid4().hex
    script = bundle(config_rounds, mode, token)
    (ROOT / 'next_browser_step.js').write_text(script)
    clipboard_write(script)
    print('\nBROWSER CONSOLE STEP: this clipboard contains JavaScript, not C.')
    print('On the competition page press Option+Command+J to open Chrome Developer Tools.')
    print('Select its Console tab, click the > prompt, paste, then press Enter.')
    print('Keep this out of the competition C editor. Wait for [QRT] DONE.')
    while True:
        input('Then return here and press Enter: ')
        try:
            response = json.loads(clipboard_read())
            if response.get('token') != token:
                raise ValueError('Clipboard is not this batch\'s feedback.')
            return response
        except (ValueError, AttributeError) as error:
            print(str(error))
            print('Wait for the browser to finish. Its final JSON must be on the clipboard.')


def shared_feedback(saved, rounds):
    """Export a collector-compatible view for the other agent, preserving raw data."""
    by_id = {r['id']: r for r in rounds}
    rows = []
    for result in saved:
        design = by_id[result['id']]
        for original in result['observations']:
            row = dict(original)
            row['probe'] = design['file']
            row['output'] = list(design['values'])
            row['candidate'] = design['values'][0] if len(design['values']) == 1 else None
            rows.append(row)
    return {'probe': 'driver_batch', 'source': 'saved judge feedback', 'observations': rows}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--prepare-only', action='store_true')
    parser.add_argument('--manual', action='store_true', help='Use manual code-editor paste with automatic feedback collection')
    parser.add_argument('--auto', action='store_true', help='Start with browser-console automation instead of copying C source')
    args = parser.parse_args()
    rounds = prepare()
    if args.prepare_only:
        (ROOT / 'next_browser_step.js').write_text(bundle(rounds, 'auto', 'local_validation_only'))
        print('Prepared and locally checked {} full C probes. No judge runs performed.'.format(len(rounds)))
        return
    feedback_dir = ROOT / 'feedback'
    feedback_dir.mkdir(exist_ok=True)
    state_path = ROOT / 'driver_state.json'
    state = json.loads(state_path.read_text()) if state_path.exists() else {'completed': []}
    completed = {result['id'] for result in state['completed']}
    pending = [r for r in rounds if r['id'] not in completed]
    if not pending:
        print('This experiment batch is already saved. Tell the assistant: batch done.')
        return

    def save(response):
        path = feedback_dir / (time.strftime('%Y%m%d_%H%M%S') + '_' + uuid.uuid4().hex[:8] + '.json')
        path.write_text(json.dumps(response, indent=2) + '\n')
        for result in response.get('rounds', []):
            if result['id'] not in completed and result.get('observations'):
                state['completed'].append(result)
                completed.add(result['id'])
        state_path.write_text(json.dumps(state, indent=2) + '\n')
        (ROOT / 'analysis.json').write_text(json.dumps(analyse(state['completed'], rounds), indent=2) + '\n')
        (ROOT / 'driver_feedback.json').write_text(json.dumps(shared_feedback(state['completed'], rounds), indent=2) + '\n')
        print('Feedback saved locally:', path.name)

    manual = args.manual or not args.auto
    if not manual:
        print('Preparing {} diagnostic runs. The browser helper uses Run Code only.'.format(len(pending)))
        response = get_response(pending, 'auto')
        save(response)
        if response['status'] == 'manual_required':
            print('This editor needs manual pasting; switching to the supported clipboard workflow.')
            manual = True
        elif response['status'] != 'complete':
            print('Stopped:', response.get('reason', response['status']))
            print('Tell the assistant: batch stopped. Collected data is saved.')
            return
    if manual:
        for current in rounds:
            if current['id'] in completed:
                continue
            (ROOT / 'template.c').write_text(current['source'])
            clipboard_write(current['source'])
            print('\nFull C code copied for ' + current['id'])
            input('Website code editor: Command+A, Command+V, Run Code. When finished, press Enter here: ')
            response = get_response([current], 'collect')
            save(response)
            if response['status'] != 'complete' or any(row.get('error') for r in response.get('rounds', []) for row in r['observations']):
                print('Stopped for feedback inspection. Tell the assistant: batch stopped.')
                return
    print('\nBatch saved and initial mathematical analysis written to p3/analysis.json.')
    print('Tell the assistant: batch done. No code or results need to be pasted into chat.')


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nStopped. Previously saved feedback is preserved.')
