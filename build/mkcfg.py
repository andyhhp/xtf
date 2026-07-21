#!/usr/bin/env python3
"""
Construct an xl configuration file for a test (from various fragments), and
substitue variables appropriately.
"""

import os
import sys

# Usage: mkcfg.py $OUT $DEFAULT-CFG $EXTRA-CFG $VARY-CFG
_, out, defcfg, vcpus, extracfg, varycfg = sys.argv

# Evaluate environment and name from $OUT
_, env, name = os.path.basename(out).split('.')[0].split('-', 2)

# Possibly split apart the variation suffix
variation = ''
if '~' in name:
    parts = name.split('~', 1)
    name, variation = parts[0], '~' + parts[1]


def expand(text: str) -> str:
    """Expand certain variables in text"""
    return (
        text.replace("@@NAME@@", name)
        .replace("@@ENV@@", env)
        .replace("@@VCPUS@@", vcpus)
        .replace("@@XTFDIR@@", os.environ["xtfdir"])
        .replace("@@VARIATION@@", variation)
    )


with open(defcfg) as f:
    config = f.read()

if extracfg:
    config += "\n# Test Extra Configuration:\n"
    with open(extracfg) as f:
        config += f.read()

if varycfg:
    config += "\n# Test Variation Configuration:\n"
    with open(varycfg) as f:
        config += f.read()

cfg = expand(config)

with open(out, "w") as f:
    f.write(cfg)
