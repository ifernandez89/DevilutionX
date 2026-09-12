#!/usr/bin/env python3
"""
patch_devilutionx_js.py - Patches Emscripten's generated devilutionx.js to:
1. Prevent querySelector("0") / querySelector(0) SyntaxError crashes in findEventTarget.
"""

import sys
import os

def patch_file(filepath):
    if not os.path.exists(filepath):
        print(f"[SKIP] File not found: {filepath}")
        return False

    with open(filepath, 'r', encoding='utf-8') as f:
        content = f.read()

    target_str = 'specialHTMLTargets=[0,typeof document!="undefined"?document:0,typeof window!="undefined"?window:0];var findEventTarget=target=>{target=maybeCStringToJsString(target);var domElement=specialHTMLTargets[target]||(typeof document!="undefined"?document.querySelector(target):undefined);return domElement};'
    replace_str = 'specialHTMLTargets=[typeof Module!="undefined"&&Module["canvas"]?Module["canvas"]:(typeof document!="undefined"?(document.getElementById("canvas")||0):0),typeof document!="undefined"?document:0,typeof window!="undefined"?window:0];var findEventTarget=target=>{target=maybeCStringToJsString(target);if(!target||target===0||target==="0")return specialHTMLTargets[0]||(typeof Module!="undefined"&&Module["canvas"])||(typeof document!="undefined"?document.getElementById("canvas"):undefined);var domElement=specialHTMLTargets[target]||(typeof document!="undefined"?document.querySelector(target):undefined);return domElement};'

    modified = False
    if target_str in content:
        content = content.replace(target_str, replace_str)
        modified = True
        print(f"[SUCCESS] Patched findEventTarget in {filepath}")

    # No graphics injection - diablo.ini uses canonical native defaults

    if modified:
        with open(filepath, 'w', encoding='utf-8') as f:
            f.write(content)
        return True
    elif replace_str in content:
        print(f"[ALREADY PATCHED] {filepath} already has fixes applied")
        return True
    else:
        print(f"[INFO] Target pattern not found in {filepath}")
        return False

if __name__ == '__main__':
    paths = sys.argv[1:] if len(sys.argv) > 1 else [
        'Packaging/emscripten/devilutionx.js',
        'build-web/devilutionx.js'
    ]
    for p in paths:
        patch_file(p)
