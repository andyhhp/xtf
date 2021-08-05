#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Construct an xl configuration file for a test (from various fragments), and
substitue variables appropriately.
"""

import sys, os

# Usage: mkcfg.py $OUT $DEFAULT-CFG $EXTRA-CFG $VARY-CFG
_, out, defcfg, vcpus, extracfg, varycfg = sys.argv

# Evaluate subarch, environment and name from $OUT
_, subarch, env, name = out.split('.')[0].split('-', 3)

# Possibly split apart the variation suffix
variation = ''
if '~' in name:
    parts = name.split('~', 1)
    name, variation = parts[0], '~' + parts[1]

def expand(text):
    """ Expand certain variables in text """
    return (text
            .replace("@@NAME@@",   name)
            .replace("@@ENV@@",    env)
            .replace("@@VCPUS@@",  vcpus)
            .replace("@@XTFDIR@@", os.environ["xtfdir"])
            .replace("@@VARIATION@@", variation)
            .replace("@@SUBARCH@@",   subarch)
        )

config = open(defcfg).read()

if extracfg:
    config += "\n# Test Extra Configuration:\n"
    config += open(extracfg).read()

if varycfg:
    config += "\n# Test Variation Configuration:\n"
    config += open(varycfg).read()

cfg = expand(config)

open(out, "w").write(cfg)
