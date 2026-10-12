"""Scratch-repo check of merge_resolve's config/tags three-way handler (cc13-tombstone1).

Usage: python tools/merge_resolve_tags_check.py   (builds throwaway repos under %TEMP%, then replays
merge 044466966, which resurrected 19 deleted rows; docs/CC13_MERGE_TOMBSTONES.md)
"""
import json
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import merge_resolve  # noqa: E402

SHARD = 'config/tags/00960000.jsonl'


def row(addr, cat='stl_probable', name=None):
    return json.dumps({'address': addr, 'name': name or f'FUN_{addr}', 'category': cat})


def git(repo, *args, check=True):
    return subprocess.run(['git', *args], cwd=repo, capture_output=True, text=True, check=check)


def scenario(label, base, ours, theirs, expect):
    repo = Path(tempfile.mkdtemp(prefix='tomb_'))
    try:
        git(repo, 'init', '-q', '-b', 'main')
        git(repo, 'config', 'user.email', 't@t'); git(repo, 'config', 'user.name', 't')
        f = repo / SHARD
        f.parent.mkdir(parents=True)

        def commit(rows, msg):
            f.write_text(''.join(r + '\n' for r in rows), encoding='utf-8', newline='\n')
            git(repo, 'add', '-A'); git(repo, 'commit', '-q', '-m', msg)
        commit(base, 'base')
        git(repo, 'checkout', '-q', '-b', 'theirs')
        commit(theirs, 'theirs')
        git(repo, 'checkout', '-q', 'main')
        commit(ours, 'ours')
        merged = git(repo, 'merge', '-q', '--no-edit', 'theirs', check=False)
        if merged.returncode == 0:
            how = 'git clean merge'
            result = f.read_text(encoding='utf-8').splitlines()
        else:
            conflicted, unresolved = merge_resolve.resolve_all(repo)
            how = 'refused' if unresolved else 'merge_resolve'
            result = None if unresolved else f.read_text(encoding='utf-8').splitlines()
        ok = (result is None and expect is None) or (result is not None and expect is not None and sorted(result) == sorted(expect))
        print(f'{"PASS" if ok else "FAIL"} {label}: {how}; {len(result) if result else 0} rows')
        return ok
    finally:
        shutil.rmtree(repo, ignore_errors=True)


def main():
    a = [row(f'0096{i:02X}00') for i in range(0, 40, 2)]
    ok = True
    # 1. both sides append (in the middle, same spot, so git conflicts)
    ours = a[:10] + [row('00961100', 'x')] + a[10:]
    theirs = a[:10] + [row('00961180', 'y')] + a[10:]
    ok &= scenario('both append at one spot', a, ours, theirs, ours[:11] + [theirs[10]] + a[10:])
    # 2. one side deletes rows, the other appends next to them
    ours = a[:9] + a[12:]
    theirs = a[:10] + [row('00961300', 'z')] + a[10:]
    ok &= scenario('delete vs nearby append', a, ours, theirs, a[:9] + [theirs[10]] + a[12:])
    # 3. one side deletes a row while the other edits the same row: must refuse
    ours = a[:10] + a[11:]
    theirs = a[:10] + [row(a[10][13:21], 'stl_instantiation')] + a[11:]
    ok &= scenario('delete vs edit of one row', a, ours, theirs, None)
    # 4. edits elsewhere plus deletions, interleaved with appends (the 044466966 shape)
    ours = [r for i, r in enumerate(a) if i not in (3, 4, 5)] + [row('00964000', 'new')]
    theirs = a[:2] + [row('00960100', 'add')] + a[2:] + [row('00965000', 'new2')]
    theirs[8] = row(a[7][13:21], 'edited')
    expect = [r for i, r in enumerate(a) if i not in (3, 4, 5, 7)] + [row('00960100', 'add'), row(a[7][13:21], 'edited'),
                                                                       row('00964000', 'new'), row('00965000', 'new2')]
    ok &= scenario('deletions + unrelated edit + appends', a, ours, theirs, expect)
    # 5. both sides delete different rows at one address: compose
    b = a + [row(a[-1][13:21], 'second')]
    ok &= scenario('deletions compose', b, b[:-2] + [b[-1], row('00967000')], b[:-1],
                   a[:-1] + [row('00967000')])
    # replay of the real merge 044466966 (base 11a736da9, ours cef29989a, theirs 02e21a51f)
    repo = Path(__file__).resolve().parents[1]
    for shard in ('config/tags/00960000.jsonl', 'config/tags/00900000.jsonl'):
        base, o, t = (git(repo, 'show', f'{c}:{shard}').stdout for c in ('11a736da9', 'cef29989a', '02e21a51f'))
        text = merge_resolve.three_way_rows(base, o, t, shard)
        if text is None:
            print(f'replay {shard}: refused')
            continue
        dropped = {json.loads(l)['address'] for l in base.splitlines() if l.strip()} - {json.loads(l)['address'] for l in t.splitlines() if l.strip()}
        back = sorted(dropped & {json.loads(l)['address'] for l in text.splitlines() if l.strip()})
        print(f'replay {shard}: {len(text.splitlines())} rows; addresses theirs dropped that came back: {back}')
    print('ALL PASS' if ok else 'SOME FAILED')


main()
