"""Where shared, ignored workspace data lives when several git worktrees share one repository.

Exports (snapshot, call graph, per-function pseudocode) are produced once from the Ghidra
project and are the same for every worktree, so they live in the MAIN checkout's exports/bsp.
A worktree resolves that directory through git (`rev-parse --git-common-dir`) instead of a
junction or symlink: reparse points inside a worktree are traversed by `git worktree remove`
and `git clean`, which once emptied the shared directory. $BSP_EXPORTS_DIR overrides.
"""
import os
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def main_root():
    """Root of the main checkout (the worktree that owns .git); ROOT itself when not a linked worktree."""
    try:
        common = subprocess.run(['git', 'rev-parse', '--git-common-dir'], cwd=ROOT,
                                capture_output=True, text=True, check=True).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return ROOT
    common_path = Path(common)
    if not common_path.is_absolute():
        common_path = (ROOT / common_path).resolve()
    return common_path.parent if common_path.name == '.git' else ROOT


def exports_dir():
    env = os.environ.get('BSP_EXPORTS_DIR')
    if env:
        return Path(env)
    return main_root() / 'exports' / 'bsp'
