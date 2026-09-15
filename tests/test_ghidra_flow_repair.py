import importlib.util
from pathlib import Path
import unittest

path = Path(__file__).resolve().parents[1] / 'tools' / 'ghidra_flow_repair.py'
spec = importlib.util.spec_from_file_location('ghidra_flow_repair', path)
flow = importlib.util.module_from_spec(spec)
spec.loader.exec_module(flow)


class ExplicitCatchTailTest(unittest.TestCase):
    def test_rethrow_requires_exact_exit_and_explicit_mode(self):
        with self.assertRaisesRegex(SystemExit, 'requires --tail-end'):
            flow.main(['00444252', '--tail-rethrow'])
        rows = [(0x1000, 2, 'push', '0'), (0x1002, 2, 'push', '0'),
                (0x1004, 5, 'call', '0xbf6885')]
        self.assertEqual(flow.tail_terminator(rows, 0x1009, True), 'cxx_rethrow_00bf6885')
        for candidate, end, enabled in [
            (rows, 0x1009, False), (rows, 0x1008, True),
            (rows[:2] + [(0x1004, 5, 'call', '0xbf65ac')], 0x1009, True),
            ([(0x1000, 2, 'push', '1')] + rows[1:], 0x1009, True),
        ]:
            with self.assertRaises(ValueError):
                flow.tail_terminator(candidate, end, enabled)
        self.assertEqual(flow.tail_terminator([(0x1000, 1, 'ret', '')], 0x1001), 'ret')


if __name__ == '__main__':
    unittest.main()
