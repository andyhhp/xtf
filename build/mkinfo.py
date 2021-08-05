#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, json

# Usage: mkcfg.py $OUT $NAME $CATEGORY $SUBARCH $ENVS $VARIATIONS
_, out, name, cat, subarch, envs, variations = sys.argv

template = {
    "name": name,
    "category": cat,
    "subarch": subarch,
    "environments": [],
    "variations": [],
    }

if envs:
    template["environments"] = envs.split(" ")
if variations:
    template["variations"] = variations.split(" ")

open(out, "w").write(
    json.dumps(template, indent=4, separators=(',', ': '))
    + "\n"
    )
