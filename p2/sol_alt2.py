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
                    if value == 0:
                        break
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


def _eject_pack(words, n, forbidden, alphabet, rng, deadline, effort=5):
    m = len(words)
    board = [""] * (n * n)
    owners = [0] * (n * n)
    paths = [None] * m
    positions = {c: set() for c in alphabet}
    pending = set(range(m))
    weights = [1.0] * m
    directions = ((0, 1), (1, 0), (1, 1), (1, -1),
                  (0, -1), (-1, 0), (-1, -1), (-1, 1))
    pattern, maxlen = _pattern(n, forbidden)

    def forbidden_after(path):
        if pattern is None:
            return False
        rows, cols, down, up = set(), set(), set(), set()
        for cell in path:
            r, c = divmod(cell, n)
            rows.add(r)
            cols.add(c)
            down.add((c - r) % n)
            up.add((c + r) % n)
        lines = []
        lines.extend("".join(board[r * n + c] or " " for c in range(n)) for r in rows)
        lines.extend("".join(board[r * n + c] or " " for r in range(n)) for c in cols)
        lines.extend("".join(board[r * n + (s + r) % n] or " " for r in range(n)) for s in down)
        lines.extend("".join(board[r * n + (s - r) % n] or " " for r in range(n)) for s in up)
        return any(pattern.search(line + line[:maxlen - 1]) for line in lines)

    best_pending = m
    stagnation = 0
    for iteration in range(m * effort + 100):
        if time.perf_counter() >= deadline:
            return None
        if not pending:
            grid = _complete(board, n, forbidden, alphabet, rng, deadline)
            if grid is not None:
                return ["".join(grid[r * n:(r + 1) * n]) for r in range(n)]
            return None
        if len(pending) < best_pending:
            best_pending, stagnation = len(pending), 0
        else:
            stagnation += 1
            if stagnation > max(80, m * 2):
                return None
        # Difficult words gain weight each time they are displaced.
        i = max(pending, key=lambda j: len(words[j]) * weights[j] + rng.random() * 2)
        word = words[i]
        length = len(word)
        candidates = set()
        # Cross an already present equal letter, and also explore unanchored starts.
        for k, ch in enumerate(word):
            cells = positions[ch]
            if not cells:
                continue
            sample = list(cells)
            if len(sample) > 8:
                sample = rng.sample(sample, 8)
            for cell in sample:
                r, c = divmod(cell, n)
                for d in range(8):
                    dr, dc = directions[d]
                    candidates.add((((r - k * dr) % n) * n + (c - k * dc) % n, d))
        for _ in range(80):
            candidates.add((rng.randrange(n * n), rng.randrange(8)))
        ranked = []
        for start, d in candidates:
            r, c = divmod(start, n)
            dr, dc = directions[d]
            overlap, bad, cells = 0, 0, []
            for ch in word:
                cell = r * n + c
                cells.append(cell)
                if board[cell] == ch:
                    overlap += 1
                elif board[cell]:
                    bad |= owners[cell]
                r = (r + dr) % n
                c = (c + dc) % n
            cost = 0.0
            tmp = bad
            while tmp:
                bit = tmp & -tmp
                j = bit.bit_length() - 1
                cost += weights[j] * len(words[j])
                tmp -= bit
            cost -= overlap * 0.20
            ranked.append((cost + rng.random() * 0.1, cells, bad))
        ranked.sort(key=lambda item: item[0])
        inserted = False
        for _, cells, bad in ranked[:12]:
            removed = []
            tmp = bad
            while tmp:
                bit = tmp & -tmp
                j = bit.bit_length() - 1
                removed.append(j)
                for cell in paths[j]:
                    owners[cell] &= ~bit
                    if not owners[cell]:
                        positions[board[cell]].discard(cell)
                        board[cell] = ""
                tmp -= bit
            for cell, ch in zip(cells, word):
                board[cell] = ch
                positions[ch].add(cell)
                owners[cell] |= 1 << i
            if forbidden_after(cells):
                for cell in cells:
                    owners[cell] &= ~(1 << i)
                    if not owners[cell]:
                        positions[board[cell]].discard(cell)
                        board[cell] = ""
                for j in removed:
                    for cell, ch in zip(paths[j], words[j]):
                        board[cell] = ch
                        positions[ch].add(cell)
                        owners[cell] |= 1 << j
                continue
            paths[i] = cells
            pending.remove(i)
            for j in removed:
                paths[j] = None
                pending.add(j)
                weights[j] += 0.13
            inserted = True
            break
        if not inserted:
            weights[i] += 0.2
    return None


def generate_grid(required_words, forbidden_words):
    deadline = time.perf_counter() + 1.5
    words, forbidden, alphabet = _prepare(required_words, forbidden_words)
    rng = random.Random(12345)
    best = _baseline(words, forbidden, alphabet, rng, deadline)
    if not words:
        return best
    lower = max(len(words[0]), int(math.ceil(math.sqrt(len(set("".join(words)))))))
    total = sum(map(len, words))
    target = max(lower, int(math.ceil(math.sqrt(total * 0.85))))
    n = min(len(best) - 1, target)
    failures = 0
    while n >= lower and n < len(best) and time.perf_counter() < deadline:
        attempt_end = min(deadline, time.perf_counter() + 0.25)
        result = _eject_pack(words, n, forbidden, alphabet, rng, attempt_end, 10)
        if result is not None:
            best = result
            n -= 1
            failures = 0
        else:
            failures += 1
            if failures >= 3 or len(best) > target + 4:
                n += 1
                failures = 0
        if n >= len(best):
            break
    return best


# Local test adapter only; the online judge calls generate_grid directly.
if __name__ == "__main__":
    import json
    import sys
    data = json.load(sys.stdin)
    answer = generate_grid(data["required_words"], data["forbidden_words"])
    sys.stdout.write("\n".join(answer) + "\n")
