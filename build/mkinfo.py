#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys, os, json

template = {
    "name": sys.argv[2],
    "category": sys.argv[3],
    "environments": sys.argv[4].split(" "),
    }

open(sys.argv[1], "w").write(
    json.dumps(template, indent=4, separators=(',', ': '))
    + "\n"
    )
