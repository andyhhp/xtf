#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
Construct an xl configuration file for a test (from various fragments), and
substitue variables appropriately.
"""

import sys, os

# Usage: mkcfg.py $OUT $DEFAULT-CFG $EXTRA-CFG
_, out, defcfg, extracfg = sys.argv

# Evaluate environment and name from $OUT
_, env, name = out.split('.')[0].split('-', 2)

def expand(text):
    """ Expand certain variables in text """
    return (text
            .replace("@@NAME@@",   name)
            .replace("@@ENV@@",    env)
            .replace("@@XTFDIR@@", os.environ["xtfdir"])
        )

config = open(defcfg).read()

if extracfg:
    config += "\n# Test Extra Configuration:\n"
    config += open(extracfg).read()

cfg = expand(config)

open(out, "w").write(cfg)
