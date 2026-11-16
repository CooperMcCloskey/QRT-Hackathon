"""Local adapter for the function-only submission; do not paste this file."""
import json
import os
import sys
import submission

if os.environ.get("P2_FORCE_PY38_COUNT") == "1":
    submission._popcount = lambda value: bin(value).count("1")

data = json.load(sys.stdin)
grid = submission.generate_grid(data["required_words"], data["forbidden_words"])
sys.stdout.write("\n".join(grid) + "\n")
