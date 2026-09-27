"""`bsp.py sync` fast-forwards only a branch whose tip is on origin/main, and refuses otherwise.

The refusal is the load-bearing case: four workers on 2026-09-27 read a refused `--ff-only` as
"main moved" when their own unlanded commit was the cause. Throwaway repos only.
"""
import contextlib
import io
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import branch_sync


def git(cwd, *args):
    return subprocess.run(['git', *args], cwd=str(cwd), check=True, capture_output=True, text=True).stdout.strip()


def commit(cwd, name):
    (Path(cwd) / name).write_text(name, encoding='utf-8')
    git(cwd, 'add', name)
    git(cwd, '-c', 'user.name=t', '-c', 'user.email=t@t', 'commit', '-q', '-m', name)
    return git(cwd, 'rev-parse', 'HEAD')


class SyncAncestorDecision(unittest.TestCase):
    def test_fast_forward_then_refuse_unlanded(self):
        tmp = Path(tempfile.mkdtemp())
        git(tmp, 'init', '-q', '--bare', '-b', 'main', 'origin.git')
        git(tmp, 'clone', '-q', 'origin.git', 'lead')
        lead = tmp / 'lead'
        git(lead, 'checkout', '-q', '-b', 'main')
        commit(lead, 'a')
        git(lead, 'push', '-q', 'origin', 'main')
        git(tmp, 'clone', '-q', 'origin.git', 'worker')
        worker = tmp / 'worker'
        git(worker, 'checkout', '-q', '-b', 'agent/w')

        landed = commit(lead, 'b')                       # main moves; the worker has nothing unlanded
        git(lead, 'push', '-q', 'origin', 'main')
        with contextlib.redirect_stdout(io.StringIO()):
            self.assertEqual(branch_sync.sync(worker), 0)
        self.assertEqual(git(worker, 'rev-parse', 'HEAD'), landed)

        own = commit(worker, 'c')                        # the worker's own commit, not on main
        commit(lead, 'd')
        git(lead, 'push', '-q', 'origin', 'main')
        out = io.StringIO()
        with contextlib.redirect_stdout(out):
            self.assertEqual(branch_sync.sync(worker), 1)
        self.assertEqual(git(worker, 'rev-parse', 'HEAD'), own)   # nothing merged
        self.assertIn('unlanded: report to the lead', out.getvalue())
        self.assertIn(own[:7], out.getvalue())
        self.assertTrue(branch_sync.tip_line(worker).startswith('1 commits not on main'))


if __name__ == '__main__':
    unittest.main()
