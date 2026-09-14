"""Mechanical resolution of worker-branch merge conflicts.

Handles: cmake/startup.cmake (union of registration lines), config/names shards (union by address,
incoming side wins for records it changed), and any text file where both sides inserted at the same
spot relative to the merge base (independent test cases added before the same closing line, appends).
Usage as CLI: python tools/merge_resolve.py <worktree>  -> resolves and stages what it can, reports the rest.
"""
import json
import re
import subprocess
import sys
from pathlib import Path


def stage_text(worktree, path, stage):
    return subprocess.run(['git', 'show', f':{stage}:{path}'], cwd=worktree, capture_output=True, text=True).stdout


def common_prefix_len(*strings):
    n = 0
    shortest = min(len(s) for s in strings)
    while n < shortest and all(s[n] == strings[0][n] for s in strings):
        n += 1
    return n


def same_spot_insertions(base, ours, theirs):
    """If ours == P + X + S and theirs == P + Y + S with base == P + S, return P + X + Y + S, else None."""
    if not base or base == ours or base == theirs:
        return None
    p = common_prefix_len(base, ours, theirs)
    # back off to a line boundary so insertions stay whole lines
    p = base.rfind('\n', 0, p) + 1
    rb, ro, rt = base[p:][::-1], ours[p:][::-1], theirs[p:][::-1]
    s = common_prefix_len(rb, ro, rt)
    s = rb.rfind('\n', 0, s) + 1 if s else 0
    suffix = base[len(base) - s:] if s else ''
    if base != base[:p] + suffix:
        return None
    x = ours[p:len(ours) - s] if s else ours[p:]
    y = theirs[p:len(theirs) - s] if s else theirs[p:]
    if not x or not y:
        return None
    return base[:p] + x + y + suffix


def resolve(worktree, path):
    worktree = Path(worktree)
    base, ours, theirs = (stage_text(worktree, path, i) for i in (1, 2, 3))
    if path == 'cmake/startup.cmake':
        lines = [l for l in ours.splitlines() if l.strip()]
        header = [l for l in lines if l.startswith('#') or l.startswith('cmake_minimum_required')]
        regs = []
        for l in ours.splitlines() + theirs.splitlines():
            if l.startswith('cmake_language(') and l not in regs:
                regs.append(l)
        # a target defined on both sides (add_executable/add_library with the same name) keeps the
        # incoming definition only: two definitions make CMake refuse the configure
        target_re = re.compile(r'CALL add_(?:executable|library) (\S+)')
        base_def = {}
        for l in base.splitlines():
            m = target_re.search(l)
            if m:
                base_def[m.group(1)] = l
        candidates = {}
        for l in regs:
            m = target_re.search(l)
            if m:
                candidates.setdefault(m.group(1), []).append(l)
        keep = {}
        for target, lines_for in candidates.items():
            # the side that changed the definition wins; if both changed, the incoming side does
            changed = [l for l in lines_for if l != base_def.get(target)]
            keep[target] = (changed or lines_for)[-1]
        regs = [l for l in regs if not target_re.search(l) or keep[target_re.search(l).group(1)] == l]
        # Sorting is safe for the target_sources / add_executable registrations, which
        # are order-independent, but NOT for a deferred `include`. cmake/startup.cmake
        # ends with
        #   cmake_language(DEFER CALL include ".../cmake/native_data_placement.cmake")
        # and that file runs target_link_options on bsp_game, so it has to be deferred
        # AFTER the deferred add_executable that creates the target. "include" sorts
        # before "target_sources" alphabetically, so a plain sorted() hoists it to the
        # top of the registry and the next configure dies with
        #   Cannot specify link options for target "bsp_game" which is not built by
        #   this project
        # which is what happened integrating agent/cc7-mount-frame-scale. Keep those
        # lines pinned to the end, in their original relative order.
        def order_dependent(line):
            return 'CALL include ' in line
        pinned = [l for l in regs if order_dependent(l)]
        sortable = sorted(l for l in regs if not order_dependent(l))
        (worktree / path).write_text('\n'.join(header + sortable + pinned) + '\n', encoding='utf-8', newline='\n')
        return 'registry union'
    if path.startswith('config/names/') and path.endswith('.jsonl'):
        base_by = {json.loads(l)['address']: json.loads(l) for l in base.splitlines() if l.strip()}
        merged = {json.loads(l)['address']: json.loads(l) for l in ours.splitlines() if l.strip()}
        for l in theirs.splitlines():
            if not l.strip():
                continue
            r = json.loads(l)
            if base_by.get(r['address']) != r:
                merged[r['address']] = r
        rows = sorted(merged.values(), key=lambda r: int(r['address'], 16))
        (worktree / path).write_text(''.join(json.dumps(r, ensure_ascii=False) + '\n' for r in rows), encoding='utf-8', newline='\n')
        return 'address union'
    text = same_spot_insertions(base, ours, theirs)
    if text is not None:
        (worktree / path).write_text(text, encoding='utf-8', newline='')
        return 'same-spot insertions kept from both sides'
    text = keep_both_pure_insertions(worktree, path, base, ours, theirs)
    if text is not None:
        (worktree / path).write_text(text, encoding='utf-8', newline='')
        return 'pure insertions on both sides kept (ours then theirs)'
    return None


def keep_both_pure_insertions(worktree, path, base, ours, theirs):
    """Use git merge-file --diff3 to see each conflict block's base region; when every block's base
    region is empty (both sides inserted new text at the same place), keep ours followed by theirs."""
    import re
    import tempfile
    with tempfile.TemporaryDirectory() as tmp:
        names = []
        for label, text in (('ours', ours), ('base', base), ('theirs', theirs)):
            p = Path(tmp) / label
            p.write_text(text, encoding='utf-8', newline='')
            names.append(str(p))
        out = subprocess.run(['git', 'merge-file', '-p', '--diff3', '-L', 'ours', '-L', 'base', '-L', 'theirs', *names],
                             cwd=worktree, capture_output=True, text=True).stdout
    pattern = re.compile(r'<<<<<<< ours\n(.*?)\|\|\|\|\|\|\| base\n(.*?)=======\n(.*?)>>>>>>> theirs\n', re.S)
    blocks = pattern.findall(out)
    if not blocks or any(b[1].strip() for b in blocks):
        return None
    return pattern.sub(lambda m: m.group(1) + m.group(3), out)


def resolve_all(worktree):
    worktree = Path(worktree)
    conflicted = subprocess.run(['git', 'diff', '--name-only', '--diff-filter=U'], cwd=worktree,
                                capture_output=True, text=True).stdout.split()
    unresolved = []
    for path in conflicted:
        how = resolve(worktree, path)
        if how:
            subprocess.run(['git', 'add', path], cwd=worktree, check=True)
            print(f'resolved {path}: {how}')
        else:
            unresolved.append(path)
    return conflicted, unresolved


if __name__ == '__main__':
    conflicted, unresolved = resolve_all(sys.argv[1] if len(sys.argv) > 1 else '.')
    print(f'{len(conflicted) - len(unresolved)} of {len(conflicted)} conflicts resolved' + (f'; unresolved: {unresolved}' if unresolved else ''))
    sys.exit(1 if unresolved else 0)
