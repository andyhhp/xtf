#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""
mkcfg.py - Generate a configuration JSON file based on provided parameters.

Usage:
    python mkcfg.py $OUT $NAME $CATEGORY $ENVS $VARIATIONS

Arguments:

    $OUT:        Path to the output file where the generated JSON configuration
                 will be saved.
    $NAME:       Name to be assigned in the configuration.
    $CATEGORY:   Category designation in the configuration.
    $ENVS:       Optional space-separated list of environments (can be empty).
    $VARIATIONS: Optional space-separated list of variations (can be empty).

Description:

    This script generates a JSON configuration file using provided parameters
    and saves it to the specified output file. The generated JSON structure
    includes fields for 'name', 'category', 'environments', and 'variations'.
    The 'environments' and 'variations' fields can be populated with
    space-separated lists if corresponding arguments ($ENVS and $VARIATIONS)
    are provided.

Example:

    python mkcfg.py config.json ExampleConfig Utilities prod dev test

    This example will create a configuration file named 'config.json' with
       'name' as 'ExampleConfig',
       'category' as 'Utilities', and
       'environments' as ['prod', 'dev', 'test'].
"""

import sys, json

# Usage: mkcfg.py $OUT $NAME $CATEGORY $ENVS $VARIATIONS
_, out, name, cat, envs, variations = sys.argv  # pylint: disable=unbalanced-tuple-unpacking

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
