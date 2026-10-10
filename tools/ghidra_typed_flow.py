"""Capture exact typed flow metadata without script or mutation fallbacks.

Requires the separately reviewed /get_flow_metadata plugin endpoint. Every raw
HTTP response and status is saved before validation. No endpoint availability
probe, autostart, script opt-in, analysis or repair is performed here.
"""
import argparse
import base64
from datetime import datetime, timezone
import hashlib
import json
import ntpath
from pathlib import Path
import re
from urllib.error import HTTPError, URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen

from ghidra_export import Client

ROOT = Path(__file__).resolve().parents[1]
ENDPOINT = 'get_flow_metadata'
MAX_ADDRESSES = 64
MAX_RANGES = 4096


class InvalidMetadata(ValueError):
    pass


def require(condition, message):
    if not condition:
        raise InvalidMetadata(message)


def query_address(value):
    match = re.fullmatch(r'(?:(?P<space>[A-Za-z0-9_.-]+):)?(?:0[xX])?(?P<hex>[0-9a-fA-F]{1,8})', value)
    if not match:
        raise ValueError('Expected a 32-bit hexadecimal address, optionally space:address')
    return match['space'], int(match['hex'], 16)


def address(value, label):
    require(isinstance(value, dict), label + ': expected address object')
    space, offset = value.get('space'), value.get('offset')
    require(isinstance(space, str) and re.fullmatch(r'[A-Za-z0-9_.-]+', space), label + ': invalid address space')
    require(isinstance(offset, str) and re.fullmatch(r'[0-9a-fA-F]{1,8}', offset), label + ': invalid 32-bit offset')
    return space, int(offset, 16)


def decimal(value, label, minimum=0):
    require(isinstance(value, str) and re.fullmatch(r'-?\d+', value), label + ': expected decimal string')
    result = int(value)
    require(minimum <= result <= (1 << 63) - 1, label + ': out of range')
    return result


def integer(value, label, minimum=0, maximum=MAX_RANGES):
    require(type(value) is int and minimum <= value <= maximum, label + ': invalid integer')
    return value


def boolean(value, label):
    require(type(value) is bool, label + ': expected boolean')


def windows_path(value):
    require(isinstance(value, str) and ntpath.isabs(value), 'Expected absolute Windows project path')
    return ntpath.normcase(ntpath.normpath(value))


def validate_identity(verified, config):
    require(isinstance(verified, (tuple, list)) and len(verified) == 2, 'Missing Client.verify identity')
    project, program = verified
    require(project.get('project_name') == config['project'], 'Wrong verified project name')
    for key, expected in [('path', config['program_path']), ('language', config['language']), ('image_base', config['image_base'])]:
        require(program.get(key) == expected, 'Wrong verified ' + key)
    defaults = [item['name'] for item in program.get('address_spaces', []) if item.get('is_default') is True]
    require(len(defaults) == 1, 'Verified identity has no unique default address space')
    return defaults[0]


def validate_function_flags(value, label, with_target=True):
    require(isinstance(value, dict), label + ': missing function flags')
    entry = address(value.get('entry'), label + '.entry')
    require(isinstance(value.get('name'), str) and value['name'], label + ': missing function name')
    boolean(value.get('no_return'), label + '.no_return')
    boolean(value.get('is_thunk'), label + '.is_thunk')
    if with_target:
        require(value.get('thunk_target_recursive') is False, label + ': thunk query must be direct')
        require('direct_thunk_target' in value, label + ': missing explicit thunk target')
        target = value['direct_thunk_target']
        if target is not None:
            require(value['is_thunk'], label + ': non-thunk has target')
            validate_function_flags(target, label + '.direct_thunk_target', False)
    return entry


def validate_body(value, label, max_ranges):
    require(isinstance(value, dict) and value.get('complete') is True, label + ': incomplete body')
    ranges = value.get('ranges')
    require(isinstance(ranges, list), label + ': missing exact ranges')
    count = integer(value.get('range_count'), label + '.range_count', maximum=max_ranges)
    returned = integer(value.get('returned_range_count'), label + '.returned_range_count', maximum=max_ranges)
    require(count == returned == len(ranges), label + ': range counts do not match')
    previous, total, decoded = {}, 0, []
    for index, item in enumerate(ranges):
        require(isinstance(item, dict), label + ': invalid range')
        start = address(item.get('start'), f'{label}[{index}].start')
        end = address(item.get('end_inclusive'), f'{label}[{index}].end')
        require(start[0] == end[0] and start[1] <= end[1], label + ': invalid range bounds')
        require(start[0] not in previous or start[1] > previous[start[0]], label + ': overlapping or unordered ranges')
        length = decimal(item.get('length'), label + '.length', 1)
        require(length == end[1] - start[1] + 1, label + ': range length mismatch')
        previous[start[0]] = end[1]
        total += length
        decoded.append((start[0], start[1], end[1]))
    require(decimal(value.get('address_count'), label + '.address_count') == total, label + ': body address count mismatch')
    return decoded


def contains(ranges, query):
    return any(space == query[0] and low <= query[1] <= high for space, low, high in ranges)


def validate_unit(unit, label, query, exact, instruction=False):
    if unit is None:
        return None
    require(isinstance(unit, dict), label + ': invalid code unit')
    start = address(unit.get('address'), label + '.address')
    length = integer(unit.get('length'), label + '.length', 1, 0xffffffff)
    require(start[0] == query[0] and start[1] <= query[1] < start[1] + length, label + ': does not contain query')
    if exact:
        require(start == query, label + ': is not an exact start')
    require(isinstance(unit.get('java_class'), str) and unit['java_class'], label + ': missing class')
    require(unit.get('kind') in ('instruction', 'data', 'other'), label + ': invalid kind')
    require('defined' in unit, label + ': missing explicit defined value')
    if unit['kind'] == 'data':
        boolean(unit['defined'], label + '.defined')
    else:
        require(unit['defined'] is None, label + ': non-data defined value must be null')
    if instruction:
        require(unit['kind'] == 'instruction', label + ': expected instruction')
    return start, length, unit['kind']


def validate_response(payload, config, requested, default_space, max_ranges):
    require(isinstance(payload, dict), 'Response is not a JSON object')
    require(not payload.get('error') and payload.get('success') is not False, 'Endpoint returned an error')
    require(type(payload.get('schema')) is int and payload['schema'] == 1, 'Wrong metadata schema')
    require(payload.get('complete') is True, 'Incomplete metadata response')
    program = payload.get('program')
    require(isinstance(program, dict), 'Missing response program identity')
    require(program.get('path') == config['program_path'] and program.get('language') == config['language'], 'Wrong response program identity')
    require(program.get('default_address_space') == default_space, 'Wrong default address space')
    require(address(program.get('image_base'), 'image_base') == (default_space, int(config['image_base'], 16)), 'Wrong response image base')
    project = program.get('project')
    require(isinstance(project, dict) and project.get('name') == config['project'], 'Missing or wrong runtime project locator')
    marker = windows_path(project.get('marker_file'))
    expected_marker = windows_path(config['project_file'])
    require(marker == expected_marker, 'Wrong absolute runtime GPR marker path')
    require(windows_path(project.get('location')) == ntpath.dirname(expected_marker), 'Wrong runtime project location')
    expected_space, expected_offset = query_address(requested)
    query = address(payload.get('query_address'), 'query_address')
    require(query == (expected_space or default_space, expected_offset), 'Response query address mismatch')
    before = decimal(payload.get('modification_number_before'), 'modification_number_before', -(1 << 63))
    after = decimal(payload.get('modification_number_after'), 'modification_number_after', -(1 << 63))
    require(before == after, 'Program changed during metadata read')

    listing = payload.get('listing')
    require(isinstance(listing, dict), 'Missing listing record')
    units = {}
    for key in ('instruction_at', 'instruction_containing', 'code_unit_at', 'code_unit_containing'):
        require(key in listing, 'Missing explicit listing null state: ' + key)
        units[key] = validate_unit(listing[key], key, query, key.endswith('_at'), key.startswith('instruction'))
    if units['instruction_at'] is not None:
        require(units['instruction_containing'] == units['instruction_at'], 'Exact instruction differs from containing instruction')
        require(units['code_unit_at'] == units['instruction_at'] and units['code_unit_containing'] == units['instruction_at'], 'Instruction/code-unit observations differ')
    elif units['instruction_containing'] is not None:
        require(units['instruction_containing'][0] != query, 'Missing exact instruction contradicts containing start')
    for suffix in ('at', 'containing'):
        instruction_unit, code_unit = units['instruction_' + suffix], units['code_unit_' + suffix]
        if instruction_unit is not None or (code_unit is not None and code_unit[2] == 'instruction'):
            require(instruction_unit == code_unit, 'Instruction/code-unit null states differ')
    if units['code_unit_at'] is not None:
        require(units['code_unit_at'] == units['code_unit_containing'], 'Exact code unit differs from containing code unit')
    elif units['code_unit_containing'] is not None:
        require(units['code_unit_containing'][0] != query, 'Missing exact code unit contradicts containing start')

    require('instruction_flow' in payload, 'Missing explicit instruction_flow value')
    flow = payload['instruction_flow']
    default_targets, effective_targets, is_call = set(), set(), False
    if units['instruction_at'] is None:
        require(flow is None, 'Missing instruction has synthesized flow')
    else:
        require(isinstance(flow, dict), 'Exact instruction has no flow record')
        require(flow.get('flow_override') in ('NONE', 'BRANCH', 'CALL', 'CALL_RETURN', 'RETURN'), 'Unknown flow override')
        for key in ('default_flow', 'effective_flow'):
            item = flow.get(key)
            require(isinstance(item, dict) and isinstance(item.get('name'), str) and item['name'], 'Missing ' + key)
            for flag in ('is_call', 'is_jump', 'is_conditional', 'is_computed', 'is_terminal'):
                boolean(item.get(flag), key + '.' + flag)
            is_call |= item['is_call']
        boolean(flow.get('fallthrough_overridden'), 'fallthrough_overridden')
        for key in ('default_fallthrough', 'effective_fallthrough'):
            require(key in flow, 'Missing explicit ' + key)
            if flow[key] is not None:
                address(flow[key], key)
        for key, targets in [('default_targets', default_targets), ('effective_targets', effective_targets)]:
            require(isinstance(flow.get(key), list), 'Missing ' + key)
            for target in flow[key]:
                targets.add(address(target, key))
            require(len(targets) == len(flow[key]), 'Duplicate ' + key)

    require(isinstance(payload.get('functions'), list) and len(payload['functions']) <= 2, 'Invalid queried-function records')
    functions = {}
    for item in payload['functions']:
        entry = validate_function_flags(item, 'function')
        require(entry not in functions, 'Duplicate function record')
        ranges = validate_body(item.get('body'), 'function.body', max_ranges)
        require(contains(ranges, entry), 'Function entry is outside its reported body')
        functions[entry] = ranges
    references = set()
    for key in ('function_at_query', 'function_containing_query'):
        require(key in payload, 'Missing explicit ' + key)
        if payload[key] is not None:
            entry = address(payload[key], key)
            require(entry in functions and contains(functions[entry], query), key + ': missing body membership')
            if key == 'function_at_query':
                require(entry == query, 'Exact function entry differs from query')
            references.add(entry)
    require(references == set(functions), 'Unreferenced or missing queried function')
    if payload['function_at_query'] is not None:
        require(payload['function_at_query'] == payload['function_containing_query'], 'Exact function differs from containing function')
    elif payload['function_containing_query'] is not None:
        require(address(payload['function_containing_query'], 'function_containing_query') != query, 'Missing exact function contradicts containing entry')

    calls = payload.get('direct_call_targets')
    require(isinstance(calls, list), 'Missing direct call target records')
    expected_targets = default_targets | effective_targets if is_call else set()
    observed_targets = set()
    for call in calls:
        require(isinstance(call, dict), 'Invalid call target record')
        target = address(call.get('address'), 'call_target.address')
        require(target not in observed_targets, 'Duplicate call target')
        observed_targets.add(target)
        for key, targets in [('default_target', default_targets), ('effective_target', effective_targets)]:
            boolean(call.get(key), 'call_target.' + key)
            require(call[key] == (target in targets), 'Call target origin mismatch')
        require('function_at_target' in call, 'Missing explicit call target function')
        if call['function_at_target'] is not None:
            require(validate_function_flags(call['function_at_target'], 'callee') == target, 'Callee is not at exact target')
    require(observed_targets == expected_targets, 'Incomplete or inferred call target records')
    return before


def fetch_raw(config, requested, max_ranges):
    url = config['ghidra_url'].rstrip('/') + '/' + ENDPOINT + '?' + urlencode(
        {'address': requested, 'program': config['program'], 'max_ranges': max_ranges})
    result = {'address': requested, 'url': url, 'http_status': None, 'transport_error': None}
    raw = b''
    try:
        with urlopen(Request(url, headers={'Accept': 'application/json'}, method='GET'), timeout=60) as response:
            result['http_status'], raw = response.status, response.read()
    except HTTPError as error:
        result['http_status'], raw = error.code, error.read()
    except (OSError, URLError) as error:
        result['transport_error'] = type(error).__name__ + ': ' + str(error)
    result.update(raw_response=raw.decode('utf-8', errors='replace'),
                  raw_response_base64=base64.b64encode(raw).decode('ascii'), raw_sha256=hashlib.sha256(raw).hexdigest())
    return result


def persist(path, record):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8', newline='\n')
    temporary.replace(path)


def capture(config, addresses, output, max_ranges=MAX_RANGES, client_factory=Client, fetch=fetch_raw):
    require(1 <= len(addresses) <= MAX_ADDRESSES, 'Use from 1 through 64 explicit addresses')
    integer(max_ranges, 'max_ranges', 1, MAX_RANGES)
    for value in addresses:
        query_address(value)
    record = {'schema': 1, 'endpoint': ENDPOINT, 'read_only': True,
              'utc': datetime.now(timezone.utc).isoformat(), 'requested_addresses': addresses,
              'max_ranges': max_ranges, 'verified_before': None, 'verified_after': None,
              'responses': [], 'validation': {'accepted': False, 'state': 'pending', 'errors': []}}
    persist(output, record)
    failures = []
    try:
        client = client_factory(config)
        record['verified_before'] = client.verify()
        persist(output, record)
        default_space = validate_identity(record['verified_before'], config)
        try:
            for requested in addresses:
                response = fetch(config, requested, max_ranges)
                record['responses'].append(response)
                persist(output, record)  # Preserve raw bytes/status BEFORE parsing or validating.
                if response['http_status'] != 200 or response.get('transport_error'):
                    break
        finally:
            record['verified_after'] = client.verify()
            persist(output, record)
        require(validate_identity(record['verified_after'], config) == default_space, 'Default address space changed across batch')
        require(len(record['responses']) == len(addresses), 'Metadata batch incomplete')
        observed_modification = None
        for requested, response in zip(addresses, record['responses']):
            require(response.get('address') == requested, 'Capture query order mismatch')
            require(response.get('http_status') == 200 and not response.get('transport_error'), 'Metadata endpoint unavailable or HTTP failure')
            raw = base64.b64decode(response['raw_response_base64'], validate=True)
            require(hashlib.sha256(raw).hexdigest() == response['raw_sha256'], 'Retained response hash mismatch')
            payload = json.loads(raw.decode('utf-8'))
            modification = validate_response(payload, config, requested, default_space, max_ranges)
            if observed_modification is not None:
                require(modification == observed_modification, 'Program changed across metadata batch')
            observed_modification = modification
        record['modification_number'] = str(observed_modification)
    except Exception as error:
        failures.append(type(error).__name__ + ': ' + str(error))
    record['validation'] = {'accepted': not failures, 'state': 'rejected' if failures else 'accepted', 'errors': failures}
    persist(output, record)
    return record


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('addresses', nargs='+')
    parser.add_argument('--output', required=True)
    parser.add_argument('--max-ranges', type=int, default=MAX_RANGES)
    parser.add_argument('--config')
    args = parser.parse_args(argv)
    output = (ROOT / args.output).resolve()
    if not output.is_relative_to((ROOT / 'local').resolve()) or output.suffix != '.json':
        parser.error('--output must name a JSON file under local/')
    config_path = ROOT / 'config/target.json'
    if args.config:
        config_path = (ROOT / args.config).resolve()
        if not config_path.is_relative_to((ROOT / 'local').resolve()) or config_path.suffix != '.json':
            parser.error('--config override must name a JSON file under local/')
    config = json.loads(config_path.read_text(encoding='utf-8'))
    record = capture(config, args.addresses, output, args.max_ranges)
    print('Retained complete typed metadata capture: ' + output.relative_to(ROOT).as_posix())
    if not record['validation']['accepted']:
        for error in record['validation']['errors']:
            print('Rejected: ' + error[:400])
        return 1
    print(f'Accepted {len(record["responses"])} typed metadata responses at modification {record["modification_number"]}. No analysis or repair performed.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
