// Paste into the browser developer console AFTER Run Code has finished.
// Reads displayed results, clicks test tabs, scrolls the case list, copies JSON.
// No network requests, submissions, or code-editor changes.
(async () => {
    const clipboardCopy = typeof copy === 'function' ? copy : null;
    const pause = ms => new Promise(resolve => setTimeout(resolve, ms));
    const number = '([-+]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][-+]?\\d+)?)';
    const scorePattern = new RegExp(
        't\\s*=\\s*' + number + '\\s*,\\s*N\\s*=\\s*(\\d+)' +
        '\\s*,\\s*raw_noisy_score\\s*=\\s*' + number +
        '\\s*,\\s*raw_noiseless_target\\s*=\\s*' + number +
        '\\s*,\\s*normalized_noisy_score\\s*=\\s*' + number
    );
    const debugPattern = new RegExp('QRT_RUN_A t=' + number + ' x=' + number);
    const data = {probe: 'run_A.c', source: 'visible judge feedback', observations: []};
    const seen = new Set();
    const discover = () => Array.from(document.querySelectorAll('span,div,button,a,p'))
        .filter(element => element.children.length === 0 &&
            /^Test Case\s*#?\s*\d+$/i.test(element.textContent.trim()));
    const scrollerFor = element => {
        for (let parent = element.parentElement; parent; parent = parent.parentElement) {
            if (parent.scrollHeight > parent.clientHeight + 8 &&
                /auto|scroll/.test(getComputedStyle(parent).overflowY)) return parent;
        }
        return null;
    };
    const initial = discover();
    if (!initial.length) {
        console.error('No test tabs found. Open All Cases in Test Results, then run this again.');
        return;
    }
    let scroller = scrollerFor(initial[0]);
    if (scroller) {
        scroller.scrollTop = 0;
        await pause(350);
    }
    let idleRounds = 0;
    const deadline = Date.now() + 120000;
    window.qrtStopCollection = false;
    while (Date.now() < deadline && !window.qrtStopCollection && seen.size < 250) {
        const labels = discover();
        const next = labels.find(element => !seen.has(element.textContent.trim()));
        if (next) {
            const testCase = next.textContent.trim();
            seen.add(testCase);
            next.scrollIntoView({block: 'nearest'});
            next.click();
            await pause(450);
            let body = document.body.innerText;
            let match = scorePattern.exec(body);
            for (let retry = 0; !match && retry < 5; retry++) {
                await pause(200);
                body = document.body.innerText;
                match = scorePattern.exec(body);
            }
            if (match) {
                const debug = debugPattern.exec(body);
                data.observations.push({
                    test_case: testCase, t: Number(match[1]), N: Number(match[2]),
                    raw_noisy_score: Number(match[3]),
                    raw_noiseless_target: Number(match[4]),
                    normalized_noisy_score: Number(match[5]),
                    exact_t: debug ? Number(debug[1]) : null,
                    candidate: debug ? Number(debug[2]) : null,
                    message: match[0]
                });
            } else {
                const start = body.indexOf('Compiler Message');
                data.observations.push({test_case: testCase, error:
                    start >= 0 ? body.slice(start, start + 1000) : 'No payoff message found'});
            }
            window.qrtProbeFeedback = data;
            console.log('Collected ' + testCase + ' (' + seen.size + ')');
            idleRounds = 0;
            continue;
        }
        scroller = labels.length ? scrollerFor(labels[0]) || scroller : scroller;
        if (!scroller || ++idleRounds > 3) break;
        scroller.scrollTop += Math.max(100, scroller.clientHeight * 0.8);
        await pause(350);
    }
    const text = JSON.stringify(data, null, 2);
    window.qrtProbeFeedback = data;
    console.log(text);
    if (clipboardCopy) {
        clipboardCopy(text);
        console.log('Copied ' + data.observations.length + ' cases. Paste into our chat.');
    } else {
        try {
            await navigator.clipboard.writeText(text);
            console.log('Copied ' + data.observations.length + ' cases. Paste into our chat.');
        } catch (_) {
            console.log('Clipboard unavailable; copy the JSON printed above.');
        }
    }
})();
