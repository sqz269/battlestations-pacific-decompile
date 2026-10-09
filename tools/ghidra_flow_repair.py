"""Report possible fall-through gaps without changing Ghidra or repair records.

Historical false no-return discovery can leave missing instructions after a CALL.
A gap alone does not identify the current call-site FlowOverride, separate
fallthrough override, or callee no-return flag. This tool compares the stored
listing with disk bytes and reports a conditional plan; it never calls a mutator
to discover those properties. The available typed read API does not expose the
exact preimages needed to authorize a repair.

Usage:
  python tools/ghidra_flow_repair.py <function> [<function> ...] [--dry-run]
  python tools/ghidra_flow_repair.py <function> --tail-end <end_exclusive>
  python tools/ghidra_flow_repair.py <catch> --tail-end <end_exclusive> --tail-rethrow

The optional tail bound is explicit evidence from the native listing, not an
inferred function boundary. It detects a possible final gap when there is no
following listed instruction. Reporting a gap does not establish its cause,
validate the whole Native body, decode anything in Ghidra, or extend the stored
function body. The tail_terminator helper retains its explicit RET/rethrow
validation for callers; report mode does not admit a repair from a tail bound.

Legacy --record <path> remains accepted and leaves that file untouched in report
mode. --apply is refused before any Ghidra request: no attested bounded, atomic
repair route with exact metadata preimages is currently implemented here.
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from ghidra_export import Client  # noqa: E402

LINE = re.compile(r'^([0-9a-fA-F]{8}):\s*(\S+)(.*)$')
APPLY_UNSUPPORTED = (
    'Unsupported --apply: an attested bounded/atomic callable repair route and '
    'exact flow/fallthrough, body and metadata preimages are required. The legacy '
    'flow-clearing/disassembly/save path is disabled. No Ghidra requests or '
    'mutations were made. Run without --apply for a conditional read-only report.'
)


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


def tail_terminator(decoded, end, rethrow=False):
    """Require a fully decoded, explicitly selected native exit sequence."""
    if not decoded or decoded[-1][0] + decoded[-1][1] != end:
        raise ValueError('explicit tail must end exactly after a decoded instruction')
    if not rethrow and decoded[-1][2] == 'ret':
        return 'ret'
    suffix = [(row[2], row[3]) for row in decoded[-3:]]
    if rethrow and suffix == [('push', '0'), ('push', '0'), ('call', '0xbf6885')]:
        return 'cxx_rethrow_00bf6885'
    expected = 'PUSH 0; PUSH 0; CALL 00BF6885' if rethrow else 'RET'
    raise ValueError(f'explicit tail must follow a fully decoded {expected}')


def main(argv):
    argv = list(argv)
    # Fail before client creation, reads, record access or any potential write.
    # A future route needs independent review; endpoint names or a dry-run flag
    # do not attest bounded disassembly, atomic rollback or exact preimages.
    if '--apply' in argv:
        sys.exit(APPLY_UNSUPPORTED)
    tail_rethrow = '--tail-rethrow' in argv
    if '--record' in argv:
        i = argv.index('--record')
        if i + 1 == len(argv) or argv[i + 1].startswith('--'):
            sys.exit('--record requires a path (report mode leaves it unchanged)')
        del argv[i:i + 2]
    tail_end = None
    if '--tail-end' in argv:
        i = argv.index('--tail-end')
        tail_end = int(argv[i + 1], 16)
        del argv[i:i + 2]
    if tail_rethrow and tail_end is None:
        sys.exit('--tail-rethrow requires --tail-end')
    functions = [a.lower().replace('0x', '').zfill(8) for a in argv if not a.startswith('--')]
    if not functions:
        sys.exit(__doc__)
    cfg = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    client = Client(cfg)
    client.verify()
    _, _, text_lo, code = load_pe(cfg)
    if tail_end is not None:
        if len(functions) != 1:
            sys.exit('--tail-end requires exactly one function')
        start = int(functions[0], 16)
        if not (text_lo <= start < tail_end <= text_lo + len(code)) or tail_end - start >= 0x4000:
            sys.exit('Explicit tail range must be a bounded interval inside .text')
    for function in functions:
        rows = listing(client, function)
        if tail_end is not None and rows and rows[-1][0] >= tail_end:
            sys.exit(f'{function}: explicit tail end precedes stored instructions')
        gaps = find_gaps(rows, text_lo, code, tail_end)
        print(f'{function}: {len(rows)} listed instructions, {len(gaps)} gap(s)')
        for g in gaps:
            print(f"  after {g['mnemonic']} {g['operands']} at {g['site']}: {g['gap_start']}..{g['gap_end_exclusive']} "
                  f"({g['bytes']} bytes){'' if g['after_call'] else '  [not after a CALL; left alone]'}")
    print('Conditional plan only: exact flow/fallthrough and no-return properties '
          'were not read; no available typed read endpoint exposes them here. '
          'Gap bytes do not establish a CALL_RETURN override or a complete body. '
          'Ghidra and repair records are unchanged; --apply is unsupported.')


if __name__ == '__main__':
    main(sys.argv[1:])
