"""Check and rebuild the shared exports (main checkout's exports/bsp) after a loss.

The exports are regenerated from the Ghidra project, so a wipe is recoverable but slow:
  1. `ghidra_export.py snapshot` when functions.json is gone (a few minutes);
  2. `bsp.py index` so the local index sees the snapshot;
  3. re-decompile every non-thunk function that has a ledger name, a reconstruction record or a
     tag (about 40k on 2026-10-13), in chunks, skipping ones already on disk. Rerunning resumes.
Ghidra must be reachable (`bsp.py ghidra ensure`); the re-export takes hours, so run it in the
background and leave other index rebuilds alone while it holds the index open.
"""
import sqlite3
import subprocess
import sys
from pathlib import Path

import workspace

ROOT = Path(__file__).resolve().parents[1]
DB = ROOT / 'local/bsp_index.sqlite'
CHUNK = 200


def on_disk():
    functions = workspace.exports_dir() / 'functions'
    if not functions.is_dir():
        return set()
    return {p.name.lower() for p in functions.iterdir() if (p / 'decompiled.c').exists()}


def wanted():
    """Hex addresses of every non-thunk function the ledgers or tags know about."""
    if not DB.exists():
        return []
    db = sqlite3.connect(f'file:{DB.as_posix()}?mode=ro', uri=True)
    try:
        rows = db.execute("""SELECT hex FROM functions WHERE (thunk IS NULL OR thunk = 0) AND (
            (ledger_name IS NOT NULL AND ledger_name != '') OR (recon_kind IS NOT NULL AND recon_kind != '')
            OR (tag_category IS NOT NULL AND tag_category != '')) ORDER BY address""").fetchall()
    finally:
        db.close()  # an open handle blocks other index rebuilds on Windows
    return [h.lower() for (h,) in rows]


def check():
    exports = workspace.exports_dir()
    snapshot = (exports / 'functions.json').exists()
    disk = on_disk()
    want = wanted()
    missing = [a for a in want if a not in disk]
    print(f'exports {exports}: snapshot {"present" if snapshot else "MISSING"}, {len(disk)} decompiled on disk, '
          f'{len(want)} wanted by the ledgers/tags, {len(missing)} of those missing')
    if not snapshot or (want and len(missing) > len(want) // 10):
        print('looks damaged: run `python tools/bsp.py exports restore` (hours; run it in the background)')
        return 1
    return 0


def restore():
    exports = workspace.exports_dir()
    if not (exports / 'functions.json').exists():
        print('snapshot missing: running ghidra_export.py snapshot', flush=True)
        run = subprocess.run([sys.executable, str(ROOT / 'tools/ghidra_export.py'), 'snapshot'], cwd=ROOT)
        if run.returncode:
            return run.returncode
    run = subprocess.run([sys.executable, str(ROOT / 'tools/bsp.py'), 'index'], cwd=ROOT)
    if run.returncode:
        return run.returncode
    disk = on_disk()
    todo = [a for a in wanted() if a not in disk]
    print(f'{len(todo)} function(s) to re-export', flush=True)
    failures = 0
    for i in range(0, len(todo), CHUNK):
        part = todo[i:i + CHUNK]
        run = subprocess.run([sys.executable, str(ROOT / 'tools/ghidra_export.py'), 'decompile', '--addresses', *part],
                             cwd=ROOT, capture_output=True, text=True)
        failures += bool(run.returncode)
        print(f'{i + len(part)}/{len(todo)} rc={run.returncode} {run.stderr.strip()[-200:]}', flush=True)
    subprocess.run([sys.executable, str(ROOT / 'tools/bsp.py'), 'index'], cwd=ROOT)
    return 1 if failures else 0
