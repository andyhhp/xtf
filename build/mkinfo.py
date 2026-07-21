#!/usr/bin/env python3
"""Generate test info JSON files from command-line arguments."""

import json
import sys

# Usage: mkcfg.py $OUT $NAME $CATEGORY $ENVS $VARIATIONS
_, out, name, cat, envs, variations = sys.argv

template = {
    "name": name,
    "category": cat,
    "environments": [],
    "variations": [],
}

if envs:
    template["environments"] = envs.split(" ")
if variations:
    template["variations"] = variations.split(" ")

with open(out, "w") as f:
    f.write(json.dumps(template, indent=4, separators=(",", ": ")) + "\n")
