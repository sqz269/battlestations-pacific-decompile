import os, sys, re
root = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))  # repo root
n = int(sys.argv[2]) if len(sys.argv) > 2 else 40
t = open(os.path.join(root, 'local', 'stlr_decomp', sys.argv[1] + '.c'), encoding='utf-8').read()
lines = [l.rstrip() for l in t.splitlines() if l.strip() and not re.match(r'\s*(undefined\d?|int|uint|char|float|void|code|byte|ushort|short|bool|int \*|uint \*|undefined4 \*|int \*\*)\s+\**\w+(\s*\[\d+\])?;$', l)]
print('\n'.join(lines[:n]))
