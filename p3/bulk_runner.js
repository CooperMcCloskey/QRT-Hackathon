// Console batch runner. Sources and the existing working collector are prepended.
(async () => {
    const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
    if (window.qrtBulk && window.qrtBulk.running) {
        window.qrtBulk.running = false;
        await pause(800);
    }
    document.querySelectorAll('[data-qrt-bulk-panel]').forEach(p => p.remove());
    Array.from(document.querySelectorAll('body > div')).filter(p => p.style.position === 'fixed' && p.textContent.startsWith('QRT experiment batch')).forEach(p => p.remove());
    window.qrtDriverStop = true;
    const state = window.qrtBulk = {version: 1, running: true, rounds: [], source: 'bulk_console_v1'};
    try {
        const previous = JSON.parse(localStorage.getItem('qrt_bulk_results_v1') || 'null');
        if (previous && Array.isArray(previous.rounds)) state.rounds = previous.rounds;
    } catch (_) {}
    const copyConsole = typeof copy === 'function' ? copy : null;
    const panel = document.createElement('div');
    panel.setAttribute('data-qrt-bulk-panel','v2');
    panel.style.cssText = 'position:fixed;top:12px;right:12px;z-index:2147483647;width:345px;padding:16px;background:#11251b;color:white;border:2px solid #31cf79;border-radius:12px;font:14px/1.5 system-ui;box-shadow:0 4px 30px #0008';
    const title = document.createElement('b'); title.textContent = 'QRT experiment batch';
    const message = document.createElement('div'); message.style.margin = '10px 0';
    panel.append(title, message);
    const controls = document.createElement('div'); panel.append(controls);
    const footer = document.createElement('div'); footer.style.cssText = 'margin-top:10px;font-size:12px'; panel.append(footer);
    document.body.append(panel);
    const show = text => { message.textContent = text; console.log('[QRT] ' + text); };
    async function writeClipboard(text) {
        try { await navigator.clipboard.writeText(text); return; } catch (_) {}
        if (copyConsole) { copyConsole(text); return; }
        throw Error('Clipboard unavailable. Use the console copy command shown below.');
    }
    const download = () => {
        const url = URL.createObjectURL(new Blob([JSON.stringify(state)], {type:'application/json'}));
        const link = document.createElement('a'); link.href = url; link.download = 'qrt_bulk_feedback.json'; link.click();
        setTimeout(() => URL.revokeObjectURL(url), 1000);
    };
    const save = () => {
        try { localStorage.setItem('qrt_bulk_results_v1', JSON.stringify(state)); } catch (_) {}
        footer.textContent = state.rounds.length + ' completed runs saved in this browser.';
    };
    function button(text, callback) {
        const b = document.createElement('button'); b.textContent = text;
        b.style.cssText='display:block;width:100%;margin:6px 0;padding:9px;background:#31cf79;color:#07130b;border:0;border-radius:5px;cursor:pointer;font-weight:700';
        b.onclick = callback; controls.append(b); return b;
    }
    const runButton = () => Array.from(document.querySelectorAll('button')).find(b => /^Run Code$/i.test(b.textContent.trim()));
    const visibleRound = () => {
        const text = document.body.innerText;
        const match = text.match(/QRT_BULK_([A-Z0-9_]+)\s+t=[-+]?\d/);
        return match ? match[1] : null;
    };
    async function manualRun(round) {
        controls.replaceChildren();
        show('Next: ' + round.id + '. Copy C, paste into the contest editor, then click Run Code.');
        button('1. COPY COMPLETE C CODE', async () => {
            try { await writeClipboard(round.source); show('C copied. Click the contest C editor → Command+A → Command+V → Run Code.'); }
            catch(error) { show(error.message); }
        });
        await new Promise(resolve => {
            button('2. RUN FINISHED — COLLECT ALL CASES', () => {
                if (visibleRound() !== round.id) {
                    show('P05/run marker not yet visible for ' + round.id + '. Finish pasting the C file and click Run Code, then wait for the test results and press this button again.');
                    return;
                }
                resolve();
            });
        });
    }
    async function collect(round) {
        show('Reading all 51 cases for ' + round.id + '. Leave this tab open.');
        controls.replaceChildren();
        const result = await new Promise((resolve, reject) => {
            let finished = false;
            window.qrtBulkCollectorDone = out => { if (!finished) { finished=true; resolve(out); } };
            try {
                const executing = (new Function(QRT_BULK_COLLECTOR))();
                if (executing && executing.catch) executing.catch(reject);
            } catch(error) { reject(error); }
            setTimeout(() => {if (!finished) {finished=true;reject(Error('Collector timed out. Saved earlier runs are preserved.'));}},210000);
        });
        const valid = result.observations.filter(o => 'raw_noisy_score' in o);
        const distinct = new Set(valid.map(o => o.message)).size;
        if (valid.length < 45) throw Error('Only ' + valid.length + ' cases collected. Batch paused for inspection.');
        if (distinct < 3 && (!Array.isArray(round.values) || round.values.some(x => x !== 0))) throw Error('Repeated case messages detected. Batch paused rather than fitting unreliable data.');
        if (valid.some(row => row.N !== (round.N || round.values.length))) throw Error('Judge result array length does not match this probe.');
        for (const row of result.observations) {
            if (Array.isArray(round.values)) row.output = round.values;
            row.candidate = Array.isArray(round.values) && round.values.length === 1 ? round.values[0] : null;
        }
        return {id:round.id, values:round.values, design:round.design, N:round.N, probe:round.file, observations:result.observations, collector_diagnostics:result.diag};
    }
    try {
        show('Ready. Copy the complete C file below; after running it, press COLLECT ALL CASES.');
        await pause(800);
        for (const round of QRT_BULK_ROUNDS) {
            if (!state.running) break;
            if (state.rounds.some(r => r.id === round.id)) continue;
            await manualRun(round);
            const response = await collect(round);
            state.rounds.push(response); save();
        }
        state.status = state.running ? 'complete' : 'stopped';
        show('Batch finished. Click COPY ALL RESULTS, then run the import command in Terminal.');
    } catch (error) {
        state.status='paused'; state.error=error.message;
        show('Paused: ' + error.message);
    } finally {
        state.running=false; save(); controls.replaceChildren();
        button('COPY ALL RESULTS', async()=> {await writeClipboard(JSON.stringify(state));show('Results copied. Now run the terminal import command.');});
        button('DOWNLOAD RESULTS', download);
        button('CLOSE PANEL', ()=>panel.remove());
        console.log('QRT batch saved:',state.rounds.length,'runs. To copy manually: copy(JSON.stringify(window.qrtBulk))');
    }
})();
