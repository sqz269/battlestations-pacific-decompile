"""Find and repair fall-through gaps left by CALL_RETURN flow overrides inside a function.

Ghidra's non-returning-function discovery marks call sites to the CRT free helpers (00bf65ac,
00bf6989, 00bf9dc8) as CALL_RETURN, so the bytes after such a call are never disassembled and
the decompiler emits a spurious return and drops the reachable block. This tool compares
Ghidra's stored listing of a function against a linear Capstone decode of the disk image,
reports every gap that follows a CALL, and with --apply clears the override at that call
site and disassembles the gap (the recipe used by the earlier one-off repairs under
reports/*_flow_repair.json). It never touches the callee's own no-return flag, which would
re-trigger the discovery, and never creates or deletes functions.

Usage:
  python tools/ghidra_flow_repair.py <function> [<function> ...]            # report only
  python tools/ghidra_flow_repair.py <function> --apply [--record reports/x.json]
  python tools/ghidra_flow_repair.py <function> --tail-end <end_exclusive> [--apply]

The optional tail bound is explicit evidence from the native listing, not an
inferred function boundary. It detects a final CALL_RETURN that truncated the
stored body and otherwise has no following instruction against which to find a gap.
Decoding the tail does not necessarily extend Ghidra's stored function body;
the report records that distinction and warns when its listing remains truncated.
"""
import hashlib
import json
import re
import sys
import time
from pathlib import Path
from urllib.parse import urlencode
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from coordination import check_writable, ghidra_lock, owner_name  # noqa: E402
from ghidra_export import Client  # noqa: E402

LINE = re.compile(r'^([0-9a-fA-F]{8}):\s*(\S+)(.*)$')


def load_pe(cfg):
    import pefile
    pe = pefile.PE(cfg['binary'], fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    section = next(s for s in pe.sections if s.Name.rstrip(b'\x00') == b'.text')
    return pe, base, base + section.VirtualAddress, section.get_data()


def listing(client, function):
    text = client.get('disassemble_function', address=function)
    if isinstance(text, (dict, list)):
        text = json.dumps(text)
    rows = []
    for line in str(text).splitlines():
        m = LINE.match(line.strip())
        if m:
            rows.append((int(m.group(1), 16), m.group(2).upper(), m.group(3).strip()))
    return rows


def find_gaps(rows, text_lo, code, tail_end=None):
    """Gaps between consecutive listed instructions where the earlier one is a CALL."""
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    gaps = []
    following = rows[1:]
    if rows and tail_end is not None:
        following = following + [(tail_end, 'EXPLICIT_END', '')]
    for (a, mnemonic, operands), (b, _, _) in zip(rows, following):
        ins = next(md.disasm_lite(code[a - text_lo:a - text_lo + 16], a), None)
        end = a + (ins[1] if ins else 1)
        if b > end:
            gaps.append({'site': f'{a:08x}', 'mnemonic': mnemonic, 'operands': operands,
                         'gap_start': f'{end:08x}', 'gap_end_exclusive': f'{b:08x}', 'bytes': b - end,
                         'after_call': mnemonic == 'CALL'})
    return gaps


def main(argv):
    argv = list(argv)
    apply = '--apply' in argv
    record_path = ROOT / 'reports/flow_repairs.json'
    if '--record' in argv:
        i = argv.index('--record')
        record_path = ROOT / argv[i + 1]
        del argv[i:i + 2]
    tail_end = None
    if '--tail-end' in argv:
        i = argv.index('--tail-end')
        tail_end = int(argv[i + 1], 16)
        del argv[i:i + 2]
    functions = [a.lower().replace('0x', '').zfill(8) for a in argv if not a.startswith('--')]
    if not functions:
        sys.exit(__doc__)
    cfg = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    client = Client(cfg)
    client.verify()
    pe, base, text_lo, code = load_pe(cfg)
    if tail_end is not None:
        if len(functions) != 1:
            sys.exit('--tail-end requires exactly one function')
        start = int(functions[0], 16)
        if not (text_lo <= start < tail_end <= text_lo + len(code)) or tail_end - start >= 0x4000:
            sys.exit('Explicit tail range must be a bounded interval inside .text')
    record = json.loads(record_path.read_text(encoding='utf-8')) if record_path.exists() else {'repairs': []}

    def post(endpoint, body):
        req = Request(cfg['ghidra_url'] + '/' + endpoint + '?' + urlencode({'program': cfg['program']}),
                      data=json.dumps(body).encode(), headers={'Content-Type': 'application/json'}, method='POST')
        with urlopen(req, timeout=300) as response:
            result = json.loads(response.read())
        if result.get('error') or result.get('success') is False or result.get('status') == 'error':
            raise RuntimeError(result)
        return result

    plan = {}
    for function in functions:
        rows = listing(client, function)
        if tail_end is not None and rows and rows[-1][0] >= tail_end:
            sys.exit(f'{function}: explicit tail end precedes stored instructions')
        gaps = find_gaps(rows, text_lo, code, tail_end)
        plan[function] = gaps
        print(f'{function}: {len(rows)} listed instructions, {len(gaps)} gap(s)')
        for g in gaps:
            print(f"  after {g['mnemonic']} {g['operands']} at {g['site']}: {g['gap_start']}..{g['gap_end_exclusive']} "
                  f"({g['bytes']} bytes){'' if g['after_call'] else '  [not after a CALL; left alone]'}")
    if not apply:
        print('report only; add --apply to clear the call-site overrides and disassemble the gaps')
        return
    owner = owner_name()
    with ghidra_lock(owner=owner, wait_seconds=120, purpose='flow repair after CALL_RETURN overrides'):
        check_writable(functions, owner=owner)
        for function, gaps in plan.items():
            todo = [g for g in gaps if g['after_call']]
            if not todo:
                continue
            before = str(client.get('decompile_function', address=function, timeout=300))
            entry = {'function': function, 'owner': owner, 'when': time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime()),
                     'decompile_lines_before': len(before.splitlines()), 'gaps': [], 'events': []}
            if tail_end is not None:
                entry['explicit_tail_end_exclusive'] = f'{tail_end:08x}'
                start = int(function, 16)
                native_body = pe.get_data(start - base, tail_end - start)
                live_body = client.get('read_memory', address=function, length=tail_end - start)
                if not isinstance(live_body, dict) or bytes.fromhex(live_body['hex']) != native_body:
                    raise RuntimeError(f'{function}: explicit body bytes differ from disk')
                from capstone import CS_ARCH_X86, CS_MODE_32, Cs
                decoded_body = list(Cs(CS_ARCH_X86, CS_MODE_32).disasm_lite(native_body, start))
                if not decoded_body or decoded_body[-1][0] + decoded_body[-1][1] != tail_end or decoded_body[-1][2] != 'ret':
                    raise RuntimeError(f'{function}: explicit tail must follow a fully decoded RET')
                entry['explicit_body_sha256'] = hashlib.sha256(native_body).hexdigest()
            for g in todo:
                lo, hi = int(g['gap_start'], 16), int(g['gap_end_exclusive'], 16)
                disk = pe.get_data(lo - base, hi - lo)
                mem = client.get('read_memory', address=g['gap_start'], length=hi - lo)
                live = bytes.fromhex(mem['hex'] if isinstance(mem, dict) else ''.join(ch for ch in str(mem) if ch in '0123456789abcdefABCDEF'))[:hi - lo]
                if disk != live:
                    entry['events'].append({'gap': g, 'skipped': 'Ghidra bytes differ from the disk image'})
                    print(f"  {function}: skipping {g['gap_start']}, bytes differ from disk")
                    continue
                g['sha256'] = hashlib.sha256(disk).hexdigest()
                cleared = post('clear_instruction_flow_override', {'address': g['site']})
                disassembled = post('disassemble_bytes', {'start_address': g['gap_start'], 'end_address': g['gap_end_exclusive']})
                entry['events'].append({'gap': g, 'clear': cleared.get('message', cleared), 'disassemble': disassembled.get('message', disassembled)})
                entry['gaps'].append(g)
            if tail_end is not None:
                entry['body_before'] = client.get('get_function_by_address', address=function)
                record['repairs'].append(entry)
                record_path.parent.mkdir(parents=True, exist_ok=True)
                record_path.write_text(json.dumps(record, indent=1) + '\n', encoding='utf-8', newline='\n')
                # A stored body can stop at the first CALL_RETURN. Clear the
                # same erroneous override on later verified CRT free calls,
                # which the stored listing therefore cannot expose yet.
                for address, _, mnemonic, operand in decoded_body:
                    if mnemonic != 'call' or operand not in ('0xbf65ac', '0xbf6989', '0xbf9dc8'):
                        continue
                    cleared = post('clear_instruction_flow_override', {'address': f'{address:08x}'})
                    entry['events'].append({'explicit_tail_call': f'{address:08x}', 'clear': cleared})
            after = str(client.get('decompile_function', address=function, timeout=300))
            remaining = find_gaps(listing(client, function), text_lo, code, tail_end)
            entry['decompile_lines_after'] = len(after.splitlines())
            entry['gaps_remaining'] = len([g for g in remaining if g['after_call']])
            if tail_end is not None:
                entry['body_after'] = client.get('get_function_by_address', address=function)
                entry['stored_body_tail_complete'] = not any(g['gap_end_exclusive'] == f'{tail_end:08x}' for g in remaining)
                if not entry['stored_body_tail_complete']:
                    entry['body_extension_limit'] = 'Explicit tail decoding/CRT overrides do not extend stored function-body metadata; the body still requires a separate supported repair.'
            if tail_end is None:
                record['repairs'].append(entry)
            record_path.parent.mkdir(parents=True, exist_ok=True)
            record_path.write_text(json.dumps(record, indent=1) + '\n', encoding='utf-8', newline='\n')
            print(f"{function}: repaired {len(entry['gaps'])} gap(s); decompile {entry['decompile_lines_before']} -> "
                  f"{entry['decompile_lines_after']} lines; {entry['gaps_remaining']} call gap(s) remain")
            if tail_end is not None and not entry['stored_body_tail_complete']:
                print(f'{function}: WARNING: stored function-body tail is still incomplete; do not claim full flow repair')
        for attempt in range(12):
            try:
                post('save_program', {})
                break
            except RuntimeError as exc:
                if 'active transaction' not in str(exc) or attempt == 11:
                    raise
                time.sleep(5)
    print(f'saved; record {record_path.relative_to(ROOT)}')


if __name__ == '__main__':
    main(sys.argv[1:])
