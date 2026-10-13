"""Safe removal of linked worktrees, one at a time or in bulk.

Worktrees share the main checkout's exports/bsp. Some older trees held a junction to it, and a
raw `git worktree remove`, `git clean` or `Remove-Item -Recurse` follows a junction and deletes
the TARGET's contents (2026-10-13: the shared exports were emptied that way during a mass prune).
Everything here deletes with its own walker, which never descends into a reparse point (junction,
directory symlink or mount point): it detaches the link itself and leaves the target alone.

Each removal is also guarded by a count of the shared exports before and after; a drop stops the
whole run. Before deleting a tree, small hand-written inputs under its local/ (order files,
launch scripts, notes) are copied to an archive directory, because game logs refer to them by path.

Bulk mode (`gc`) only removes trees that are clean, idle, leased by nobody and not running any
process; it is a dry run unless --apply is given. Removing a tree never deletes its branch unless
the branch is merged into origin/main and --delete-merged-branches is given.
"""
import json
import os
import re
import shutil
import stat
import subprocess
import sys
import time
from pathlib import Path

import coordination
import workspace

REPARSE = getattr(stat, 'FILE_ATTRIBUTE_REPARSE_POINT', 0x400)
DIRECTORY = getattr(stat, 'FILE_ATTRIBUTE_DIRECTORY', 0x10)
READONLY = getattr(stat, 'FILE_ATTRIBUTE_READONLY', 0x1)

ARCHIVE_SUFFIXES = {'.txt', '.ps1', '.py', '.json', '.lua', '.md', '.csv', '.cmd', '.bat', '.cfg', '.ini'}
ARCHIVE_SKIP = ('.log', '.out', '.done', '.err')
ARCHIVE_FILE_LIMIT = 512 * 1024
ARCHIVE_TREE_LIMIT = 64 * 1024 * 1024


def archive_root():
    env = os.environ.get('BSP_WORKTREE_ARCHIVE')
    if env:
        return Path(env)
    main = workspace.main_root()
    # deliberately not "<main>-<name>": that pattern is how worktrees are named
    return main.parent / 'bsp-worktree-archive'


def _long(path):
    text = os.path.abspath(str(path))
    if os.name != 'nt' or text.startswith('\\\\?\\'):
        return text
    if text.startswith('\\\\'):
        return '\\\\?\\UNC\\' + text[2:]
    return '\\\\?\\' + text


def _short(path):
    text = str(path)
    if text.startswith('\\\\?\\UNC\\'):
        return '\\\\' + text[8:]
    return text[4:] if text.startswith('\\\\?\\') else text


def _attributes(st):
    return getattr(st, 'st_file_attributes', 0)


def is_reparse(path):
    try:
        st = os.lstat(_long(path))
    except OSError:
        return False
    return bool(_attributes(st) & REPARSE) or stat.S_ISLNK(st.st_mode)


def walk_entries(root):
    """Yield (path, lstat, is_reparse) for everything under root without following any reparse point."""
    stack = [_long(root)]
    while stack:
        current = stack.pop()
        try:
            with os.scandir(current) as entries:
                for entry in entries:
                    try:
                        st = entry.stat(follow_symlinks=False)
                    except OSError:
                        continue
                    reparse = bool(_attributes(st) & REPARSE) or stat.S_ISLNK(st.st_mode)
                    yield entry.path, st, reparse
                    if not reparse and stat.S_ISDIR(st.st_mode):
                        stack.append(entry.path)
        except OSError:
            continue


def reparse_points(root):
    return [_short(p) for p, _, reparse in walk_entries(root) if reparse]


def _detach(path, st):
    """Remove a reparse point itself, never its target."""
    if _attributes(st) & DIRECTORY or stat.S_ISDIR(st.st_mode):
        os.rmdir(path)  # a junction or directory symlink: removes the link only
    else:
        os.unlink(path)


def _unlink_file(path, st):
    try:
        os.unlink(path)
    except PermissionError:
        if _attributes(st) & READONLY or not st.st_mode & stat.S_IWRITE:
            os.chmod(path, stat.S_IWRITE)
            os.unlink(path)
        else:
            raise


def delete_tree(root):
    """Delete root bottom-up. Reparse points are detached, never entered. Returns (bytes, detached, errors)."""
    root_long = _long(root)
    if is_reparse(root):
        _detach(root_long, os.lstat(root_long))
        return 0, [_short(root_long)], []
    freed, detached, errors = 0, [], []
    stack = [(root_long, False)]
    while stack:
        current, visited = stack.pop()
        if visited:
            try:
                os.rmdir(current)
            except OSError as error:
                errors.append(f'{_short(current)}: {error.strerror or error}')
            continue
        stack.append((current, True))
        try:
            with os.scandir(current) as entries:
                children = list(entries)
        except OSError as error:
            errors.append(f'{_short(current)}: {error.strerror or error}')
            continue
        for entry in children:
            try:
                st = entry.stat(follow_symlinks=False)
                if _attributes(st) & REPARSE or stat.S_ISLNK(st.st_mode):
                    _detach(entry.path, st)
                    detached.append(_short(entry.path))
                elif stat.S_ISDIR(st.st_mode):
                    stack.append((entry.path, False))
                else:
                    _unlink_file(entry.path, st)
                    freed += st.st_size
            except OSError as error:
                errors.append(f'{_short(entry.path)}: {error.strerror or error}')
    return freed, detached, errors


def exports_count():
    """Number of exported function directories, or -1 when the exports are not there at all."""
    functions = workspace.exports_dir() / 'functions'
    try:
        with os.scandir(functions) as entries:
            return sum(1 for _ in entries)
    except OSError:
        return -1


def archive_inputs(tree, name):
    """Copy small hand-written inputs from tree/local to the archive. Returns (files, bytes, target)."""
    local = Path(tree) / 'local'
    if not local.is_dir() or is_reparse(local):
        return 0, 0, None
    target = archive_root() / name
    copied = total = 0
    for path, st, reparse in walk_entries(local):
        if reparse or not stat.S_ISREG(st.st_mode):
            continue
        short = Path(_short(path))
        lower = short.name.lower()
        if short.suffix.lower() not in ARCHIVE_SUFFIXES or lower.endswith(ARCHIVE_SKIP):
            continue
        if st.st_size > ARCHIVE_FILE_LIMIT or total + st.st_size > ARCHIVE_TREE_LIMIT:
            continue
        rel = short.relative_to(Path(tree))
        destination = target / rel
        try:
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(path, _long(destination))
        except OSError:
            continue
        copied += 1
        total += st.st_size
    if copied:
        (target / 'ARCHIVED.json').write_text(json.dumps({
            'tree': str(tree), 'archived_utc': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
            'files': copied, 'bytes': total}, indent=1) + '\n', encoding='utf-8')
    return copied, total, target if copied else None


# ------------------------------------------------------------------ worktree inventory

def git(*args, cwd=None, check=False):
    run = subprocess.run(['git', *args], cwd=cwd or workspace.main_root(), capture_output=True, text=True)
    if check and run.returncode:
        raise RuntimeError((run.stdout + run.stderr).strip())
    return run


def worktrees():
    """Linked worktrees from `git worktree list --porcelain` (the main checkout excluded)."""
    rows, current = [], {}
    for line in git('worktree', 'list', '--porcelain').stdout.splitlines() + ['']:
        if not line.strip():
            if current:
                rows.append(current)
            current = {}
            continue
        key, _, value = line.partition(' ')
        current[key] = value or True
    main = os.path.normcase(str(workspace.main_root().resolve()))
    out = []
    for row in rows:
        path = Path(row['worktree'])
        if os.path.normcase(str(path.resolve())) == main or 'bare' in row:
            continue
        branch = row.get('branch')
        out.append({'path': path, 'name': path.name, 'head': row.get('HEAD'),
                    'branch': branch[len('refs/heads/'):] if isinstance(branch, str) else None,
                    'locked': 'locked' in row, 'prunable': 'prunable' in row})
    return out


def admin_dir(tree):
    marker = Path(tree) / '.git'
    try:
        text = marker.read_text(encoding='utf-8').strip()
    except OSError:
        return None
    if not text.startswith('gitdir:'):
        return None
    return Path(text[len('gitdir:'):].strip())


def last_reflog_time(admin):
    """Timestamp of the newest HEAD reflog entry, read from its text (the files' mtimes are unreliable:
    on 2026-10-13 something touched every worktree's admin HEAD at once)."""
    try:
        with open(admin / 'logs' / 'HEAD', 'rb') as handle:
            handle.seek(0, os.SEEK_END)
            handle.seek(max(0, handle.tell() - 4096))
            tail = handle.read().decode('utf-8', 'replace').splitlines()
    except OSError:
        return 0.0
    for line in reversed(tail):
        match = re.search(r'> (\d{9,11}) [+-]\d{4}\t', line)
        if match:
            return float(match.group(1))
    return 0.0


def last_activity(tree):
    """Newest of: the last HEAD reflog entry, the index mtime (staging, checkout) and the top level of
    local/ (runs and scripts write there)."""
    stamps = []
    admin = admin_dir(tree)
    if admin:
        stamps.append(last_reflog_time(admin))
        try:
            stamps.append((admin / 'index').stat().st_mtime)
        except OSError:
            pass
    local = Path(tree) / 'local'
    if local.is_dir() and not is_reparse(local):
        try:
            with os.scandir(local) as entries:
                for entry in entries:
                    try:
                        stamps.append(entry.stat(follow_symlinks=False).st_mtime)
                    except OSError:
                        pass
        except OSError:
            pass
    return max(stamps) if stamps else 0.0


def dirty_summary(tree):
    """Tracked changes plus untracked files that are NOT ignored ('' when clean, None when git failed)."""
    # --no-optional-locks: do not refresh (rewrite) the index, which would make the tree look active
    run = subprocess.run(['git', '--no-optional-locks', 'status', '--porcelain', '--untracked-files=normal'], cwd=tree,
                         capture_output=True, text=True)
    if run.returncode:
        return None
    lines = [l for l in run.stdout.splitlines() if l.strip()]
    return '' if not lines else f'{len(lines)} changed: ' + ', '.join(l[3:] for l in lines[:3])


def processes_by_tree(trees):
    """Map tree path -> list of 'pid name' for processes whose exe or command line mentions it."""
    if os.name != 'nt':
        return {}
    script = ('Get-CimInstance Win32_Process | Select-Object ProcessId,Name,ExecutablePath,CommandLine '
              '| ConvertTo-Json -Compress')
    run = subprocess.run(['powershell', '-NoProfile', '-Command', script], capture_output=True, text=True)
    try:
        procs = json.loads(run.stdout or '[]')
    except json.JSONDecodeError:
        return {}
    found = {}
    # the tree itself, not a longer sibling such as <tree>-2 or <tree>b
    patterns = [(t, re.compile(re.escape(os.path.normcase(str(t)).rstrip('\\')) + r'(?![\w-])')) for t in trees]
    mine = {os.getpid(), os.getppid()}
    for p in procs if isinstance(procs, list) else [procs]:
        if p.get('ProcessId') in mine:
            continue
        text = os.path.normcase(f"{p.get('ExecutablePath') or ''} {p.get('CommandLine') or ''}".replace('/', '\\'))
        for tree, pattern in patterns:
            if pattern.search(text):
                found.setdefault(tree, []).append(f"{p.get('ProcessId')} {p.get('Name')}")
    return found


def leases_by_tree(trees, branches=None):
    """Active leases held from a tree, or by its branch's owner name (owner is the branch by default)."""
    found = {}
    rows = coordination.active_leases()
    branches = branches or {}
    for tree in trees:
        key = os.path.normcase(str(tree))
        owner = branches.get(tree)
        for row in rows:
            if os.path.normcase(str(row.get('worktree', ''))) == key or (owner and row.get('owner') == owner):
                found.setdefault(tree, []).append(row.get('packet') or row.get('id'))
    return found


def merged_into(ref, target):
    if not ref:
        return False
    return git('merge-base', '--is-ancestor', ref, target).returncode == 0


# ------------------------------------------------------------------ removal

class ExportsLoss(RuntimeError):
    pass


def remove_tree(tree, branch=None, delete_branch=False, archive=True, log=print):
    """Archive inputs, delete the tree with the reparse-safe walker, prune git's entry, check the exports."""
    tree = Path(tree)
    main = workspace.main_root()
    exports = workspace.exports_dir()
    if os.path.normcase(str(tree.resolve())) == os.path.normcase(str(main.resolve())):
        raise RuntimeError('refusing to remove the main checkout')
    try:
        exports.resolve().relative_to(tree.resolve())
        raise RuntimeError(f'refusing: the shared exports {exports} resolve inside {tree}')
    except ValueError:
        pass
    before = exports_count()
    freed, detached = 0, []
    if tree.exists() or is_reparse(tree):
        if archive:
            files, size, target = archive_inputs(tree, tree.name)
            if files:
                log(f'  archived {files} input file(s), {size // 1024} KB -> {target}')
        freed, detached, errors = delete_tree(tree)
        for link in detached:
            log(f'  detached reparse point {link}')
        if errors:
            log(f'  {len(errors)} item(s) could not be deleted, first: {errors[0]}')
    after = exports_count()
    if before >= 0 and after < before:
        raise ExportsLoss(f'shared exports dropped from {before} to {after} entries while removing {tree}; '
                          'stopping. Restore with: python tools/bsp.py exports restore')
    git('worktree', 'prune')
    remaining = tree.exists()
    log(f"  {'PARTLY removed' if remaining else 'removed'} {tree} ({freed / 2**30:.2f} GB freed)")
    if delete_branch and branch and not remaining:
        if merged_into(branch, 'origin/main'):
            # -D: `-d` compares with the main checkout's HEAD, which is often a stale local main;
            # the commits are already in origin/main, which is what was just checked
            run = git('branch', '-D', branch)
            log('  ' + ((run.stdout + run.stderr).strip() or f'deleted branch {branch}'))
        else:
            log(f'  branch {branch} kept: not merged into origin/main')
    return freed, not remaining


def gc(args, log=print):
    git('fetch', '-q', 'origin')
    trees = worktrees()
    if args.match:
        trees = [t for t in trees if any(m.lower() in t['name'].lower() for m in args.match)]
    here = os.path.normcase(str(Path.cwd().resolve()))
    procs = processes_by_tree([t['path'] for t in trees])
    leases = leases_by_tree([t['path'] for t in trees], {t['path']: t['branch'] for t in trees})
    now = time.time()
    picked, skipped = [], {}
    for t in trees:
        path = t['path']
        reason = None
        if t['prunable'] or not path.exists():
            reason = None  # stale entry: prune only
        elif here == os.path.normcase(str(path.resolve())) or here.startswith(os.path.normcase(str(path.resolve())) + os.sep):
            reason = 'current directory'
        elif t['locked']:
            reason = 'locked'
        elif path in leases:
            reason = 'active lease ' + ','.join(map(str, leases[path]))
        elif path in procs:
            reason = 'running ' + procs[path][0]
        elif now - last_activity(path) < args.idle_hours * 3600:
            reason = f'active within {args.idle_hours:g} h'
        else:
            dirty = dirty_summary(path)
            if dirty is None:
                reason = 'git status failed'
            elif dirty:
                reason = 'uncommitted work (' + dirty + ')'
            elif args.merged_only and not merged_into(t['branch'] or t['head'], 'origin/main'):
                reason = 'branch not merged into origin/main'
        if reason:
            skipped.setdefault(reason.split(' (')[0] if reason.startswith('uncommitted') else reason, []).append(t['name'])
        else:
            picked.append(t)
    log(f"{len(trees)} linked worktree(s); {len(picked)} removable; {sum(len(v) for v in skipped.values())} kept")
    for reason, names in sorted(skipped.items(), key=lambda kv: -len(kv[1]))[:12]:
        log(f"  kept ({reason}): {len(names)}: {', '.join(names[:6])}{' ...' if len(names) > 6 else ''}")
    if not args.apply:
        for t in picked[:args.limit]:
            log(f"  would remove {t['name']} ({t['branch'] or 'detached'})")
        if len(picked) > args.limit:
            log(f'  ... and {len(picked) - args.limit} more')
        log('dry run: pass --apply to remove')
        return 0
    total = removed = 0
    for t in picked[:args.limit]:
        log(f"{t['name']}:")
        try:
            freed, ok = remove_tree(t['path'], t['branch'], args.delete_merged_branches, not args.no_archive, log)
        except ExportsLoss as error:
            log(f'STOP: {error}')
            return 2
        total += freed
        removed += ok
    log(f'removed {removed} worktree(s), {total / 2**30:.1f} GB freed')
    return 0
