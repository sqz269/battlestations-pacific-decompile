import json
import tempfile
import unittest
from pathlib import Path
from unittest.mock import Mock

from tools.ghidra_export import Client, ROOT, decompile, pe_summary, write


class ExporterTests(unittest.TestCase):
    def setUp(self):
        self.config = json.loads((ROOT / 'config/target.json').read_text())

    def test_rejects_other_project_with_same_program_name(self):
        client = Client(self.config)
        client.get = Mock(side_effect=[{'project_name': 'wows'},
            {'path': '/battlestationspacific.exe', 'language': self.config['language'],
             'image_base': self.config['image_base']}])
        with self.assertRaisesRegex(RuntimeError, 'Wrong Ghidra project'):
            client.verify()

    def test_rejects_wrong_program_path(self):
        client = Client(self.config)
        client.get = Mock(side_effect=[{'project_name': 'bsp'},
            {'path': '/BattlestationsPacific/battlestationspacific.exe',
             'language': self.config['language'], 'image_base': self.config['image_base']}])
        with self.assertRaisesRegex(RuntimeError, 'Wrong target program_path'):
            client.verify()

    def test_partial_export_is_retried_then_complete_export_is_skipped(self):
        client = Mock(config=self.config)
        client.get.side_effect = ['void f(void) { return; }', '00401130: RET']
        with tempfile.TemporaryDirectory() as temp:
            output = Path(temp)
            write(output / 'functions.json', [{'address': '00401130', 'name': 'f'}])
            write(output / 'functions/00401130/decompiled.c', 'partial')
            decompile(client, output, ['00401130'])
            self.assertEqual(client.get.call_count, 2)
            self.assertTrue((output / 'functions/00401130/metadata.json').exists())
            decompile(client, output, ['00401130'])
            self.assertEqual(client.get.call_count, 2)

    def test_rejects_non_pe_input(self):
        with tempfile.TemporaryDirectory() as temp:
            path = Path(temp) / 'bad.exe'
            path.write_bytes(b'not executable')
            with self.assertRaisesRegex(ValueError, 'Not a DOS/PE'):
                pe_summary(path)


if __name__ == '__main__':
    unittest.main()
