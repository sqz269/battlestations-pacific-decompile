"""The four cases merge_resolve must refuse rather than guess at.

Each one was a live bug that silently produced wrong content during the September 2026 integration
of the stranded orchestrator branches, and each was found by reading the tool's output rather than
by a test. The value of this tool is that a refusal is honest, so these are the load-bearing cases.

Nothing here touches the repository or ~/.bsp: the merge-state case builds a throwaway git repo and
the rest drive the handlers with literal stage text.
"""
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from tools import merge_resolve


def record(address, name='f', kind='function', **extra):
    return json.dumps({'address': address, 'name': name, 'kind': kind, **extra}) + '\n'


class MergeResolveRefusals(unittest.TestCase):
    def resolve(self, path, base, ours, theirs):
        """Run one handler with literal stage contents in a scratch directory."""
        directory = Path(tempfile.mkdtemp())
        (directory / path).parent.mkdir(parents=True, exist_ok=True)
        original = merge_resolve.stage_text
        merge_resolve.stage_text = lambda w, p, stage: {1: base, 2: ours, 3: theirs}[stage]
        try:
            how = merge_resolve.resolve(directory, path)
        finally:
            merge_resolve.stage_text = original
        text = (directory / path).read_text(encoding='utf-8') if how else None
        return how, text

    def test_reconstruction_shard_refuses_competing_records_for_one_address(self):
        """Two sides reconstructing the same new function is a judgement, not a union.

        An address-keyed union would also drop records: a shard legitimately holds a fragment beside
        a function at one address, which is why the handler is line-level.
        """
        base = record('00b50000')
        shard = 'config/reconstruction/00b50000.jsonl'
        how, _ = self.resolve(shard, base, base + record('00b50020', 'ours'),
                              base + record('00b50020', 'theirs'))
        self.assertIsNone(how, 'competing reconstructions of one new address must not merge')

        # the same address with a fragment alongside a function is normal and must survive
        pair = record('00b50000', 'f', 'function') + record('00b50000', 'g', 'fragment')
        how, text = self.resolve(shard, pair, pair + record('00b50020'), pair + record('00b50030'))
        self.assertIsNotNone(how)
        self.assertEqual(len([l for l in text.splitlines() if l.strip()]), 4)

    def test_reconstruction_shard_refuses_competing_records_while_one_side_rewrites(self):
        """The same collision on the asymmetric path, which is where it actually happened.

        When one side rewrites base records the handler keeps that side whole and replays the other's
        appends. The first version of that shortcut never compared the two sides' NEW addresses, so
        two independent reconstructions of one function would have merged silently.
        """
        base = record('00b50000') + record('00b50010')
        shard = 'config/reconstruction/00b50000.jsonl'
        ours = record('00b50000', status='revised') + record('00b50010') + record('00b50099', 'main_version')
        theirs = base + record('00b50099', 'branch_version')
        how, _ = self.resolve(shard, base, ours, theirs)
        self.assertIsNone(how, 'a one-sided rewrite must not smuggle through a competing new record')

        # the same shape without the collision still resolves, keeping the rewrite and the append
        how, text = self.resolve(shard, base, record('00b50000', status='revised') + record('00b50010'),
                                 base + record('00b50020'))
        self.assertIsNotNone(how)
        self.assertEqual(len([l for l in text.splitlines() if l.strip()]), 3)

    def test_reconstruction_shard_refuses_when_both_sides_rewrote_the_base(self):
        base = record('00b50000')
        shard = 'config/reconstruction/00b50000.jsonl'
        how, _ = self.resolve(shard, base, record('00b50000', status='ours'),
                              record('00b50000', status='theirs'))
        self.assertIsNone(how, 'two in-place rewrites of one record need the evidence to settle')

    def test_names_shard_refuses_when_both_sides_edited_one_address(self):
        """The old behaviour kept the incoming record and discarded ours without a word."""
        base = json.dumps({'address': '00401000', 'name': 'A', 'evidence': 'base'}) + '\n'
        ours = json.dumps({'address': '00401000', 'name': 'A', 'evidence': 'ours'}) + '\n'
        theirs = json.dumps({'address': '00401000', 'name': 'A', 'evidence': 'theirs'}) + '\n'
        how, _ = self.resolve('config/names/00400000.jsonl', base, ours, theirs)
        self.assertIsNone(how, 'both sides re-evidencing one address must not silently pick one')

    def test_add_add_refuses_instead_of_concatenating_two_files(self):
        """With no base every conflict block looks like an insertion, so the old rule glued both
        whole files together: duplicate C++ definitions, or two JSON documents in one report."""
        how, _ = self.resolve('src/new.cpp', '', 'void f_00401000() {}\n', 'void g_00401000() {}\n')
        self.assertIsNone(how, 'a file new on both sides is a choice, not an insertion')

        # a genuine same-spot insertion still resolves, keeping both sides
        how, text = self.resolve('tests/t.cpp', 'a\nb\nEND\n', 'a\nb\nOURS\nEND\n', 'a\nb\nTHEIRS\nEND\n')
        self.assertIsNotNone(how)
        self.assertIn('OURS', text)
        self.assertIn('THEIRS', text)

    def test_resolve_all_refuses_outside_a_merge(self):
        """Stage 1 is the merge base only during a merge; in a cherry-pick it is the pick's parent,
        which made the startup.cmake union mis-identify which side had changed a target."""
        directory = Path(tempfile.mkdtemp())

        def git(*args):
            return subprocess.run(['git', *args], cwd=directory, capture_output=True, text=True)

        shard = directory / 'config/names/00400000.jsonl'
        shard.parent.mkdir(parents=True)
        git('init', '-q', '-b', 'main')
        git('config', 'user.email', 'test@example.invalid')
        git('config', 'user.name', 'test')
        shard.write_text(json.dumps({'address': '00400000', 'name': 'A'}) + '\n', encoding='utf-8', newline='\n')
        git('add', '-A'), git('commit', '-qm', 'base')

        # clean tree, nothing in progress
        self.assertEqual(merge_resolve.resolve_all(directory), ([], ['<not a merge>']))

        git('checkout', '-qb', 'side')
        shard.write_text(json.dumps({'address': '00400000', 'name': 'A'}) + '\n'
                         + json.dumps({'address': '00400010', 'name': 'S'}) + '\n', encoding='utf-8', newline='\n')
        git('add', '-A'), git('commit', '-qm', 'side')
        git('checkout', '-q', 'main')
        shard.write_text(json.dumps({'address': '00400000', 'name': 'A'}) + '\n'
                         + json.dumps({'address': '00400020', 'name': 'M'}) + '\n', encoding='utf-8', newline='\n')
        git('add', '-A'), git('commit', '-qm', 'main')

        git('cherry-pick', 'side')
        if (directory / '.git/CHERRY_PICK_HEAD').exists():
            self.assertEqual(merge_resolve.resolve_all(directory), ([], ['<not a merge>']),
                             'a cherry-pick must be refused: stage 1 is not the merge base')
        git('cherry-pick', '--abort')

        git('merge', 'side')
        conflicted, unresolved = merge_resolve.resolve_all(directory)
        self.assertEqual(unresolved, [], 'a real merge must still resolve')
        self.assertEqual(len([l for l in shard.read_text(encoding='utf-8').splitlines() if l.strip()]), 3)


if __name__ == '__main__':
    unittest.main()
