#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, json

# Usage: mkcfg.py $OUT $NAME $CATEGORY $ENVS $VARIATIONS
_, out, name, cat, envs, variations = sys.argv

template = {
    "name": name,
    "category": cat,
    "environments": envs.split(" "),
    "variations": variations.split(" "),
    }

open(out, "w").write(
    json.dumps(template, indent=4, separators=(',', ': '))
    + "\n"
    )
