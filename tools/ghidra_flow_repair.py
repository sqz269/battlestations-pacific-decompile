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


def find_gaps(rows, text_lo, code):
    """Gaps between consecutive listed instructions where the earlier one is a CALL."""
    from capstone import CS_ARCH_X86, CS_MODE_32, Cs
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    gaps = []
    for (a, mnemonic, operands), (b, _, _) in zip(rows, rows[1:]):
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
    functions = [a.lower().replace('0x', '').zfill(8) for a in argv if not a.startswith('--')]
    if not functions:
        sys.exit(__doc__)
    cfg = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    client = Client(cfg)
    client.verify()
    pe, base, text_lo, code = load_pe(cfg)
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
        gaps = find_gaps(rows, text_lo, code)
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
            after = str(client.get('decompile_function', address=function, timeout=300))
            remaining = find_gaps(listing(client, function), text_lo, code)
            entry['decompile_lines_after'] = len(after.splitlines())
            entry['gaps_remaining'] = len([g for g in remaining if g['after_call']])
            record['repairs'].append(entry)
            record_path.parent.mkdir(parents=True, exist_ok=True)
            record_path.write_text(json.dumps(record, indent=1) + '\n', encoding='utf-8', newline='\n')
            print(f"{function}: repaired {len(entry['gaps'])} gap(s); decompile {entry['decompile_lines_before']} -> "
                  f"{entry['decompile_lines_after']} lines; {entry['gaps_remaining']} call gap(s) remain")
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
