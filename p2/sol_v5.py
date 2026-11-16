"""Toroidal overlap packing; self-contained Python 3.8 judge submission."""
import collections
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
    # Every placed/filler letter belongs to alphabet, so these are redundant.
    forbidden = [w for w in forbidden if not any(ch in banned_letters for ch in w)]
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
    trie = {}
    for word in sorted(words, key=len):
        node = trie
        for ch in word:
            if "" in node:
                break
            node = node.setdefault(ch, {})
        else:
            node[""] = {}

    def expression(node):
        if "" in node:
            return ""
        pieces = [ch + expression(child) for ch, child in sorted(node.items())]
        if len(pieces) == 1:
            return pieces[0]
        if all(len(piece) == 1 for piece in pieces):
            return "[" + "".join(pieces) + "]"
        return "(?:" + "|".join(pieces) + ")"

    return re.compile(expression(trie)), max(map(len, words))


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
                if value == 0:
                    break
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


_popcount = getattr(int, "bit_count", lambda value: bin(value).count("1"))


class _Packing:
    def __init__(self, n, forbidden, rng, deadline):
        self.n = n
        self.full = (1 << n) - 1
        self.paths = _lines(n)
        self.memberships = [[] for _ in range(n * n)]
        for line, path in enumerate(self.paths):
            for offset, cell in enumerate(path):
                self.memberships[cell].append((line, 1 << offset))
        self.grid = [""] * (n * n)
        self.free = [self.full] * (4 * n)
        self.letters = [{} for _ in self.paths]
        self.pattern, self.maxlen = _pattern(n, forbidden)
        self.rng = rng
        self.deadline = deadline

    def put(self, line, start, word):
        n = self.n
        path = self.paths[line]
        changed = []
        affected = set()
        for k, ch in enumerate(word):
            cell = path[(start + k) % n]
            if self.grid[cell]:
                continue  # Compatibility was proved by the candidate masks.
            changed.append(cell)
            self.grid[cell] = ch
            for ln, bit in self.memberships[cell]:
                affected.add(ln)
                self.free[ln] ^= bit
                self.letters[ln][ch] = self.letters[ln].get(ch, 0) | bit
        if self.pattern is not None:
            for ln in affected:
                s = "".join(self.grid[cell] or "." for cell in self.paths[ln])
                if self.pattern.search(s + s[:self.maxlen - 1]) is not None:
                    for cell in changed:
                        ch = self.grid[cell]
                        for other, bit in self.memberships[cell]:
                            self.free[other] |= bit
                            self.letters[other][ch] ^= bit
                        self.grid[cell] = ""
                    return False
        return True

    def place(self, word):
        n = self.n
        full = self.full
        length = len(word)
        word_mask = (1 << length) - 1
        buckets = [[] for _ in range(length + 1)]
        counts = [0] * (length + 1)
        versions = (word,) if word == word[::-1] else (word, word[::-1])
        for line in range(4 * n):
            if line % 32 == 0 and time.perf_counter() >= self.deadline:
                return False
            free = self.free[line]
            letters = self.letters[line]
            occupied = full ^ free
            for text in versions:
                allowed = full
                positive = 0
                for k, ch in enumerate(text):
                    equal = letters.get(ch, 0)
                    mask = free | equal
                    allowed &= (mask >> k) | ((mask << (n - k)) & full)
                    if not allowed:
                        break
                    positive |= (equal >> k) | ((equal << (n - k)) & full)
                if not allowed:
                    continue
                aligned = allowed & positive
                if not aligned:
                    offset = self.rng.randrange(n)
                    rotated = (allowed >> offset) | ((allowed << (n - offset)) & full)
                    start = ((rotated & -rotated).bit_length() - 1 + offset) % n
                    options = [(0, start)]
                else:
                    allowed = aligned
                    options = []
                    while allowed:
                        bit = allowed & -allowed
                        allowed -= bit
                        start = bit.bit_length() - 1
                        overlap = _popcount(((occupied >> start) |
                                             (occupied << (n - start))) & word_mask)
                        if overlap == length:
                            return True
                        options.append((overlap, start))
                for overlap, start in options:
                    counts[overlap] += 1
                    bucket = buckets[overlap]
                    candidate = (line, start, text)
                    if len(bucket) < 24:
                        bucket.append(candidate)
                    else:
                        slot = self.rng.randrange(counts[overlap])
                        if slot < 24:
                            bucket[slot] = candidate
        for bucket in reversed(buckets):
            self.rng.shuffle(bucket)
            for line, start, text in bucket:
                if self.put(line, start, text):
                    return True
        return False


def _pack(words, forbidden, alphabet, n, rng, deadline, attempt):
    packing = _Packing(n, forbidden, rng, deadline)
    if attempt == 0:
        order = words
    else:
        order = sorted(words, key=lambda word: -len(word) + rng.random() * (2 + attempt % 5))
    for word in order:
        if not packing.place(word):
            return None
    filled = _complete(packing.grid, n, forbidden, alphabet, rng, deadline)
    if filled is None:
        return None
    return ["".join(filled[r * n:(r + 1) * n]) for r in range(n)]


def generate_grid(required_words, forbidden_words):
    deadline = time.perf_counter() + 1.5
    required = list(required_words)
    words, forbidden, alphabet = _prepare(required, forbidden_words)
    rng = random.Random(12345)
    counts = collections.Counter()
    for word in words:
        counts |= collections.Counter(word)
    lower = max(len(words[0]) if words else 1,
                int(math.ceil(math.sqrt(sum(counts.values())))))
    if words and lower == len(words[0]):
        pattern, extent = _pattern(lower, forbidden)
        if pattern is not None:
            for word in words:
                if len(word) == lower and pattern.search(word + word[:extent - 1]):
                    lower += 1
                    break
    if words and len(words) <= 12:
        quick = _pack(words, forbidden, alphabet, lower, rng,
                      min(deadline, time.perf_counter() + 0.08), 0)
        if quick is not None:
            return quick
    try:
        best = _baseline(words, forbidden, alphabet, rng,
                         min(deadline, time.perf_counter() + 0.35))
    except ValueError:
        best = None
    if not words:
        if best is not None:
            return best
        raise ValueError("No feasible grid found")
    target = max(lower, int(math.sqrt(sum(map(len, required)) / 1.7)))
    n = max(lower, int(math.ceil(math.sqrt(sum(map(len, words)) / 1.35))))
    if best is not None:
        n = min(n, len(best) - 1)
    else:
        n = lower
    failed = lower - 1
    attempt = 0
    try:
        while time.perf_counter() < deadline and (best is None or len(best) > lower):
            answer = _pack(words, forbidden, alphabet, n, rng, deadline, attempt)
            attempt += 1
            if answer is not None:
                best = answer
                if n <= lower or n <= target:
                    break
                n = max(lower, (failed + n) // 2)
            else:
                failed = max(failed, n)
                if best is None:
                    n = lower + attempt // 3
                elif n < len(best) - 1:
                    n = min(len(best) - 1, max(n + 1, (n + len(best)) // 2))
                else:
                    n = max(lower, len(best) - 1)
    except Exception:
        if best is None:
            raise
    if best is None:
        raise ValueError("No feasible grid found within the search budget")
    return best


# Local test adapter only; the online judge calls generate_grid directly.
if __name__ == "__main__":
    import json
    import sys
    data = json.load(sys.stdin)
    answer = generate_grid(data["required_words"], data["forbidden_words"])
    sys.stdout.write("\n".join(answer) + "\n")
