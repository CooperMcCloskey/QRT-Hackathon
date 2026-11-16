(function(){const QRT_BULK_ROUNDS=[{"id": "REBUILD_CC44B484_2", "file": "sol_refined_CC44B484_v2.c", "values": null, "N": 4096, "design": {"kind": "last_data_updated_reconstruction", "scale": 1.25, "tag": "CC44B484"}, "source": "#include <assert.h>\n#include <ctype.h>\n#include <limits.h>\n#include <math.h>\n#include <stdbool.h>\n#include <stddef.h>\n#include <stdint.h>\n#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n\nchar* readline();\nchar* ltrim(char*);\nchar* rtrim(char*);\n\nfloat parse_float(char*);\n\n\n\n/*\n * Complete the 'probe' function below.\n *\n * The function is expected to return a FLOAT_ARRAY.\n * The function accepts FLOAT t as parameter.\n */\n\n/*\n * To return the float array from the function, you should:\n *     - Store the size of the array to be returned in the result_count variable\n *     - Allocate the array statically or dynamically\n *\n * For example,\n * float* return_float_array_using_static_allocation(int* result_count) {\n *     *result_count = 5;\n *\n *     static float a[5] = {1.1, 2.2, 3.3, 4.4, 5.5};\n *\n *     return a;\n * }\n *\n * float* return_float_array_using_dynamic_allocation(int* result_count) {\n *     *result_count = 5;\n *\n *     float* a = malloc(5 * sizeof(float));\n *\n *     for (int i = 0; i < 5; i++) {\n *         *(a + i) = i + 1.1;\n *     }\n *\n *     return a;\n * }\n *\n */\nfloat* probe(float t, int* result_count) {\n    static const double C[9][22] = {{0.0097854813365,0.83613970226,0.206274058388,0.0134077398103,0.261501294269,-0.0163010425723,-0.0193331736238,0.0157231761704,-0.00114057957119,-0.0148308994775,-0.068332252607,0.0120120055886,0.0485293557807,-0.00797238210976,-0.011604606146,0.00120908903627,0.0025602909045,0.00204683871468,-0.00468820871931,0.00175470735164,-0.00157058531176,0.00349579043452},\n{-0.0355871294549,2.44401745637,1.17712213698,0.0091233768945,-0.126462353827,-0.0172738698254,0.0959090465281,-0.0299518480884,0.0479923509213,-0.00027765798062,0.137779344588,0.00116511697248,-0.0563290052282,0.0282736607989,0.00959281532682,-0.00557050146609,-0.0223085772845,0.0104765117055,-0.00825521399845,0.00721553972361,-0.000451494722108,-0.00490590952717},\n{-0.169812802334,-0.0403653007268,0.105419636976,0.0685096555116,0.0465023166394,-0.0355770047532,-0.0211106159906,-0.0654245824436,-0.0458565517173,0.00918313189112,0.00828507141602,-0.00599949884329,0.00368470827777,-0.00403137028972,0.000157447616122,0.00398161719724,0.000379885502728,-0.00196040715566,0.00203977075251,-0.00205878132011,0.00135157416746,-0.00127960698444},\n{0.00593997303726,0.733191020576,0.259088253273,0.0188561840181,0.0332882798916,-0.00391971455125,0.055974630339,0.0255583796701,-0.00271162254793,-0.001648621934,-0.0314086653942,0.00326304900255,0.00569126647136,0.00410946197798,-0.00121676581178,-0.0010850899224,-0.00218320157075,-0.00187684071505,0.000723946483665,0.000251189163516,-0.000604223679223,-9.08872794641e-05},\n{-0.0383072101574,0.714780701333,0.250611579635,0.0480422589795,-0.0407304299481,-0.00650762124612,0.00850217239614,0.0634777129563,-0.0378463149038,-0.0983526242971,0.00928977896587,0.0597458672129,0.00495717548148,-0.0101728750523,-0.00936102424764,0.014107707098,-0.00921264685228,-0.0201811081327,0.00626959368563,-0.000477280175358,0.00201113785795,-0.00897281135935},\n{-0.0354524918135,-1.25369226056,-0.477765728344,0.0586675777776,0.0600775065168,0.022536675359,-0.10736885432,-0.0301273171874,0.0179295004092,0.00517954548732,0.00225432323749,-0.0119110286225,0.0153129747042,-0.00416173179606,0.00290130053084,-0.00443215434174,-0.0024145738153,0.00137614407858,-0.00248525206189,-3.82181091295e-05,0.00171913062349,-0.000405234994168},\n{0.000792303323403,-1.77355787072,-0.738081406,-0.06646052151,-0.0689244248377,0.118554358534,-0.0114219344356,-0.129763438638,-0.0106951138832,0.12299881903,0.00709160758732,-0.054821124536,0.0134850254022,-0.0190305985921,0.0312986369945,-0.0271980947552,-0.0149915357907,0.0118754214515,-0.0117678866445,-0.000352229016681,0.00683653877196,0.00598922372463},\n{0.00536501851777,0.0360315548249,-0.0196333570874,0.00597470884312,0.00143523894367,0.00102483627069,-0.00118936329094,5.37949910585e-05,0.000370760557019,-5.76141344278e-05,0.000177981345057,3.39818061252e-06,-7.07486126842e-06,3.41480640789e-05,4.99658217002e-05,-2.71082032434e-05,-7.92196337581e-06,-1.51496857377e-05,6.88083647326e-06,1.53185956792e-06,-7.71474121908e-06,-5.28553114137e-06},\n{-0.000801816729328,-0.089664157097,0.0115721690493,-0.0796522064285,0.0141502668838,0.000514109117077,-0.00349068433744,0.00345978360286,-0.00114248890189,-0.000531964545866,1.28968754751e-05,0.000303019550738,-0.000263405908744,8.97500761831e-05,0.000154642777343,0.000131461007224,6.95001168678e-06,1.48318087292e-05,-8.18413730651e-06,7.36717773006e-07,5.82068179575e-06,-3.70523711368e-06}};\n    static const int W[9][2] = {{0,0}, {1,1}, {1,2}, {2,1}, {2,2}, {3,1}, {3,2}, {4,1}, {4,2}};\n    static float answer[4096];\n    double z = isfinite(t) ? fmax(0.0, fmin(1.0, (double)t)) : 0;\n    static const double TARGET[51] = {0.0148,0.0131,0.014,0.0162,0.0204,0.0244,0.0254,0.0248,0.0238,0.0253,0.0393,0.0587,0.0754,0.0886,0.0959,0.0969,0.0915,0.0867,0.0892,0.0986,0.1104,0.1202,0.1318,0.1274,0.1059,0.0925,0.1109,0.1334,0.1352,0.125,0.1229,0.124,0.1278,0.1308,0.1274,0.1291,0.1397,0.1387,0.1174,0.0853,0.0524,0.0284,0.0203,0.022,0.0247,0.0249,0.0253,0.0269,0.029,0.0308,0.0324};\n    double basis[22], a[9] = {0};\n    basis[0] = 1; basis[1] = z-.5;\n    for (int k=1;k<=20;k++) basis[k+1]=cos(3.141592653589793*k*z);\n    for (int j=0;j<9;j++) for(int k=0;k<22;k++) a[j]+=C[j][k]*basis[k];\n    double at=z*50.0;\n    int lo=(int)floor(at), hi=lo<50 ? lo+1 : 50;\n    double target=TARGET[lo]+(at-lo)*(TARGET[hi]-TARGET[lo]);\n    double energy=0;\n    for(int j=0;j<9;j++) energy+=a[j]*a[j]*(W[j][1]==0 ? 1.0 : .5);\n    double scale=1.25;\n    if(energy*scale*scale>2*target && energy>0) scale=sqrt(2*target/energy);\n    for (int i=0;i<4096;i++) {\n        double u=6.283185307179586*i/4095.0, value=0;\n        for (int j=0;j<9;j++) value+=a[j]*(W[j][1]==0 ? 1 : W[j][1]==1 ? cos(W[j][0]*u) : sin(W[j][0]*u));\n        value*=scale;\n        answer[i]=isfinite(value) ? (float)fmax(-3.0,fmin(3.0,value)) : 0;\n    }\n    *result_count=4096;\n    printf(\"QRT_BULK_REBUILD_CC44B484_2 t=%.9g x=%.9g\\n\",(double)t,(double)answer[0]);\n    return answer;\n}\n\nint main()\n{\n    FILE* fptr = fopen(getenv(\"OUTPUT_PATH\"), \"w\");\n\n    float t = parse_float(ltrim(rtrim(readline())));\n\n    int result_count;\n    float* result = probe(t, &result_count);\n\n    for (int i = 0; i < result_count; i++) {\n        fprintf(fptr, \"%f\", *(result + i));\n\n        if (i != result_count - 1) {\n            fprintf(fptr, \"\\n\");\n        }\n    }\n\n    fprintf(fptr, \"\\n\");\n\n    fclose(fptr);\n\n    return 0;\n}\n\nchar* readline() {\n    size_t alloc_length = 1024;\n    size_t data_length = 0;\n\n    char* data = malloc(alloc_length);\n\n    while (true) {\n        char* cursor = data + data_length;\n        char* line = fgets(cursor, alloc_length - data_length, stdin);\n\n        if (!line) {\n            break;\n        }\n\n        data_length += strlen(cursor);\n\n        if (data_length < alloc_length - 1 || data[data_length - 1] == '\\n') {\n            break;\n        }\n\n        alloc_length <<= 1;\n\n        data = realloc(data, alloc_length);\n\n        if (!data) {\n            data = '\\0';\n\n            break;\n        }\n    }\n\n    if (data[data_length - 1] == '\\n') {\n        data[data_length - 1] = '\\0';\n\n        data = realloc(data, data_length);\n\n        if (!data) {\n            data = '\\0';\n        }\n    } else {\n        data = realloc(data, data_length + 1);\n\n        if (!data) {\n            data = '\\0';\n        } else {\n            data[data_length] = '\\0';\n        }\n    }\n\n    return data;\n}\n\nchar* ltrim(char* str) {\n    if (!str) {\n        return '\\0';\n    }\n\n    if (!*str) {\n        return str;\n    }\n\n    while (*str != '\\0' && isspace(*str)) {\n        str++;\n    }\n\n    return str;\n}\n\nchar* rtrim(char* str) {\n    if (!str) {\n        return '\\0';\n    }\n\n    if (!*str) {\n        return str;\n    }\n\n    char* end = str + strlen(str) - 1;\n\n    while (end >= str && isspace(*end)) {\n        end--;\n    }\n\n    *(end + 1) = '\\0';\n\n    return str;\n}\n\nfloat parse_float(char* str) {\n    char* endptr;\n    float value = strtof(str, &endptr);\n\n    if (endptr == str || *endptr != '\\0') {\n        exit(EXIT_FAILURE);\n    }\n\n    return value;\n}\n"}];
const QRT_BULK_COLLECTOR="// Read-only results reader, v2. Paste into the browser console after Run Code.\n// Clicks the \"Test Case N\" tabs and reads their text. Never touches the editor.\nreturn (async () => {\n    window.qrtDriverStop = true;          // stop any older helper script still running\n    const pause = ms => new Promise(r => setTimeout(r, ms));\n    const NUM = '([-+]?(?:\\\\d+(?:\\\\.\\\\d*)?|\\\\.\\\\d+)(?:[eE][-+]?\\\\d+)?)';\n    const SCORE = 't\\\\s*=\\\\s*' + NUM + '\\\\s*,\\\\s*N\\\\s*=\\\\s*(\\\\d+)\\\\s*,\\\\s*raw_noisy_score\\\\s*=\\\\s*' + NUM +\n        '\\\\s*,\\\\s*raw_noiseless_target\\\\s*=\\\\s*' + NUM;\n    const DBG = 'QRT_\\\\w+[^\\\\n]*?t=' + NUM + '(?:\\\\s+x=' + NUM + ')?';\n    const out = {version: 2, method: null, diag: {}, observations: []};\n    const rec = (label, m, d) => ({\n        test_case: label, t: +m[1], N: +m[2], raw_noisy_score: +m[3], raw_noiseless_target: +m[4],\n        exact_t: d ? +d[1] : null, candidate: d && d[2] !== undefined ? +d[2] : null, message: m[0]\n    });\n    const finish = () => {\n        const text = JSON.stringify(out);\n        window.qrtReaderResult = text;\n        window.qrtBulkCollectorDone(out);\n        console.log('QRT reader: method=' + out.method + ', cases=' + out.observations.length +\n            ', distinct results=' + new Set(out.observations.map(o => o.message)).size +\n            '. DONE. Now type  qrtReaderResult  and press Enter, then right-click the text and choose Copy string contents.');\n    };\n\n    // Labels: innermost elements whose whole text is \"Test Case N\".\n    const labels = () => {\n        const els = Array.from(document.querySelectorAll('body *'))\n            .filter(e => /^Test\\s*Case\\s*#?\\s*\\d+$/i.test((e.textContent || '').trim()));\n        return els.filter(e => !els.some(o => o !== e && e.contains(o)));\n    };\n    const numOf = e => +(e.textContent.match(/\\d+/) || [0])[0];\n\n    // Method A: every case's result is already in the page (hidden panels).\n    const all = document.body.textContent;\n    const sA = Array.from(all.matchAll(new RegExp(SCORE, 'g')));\n    const dA = Array.from(all.matchAll(new RegExp(DBG, 'g')));\n    const nLabels = new Set(labels().map(numOf)).size;\n    out.diag = {labels: nLabels, scores_in_page: sA.length, debug_in_page: dA.length,\n                distinct_in_page: new Set(sA.map(m => m[0])).size};\n    if (sA.length >= 2 && sA.length >= nLabels && out.diag.distinct_in_page > 1) {\n        out.method = 'page';\n        sA.forEach((m, i) => out.observations.push(rec('Test Case ' + (i + 1), m,\n            dA.length === sA.length ? dA[i] : null)));\n        return finish();\n    }\n\n    // Method B: click each tab and wait until the visible result changes.\n    const visible = () => {\n        const txt = document.body.innerText;\n        return {s: new RegExp(SCORE).exec(txt), d: new RegExp(DBG).exec(txt)};\n    };\n    const fire = el => {\n        const target = el.closest('[role=\"tab\"],li,a,button,[class*=\"tab\"],[class*=\"Tab\"],[class*=\"case\"],[class*=\"Case\"]') || el;\n        for (const type of ['pointerdown', 'mousedown', 'pointerup', 'mouseup', 'click']) {\n            const Ev = type.startsWith('pointer') && window.PointerEvent ? PointerEvent : MouseEvent;\n            target.dispatchEvent(new Ev(type, {bubbles: true, cancelable: true, view: window}));\n        }\n    };\n    let prev = visible().s ? visible().s[0] : null, unchanged = 0;\n    const total = Math.max(nLabels, 1);\n    for (let i = 1; i <= total; i++) {\n        let el = null;\n        for (let tries = 0; tries < 8 && !el; tries++) {\n            el = labels().find(e => numOf(e) === i);\n            if (!el) {\n                const any = labels().pop();\n                if (any) any.scrollIntoView({block: 'end'});\n                await pause(250);\n            }\n        }\n        if (!el) { out.observations.push({test_case: 'Test Case ' + i, error: 'tab not found'}); continue; }\n        el.scrollIntoView({block: 'nearest'});\n        fire(el);\n        let v = visible();\n        const start = Date.now();\n        while (v.s && prev && v.s[0] === prev && Date.now() - start < 2500) {\n            await pause(120);\n            v = visible();\n        }\n        if (v.s && prev && v.s[0] === prev) unchanged++;\n        if (v.s) { out.observations.push(rec('Test Case ' + i, v.s, v.d)); prev = v.s[0]; }\n        else out.observations.push({test_case: 'Test Case ' + i, error: 'no result text visible'});\n        if (i % 10 === 0) console.log('QRT reader: ' + i + ' of ' + total);\n    }\n    out.method = 'click';\n    out.diag.unchanged_after_click = unchanged;\n    if (unchanged > total / 2) {\n        const el = labels()[1];\n        out.diag.tab_html = el ? (el.closest('li,div') || el).outerHTML.slice(0, 600) : null;\n        out.diag.parent_html = el && el.parentElement && el.parentElement.parentElement\n            ? el.parentElement.parentElement.outerHTML.slice(0, 1500) : null;\n    }\n    finish();\n})();\n";
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

})();
