#!/usr/bin/env python3
"""Save the working Claude collector's clipboard JSON without browser automation."""
import hashlib
import json
import math
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parent


def ingest(text):
    data = json.loads(text)
    if not isinstance(data, dict):
        raise ValueError('Clipboard does not contain collector results. Wait for the collector to finish.')
    if isinstance(data.get('rounds'), list):
        rows=[]
        for result in data['rounds']:
            for original in result.get('observations',[]):
                row=dict(original)
                row['probe']=result.get('probe',result.get('id','bulk_round'))
                row['round_id']=result.get('id')
                row['output']=result.get('values',row.get('output'))
                if isinstance(row['output'],list) and len(row['output'])==1:
                    row['candidate']=row['output'][0]
                rows.append(row)
    elif isinstance(data.get('observations'),list):
        rows=data['observations']
    else:
        raise ValueError('Clipboard does not contain collector results. Wait for the collector to finish.')
    if not rows:
        raise ValueError('Collector returned no observations.')
    digest = hashlib.sha256(json.dumps(data, sort_keys=True).encode()).hexdigest()[:16]
    folder = ROOT / 'feedback'
    folder.mkdir(exist_ok=True)
    path = folder / ('collector_' + digest + '.json')
    if path.exists():
        print('These results are already saved:', path.name)
        return
    path.write_text(json.dumps(data, indent=2) + '\n')
    valid = [r for r in rows if all(k in r for k in ('t', 'N', 'raw_noisy_score', 'raw_noiseless_target'))]
    fingerprints = {(r['t'], r['N'], r['raw_noisy_score'], r['raw_noiseless_target']) for r in valid}
    if len(valid) > 3 and len(fingerprints) == 1:
        print('Saved for review: all case messages are identical. No independent observations added.')
        print('Tell the assistant: feedback saved, identical messages.')
        return
    imported = 0
    with (ROOT / 'observations.jsonl').open('a') as stream:
        for row in valid:
            candidate = row.get('candidate')
            if row['N'] != 1 or candidate is None:
                continue
            values = [row['t'], candidate, row['raw_noisy_score'], row['raw_noiseless_target']]
            if not all(isinstance(x, (int, float)) and math.isfinite(x) for x in values):
                continue
            record = dict(row)
            record.update({'probe': row.get('probe',data.get('probe', 'external_collector_run')),
                           'output': [candidate], 'source': str(path.relative_to(ROOT)),
                           'collection_id': digest})
            stream.write(json.dumps(record) + '\n')
            imported += 1
    print('Saved {} case records; {} distinct messages; {} scalar observations imported.'.format(len(rows), len(fingerprints), imported))
    print('Tell the assistant: feedback saved. The assistant can read the files directly.')


def read_results():
    """Prefer the persisted browser batch; copying a shell command changes the clipboard."""
    clipboard = subprocess.check_output(['pbpaste'], text=True)
    try:
        data = json.loads(clipboard)
        if isinstance(data, dict) and (isinstance(data.get('rounds'), list) or isinstance(data.get('observations'), list)):
            return clipboard
    except ValueError:
        pass
    print('Clipboard contains no results. Reading the saved batch from your contest browser...', flush=True)
    from arc_driver import browser_json
    data = browser_json("localStorage.getItem('qrt_bulk_results_v1')")
    if not isinstance(data, dict) or not isinstance(data.get('rounds'), list) or not data['rounds']:
        raise ValueError('No completed batch found in this tab. Keep the original Black box contest tab selected and run this command again.')
    print('Recovered {} completed runs from the browser.'.format(len(data['rounds'])), flush=True)
    return json.dumps(data)


if __name__ == '__main__':
    try:
        ingest(read_results())
    except (ValueError, RuntimeError, subprocess.SubprocessError) as error:
        print(error)
        sys.exit(1)
