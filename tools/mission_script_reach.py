"""Map each --menu-select id to the mission script it really runs, and count live native calls.

A menu id's script is NOT the COTP file of the same name. missiontree.lua gives the id's
sceneFile; the scene header's `StageScript = S "<dir>\\<name>"` names the script, loaded as
scripts/missions/<dir>/<name>.lua (the run log's `mission script name:` line shows the same).

Usage:
  python tools/mission_script_reach.py NativeA NativeB ...   # rows whose script calls any of them
  python tools/mission_script_reach.py                       # every row with its script

Counts are call sites outside `--` comments and `--[[ ]]` blocks, not calls reached at run time;
check the run's native census before calling a row exercised.
"""
import json
import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))


def game_root():
    with open(os.path.join(HERE, '..', 'config', 'target.json'), encoding='utf-8') as f:
        return os.path.dirname(json.load(f)['binary'])


def rows(root):
    path = os.path.join(root, 'scripts', 'datatables', 'missiontree.lua')
    mid = None
    with open(path, encoding='latin-1') as f:
        for line in f:
            m = re.search(r'\["id"\]\s*=\s*"([^"]+)"', line)
            if m:
                mid = m.group(1)
            m = re.search(r'sceneFile"\]\s*=\s*[^"]*"([^"]*)"', line)
            if m and mid:
                scene = m.group(1).strip()
                scene = re.sub(r'(?i)^universe/scenes/missions/', '', scene)
                yield mid, scene


def stage_script(root, scene):
    scn = os.path.join(root, 'universe', 'scenes', 'missions', scene.replace('/', os.sep))
    if not os.path.exists(scn):
        return None, 'no scene file ' + scene
    with open(scn, 'rb') as f:
        m = re.search(rb'StageScript = S "([^"]+)"', f.read())
    if not m:
        return None, 'no StageScript in ' + scene
    name = m.group(1).decode('latin-1').replace('\\\\', '/').replace('\\', '/')
    rel = 'scripts/missions/' + name + '.lua'
    path = os.path.join(root, rel.replace('/', os.sep))
    if not os.path.exists(path):
        return None, 'missing ' + rel
    return path, rel


def count_calls(path, natives):
    with open(path, encoding='latin-1') as f:
        text = re.sub(r'--\[\[.*?\]\]', '', f.read(), flags=re.S)
    code = [ln.split('--', 1)[0] for ln in text.splitlines()]
    return [sum(len(re.findall(r'\b' + re.escape(n) + r'\s*\(', ln)) for ln in code) for n in natives]


def main(argv):
    natives = argv[1:]
    root = game_root()
    for mid, scene in rows(root):
        path, rel = stage_script(root, scene)
        if path is None:
            if not natives:
                print(f'{mid}: ({rel})')
            continue
        if not natives:
            print(f'{mid}: {rel}')
            continue
        counts = count_calls(path, natives)
        if any(counts):
            hits = ' '.join(f'{n}={c}' for n, c in zip(natives, counts) if c)
            print(f'{mid}: {rel} {hits}')
    return 0


if __name__ == '__main__':
    sys.exit(main(sys.argv))
