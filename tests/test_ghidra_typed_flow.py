"""Focused typed-flow format and capture guards; no Ghidra/network execution."""
import base64
from copy import deepcopy
import hashlib
import io
import json
from pathlib import Path
import sys
from tempfile import TemporaryDirectory
from types import SimpleNamespace
import unittest
from unittest import mock
from urllib.parse import parse_qs, urlparse

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import ghidra_typed_flow as flow

CONFIG = {'project': 'bsp', 'program': 'battlestationspacific.exe',
          'program_path': '/battlestationspacific.exe', 'language': 'x86:LE:32:default',
          'image_base': '00400000', 'project_file': 'C:/Users/sqz269/bsp.gpr',
          'ghidra_url': 'http://127.0.0.1:8089'}
IDENTITY = [{'project_name': 'bsp'},
            {'path': CONFIG['program_path'], 'language': CONFIG['language'],
             'image_base': CONFIG['image_base'], 'address_spaces': [{'name': 'ram', 'is_default': True}]}]


def addr(value):
    return {'space': 'ram', 'offset': f'{value:08x}'}


def unit(value, length=5, kind='instruction'):
    return {'address': addr(value), 'length': length, 'java_class': 'fixture.' + kind,
            'kind': kind, 'defined': False if kind == 'data' else None}


def flags(value, thunk=False, target=None):
    result = {'entry': addr(value), 'name': 'fixture', 'no_return': False,
              'is_thunk': thunk, 'thunk_target_recursive': False, 'direct_thunk_target': target}
    return result


def specimen(gap=False, modification=17):
    function = flags(0x401000)
    function['body'] = {'complete': True, 'range_count': 2, 'returned_range_count': 2,
                        'address_count': '9', 'ranges': [
                            {'start': addr(0x401000), 'end_inclusive': addr(0x401004), 'length': '5'},
                            {'start': addr(0x401008), 'end_inclusive': addr(0x40100b), 'length': '4'}]}
    default = {'name': 'UNCONDITIONAL_CALL', 'is_call': True, 'is_jump': False,
               'is_conditional': False, 'is_computed': False, 'is_terminal': False}
    effective = dict(default, name='CALL_TERMINATOR', is_terminal=True)
    target = {'entry': addr(0x403000), 'name': 'target', 'no_return': True, 'is_thunk': False}
    listing = {key: unit(0x401000) for key in
               ('instruction_at', 'instruction_containing', 'code_unit_at', 'code_unit_containing')}
    result = {'schema': 1, 'complete': True,
              'program': {'path': CONFIG['program_path'], 'language': CONFIG['language'],
                          'image_base': addr(0x400000), 'default_address_space': 'ram',
                          'project': {'name': 'bsp', 'location': 'C:\\Users\\sqz269',
                                      'marker_file': 'C:\\Users\\sqz269\\bsp.gpr'}},
              'query_address': addr(0x401000), 'modification_number_before': str(modification),
              'modification_number_after': str(modification), 'listing': listing,
              'instruction_flow': {'flow_override': 'CALL_RETURN', 'default_flow': default,
                                   'effective_flow': effective, 'default_fallthrough': addr(0x401005),
                                   'effective_fallthrough': None, 'fallthrough_overridden': False,
                                   'default_targets': [addr(0x402000)], 'effective_targets': [addr(0x402000)]},
              'function_at_query': addr(0x401000), 'function_containing_query': addr(0x401000),
              'functions': [function], 'direct_call_targets': [
                  {'address': addr(0x402000), 'default_target': True, 'effective_target': True,
                   'function_at_target': flags(0x402000, True, target)}]}
    if gap:
        result.update(query_address=addr(0x401005), instruction_flow=None,
                      function_at_query=None, function_containing_query=None,
                      functions=[], direct_call_targets=[])
        result['listing'] = {'instruction_at': None, 'instruction_containing': None,
                             'code_unit_at': unit(0x401005, 1, 'data'),
                             'code_unit_containing': unit(0x401005, 1, 'data')}
    return result


def raw_response(payload, requested='00401000', status=200):
    raw = json.dumps(payload).encode()
    return {'address': requested, 'url': 'fixture', 'http_status': status, 'transport_error': None,
            'raw_response': raw.decode(), 'raw_response_base64': base64.b64encode(raw).decode(),
            'raw_sha256': hashlib.sha256(raw).hexdigest()}


class TypedFlowContractTest(unittest.TestCase):
    def test_discontiguous_body_gap_and_independent_flow_thunk_flags(self):
        call = specimen()
        self.assertEqual(flow.validate_response(call, CONFIG, 'ram:00401000', 'ram', 4096), 17)
        self.assertEqual(flow.validate_response(specimen(True), CONFIG, '00401005', 'ram', 4096), 17)
        self.assertFalse(call['direct_call_targets'][0]['function_at_target']['no_return'])
        self.assertTrue(call['direct_call_targets'][0]['function_at_target']['direct_thunk_target']['no_return'])
        call['program']['project']['location'] = '/C:/Users/sqz269/'
        self.assertEqual(flow.validate_response(call, CONFIG, '00401000', 'ram', 4096), 17)

    def test_rejects_incomplete_wrong_identity_and_inconsistent_records(self):
        invalid = []
        body = specimen(); body['functions'][0]['body']['address_count'] = '12'; invalid.append(body)
        body = specimen(); body['functions'][0]['body']['complete'] = False; invalid.append(body)
        body = specimen(); body['functions'][0]['body']['ranges'][1]['start'] = addr(0x401004); invalid.append(body)
        gap = specimen(True); del gap['listing']['instruction_at']; invalid.append(gap)
        gap = specimen(True); gap['instruction_flow'] = specimen()['instruction_flow']; invalid.append(gap)
        project = specimen(); project['program']['project']['marker_file'] = 'C:/other/bsp.gpr'; invalid.append(project)
        project = specimen(); project['program']['project']['marker_file'] = '/C:/Users/sqz269/bsp.gpr'; invalid.append(project)
        for location in ('/D:/Users/sqz269/', '/C:/other/', '/Users/sqz269/',
                         'C:Users/sqz269/', 'file:///C:/Users/sqz269/'):
            project = specimen(); project['program']['project']['location'] = location; invalid.append(project)
        modified = specimen(); modified['modification_number_after'] = '18'; invalid.append(modified)
        schema = specimen(); schema['schema'] = 2; invalid.append(schema)
        query = specimen(); query['query_address'] = addr(0x401001); invalid.append(query)
        listing = specimen(); listing['listing']['instruction_at'] = None; listing['listing']['instruction_containing'] = None; listing['instruction_flow'] = None; invalid.append(listing)
        function = specimen(); function['function_at_query'] = None; invalid.append(function)
        target = specimen(); target['direct_call_targets'][0]['function_at_target']['entry'] = addr(0x402001); invalid.append(target)
        for index, payload in enumerate(invalid):
            with self.subTest(case=index), self.assertRaises(flow.InvalidMetadata):
                flow.validate_response(payload, CONFIG, '00401005' if payload.get('query_address') == addr(0x401005) else '00401000', 'ram', 4096)
        with self.assertRaises(flow.InvalidMetadata):
            flow.validate_response(specimen(), CONFIG, '00401000', 'ram', 1)

    def test_raw_response_is_persisted_before_validation_and_failure_is_retained(self):
        client = mock.Mock(); client.verify.return_value = deepcopy(IDENTITY)
        with TemporaryDirectory() as directory:
            output = Path(directory) / 'capture.json'
            real_validate = flow.validate_response
            def validate_after_persistence(*args):
                saved = json.loads(output.read_text())
                self.assertEqual(saved['validation']['state'], 'pending')
                self.assertEqual(saved['responses'][0]['raw_sha256'], raw_response(specimen())['raw_sha256'])
                return real_validate(*args)
            with mock.patch.object(flow, 'validate_response', side_effect=validate_after_persistence):
                record = flow.capture(CONFIG, ['00401000'], output,
                                      client_factory=lambda _: client, fetch=lambda *_: raw_response(specimen()))
            self.assertTrue(record['validation']['accepted'])
            self.assertEqual(client.verify.call_count, 2)
            client.reset_mock()
            failed = raw_response({'error': 'endpoint unavailable'}, status=404)
            record = flow.capture(CONFIG, ['00401000'], output, client_factory=lambda _: client, fetch=lambda *_: failed)
            self.assertFalse(record['validation']['accepted'])
            self.assertEqual(json.loads(output.read_text())['responses'][0], failed)
            self.assertEqual(client.verify.call_count, 2)

    def test_changed_cross_batch_modification_is_rejected(self):
        client = mock.Mock(); client.verify.return_value = deepcopy(IDENTITY)
        with TemporaryDirectory() as directory:
            responses = iter([raw_response(specimen()), raw_response(specimen(True, 18), '00401005')])
            record = flow.capture(CONFIG, ['00401000', '00401005'], Path(directory) / 'capture.json',
                                  client_factory=lambda _: client, fetch=lambda *_: next(responses))
            self.assertFalse(record['validation']['accepted'])
            self.assertIn('changed across metadata batch', record['validation']['errors'][0])
            self.assertEqual(len(record['responses']), 2)

    def test_transport_and_bsp_dispatch_have_no_script_or_autostart_path(self):
        requests = []
        class Reply(io.BytesIO):
            status = 200
        def transport(request, **kwargs):
            requests.append(request)
            self.assertEqual(request.get_method(), 'GET')
            url = urlparse(request.full_url)
            self.assertEqual(url.path, '/get_flow_metadata')
            self.assertEqual(parse_qs(url.query)['program'], ['battlestationspacific.exe'])
            return Reply(json.dumps(specimen()).encode())
        with mock.patch.object(flow, 'urlopen', side_effect=transport):
            response = flow.fetch_raw(CONFIG, '00401000', 4096)
        self.assertEqual(response['http_status'], 200)
        self.assertEqual(len(requests), 1)
        import bsp
        arguments = SimpleNamespace(ghidra_command='typed-flow', addresses=['00401000'],
                                    output='local/fixture.json', max_ranges=4096, config=None)
        with mock.patch.object(bsp, 'client', side_effect=AssertionError('autostart-capable client used')), \
             mock.patch.object(flow, 'main', return_value=0) as run, \
             self.assertRaises(SystemExit) as exit_status:
            bsp.ghidra_cmd(arguments)
        self.assertEqual(exit_status.exception.code, 0)
        run.assert_called_once_with(['00401000', '--output', 'local/fixture.json', '--max-ranges', '4096'])


if __name__ == '__main__':
    unittest.main()
