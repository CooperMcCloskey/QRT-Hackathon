"""Single-file Python 3.8 baseline. The public judge entry is generate_grid."""
import math
import random
import re
import time

_ALPHABET = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"


def _prepare(required_words, forbidden_words):
    required = list(required_words)
    forbidden = list(forbidden_words)
    if any(not isinstance(w, str) or any(c not in _ALPHABET for c in w)
           for w in required + forbidden):
        raise ValueError("Words must contain uppercase A-Z only")
    if "" in forbidden:
        raise ValueError("An empty forbidden word makes every grid impossible")
    banned_letters = {w for w in forbidden if len(w) == 1}
    alphabet = "".join(c for c in _ALPHABET if c not in banned_letters)
    if not alphabet:
        raise ValueError("Every letter is forbidden")
    forbidden = sorted(set(forbidden), key=lambda w: (len(w), w))
    kept = []
    for word in sorted({min(w, w[::-1]) for w in required if w},
                       key=lambda w: (-len(w), w)):
        if any(f in word or f[::-1] in word for f in forbidden):
            raise ValueError("A required word necessarily contains a forbidden word")
        if not any(word in other or word[::-1] in other for other in kept):
            kept.append(word)
    return kept, forbidden, alphabet


def _pattern(n, forbidden):
    words = set()
    for word in forbidden:
        if len(word) > n:
            if any(word[i] != word[i - n] for i in range(n, len(word))):
                continue
            word = word[:n]
        words.add(word)
        words.add(word[::-1])
    if not words:
        return None, 0
    return re.compile("|".join(sorted(words, key=lambda w: (len(w), w)))), max(map(len, words))


def _lines(n):
    paths = []
    paths.extend([r * n + c for c in range(n)] for r in range(n))
    paths.extend([r * n + c for r in range(n)] for c in range(n))
    paths.extend([r * n + (s + r) % n for r in range(n)] for s in range(n))
    paths.extend([r * n + (s - r) % n for r in range(n)] for s in range(n))
    return paths


def _complete(partial, n, forbidden, alphabet, rng, deadline):
    pattern, maxlen = _pattern(n, forbidden)
    # A letter absent from forbidden words cannot introduce an occurrence.
    preferred = sorted(alphabet, key=lambda c: (sum(w.count(c) for w in forbidden), c))
    if pattern is None:
        return [c or preferred[0] for c in partial]
    paths = _lines(n)
    memberships = [[] for _ in partial]
    for line, path in enumerate(paths):
        for cell in path:
            memberships[cell].append(line)

    def match(grid, line):
        path = paths[line]
        s = "".join(grid[cell] for cell in path)
        hit = pattern.search(s + s[:maxlen - 1])
        if hit is None:
            return None
        return [path[k % n] for k in range(hit.start(), hit.end())]

    for attempt in range(5):
        if attempt and time.perf_counter() >= deadline:
            break
        grid = [c or (preferred[0] if attempt == 0 else rng.choice(preferred))
                for c in partial]
        bad = {}
        for line in range(len(paths)):
            hit = match(grid, line)
            if hit is not None:
                bad[line] = hit
        for iteration in range(max(200, 12 * n * n)):
            if not bad:
                return grid
            if iteration % 32 == 0 and time.perf_counter() >= deadline:
                return None
            line = rng.choice(list(bad))
            candidates = [cell for cell in bad[line] if not partial[cell]]
            if not candidates:
                return None
            cell = rng.choice(candidates)
            affected = memberships[cell]
            old = grid[cell]
            best_value = 100000
            choices = []
            for ch in preferred:
                if ch == old:
                    continue
                grid[cell] = ch
                value = sum(match(grid, ln) is not None for ln in affected)
                if value < best_value:
                    best_value, choices = value, [ch]
                elif value == best_value:
                    choices.append(ch)
            if not choices:
                return None
            grid[cell] = rng.choice(choices)
            for ln in affected:
                hit = match(grid, ln)
                if hit is None:
                    bad.pop(ln, None)
                else:
                    bad[ln] = hit
    return None


def _baseline(words, forbidden, alphabet, rng, deadline):
    longest = max(map(len, words), default=0)
    if not words:
        for n in range(1, 17):
            grid = _complete([""] * (n * n), n, forbidden, alphabet, rng, deadline)
            if grid is not None:
                return ["".join(grid[r * n:(r + 1) * n]) for r in range(n)]
        raise ValueError("No feasible grid found for these forbidden words")
    # Every word has a separator after it; every occupied row has a blank row.
    n = max(longest + 1, int(math.ceil(math.sqrt(2 * sum(len(w) + 1 for w in words)))))
    n += n % 2
    while True:
        shelves = []
        used = []
        for word in words:
            for index, length in enumerate(used):
                if length + len(word) + 1 <= n:
                    shelves[index].append(word)
                    used[index] += len(word) + 1
                    break
            else:
                shelves.append([word])
                used.append(len(word) + 1)
        if 2 * len(shelves) <= n:
            break
        n += 2
    for attempt in range(4):
        partial = [""] * (n * n)
        for r, shelf in enumerate(shelves):
            c = 0
            for word in shelf:
                for k, ch in enumerate(word):
                    partial[2 * r * n + c + k] = ch
                c += len(word) + 1
        grid = _complete(partial, n, forbidden, alphabet, rng, deadline)
        if grid is not None:
            return ["".join(grid[r * n:(r + 1) * n]) for r in range(n)]
        n += 2
    raise ValueError("Could not construct a feasible grid within the search budget")


def generate_grid(required_words, forbidden_words):
    deadline = time.perf_counter() + 1.5
    words, forbidden, alphabet = _prepare(required_words, forbidden_words)
    return _baseline(words, forbidden, alphabet, random.Random(12345), deadline)


# Local test adapter only; the online judge calls generate_grid directly.
if __name__ == "__main__":
    import json
    import sys
    data = json.load(sys.stdin)
    answer = generate_grid(data["required_words"], data["forbidden_words"])
    sys.stdout.write("\n".join(answer) + "\n")
