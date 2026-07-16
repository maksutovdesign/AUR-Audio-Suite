#!/usr/bin/env python3
# Removes the in-plugin theme selector (themeBox) from every module editor.
# The suite look is now fixed (Obsidian, set in Theme.h). applyThemeChoice()
# is left as harmless dead code (references no themeBox).
import os, re, glob
ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

def strip(path):
    with open(path) as f: lines = f.readlines()
    out = []
    for ln in lines:
        if 'themeBox' in ln:
            if 'presetBox' in ln:
                # mixed line: drop only the themeBox statement(s), keep presetBox
                ln = re.sub(r'themeBox\.\w+\s*\([^;]*\);\s*', '', ln)          # calls
                ln = ln.replace('presetBox, themeBox', 'presetBox')            # decl
                ln = ln.replace(', themeBox', '')
                out.append(ln)
            elif 'ComboBox' in ln:
                # declaration line sharing themeBox with other combo boxes
                ln = ln.replace(', themeBox', '').replace('themeBox, ', '').replace(' themeBox;', ';')
                if 'themeBox' in ln: continue   # was themeBox-only
                out.append(ln)
            else:
                # themeBox-only line → drop it entirely
                continue
        else:
            out.append(ln)
    new = ''.join(out)
    with open(path, 'w') as f: f.write(new)

n = 0
for ed in glob.glob(os.path.join(ROOT, '*', 'Source', 'PluginEditor.h')) + \
          glob.glob(os.path.join(ROOT, '*', 'Source', 'PluginEditor.cpp')):
    before = open(ed).read()
    if 'themeBox' in before:
        strip(ed); n += 1
        print('stripped', os.path.relpath(ed, ROOT))
print('files changed:', n)
