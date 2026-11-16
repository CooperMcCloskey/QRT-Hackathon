// Generated config is prepended by drive.py. Run in the contest page console.
(async function (config) {
    const copyOut = typeof copy === 'function' ? copy : null;
    const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
    const num = '([-+]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][-+]?\\d+)?)';
    const payoff = new RegExp('t\\s*=\\s*' + num + '\\s*,\\s*N\\s*=\\s*(\\d+)' +
        '\\s*,\\s*raw_noisy_score\\s*=\\s*' + num +
        '\\s*,\\s*raw_noiseless_target\\s*=\\s*' + num +
        '\\s*,\\s*normalized_noisy_score\\s*=\\s*' + num);
    const debug = new RegExp('QRT_DRIVER round=([a-zA-Z0-9_]+) t=' + num);
    const result = {token: config.token, status: 'running', rounds: []};
    window.qrtDriverStop = false;
    window.qrtDriverResult = result;
    const status = message => { result.phase = message; console.log('[QRT] ' + message); };
    const parseScore = (text, testCase) => {
        const match = payoff.exec(text);
        if (!match) return null;
        return {test_case: testCase, t: +match[1], N: +match[2],
            raw_noisy_score: +match[3], raw_noiseless_target: +match[4],
            normalized_noisy_score: +match[5], message: match[0]};
    };
    const labels = () => {
        const all = Array.from(document.querySelectorAll('span,div,button,a,p'))
            .filter(e => /^Test Case\s*#?\s*\d+$/i.test((e.textContent || '').trim()));
        return all.filter(e => !all.some(other => other !== e && e.contains(other)));
    };
    const scrollParent = element => {
        for (let p = element && element.parentElement; p; p = p.parentElement) {
            if (p.scrollHeight > p.clientHeight + 8 && /auto|scroll/.test(getComputedStyle(p).overflowY)) return p;
        }
        return null;
    };
    async function collectVisible(round) {
        const rows = [], seen = new Set();
        let panel = scrollParent(labels()[0]);
        if (panel) { panel.scrollTop = 0; await sleep(250); }
        let misses = 0;
        const deadline = Date.now() + 120000;
        while (!window.qrtDriverStop && Date.now() < deadline && seen.size < 250) {
            const list = labels();
            const next = list.find(e => !seen.has(e.textContent.trim()));
            if (!next) {
                if (!panel || ++misses > 3) break;
                panel.scrollTop += Math.max(100, panel.clientHeight * 0.8);
                await sleep(250);
                continue;
            }
            misses = 0;
            const name = next.textContent.trim();
            seen.add(name);
            next.scrollIntoView({block: 'nearest'});
            next.click();
            await sleep(350);
            let body = document.body.innerText;
            let row = parseScore(body, name);
            for (let i = 0; !row && i < 4; i++) {
                await sleep(200);
                body = document.body.innerText;
                row = parseScore(body, name);
            }
            if (row) {
                const stamp = debug.exec(body);
                if (stamp && stamp[1] !== round.id) throw Error('Results still belong to a different probe.');
                if (stamp) row.exact_t = +stamp[2];
                rows.push(row);
            } else {
                const start = body.indexOf('Compiler Message');
                rows.push({test_case: name, error: start < 0 ? 'No payoff message' : body.slice(start, start + 800)});
            }
            panel = scrollParent(next) || panel;
            status(round.id + ': collected ' + seen.size + ' cases');
        }
        return rows;
    }
    function extractNetwork(data, round) {
        const rows = [], stamps = new Map();
        function visit(value, path, depth) {
            if (depth > 14 || value == null) return;
            if (typeof value === 'string') {
                const index = path.filter(x => typeof x === 'number').slice(-1)[0];
                const key = index === undefined ? path.join('.') : 'Test Case ' + (index + 1);
                const row = parseScore(value, key);
                if (row) rows.push(row);
                const stamp = debug.exec(value);
                if (stamp) stamps.set(key, {id: stamp[1], t: +stamp[2]});
            } else if (Array.isArray(value)) {
                value.forEach((v, i) => visit(v, path.concat(i), depth + 1));
            } else if (typeof value === 'object') {
                Object.keys(value).forEach(k => visit(value[k], path.concat(k), depth + 1));
            }
        }
        visit(data, [], 0);
        const unique = new Map();
        for (const row of rows) {
            const stamp = stamps.get(row.test_case);
            if (stamp && stamp.id !== round.id) continue;
            if (stamp) row.exact_t = stamp.t;
            unique.set(row.test_case + '/' + row.t + '/' + row.N, row);
        }
        return Array.from(unique.values());
    }
    const runButton = () => Array.from(document.querySelectorAll('button,[role="button"]'))
        .find(e => /^Run Code$/i.test(e.textContent.trim()));
    const enabled = e => e && !e.disabled && e.getAttribute('aria-disabled') !== 'true';
    let oldFetch, oldOpen, oldSend, fetchWrapper, openWrapper, sendWrapper;
    let active = null, payloads = [];
    const inspect = (text, started) => {
        if (!active || started < active.started || typeof text !== 'string' || !text.includes('raw_noisy_score')) return;
        try {
            const rows = extractNetwork(JSON.parse(text), active.round);
            if (rows.length) payloads.push({rows, at: Date.now()});
        } catch (_) { /* Non-JSON responses are handled through the visible UI. */ }
    };
    try {
        if (config.mode === 'collect') {
            result.rounds.push({id: config.rounds[0].id, observations: await collectVisible(config.rounds[0]), transport: 'visible UI'});
        } else {
            const watching = config.mode === 'watch';
            const setCode = null; // Editor scripting is prohibited; manual paste only.
            if (!watching && (!setCode || !runButton())) {
                result.status = 'manual_required';
                result.reason = !setCode ? 'Editor automation API unavailable' : 'Run Code button not found';
                return;
            }
            // Observe only responses to requests that the page itself makes.
            // This code sends no requests and does not click Submit.
            oldFetch = window.fetch;
            fetchWrapper = async function (...args) {
                const started = Date.now();
                const response = await oldFetch.apply(this, args);
                if (active) response.clone().text().then(text => inspect(text, started)).catch(() => {});
                return response;
            };
            window.fetch = fetchWrapper;
            oldOpen = XMLHttpRequest.prototype.open;
            oldSend = XMLHttpRequest.prototype.send;
            openWrapper = function (...args) { return oldOpen.apply(this, args); };
            sendWrapper = function (...args) {
                const started = Date.now();
                this.addEventListener('load', () => {
                    try { inspect(this.responseType === 'json' ? JSON.stringify(this.response) : this.responseText, started); } catch (_) {}
                }, {once: true});
                return oldSend.apply(this, args);
            };
            XMLHttpRequest.prototype.open = openWrapper;
            XMLHttpRequest.prototype.send = sendWrapper;
            for (const round of config.rounds) {
                if (window.qrtDriverStop) break;
                if (!enabled(runButton())) throw Error('Run Code is unavailable; stopped without submitting.');
                status((watching ? 'Waiting for you to paste C and click Run Code: ' : 'Running ') + round.id);
                payloads = [];
                active = {round, started: Date.now()};

                let sawBusy = false, done = false;
                const deadline = Date.now() + (watching ? 300000 : 120000);
                while (Date.now() < deadline && !window.qrtDriverStop) {
                    await sleep(300);
                    const idle = enabled(runButton());
                    if (!idle) {
                        if (!sawBusy) status('Judge is running ' + round.id);
                        sawBusy = true;
                    }
                    const last = payloads[payloads.length - 1];
                    if (idle && last && Date.now() - last.at > 1200) { done = true; break; }
                    if (idle && sawBusy && Date.now() - active.started > 1800 &&
                        /test cases? passed|test cases? failed|compilation error|runtime error/i.test(document.body.innerText)) {
                        const stamp = debug.exec(document.body.innerText);
                        if (stamp && stamp[1] !== round.id) {
                            status('Run used earlier code. Paste the C currently on your clipboard, then click Run Code.');
                            sawBusy = false;
                            payloads = [];
                        } else if (!watching) { done = true; break; }
                        else if (!last) status('Run detected; waiting for its judge response.');
                    }
                }
                if (!done) throw Error('Could not confirm this run finished. Stopped to avoid collecting old results.');
                let rows = payloads.length ? payloads.reduce((best, p) => p.rows.length >= best.length ? p.rows : best, []) : [];
                const transport = rows.length ? 'normal judge response' : 'visible UI';
                if (!rows.length && !watching) rows = await collectVisible(round);
                if (!rows.length) throw Error('No feedback found for ' + round.id);
                result.rounds.push({id: round.id, observations: rows, transport});
                window.qrtDriverResult = result;
                active = null;
                status('Saved ' + round.id + ': ' + rows.length + ' cases');
                if (rows.some(row => row.error)) throw Error('Some cases have errors; feedback saved for inspection.');
            }
        }
        result.status = window.qrtDriverStop ? 'stopped' : 'complete';
    } catch (error) {
        result.status = 'error';
        result.reason = String(error.message || error);
    } finally {
        active = null;
        if (fetchWrapper && window.fetch === fetchWrapper) window.fetch = oldFetch;
        if (openWrapper && XMLHttpRequest.prototype.open === openWrapper) XMLHttpRequest.prototype.open = oldOpen;
        if (sendWrapper && XMLHttpRequest.prototype.send === sendWrapper) XMLHttpRequest.prototype.send = oldSend;
        window.qrtDriverResult = result;
        const text = JSON.stringify(result);
        console.log(text);
        if (!config.noClipboard) {
            if (copyOut) copyOut(text);
            else { try { await navigator.clipboard.writeText(text); } catch (_) { status('Copy the JSON above manually.'); } }
        }
        status('DONE — return to the terminal and press Enter. ' + result.status + (result.reason ? ': ' + result.reason : ''));
    }
})(QRT_CONFIG);
