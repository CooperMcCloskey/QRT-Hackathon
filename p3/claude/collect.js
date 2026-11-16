// Read-only results reader, v2. Paste into the browser console after Run Code.
// Clicks the "Test Case N" tabs and reads their text. Never touches the editor.
(async () => {
    window.qrtDriverStop = true;          // stop any older helper script still running
    const pause = ms => new Promise(r => setTimeout(r, ms));
    const NUM = '([-+]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][-+]?\\d+)?)';
    const SCORE = 't\\s*=\\s*' + NUM + '\\s*,\\s*N\\s*=\\s*(\\d+)\\s*,\\s*raw_noisy_score\\s*=\\s*' + NUM +
        '\\s*,\\s*raw_noiseless_target\\s*=\\s*' + NUM;
    const DBG = 'QRT_\\w+[^\\n]*?t=' + NUM + '(?:\\s+x=' + NUM + ')?';
    const out = {version: 2, method: null, diag: {}, observations: []};
    const rec = (label, m, d) => ({
        test_case: label, t: +m[1], N: +m[2], raw_noisy_score: +m[3], raw_noiseless_target: +m[4],
        exact_t: d ? +d[1] : null, candidate: d && d[2] !== undefined ? +d[2] : null, message: m[0]
    });
    const finish = () => {
        const text = JSON.stringify(out);
        window.qrtReaderResult = text;
        if (typeof copy === 'function') copy(text);
        console.log('QRT reader: method=' + out.method + ', cases=' + out.observations.length +
            ', distinct results=' + new Set(out.observations.map(o => o.message)).size +
            '. DONE. Now type  qrtReaderResult  and press Enter, then right-click the text and choose Copy string contents.');
    };

    // Labels: innermost elements whose whole text is "Test Case N".
    const labels = () => {
        const els = Array.from(document.querySelectorAll('body *'))
            .filter(e => /^Test\s*Case\s*#?\s*\d+$/i.test((e.textContent || '').trim()));
        return els.filter(e => !els.some(o => o !== e && e.contains(o)));
    };
    const numOf = e => +(e.textContent.match(/\d+/) || [0])[0];

    // Method A: every case's result is already in the page (hidden panels).
    const all = document.body.textContent;
    const sA = Array.from(all.matchAll(new RegExp(SCORE, 'g')));
    const dA = Array.from(all.matchAll(new RegExp(DBG, 'g')));
    const nLabels = new Set(labels().map(numOf)).size;
    out.diag = {labels: nLabels, scores_in_page: sA.length, debug_in_page: dA.length,
                distinct_in_page: new Set(sA.map(m => m[0])).size};
    if (sA.length >= 2 && sA.length >= nLabels && out.diag.distinct_in_page > 1) {
        out.method = 'page';
        sA.forEach((m, i) => out.observations.push(rec('Test Case ' + (i + 1), m,
            dA.length === sA.length ? dA[i] : null)));
        return finish();
    }

    // Method B: click each tab and wait until the visible result changes.
    const visible = () => {
        const txt = document.body.innerText;
        return {s: new RegExp(SCORE).exec(txt), d: new RegExp(DBG).exec(txt)};
    };
    const fire = el => {
        const target = el.closest('[role="tab"],li,a,button,[class*="tab"],[class*="Tab"],[class*="case"],[class*="Case"]') || el;
        for (const type of ['pointerdown', 'mousedown', 'pointerup', 'mouseup', 'click']) {
            const Ev = type.startsWith('pointer') && window.PointerEvent ? PointerEvent : MouseEvent;
            target.dispatchEvent(new Ev(type, {bubbles: true, cancelable: true, view: window}));
        }
    };
    let prev = visible().s ? visible().s[0] : null, unchanged = 0;
    const total = Math.max(nLabels, 1);
    for (let i = 1; i <= total; i++) {
        let el = null;
        for (let tries = 0; tries < 8 && !el; tries++) {
            el = labels().find(e => numOf(e) === i);
            if (!el) {
                const any = labels().pop();
                if (any) any.scrollIntoView({block: 'end'});
                await pause(250);
            }
        }
        if (!el) { out.observations.push({test_case: 'Test Case ' + i, error: 'tab not found'}); continue; }
        el.scrollIntoView({block: 'nearest'});
        fire(el);
        let v = visible();
        const start = Date.now();
        while (v.s && prev && v.s[0] === prev && Date.now() - start < 2500) {
            await pause(120);
            v = visible();
        }
        if (v.s && prev && v.s[0] === prev) unchanged++;
        if (v.s) { out.observations.push(rec('Test Case ' + i, v.s, v.d)); prev = v.s[0]; }
        else out.observations.push({test_case: 'Test Case ' + i, error: 'no result text visible'});
        if (i % 10 === 0) console.log('QRT reader: ' + i + ' of ' + total);
    }
    out.method = 'click';
    out.diag.unchanged_after_click = unchanged;
    if (unchanged > total / 2) {
        const el = labels()[1];
        out.diag.tab_html = el ? (el.closest('li,div') || el).outerHTML.slice(0, 600) : null;
        out.diag.parent_html = el && el.parentElement && el.parentElement.parentElement
            ? el.parentElement.parentElement.outerHTML.slice(0, 1500) : null;
    }
    finish();
})();
