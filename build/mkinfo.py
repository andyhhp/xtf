#!/usr/bin/env python3
# -*- coding: utf-8 -*-
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

open(out, "w").write(
    json.dumps(template, indent=4, separators=(',', ': '))
    + "\n"
    )
