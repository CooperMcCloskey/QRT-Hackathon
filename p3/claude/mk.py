#!/usr/bin/env python3
"""Write run_<NAME>.c returning xs[k] for the case with t = 0.02*k."""
import json, math, os, sys
HERE = os.path.dirname(os.path.abspath(__file__))

def lit(x):
    s = f"{x:.7g}"
    return (s if any(c in s for c in ".en") else s + ".0") + "f"

def write(name, xs):
    assert len(xs) == 51
    src = open(os.path.join(HERE, "..", "probe_v1.c")).read()
    start = src.index("float* probe(float t, int* result_count) {")
    end = src.index("\nint main()", start)
    body = f'''float* probe(float t, int* result_count) {{
    static const float XX[51] = {{{", ".join(lit(x) for x in xs)}}};
    static float answer[1];
    int k = (int)floorf(t / 0.02f + 0.5f);
    if (k < 0) k = 0;
    if (k > 50) k = 50;
    answer[0] = XX[k];
    *result_count = 1;
    printf("QRT_RUN_{name} t=%.9g x=%.9g\\n", (double)t, (double)answer[0]);
    return answer;
}}
'''
    path = os.path.join(HERE, f"run_{name}.c")
    open(path, "w").write(src[:start] + body + src[end:])
    return path
