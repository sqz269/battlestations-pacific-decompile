"""Is this worktree's branch tip on main, and if so fast-forward it (`bsp.py sync`).

A refused `git merge --ff-only main` has one cause in this workflow: the branch carries a commit
main does not have. Four workers on 2026-09-27 read that refusal as "main moved". `sync` answers
the real question first and never merges anything that is not a fast-forward.
"""
from __future__ import annotations

import subprocess
from pathlib import Path


def _git(root, *args, check=True):
    proc = subprocess.run(['git', *args], cwd=str(root), capture_output=True, text=True)
    if check and proc.returncode:
        raise RuntimeError(f"git {' '.join(args)}: {proc.stderr.strip()}")
    return proc


def ref_exists(root, ref) -> bool:
    return _git(root, 'rev-parse', '--verify', '--quiet', ref + '^{commit}', check=False).returncode == 0


def unlanded(root, bases) -> list[tuple[str, str]]:
    """Commits reachable from HEAD and from none of `bases`, as (short sha, subject), oldest first."""
    bases = [b for b in bases if ref_exists(root, b)]
    if not bases:
        raise RuntimeError('none of the base refs exists: ' + ', '.join(bases))
    out = _git(root, 'log', '--reverse', '--format=%h %s', 'HEAD', '--not', *bases).stdout
    return [tuple(line.split(' ', 1)) if ' ' in line else (line, '') for line in out.splitlines() if line]


def tip_line(root, bases=('main', 'origin/main')) -> str:
    """One line for `brief`. No fetch: it compares against the refs as they are."""
    try:
        commits = unlanded(root, bases)
    except RuntimeError as exc:
        return f'branch tip: unknown ({exc})'
    if not commits:
        return 'branch tip is on main'
    shas = ' '.join(sha for sha, _ in commits[:8]) + (' ...' if len(commits) > 8 else '')
    return f'{len(commits)} commits not on main: {shas}'


def sync(root, remote='origin', base='main', fetch=True) -> int:
    root = Path(root)
    target = f'{remote}/{base}'
    if fetch:
        f = _git(root, 'fetch', '--quiet', remote, check=False)
        if f.returncode:
            print(f'sync: fetch {remote} failed: {f.stderr.strip()}')
            return 2
    branch = _git(root, 'rev-parse', '--abbrev-ref', 'HEAD').stdout.strip()
    commits = unlanded(root, [target])
    if commits:
        print(f'sync: {branch} has {len(commits)} commit(s) not on {target}; nothing merged')
        for sha, subject in commits[:20]:
            print(f'  {sha} {subject}')
        if len(commits) > 20:
            print(f'  ... {len(commits) - 20} more')
        print('unlanded: report to the lead')
        return 1
    head = _git(root, 'rev-parse', 'HEAD').stdout.strip()
    new = _git(root, 'rev-parse', target).stdout.strip()
    if head == new:
        print(f'sync: {branch} is already at {target} ({new[:9]})')
        return 0
    moved = _git(root, 'log', '--first-parent', '--format=%h %s', f'HEAD..{target}').stdout.splitlines()
    merge = _git(root, 'merge', '--ff-only', '--quiet', target, check=False)
    if merge.returncode:
        # Only a dirty tree touching the incoming files gets here: the tip is an ancestor.
        print(f'sync: fast-forward to {target} refused by git: {merge.stderr.strip()}')
        return 2
    stat = _git(root, 'diff', '--shortstat', head, new).stdout.strip()
    print(f'sync: {branch} fast-forwarded {head[:9]} -> {new[:9]} ({len(moved)} first-parent commits; {stat})')
    for line in moved[:15]:
        print(f'  {line}')
    if len(moved) > 15:
        print(f'  ... {len(moved) - 15} more')
    return 0
