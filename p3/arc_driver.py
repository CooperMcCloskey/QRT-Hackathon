#!/usr/bin/env python3
"""Run from the normal Mac terminal; no browser-console pasting required."""
import json
from pathlib import Path
import subprocess
import time
import uuid

import drive

ROOT = Path(__file__).resolve().parent


def browser(script):
    completed = subprocess.run(['osascript', str(ROOT / 'browser_bridge.applescript'), script],
                               capture_output=True, text=True, timeout=40)
    if completed.returncode:
        raise RuntimeError(completed.stderr.strip() or 'Browser automation failed')
    return completed.stdout.strip()


def browser_json(script):
    """Arc may JSON-quote the string returned by JSON.stringify again."""
    value = browser(script)
    for _ in range(4):
        if not isinstance(value, str):
            return value
        try:
            value = json.loads(value)
        except ValueError:
            raise RuntimeError('Unexpected browser response: ' + value[:250])
    if isinstance(value, str):
        raise RuntimeError('Browser response remained a string after decoding: ' + value[:250])
    return value


def save(result, rounds):
    folder = ROOT / 'feedback'
    folder.mkdir(exist_ok=True)
    raw_path = folder / ('direct_' + result['token'] + '.json')
    raw_path.write_text(json.dumps(result, indent=2) + '\n')
    state_path = ROOT / 'driver_state.json'
    state = json.loads(state_path.read_text()) if state_path.exists() else {'completed': []}
    known = {row['id'] for row in state['completed']}
    allowed = {row['id'] for row in rounds}
    for row in result.get('rounds', []):
        if row['id'] in allowed and row['id'] not in known and row.get('observations'):
            state['completed'].append(row)
            known.add(row['id'])
    state_path.write_text(json.dumps(state, indent=2) + '\n')
    (ROOT / 'analysis.json').write_text(json.dumps(drive.analyse(state['completed'], rounds), indent=2) + '\n')
    (ROOT / 'driver_feedback.json').write_text(json.dumps(drive.shared_feedback(state['completed'], rounds), indent=2) + '\n')


def run_browser(rounds, mode, all_rounds):
    token = uuid.uuid4().hex
    config = {'token': token, 'rounds': rounds, 'mode': mode, 'noClipboard': True}
    source = '(function () { const QRT_CONFIG = ' + json.dumps(config) + ';\n'
    source += (ROOT / 'browser_driver.js').read_text() + '\n})();\n"started";'
    browser(source)
    deadline = time.monotonic() + (330 if mode == 'watch' else 150) * max(1, len(rounds))
    count = -1
    last_phase = None
    heartbeat = time.monotonic()
    while time.monotonic() < deadline:
        time.sleep(1)
        result = browser_json('JSON.stringify(window.qrtDriverResult || null)')
        if result is not None and not isinstance(result, dict):
            raise RuntimeError('Expected an object containing probe progress, got ' + type(result).__name__)
        if not result or result.get('token') != token:
            continue
        phase = result.get('phase', 'Waiting for browser feedback')
        if phase != last_phase or time.monotonic() - heartbeat >= 15:
            print(phase, flush=True)
            last_phase = phase
            heartbeat = time.monotonic()
        current = len(result.get('rounds', []))
        if current != count:
            count = current
            if current:
                save(result, all_rounds)
                print('Saved {} probe runs.'.format(current), flush=True)
        if result['status'] != 'running':
            save(result, all_rounds)
            return result
    browser('window.qrtDriverStop = true; "stopping";')
    raise RuntimeError('Browser run timed out; saved observations remain available.')


def completed_ids():
    path = ROOT / 'driver_state.json'
    return {r['id'] for r in json.loads(path.read_text())['completed']} if path.exists() else set()


def main():
    print('Keep the Black box contest page selected in your browser.', flush=True)
    page = browser_json('JSON.stringify({title:document.title, body:document.body.innerText})')
    if not isinstance(page, dict) or not isinstance(page.get('body'), str):
        raise RuntimeError('Browser did not return the contest page text: ' + repr(page)[:250])
    if 'probe' not in page['body'] and 'raw_noisy_score' not in page['body']:
        raise RuntimeError('The selected HackerRank tab does not appear to be Black box.')
    snapshot = browser_json('''JSON.stringify({
        title:document.title,
        body:document.body.innerText,
        editors:Array.from(document.querySelectorAll('.monaco-editor,.ace_editor,.CodeMirror')).slice(0,3).map(e=>({
            html:e.outerHTML.slice(0,2000),
            parents:[e,e.parentElement,e.parentElement&&e.parentElement.parentElement].filter(Boolean).map(p=>({
                tag:p.tagName, cls:p.className, keys:Object.keys(p).filter(k=>/react|editor|view/i.test(k))
            }))
        })),
        caseNodes:Array.from(document.querySelectorAll('span,div,button')).filter(e=>e.children.length===0&&/^Test Case\\s*1$/.test(e.textContent.trim())).slice(0,3).map(e=>e.parentElement.parentElement.outerHTML.slice(0,7000))
    })''')
    (ROOT / 'browser_snapshot.json').write_text(json.dumps(snapshot, indent=2) + '\n')
    rounds = drive.prepare()
    print('All ten C probes compiled and output-checked.', flush=True)
    pending = [r for r in rounds if r['id'] not in completed_ids()]
    if not pending:
        print('Batch done. Tell the assistant: batch done.')
        return
    print('Trying automatic Run Code for {} remaining probes.'.format(len(pending)), flush=True)
    response = run_browser(pending, 'auto', rounds)
    if response['status'] == 'manual_required':
        print('This editor needs manual pasting. Feedback collection is still automatic.', flush=True)
        for current in rounds:
            if current['id'] in completed_ids():
                continue
            (ROOT / 'template.c').write_text(current['source'])
            drive.clipboard_write(current['source'])
            print('\nC source copied for ' + current['id'], flush=True)
            print('Contest code editor: Command+A, Command+V, Run Code. Results will be detected automatically; no Enter needed.', flush=True)
            response = run_browser([current], 'watch', rounds)
            if response['status'] != 'complete' or any(row.get('error') for r in response.get('rounds', []) for row in r['observations']):
                raise RuntimeError(response.get('reason', 'Some test cases returned errors; feedback saved.'))
    elif response['status'] != 'complete':
        raise RuntimeError(response.get('reason', response['status']))
    print('\nBatch done. Results saved locally. Tell the assistant: batch done.')


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        try:
            browser('window.qrtDriverStop = true; "stopping";')
        except Exception:
            pass
        print('\nStopped. Saved feedback is preserved.')
    except (RuntimeError, subprocess.TimeoutExpired, ValueError) as error:
        print('\nStopped:', error)
        if 'not authorized' in str(error).lower() or '-1743' in str(error):
            print('macOS needs Terminal access to your browser to run these experiments.')
        elif 'javascript' in str(error).lower() and ('disabled' in str(error).lower() or 'not allowed' in str(error).lower()):
            print('The browser reports that JavaScript from Apple Events must be enabled for this helper.')
        print('Send the error text to the assistant. No JavaScript needs to be pasted into the contest editor.')
