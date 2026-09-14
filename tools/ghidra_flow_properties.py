"""Read exact instruction/function flow properties without changing Ghidra analysis.

Uses the bridge's supported run_script_inline endpoint with a fixed read-only
Java query. Script execution must already be enabled on that bridge. Never call
clear_instruction_flow_override as a probe: its historical dry run mutates.
The raw response, query source and verified target identity are retained under
local/. The bridge may create its ordinary temporary script/compiler files;
the query performs no program, listing, function or project writes.
"""
import argparse
from datetime import datetime, timezone
import json
from pathlib import Path
import re
import sys
from urllib.error import HTTPError
from urllib.parse import urlencode
from urllib.request import Request, urlopen

from ghidra_export import Client

ROOT = Path(__file__).resolve().parents[1]


def query_source(addresses, config):
    # Only normalized addresses and JSON-quoted target strings enter the fixed
    # source. No caller-supplied script, method name or statement is accepted.
    template = r'''
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.CodeUnit;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.Instruction;

public class __CLASS_NAME__ extends GhidraScript {
    private String q(Object value) {
        if (value == null) return "null";
        String s = value.toString();
        return "\"" + s.replace("\\", "\\\\").replace("\"", "\\\"")
            .replace("\n", "\\n").replace("\r", "\\r").replace("\t", "\\t") + "\"";
    }
    private void field(StringBuilder row, String name, Object value) {
        if (row.length() > 1) row.append(',');
        row.append(q(name)).append(':').append(q(value));
    }
    @Override public void run() throws Exception {
        if (!currentProgram.getDomainFile().getPathname().equals(__PROGRAM_PATH__) ||
            !currentProgram.getLanguageID().toString().equals(__LANGUAGE__) ||
            currentProgram.getImageBase().getOffset() != __IMAGE_BASE__L)
            throw new IllegalStateException("Wrong BSP target inside read-only query");
        String[] addresses = {__ADDRESSES__};
        StringBuilder output = new StringBuilder("[");
        for (String value : addresses) {
            Address address = currentProgram.getAddressFactory().getAddress(value);
            Instruction instruction = currentProgram.getListing().getInstructionAt(address);
            Instruction containing = currentProgram.getListing().getInstructionContaining(address);
            CodeUnit unit = currentProgram.getListing().getCodeUnitAt(address);
            Function function = currentProgram.getFunctionManager().getFunctionContaining(address);
            Function thunk = function != null && function.isThunk() ? function.getThunkedFunction(false) : null;
            StringBuilder row = new StringBuilder("{");
            field(row, "address", address);
            field(row, "instruction", instruction);
            field(row, "instruction_start", containing == null ? null : containing.getAddress());
            field(row, "code_unit_class", unit == null ? null : unit.getClass().getName());
            field(row, "code_unit_length", unit == null ? null : unit.getLength());
            field(row, "flow_override", instruction == null ? null : instruction.getFlowOverride());
            field(row, "default_flow_type", instruction == null ? null : instruction.getPrototype().getFlowType(instruction.getInstructionContext()));
            field(row, "effective_flow_type", instruction == null ? null : instruction.getFlowType());
            field(row, "default_fallthrough", instruction == null ? null : instruction.getDefaultFallThrough());
            field(row, "fallthrough", instruction == null ? null : instruction.getFallThrough());
            field(row, "fallthrough_overridden", instruction == null ? null : instruction.isFallThroughOverridden());
            field(row, "flows", instruction == null ? null : java.util.Arrays.toString(instruction.getFlows()));
            field(row, "function_entry", function == null ? null : function.getEntryPoint());
            field(row, "function_name", function == null ? null : function.getName());
            field(row, "function_body", function == null ? null : function.getBody());
            field(row, "function_no_return", function == null ? null : function.hasNoReturn());
            field(row, "function_is_thunk", function == null ? null : function.isThunk());
            field(row, "thunk_target", thunk == null ? null : thunk.getEntryPoint());
            field(row, "thunk_target_name", thunk == null ? null : thunk.getName());
            field(row, "thunk_target_no_return", thunk == null ? null : thunk.hasNoReturn());
            row.append('}');
            if (output.length() > 1) output.append(',');
            output.append(row);
        }
        output.append(']');
        println("BSP_FLOW_PROPERTIES_BEGIN");
        println(output.toString());
        println("BSP_FLOW_PROPERTIES_END");
    }
}
'''
    return (template.replace('__CLASS_NAME__', 'BspFlowProperties_' + datetime.now(timezone.utc).strftime('%Y%m%d%H%M%S%f'))
            .replace('__PROGRAM_PATH__', json.dumps(config['program_path']))
            .replace('__LANGUAGE__', json.dumps(config['language']))
            .replace('__IMAGE_BASE__', str(int(config['image_base'], 16)))
            .replace('__ADDRESSES__', ','.join(json.dumps(a) for a in addresses)))


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('addresses', nargs='+')
    parser.add_argument('--output', required=True, help='JSON response record under local/')
    args = parser.parse_args(argv)
    if len(args.addresses) > 64:
        parser.error('Use at most 64 explicit addresses per bounded query.')
    addresses = [a.lower().removeprefix('0x').zfill(8) for a in args.addresses]
    if any(not re.fullmatch(r'[0-9a-f]{8}', a) for a in addresses):
        parser.error('Expected explicit 32-bit hexadecimal addresses.')
    output = (ROOT / args.output).resolve()
    if not output.is_relative_to((ROOT / 'local').resolve()) or output.suffix != '.json':
        parser.error('--output must be a JSON file under local/.')
    config = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    client = Client(config)
    before = client.verify()
    source = query_source(addresses, config)
    payload = {'code': source, 'args': ''}
    url = config['ghidra_url'].rstrip('/') + '/run_script_inline?' + urlencode({'program': config['program']})
    request = Request(url, data=json.dumps(payload).encode(), headers={'Content-Type': 'application/json'}, method='POST')
    try:
        with urlopen(request, timeout=60) as response:
            status, raw = response.status, response.read().decode('utf-8')
    except HTTPError as exc:
        status, raw = exc.code, exc.read().decode('utf-8', errors='replace')
    after = client.verify()
    try:
        decoded = json.loads(raw)
    except json.JSONDecodeError:
        decoded = raw
    text = decoded if isinstance(decoded, str) else json.dumps(decoded)
    # Responses may wrap the script's output in an object. Search string leaves
    # without interpreting arbitrary returned code or commands.
    def strings(value):
        if isinstance(value, str):
            yield value
        elif isinstance(value, dict):
            for item in value.values():
                yield from strings(item)
        elif isinstance(value, list):
            for item in value:
                yield from strings(item)
    parsed = None
    for text in strings(decoded):
        match = re.search(r'BSP_FLOW_PROPERTIES_BEGIN\s*(\[.*?\])\s*BSP_FLOW_PROPERTIES_END', text, re.S)
        if match:
            parsed = json.loads(match.group(1))
            break
    failed = isinstance(decoded, dict) and (decoded.get('error') or decoded.get('success') is False or decoded.get('status') == 'error')
    matched = isinstance(parsed, list) and [row.get('address', '').lower() for row in parsed] == addresses
    record = {'utc': datetime.now(timezone.utc).isoformat(), 'addresses': addresses,
              'read_only': True, 'endpoint': 'run_script_inline', 'verified_before': before,
              'verified_after': after, 'query_source': source, 'http_status': status,
              'raw_response': raw, 'decoded_response': decoded, 'properties': parsed}
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(record, indent=2) + '\n', encoding='utf-8')
    print(f'Retained raw read-only flow-property response: {output.relative_to(ROOT).as_posix()}')
    if status != 200 or failed or not matched:
        print('No property rows returned; inspect the retained response. No mutation fallback was attempted.', file=sys.stderr)
        return 1
    for row in parsed:
        print(f"{row['address']} override={row['flow_override']} default={row['default_flow_type']} effective={row['effective_flow_type']} fallthrough={row['fallthrough']} function={row['function_entry']} noreturn={row['function_no_return']} thunk={row['thunk_target']}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
