"""Integrate worker branches: merge into agent/integrate, build there, fast-forward main, apply the
new reviewed names to Ghidra, refresh snapshot/index, push. Usage:
  python tools/integrate_workers.py agent/winmain-startup agent/app-run-frame [--no-push] [--skip-build]
Run from the main checkout or an orchestrator worktree. Aborts on a merge conflict or a failed
build and leaves the integrate worktree for inspection. Merge-commit trailers come from the
script checkout's local/commit-trailer.txt, or BSP_COMMIT_TRAILER_FILE when specified."""
import json
import os
import subprocess
import sys
from pathlib import Path
from workspace import main_root

SCRIPT_ROOT = Path(__file__).resolve().parents[1]
MAIN = main_root()
_TRAILER_FILE = Path(os.environ.get('BSP_COMMIT_TRAILER_FILE',
                                   str(SCRIPT_ROOT / 'local/commit-trailer.txt')))
TRAILER = _TRAILER_FILE.read_text(encoding='utf-8').strip() if _TRAILER_FILE.exists() else ''
# a second orchestrator uses its own integrate worktree: BSP_INTEGRATE=<name> selects
# ../battlestations-pacific-decompile-<name> on branch agent/<name> (create it with bsp.py worktree add <name>)
INTEGRATE_NAME = os.environ.get('BSP_INTEGRATE', 'integrate')
INTEGRATE = MAIN.parent / f'{MAIN.name}-{INTEGRATE_NAME}'
INTEGRATE_BRANCH = f'agent/{INTEGRATE_NAME}'
branches = [a for a in sys.argv[1:] if not a.startswith('--')]
no_push = '--no-push' in sys.argv
skip_build = '--skip-build' in sys.argv


def run(args, cwd, check=True, capture=True):
    result = subprocess.run(args, cwd=cwd, capture_output=capture, text=True)
    if check and result.returncode:
        sys.exit(f"FAILED in {cwd}: {' '.join(map(str, args))}\n{(result.stdout or '') [-2000:]}\n{(result.stderr or '')[-2000:]}")
    return result


def git(cwd, *args, check=True):
    return run(['git', *args], cwd, check=check).stdout.strip()


def stage_lines(path, stage):
    text = run(['git', 'show', f':{stage}:{path}'], INTEGRATE, check=False).stdout
    return [l for l in text.splitlines() if l.strip()]


def stage_text(path, stage):
    return run(['git', 'show', f':{stage}:{path}'], INTEGRATE, check=False).stdout


def append_only(path):
    """True when both sides only appended to the common base (independent test cases, registries)."""
    base, ours, theirs = stage_text(path, 1), stage_text(path, 2), stage_text(path, 3)
    return bool(base) and ours.startswith(base) and theirs.startswith(base)


def resolve_mechanical(path):
    """cmake/startup.cmake: union of registration lines. names shard: union by address, worker side wins.
    Any other append-only conflict: base + our tail + their tail."""
    if path != 'cmake/startup.cmake' and not path.startswith('config/names/'):
        base, ours, theirs = stage_text(path, 1), stage_text(path, 2), stage_text(path, 3)
        (INTEGRATE / path).write_text(base + ours[len(base):] + theirs[len(base):], encoding='utf-8', newline='')
        return
    base, ours, theirs = stage_lines(path, 1), stage_lines(path, 2), stage_lines(path, 3)
    if path == 'cmake/startup.cmake':
        header = [l for l in ours if l.startswith('#') or l.startswith('cmake_minimum_required')]
        regs = []
        for l in ours + theirs:
            if l.startswith('cmake_language(') and l not in regs:
                regs.append(l)
        (INTEGRATE / path).write_text('\n'.join(header + sorted(regs)) + '\n', encoding='utf-8', newline='\n')
        return
    sys.path.insert(0, str(INTEGRATE / 'tools'))
    import ledger  # noqa: E402
    base_by = {json.loads(l)['address']: json.loads(l) for l in base}
    merged = {json.loads(l)['address']: json.loads(l) for l in ours}
    for l in theirs:
        r = json.loads(l)
        if base_by.get(r['address']) != r:
            merged[r['address']] = r
    ledger._write_shard(INTEGRATE / path, list(merged.values()))


def manual_ff(blocking):
    """Fast-forward main without touching the other integrator's uncommitted shard edits.
    Moves the ref with an atomic compare-and-swap, refreshes the index, checks out every changed file
    except the dirty shards, and rewrites each dirty shard as the address-keyed union of its working
    copy and the merged version so the other integrator's next commit carries both sets of records."""
    sys.path.insert(0, str(MAIN / 'tools'))
    import ledger  # noqa: E402
    old = git(MAIN, 'rev-parse', 'main')
    new = git(INTEGRATE, 'rev-parse', 'HEAD')
    changed = git(MAIN, 'diff', '--name-only', old, new).splitlines()
    unions = {}
    for path in blocking:
        working = [json.loads(l) for l in (MAIN / path).read_text(encoding='utf-8').splitlines() if l.strip()]
        merged_side = [json.loads(l) for l in run(['git', 'show', f'{new}:{path}'], MAIN).stdout.splitlines() if l.strip()]
        base_side = {json.loads(l)['address']: json.loads(l) for l in run(['git', 'show', f'{old}:{path}'], MAIN, check=False).stdout.splitlines() if l.strip()}
        union = {r['address']: r for r in merged_side}
        for r in working:  # the other integrator's live edits win where they changed a record
            if base_side.get(r['address']) != r or r['address'] not in union:
                union[r['address']] = r
        unions[path] = list(union.values())
    git(MAIN, 'update-ref', 'refs/heads/main', new, old)
    git(MAIN, 'reset', '-q')
    to_checkout = [p for p in changed if p not in blocking]
    if to_checkout:
        run(['git', 'checkout', '--', *to_checkout], MAIN)
    for path, rows in unions.items():
        ledger._write_shard(MAIN / path, rows)
    print(f'main fast-forwarded {old[:8]} -> {new[:8]}; {len(to_checkout)} files refreshed; {len(unions)} dirty shard(s) merged in place')


assert INTEGRATE.exists(), f'run: python tools/bsp.py worktree add {INTEGRATE_NAME}'
base = git(MAIN, 'rev-parse', 'main')
print('main at', base[:8])
git(INTEGRATE, 'merge', '--ff-only', 'main', check=False)
if git(INTEGRATE, 'rev-parse', 'HEAD') != base:
    print('integrate branch had diverged; merging main into it')
    result = run(['git', 'merge', '--no-edit', 'main'], INTEGRATE, check=False)
    if result.returncode:
        # the same mechanical resolution as for worker branches (ledger shards, registry lines)
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        import merge_resolve  # noqa: E402
        conflicted, unresolved = merge_resolve.resolve_all(INTEGRATE)
        if not conflicted or unresolved:
            sys.exit(f'merge conflict merging main into the integrate branch; unresolved {unresolved}:\n'
                     f'{result.stdout[-1200:]}\nresolve in {INTEGRATE} or `git merge --abort` there')
        run(['git', '-c', 'core.editor=true', 'commit', '--no-edit', '-q'], INTEGRATE)
        print(f'merged main into integrate with mechanical resolution of {conflicted}')
merged = []
for branch in branches:
    ahead = git(MAIN, 'rev-list', '--count', f'main..{branch}')
    if ahead == '0':
        print(f'{branch}: nothing to merge')
        continue
    msg = INTEGRATE / 'local' / 'merge-msg.txt'
    msg.parent.mkdir(exist_ok=True)
    msg.write_text(f'Merge {branch}\n' + (f'\n{TRAILER}\n' if TRAILER else ''), encoding='utf-8')
    result = run(['git', 'merge', '--no-ff', branch, '-F', str(msg)], INTEGRATE, check=False)
    if result.returncode:
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        import merge_resolve  # noqa: E402
        conflicted, unresolved = merge_resolve.resolve_all(INTEGRATE)
        if not conflicted or unresolved:
            sys.exit(f'merge conflict merging {branch}; unresolved {unresolved}:\n{result.stdout[-1200:]}\nresolve in {INTEGRATE} or `git merge --abort` there')
        run(['git', '-c', 'core.editor=true', 'commit', '--no-edit', '-q'], INTEGRATE)
        print(f'merged {branch} with mechanical resolution of {conflicted}')
        merged.append(branch)
        continue
    merged.append(branch)
    print(f'merged {branch} ({ahead} commits)')
if not merged:
    sys.exit('nothing merged')


def duplicate_definitions(tree):
    """Top-level struct/class/enum names and inline constexpr constants declared in more than one
    header under include/bsp: two packets that branched apart can declare the same name and only
    collide once a translation unit includes both (MissionGroup, VehicleClassKindRow)."""
    import re
    where = {}
    pattern = re.compile(r'^(?:struct|class|enum class|enum|union)\s+(\w+)\s*(?::|\{)|^inline\s+constexpr\s+[\w:<>]+\s+(k\w+)\b', re.M)
    for header in sorted((tree / 'include/bsp').glob('*.hpp')):
        text = header.read_text(encoding='utf-8', errors='replace')
        for m in pattern.finditer(text):
            name = m.group(1) or m.group(2)
            where.setdefault(name, set()).add(header.name)
    return {name: sorted(files) for name, files in where.items() if len(files) > 1}


dupes = duplicate_definitions(INTEGRATE)
if dupes:
    sys.exit('duplicate top-level definitions across headers (rename in the integrate worktree, commit there, rerun):\n'
             + '\n'.join(f'  {name}: {files}' for name, files in sorted(dupes.items())))
if not skip_build:
    print('building in the integrate worktree ...')
    build = run(['powershell', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', str(INTEGRATE / 'scripts/build.ps1')], INTEGRATE, check=False)
    tail = (build.stdout + build.stderr).strip().splitlines()[-12:]
    print('\n'.join(tail))
    if build.returncode:
        sys.exit(f'build or tests failed in {INTEGRATE}; main not touched')
# fast-forward main. Two failure modes: main moved (the other integrator commits often) -> merge it
# into integrate and retry; a file the merge touches is dirty in main's working tree (the other
# integrator's uncommitted ledger edits) -> wait for it to commit, polling for up to 30 minutes.
import re as _re
import time as _time
deadline = _time.time() + 30 * 60
while True:
    ff = run(['git', 'merge', '--ff-only', INTEGRATE_BRANCH], MAIN, check=False)
    if ff.returncode == 0:
        break
    err = ff.stdout + ff.stderr
    if 'would be overwritten' in err:
        blocking = _re.findall(r'^\s+(\S+)$', err, _re.M)
        if all(b.startswith('config/names/') and b.endswith('.jsonl') for b in blocking):
            print(f'manual fast-forward: merging the other integrator\'s uncommitted shard edits by address for {blocking}', flush=True)
            manual_ff(blocking)
            break
        if _time.time() > deadline:
            sys.exit(f'gave up waiting for the other integrator to commit {blocking}; rerun later')
        print(f'waiting: uncommitted changes in main to {blocking}; retry in 60 s', flush=True)
        _time.sleep(60)
        continue
    if 'Not possible to fast-forward' in err or 'not possible' in err.lower() or 'diverg' in err.lower():
        print('main moved; merging main into integrate and retrying', flush=True)
        git(INTEGRATE, 'merge', '--no-edit', 'main')
        continue
    sys.exit(f'could not fast-forward main:\n{err[-800:]}')
print('main now at', git(MAIN, 'rev-parse', '--short', 'HEAD'))
# reviewed names added by the merged branches -> Ghidra
added = run(['git', 'diff', f'{base}..HEAD', '--', 'config/names'], MAIN).stdout
addresses = sorted({json.loads(line[1:])['address'] for line in added.splitlines()
                    if line.startswith('+') and not line.startswith('+++') and line[1:].strip().startswith('{')})
print(f'{len(addresses)} reviewed names added by the merge')
# an address leased to another owner (typically the next worker, dispatched before this merge)
# would make the annotate tool refuse the whole batch; apply the rest now and list the deferred ones
sys.path.insert(0, str(MAIN / 'tools'))
import coordination  # noqa: E402
deferred = []
for a in list(addresses):
    try:
        coordination.check_writable([a])
    except coordination.LeaseConflict as exc:
        deferred.append(a)
        addresses.remove(a)
        print(f'deferring {a}: {str(exc)[:110]}')
if deferred:
    print(f"apply later: python tools/ghidra_annotate.py --apply --addresses {' '.join(deferred)}")
# a name for an address Ghidra never defined as a function (a worker read it from the raw listing)
# would fail the plate-comment read; define those first, then apply
if addresses:
    undefined = []
    sys.path.insert(0, str(MAIN / 'tools'))
    from ghidra_export import Client  # noqa: E402
    client = Client(json.loads((MAIN / 'config/target.json').read_text(encoding='utf-8')))
    for a in list(addresses):
        info = str(client.get('get_function_by_address', address=a))
        if 'No function' in info or 'error' in info.lower():
            undefined.append(a)
            addresses.remove(a)
    if undefined:
        print(f"no Ghidra function at {' '.join(undefined)}: define them with tools/ghidra_define_function.py, then "
              f"python tools/ghidra_annotate.py --apply --addresses {' '.join(undefined)}")
if addresses:
    ann = run([sys.executable, str(MAIN / 'tools/ghidra_annotate.py'), '--apply', '--addresses', *addresses], MAIN, check=False)
    print((ann.stdout + ann.stderr).strip().splitlines()[-1])
    if ann.returncode:
        sys.exit('annotate failed; names are in the ledger but not in Ghidra')
    snap = run([sys.executable, str(MAIN / 'tools/bsp.py'), 'snapshot', '--force'], MAIN, check=False)
    print((snap.stdout + snap.stderr).strip().splitlines()[-1])
run([sys.executable, str(MAIN / 'tools/bsp.py'), 'index', '--if-stale'], MAIN, check=False)
if not no_push:
    print(git(MAIN, 'push', 'origin', 'main'))
print('done:', merged)
