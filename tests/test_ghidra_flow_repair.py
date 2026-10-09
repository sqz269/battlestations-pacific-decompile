from contextlib import redirect_stdout
import importlib.util
import io
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest import mock
from urllib.parse import urlparse

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

    def test_report_and_unsupported_apply_never_reach_mutators(self):
        requests = []
        responses = {
            'list_project_files': {'project_name': 'bsp'},
            'get_current_program_info': {'path': '/battlestationspacific.exe',
                                         'language': 'x86:LE:32:default', 'image_base': '00400000'},
            'disassemble_function': '00401000: CALL 0x00bf65ac\n00401008: RET\n',
        }

        def read_only_transport(request, **kwargs):
            url = request if isinstance(request, str) else request.full_url
            method = 'GET' if isinstance(request, str) else request.get_method()
            endpoint = urlparse(url).path.rsplit('/', 1)[-1]
            requests.append((method, endpoint))
            self.assertEqual(method, 'GET', 'report attempted an HTTP write')
            self.assertIn(endpoint, responses, 'report attempted a non-read endpoint')
            return io.BytesIO(json.dumps(responses[endpoint]).encode())

        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'config').mkdir()
            (root / 'config/target.json').write_text(json.dumps({
                'ghidra_url': 'http://127.0.0.1:1', 'program': 'battlestationspacific.exe',
                'project': 'bsp', 'program_path': '/battlestationspacific.exe',
                'language': 'x86:LE:32:default', 'image_base': '00400000',
            }))
            record = root / 'prior_repairs.json'
            prior_record = b'{"repairs":[{"unchanged":true}]}\n'
            record.write_bytes(prior_record)
            # Real Client.verify/get operate through a strict read-only fake transport.
            # Traps also cover the removed direct POST path and unmocked network use.
            with mock.patch.object(flow, 'ROOT', root), \
                 mock.patch.object(flow, 'Client', wraps=flow.Client) as factory, \
                 mock.patch.object(flow, 'load_pe', return_value=(None, 0x400000, 0x401000,
                                   bytes.fromhex('e8a7557f0083c404c3'))) as load_pe, \
                 mock.patch('ghidra_export.urlopen', side_effect=read_only_transport), \
                 mock.patch.object(flow, 'urlopen', create=True,
                                   side_effect=AssertionError('legacy mutator reached')) as legacy, \
                 mock.patch('urllib.request.urlopen',
                            side_effect=AssertionError('unmocked network reached')) as network:
                for flags in ([], ['--dry-run', '--tail-end', '00401009']):
                    requests.clear()
                    output = io.StringIO()
                    with redirect_stdout(output):
                        flow.main(['00401000', '--record', str(record), *flags])
                    self.assertEqual(requests, [('GET', name) for name in responses])
                    self.assertIn('00401005..00401008 (3 bytes)', output.getvalue())
                    self.assertIn('Conditional plan only', output.getvalue())
                    self.assertEqual(record.read_bytes(), prior_record)
                for flags in (['--apply'], ['--apply', '--dry-run', '--tail-end', '00401009']):
                    requests.clear()
                    factory.reset_mock()
                    load_pe.reset_mock()
                    with self.assertRaisesRegex(SystemExit, 'attested bounded/atomic'):
                        flow.main(['00401000', '--record', str(record), *flags])
                    self.assertEqual(requests, [])
                    factory.assert_not_called()
                    load_pe.assert_not_called()
                    self.assertEqual(record.read_bytes(), prior_record)
                legacy.assert_not_called()
                network.assert_not_called()


if __name__ == '__main__':
    unittest.main()
