"""Mechanical resolution of worker-branch merge conflicts.

Handles: cmake/startup.cmake (union of registration lines), config/names shards (union by address,
incoming side wins for records it changed), and any text file where both sides inserted at the same
spot relative to the merge base (independent test cases added before the same closing line, appends).
Usage as CLI: python tools/merge_resolve.py <worktree>  -> resolves and stages what it can, reports the rest.
"""
import collections
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
        def by_address(text):
            out = {}
            for line in text.splitlines():
                if line.strip():
                    record = json.loads(line)
                    out[record['address']] = record
            return out
        base_by, our_by, their_by = (by_address(t) for t in (base, ours, theirs))
        merged, contested = {}, []
        for address in set(base_by) | set(our_by) | set(their_by):
            b, o, t = base_by.get(address), our_by.get(address), their_by.get(address)
            if o == t:
                keep = o
            elif o == b:
                keep = t          # only the incoming side changed it
            elif t == b:
                keep = o          # only our side changed it
            else:
                contested.append(address)
                continue
            if keep is not None:
                merged[address] = keep
        if contested:
            # Both sides renamed or re-evidenced the same address differently. Taking the incoming
            # side here silently discarded the other's work, which is how main's records were lost
            # during the orch5 integration. Refuse and let a reader compare the evidence.
            return None
        rows = sorted(merged.values(), key=lambda r: int(r['address'], 16))
        (worktree / path).write_text(''.join(json.dumps(r, ensure_ascii=False) + '\n' for r in rows), encoding='utf-8', newline='\n')
        return 'address union'
    if path.startswith('config/reconstruction/') and path.endswith('.jsonl'):
        text = union_reconstruction_shard(worktree, path, base, ours, theirs)
        if text is not None:
            (worktree / path).write_text(text, encoding='utf-8', newline='\n')
            return 'reconstruction append union'
        return None  # refused on purpose; see the docstring
    text = same_spot_insertions(base, ours, theirs)
    if text is not None:
        (worktree / path).write_text(text, encoding='utf-8', newline='')
        return 'same-spot insertions kept from both sides'
    text = keep_both_pure_insertions(worktree, path, base, ours, theirs)
    if text is not None:
        (worktree / path).write_text(text, encoding='utf-8', newline='')
        return 'pure insertions on both sides kept (ours then theirs)'
    return None


def union_reconstruction_shard(worktree, path, base, ours, theirs):
    """Union appended records in a config/reconstruction shard, or refuse.

    Line-level, deliberately not keyed by address the way config/names is. A reconstruction shard
    legitimately holds several records for one address: main currently has 6322 records over 5564
    distinct addresses, and 649 (shard, address) groups hold two or more, mostly a fragment beside
    a function. No field separates them reliably, so an address-keyed union silently drops records.

    Two cases resolve. Both sides kept every base line and only appended, so the appends union. Or
    exactly one side rewrote base records while the other left them untouched, which is not a
    disagreement: that side is kept whole and the other's appends replay onto it.

    Everything else is refused so the conflict stays staged for a human, specifically
      - both sides rewrote base records in place, which only the evidence can settle,
      - an append landing on an address the other side was rewriting,
      - both sides adding different records for an address the base did not have, which is two
        competing reconstructions of the same function,
      - any line that is not JSON carrying an address.
    """
    def rows(text):
        out = []
        for line in text.splitlines():
            line = line.strip()
            if not line:
                continue
            try:
                record = json.loads(line)
            except ValueError:
                return None
            if not isinstance(record, dict) or 'address' not in record:
                return None
            out.append((json.dumps(record, ensure_ascii=False, sort_keys=True), record))
        return out

    parsed = [rows(t) for t in (base, ours, theirs)]
    if any(p is None for p in parsed):
        return None
    base_rows, our_rows, their_rows = parsed
    base_keys = {k for k, _ in base_rows}
    our_keys = {k for k, _ in our_rows}
    their_keys = {k for k, _ in their_rows}
    our_edits, their_edits = base_keys - our_keys, base_keys - their_keys
    if our_edits and their_edits:
        return None  # both sides rewrote base records in place; only evidence can settle that
    if our_edits or their_edits:
        # exactly one side rewrote base records and the other left them untouched, so there is no
        # competing opinion: keep the editing side whole and replay the other side's appends onto it.
        editor, appender = (our_rows, their_rows) if our_edits else (their_rows, our_rows)
        editor_keys = {k for k, _ in editor}
        base_addrs = {r['address'].lower() for _, r in base_rows}
        new_addrs = {r['address'].lower() for k, r in appender if k not in base_keys} - base_addrs
        editor_new = {r['address'].lower() for k, r in editor if k not in base_keys} - base_addrs
        edited_addrs = {json.loads(k)['address'].lower() for k in (our_edits or their_edits)}
        if new_addrs & edited_addrs:
            return None  # the appends land on an address the other side was rewriting
        if new_addrs & editor_new:
            return None  # both sides reconstructed the same new address; only evidence settles which
        merged = [r for _, r in editor] + [r for k, r in appender if k not in base_keys and k not in editor_keys]
        merged.sort(key=lambda r: (int(r['address'], 16), r.get('kind') or '', r.get('name') or ''))
        return ''.join(json.dumps(r, ensure_ascii=False) + '\n' for r in merged)
    our_new = [(k, r) for k, r in our_rows if k not in base_keys]
    their_new = [(k, r) for k, r in their_rows if k not in base_keys]
    base_addrs = {r['address'].lower() for _, r in base_rows}
    # a record both sides added byte for byte is the same finding reached twice, not a conflict
    agreed = {k for k, _ in our_new} & {k for k, _ in their_new}
    our_addrs = {r['address'].lower() for k, r in our_new if k not in agreed} - base_addrs
    their_addrs = {r['address'].lower() for k, r in their_new if k not in agreed} - base_addrs
    contested = our_addrs & their_addrs
    if contested:
        return None  # competing reconstructions of the same new address
    seen, merged = set(), []
    for k, r in base_rows + our_new + their_new:
        if k in seen:
            continue  # byte-identical duplicate, safe to collapse
        seen.add(k)
        merged.append(r)
    merged.sort(key=lambda r: (int(r['address'], 16), r.get('kind') or '', r.get('name') or ''))
    return ''.join(json.dumps(r, ensure_ascii=False) + '\n' for r in merged)


def keep_both_pure_insertions(worktree, path, base, ours, theirs):
    """Use git merge-file --diff3 to see each conflict block's base region; when every block's base
    region is empty (both sides inserted new text at the same place), keep ours followed by theirs.

    Refuses add/add, where the file is new on both sides and there is no base at all. Every block's
    base region is empty there, so the rule above would concatenate two whole files: two copies of
    the same C++ definitions, or two JSON objects in one document. Integrating orch3 produced exactly
    that on four files before it was caught by hand. Two independent versions of a new file are a
    judgement about which reconstruction to keep, not an insertion.
    """
    import re
    import tempfile
    if not base.strip():
        return None
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
