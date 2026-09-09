"""Static call graph and data-reference sweep of the disk executable with Capstone.

Disassembles every function range from the current export snapshot (start to next start,
inside .text) and records direct call targets, out-of-body tail jumps, and immediate
references into .rdata/.data from mov/push/lea. Output (under the ignored exports directory):
callgraph.json and datarefs.json as {"0x401010": ["0xbf6340", ...]} plus callgraph_meta.json.
This is a linear sweep over the disk bytes, not Ghidra analysis: indirect calls, jump tables
and vtable dispatch are not recorded. Used by tools/partition_candidates.py.
"""
import argparse
import bisect
import collections
import hashlib
import json
import re
import time
from datetime import datetime, timezone
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs

ROOT = Path(__file__).resolve().parents[1]
IMM = re.compile(r'0x([0-9a-f]{6,8})')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, default=ROOT / 'config/target.json')
    parser.add_argument('--output', type=Path, default=ROOT / 'exports/bsp')
    args = parser.parse_args()
    config = json.loads(args.config.read_text(encoding='utf-8'))
    binary = Path(config['binary'])
    functions = json.loads((args.output / 'functions.json').read_text())

    pe = pefile.PE(str(binary), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    sections = {s.Name.rstrip(b'\0').decode(errors='ignore'): (base + s.VirtualAddress, s.get_data()) for s in pe.sections}
    text_lo, text = sections['.text']
    text_hi = text_lo + len(text)
    rdata_lo, rdata = sections['.rdata']
    rdata_hi = rdata_lo + len(rdata)
    data_lo, data = sections['.data']
    data_hi = data_lo + len(data)

    starts = sorted(int(f['address'], 16) for f in functions if not f.get('isExternal'))
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = False
    calls = collections.defaultdict(set)
    datarefs = collections.defaultdict(set)
    began = time.time()
    instructions = 0
    for i, start in enumerate(starts):
        if not text_lo <= start < text_hi:
            continue
        end = min(starts[i + 1] if i + 1 < len(starts) else text_hi, text_hi)
        for address, size, mnemonic, operands in md.disasm_lite(text[start - text_lo:end - text_lo], start):
            instructions += 1
            if mnemonic == 'call' and operands.startswith('0x'):
                target = int(operands, 16)
                if text_lo <= target < text_hi:
                    calls[start].add(target)
            elif mnemonic == 'jmp' and operands.startswith('0x'):
                target = int(operands, 16)
                if text_lo <= target < text_hi and not start <= target < end:
                    calls[start].add(target)
            elif mnemonic in ('mov', 'push', 'lea') and '0x' in operands:
                for match in IMM.finditer(operands):
                    value = int(match.group(1), 16)
                    if rdata_lo <= value < rdata_hi or data_lo <= value < data_hi:
                        datarefs[start].add(value)
    elapsed = time.time() - began

    def dump(name, table):
        (args.output / name).write_text(json.dumps({hex(k): sorted(hex(v) for v in vs) for k, vs in sorted(table.items())}))

    dump('callgraph.json', calls)
    dump('datarefs.json', datarefs)
    edges = sum(len(v) for v in calls.values())
    meta = {'utc': datetime.now(timezone.utc).isoformat(), 'binary': str(binary),
            'disk_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
            'functions_json_entries': len(functions), 'function_starts_swept': len(starts),
            'instructions': instructions, 'callers': len(calls), 'call_edges': edges,
            'functions_with_datarefs': len(datarefs), 'seconds': round(elapsed, 1),
            'note': 'Linear Capstone sweep of disk bytes over snapshot function ranges; no indirect calls, jump tables or vtable dispatch.'}
    (args.output / 'callgraph_meta.json').write_text(json.dumps(meta, indent=1))
    print(json.dumps(meta, indent=1))


if __name__ == '__main__':
    main()
