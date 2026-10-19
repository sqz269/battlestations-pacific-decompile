"""Static call graph and data-reference sweep of the disk executable with Capstone.

Disassembles every function range from the current export snapshot (start to the next start or the
first int3 pad, inside .text). Outputs, under the ignored exports directory:

- callgraph_edges.json: {"edges": [[caller, callee, site, kind], ...], "funclet_parent": {funclet: parent}}
  with addresses as ints and these kinds:
    call        direct call; site = the call instruction
    tailjmp     direct jmp/jcc that leaves the function's range; site = the jump
    addr_taken  an immediate operand equal to a function start (push offset f, mov reg/[mem], offset f, ...);
                site = the instruction. Callbacks, thread entries, vtable stores in constructors.
    (call/tailjmp/addr_taken rows whose caller is not a snapshot function come from code between an int3 pad
     and the next snapshot start that Ghidra never defined as a function; the caller is that chunk's start,
     and "undefined" maps each chunk start to its stop. About 31k chunks, nearly all 16-aligned whole
     functions after a ret and pad; only ~850 are referenced at all.)
    data_ptr    a dword-aligned value equal to a function start in a non-code section; caller is null and
                site = the data address (vtables, function-pointer and Lua tables are in here, unattributed)
    eh          a C++ EH funclet (unwind action, catch handler) to the function whose frame handler names it:
                caller = the parent function, callee = the funclet, site = the FuncInfo record. The parent is
                found through its prologue `push offset __ehhandler` and that handler's `mov eax, offset FuncInfo`.
- callgraph.json and datarefs.json as {"0x401010": ["0xbf6340", ...]}: call + tailjmp edges and immediate
  references into .rdata/.data, unchanged for older readers (tools/partition_candidates.py), plus callgraph_meta.json.

This is a linear sweep over the disk bytes, not Ghidra analysis: register-indirect calls, jump tables and
vtable dispatch are not resolved here (tools/callgraph_indirect.py adds vslot/vcall?/lua edges).
"""
import argparse
import bisect
import collections
import hashlib
import json
import re
import struct
import time
from datetime import datetime, timezone
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs, __version__ as capstone_version

ROOT = Path(__file__).resolve().parents[1]
IMM = re.compile(r'0x([0-9a-f]{6,8})')
EH_MAGIC = (0x19930520, 0x19930521, 0x19930522)
JUMP_TABLE = re.compile(r'\[\w+\*4 \+ 0x([0-9a-f]+)\]')
JUMPS = {'jmp', 'je', 'jne', 'ja', 'jae', 'jb', 'jbe', 'jg', 'jge', 'jl', 'jle', 'js', 'jns', 'jo', 'jno', 'jp', 'jnp',
         'jecxz', 'loop', 'loope', 'loopne'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, default=ROOT / 'config/target.json')
    parser.add_argument('--output', type=Path, default=__import__('workspace').exports_dir(),
                        help='defaults to the main checkout\'s exports/bsp, shared by every worktree')
    args = parser.parse_args()
    config = json.loads(args.config.read_text(encoding='utf-8'))
    binary = Path(config['binary'])
    functions_raw = (args.output / 'functions.json').read_bytes()
    functions = json.loads(functions_raw)

    pe = pefile.PE(str(binary), fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase
    sections = {s.Name.rstrip(b'\0').decode(errors='ignore'): (base + s.VirtualAddress, s.get_data()) for s in pe.sections}
    code_sections = [(base + s.VirtualAddress, s.get_data()) for s in pe.sections if s.Characteristics & 0x20000000]
    data_sections = [(base + s.VirtualAddress, s.get_data()) for s in pe.sections if not s.Characteristics & 0x20000000]
    text_lo, text = sections['.text']
    text_hi = text_lo + len(text)
    rdata_lo, rdata = sections['.rdata']
    rdata_hi = rdata_lo + len(rdata)
    data_lo, data = sections['.data']
    data_hi = data_lo + len(data)

    def dword(va):
        for lo, blob in data_sections + code_sections:
            if lo <= va and va + 4 <= lo + len(blob):
                return struct.unpack_from('<I', blob, va - lo)[0]
        return None

    starts = sorted(int(f['address'], 16) for f in functions if not f.get('isExternal'))
    start_set = set(starts)
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    md.detail = False
    calls = collections.defaultdict(set)
    datarefs = collections.defaultdict(set)
    edges = set()  # (caller, callee, site, kind)
    handler_imms = collections.defaultdict(set)  # function -> immediates it loads (for EH handler -> FuncInfo)
    began = time.time()
    instructions = 0
    ranges = {}
    orphan_edges = attached_chunks = jump_tables = 0

    def decode(owner, fn, lo, end, covered, targets, legacy, orphan=False):
        """Linear decode of [lo, end) until int3, a byte already covered, or the end. Records edges for `owner`
        (None = code that belongs to no snapshot function), marks the bytes in `covered` (relative to fn),
        collects in-function branch and jump-table targets. Returns the stop address."""
        nonlocal instructions, orphan_edges, jump_tables
        address = lo
        for address, size, mnemonic, operands in md.disasm_lite(text[lo - text_lo:end - text_lo], lo):
            if mnemonic == 'int3' or (covered[address - fn] and not legacy):
                return address
            covered[address - fn:address - fn + size] = b'\1' * size
            instructions += 1
            if mnemonic == 'call' and operands.startswith('0x'):
                target = int(operands, 16)
                if text_lo <= target < text_hi:
                    if legacy:
                        calls[owner].add(target)
                    edges.add((owner, target, address, 'call')); orphan_edges += orphan
                continue
            if mnemonic in JUMPS and operands.startswith('0x'):
                target = int(operands, 16)
                if fn <= target < end_of[fn]:
                    targets.append(target)
                elif text_lo <= target < text_hi:
                    if legacy and mnemonic == 'jmp':
                        calls[owner].add(target)  # older readers saw unconditional tail jumps only
                    edges.add((owner, target, address, 'tailjmp')); orphan_edges += orphan
                continue
            if '0x' not in operands:
                continue
            table = JUMP_TABLE.search(operands) if mnemonic == 'jmp' else None
            if table:
                base_va = int(table.group(1), 16)
                if fn <= base_va < end_of[fn]:
                    jump_tables += 1
                    for k in range(1024):
                        va = base_va + 4 * k
                        if va + 4 > end_of[fn]:
                            break
                        v = struct.unpack_from('<I', text, va - text_lo)[0]
                        if not fn <= v < end_of[fn]:
                            break
                        covered[va - fn:va - fn + 4] = b'\1' * 4
                        targets.append(v)
                continue
            lb, rb = operands.find('['), operands.find(']')
            for match in IMM.finditer(operands):
                value = int(match.group(1), 16)
                if legacy and mnemonic in ('mov', 'push', 'lea') and (rdata_lo <= value < rdata_hi or data_lo <= value < data_hi):
                    datarefs[owner].add(value)
                if not orphan and mnemonic in ('mov', 'push', 'lea') and (rdata_lo <= value < rdata_hi or data_lo <= value < data_hi):
                    handler_imms[owner].add(value)
                # an immediate that names a function start takes its address; a constant inside brackets is
                # a memory displacement ([eax*4 + 0x4a1234] is a table), not the function
                if value in start_set and value != owner and not lb <= match.start() < rb:
                    edges.add((owner, value, address, 'addr_taken')); orphan_edges += orphan
        return end

    end_of = {}
    undefined = {}  # chunk start -> stop, for code between snapshot functions
    for i, start in enumerate(starts):
        if text_lo <= start < text_hi:
            end_of[start] = min(starts[i + 1] if i + 1 < len(starts) else text_hi, text_hi)
    for start, end in end_of.items():
        covered = bytearray(end - start)
        targets = []
        # 1. the linear pass from the start to the first int3 pad (the only pass older readers saw)
        stop = decode(start, start, start, end, covered, targets, legacy=True)
        reach = stop
        # 2. code after that stop that this function branches to (direct jumps, switch jump tables) is the rest
        #    of its body: a stray int3 or jump-table data ended the linear pass early
        while targets:
            t = targets.pop()
            if stop <= t < end and not covered[t - start] and text[t - text_lo] != 0xCC:
                attached_chunks += 1
                reach = max(reach, decode(start, start, t, end, covered, targets, legacy=False))
        ranges[start] = reach
        # 3. what is left before the next start is code Ghidra never made a function: usually a whole function
        #    after the previous one's ret and int3 pad (aligned, with its own prologue), sometimes a static
        #    initializer pushed from CRT startup stubs. Each chunk is its own caller, listed in `undefined`.
        lo = stop
        while lo < end:
            if covered[lo - start] or text[lo - text_lo] == 0xCC:
                lo += 1
                continue
            nxt = decode(lo, start, lo, end, covered, [], legacy=False, orphan=True)
            if nxt > lo:
                undefined[lo] = nxt
            lo = nxt if nxt > lo else lo + 1

    # data_ptr: aligned dwords outside code that equal a function start
    data_ptrs = 0
    for lo, blob in data_sections:
        for n, (v,) in enumerate(struct.iter_unpack('<I', blob[:len(blob) // 4 * 4])):
            if v in start_set or v in undefined:
                off = 4 * n
                edges.add((None, v, lo + off, 'data_ptr'))
                data_ptrs += 1

    # eh: parent -push-> __ehhandler -mov eax-> FuncInfo -> unwind actions and catch handlers
    funclet_parent = {}
    eh_records = 0
    handler_parents = collections.defaultdict(set)
    for caller, callee, site, kind in edges:
        if kind == 'addr_taken' and caller is not None:
            handler_parents[callee].add(caller)
    for handler, parents in handler_parents.items():
        for imm in handler_imms.get(handler, ()):
            magic = dword(imm)
            if magic not in EH_MAGIC:
                continue
            max_state, p_unwind, n_try, p_try = (dword(imm + 4), dword(imm + 8), dword(imm + 12), dword(imm + 16))
            if None in (max_state, p_unwind, n_try, p_try) or max_state > 4096 or n_try > 1024:
                continue
            eh_records += 1
            targets = set()
            for s in range(max_state):
                action = dword(p_unwind + 8 * s + 4) if p_unwind else None
                if action in start_set:
                    targets.add(action)
            for t in range(n_try):
                n_catch, p_handlers = dword(p_try + 20 * t + 12), dword(p_try + 20 * t + 16)
                if n_catch is None or n_catch > 256 or not p_handlers:
                    continue
                for k in range(n_catch):
                    h = dword(p_handlers + 16 * k + 12)
                    if h in start_set:
                        targets.add(h)
            # the frame owners are the functions whose prologue pushed this handler
            for parent in parents:
                for f in targets:
                    edges.add((parent, f, imm, 'eh'))
                    funclet_parent.setdefault(f, parent)
                edges.add((parent, handler, imm, 'eh'))
                funclet_parent.setdefault(handler, parent)
    elapsed = time.time() - began

    def dump(name, table):
        (args.output / name).write_text(json.dumps({hex(k): sorted(hex(v) for v in vs) for k, vs in sorted(table.items())}))

    dump('callgraph.json', calls)
    dump('datarefs.json', datarefs)
    ordered = sorted(edges, key=lambda e: (e[3], e[0] or 0, e[1], e[2]))
    (args.output / 'callgraph_edges.json').write_text(json.dumps(
        {'edges': [list(e) for e in ordered], 'funclet_parent': {str(k): v for k, v in sorted(funclet_parent.items())},
         'undefined': {str(k): v for k, v in sorted(undefined.items())},
         'ranges': {str(k): v for k, v in sorted(ranges.items())}}))
    kinds = collections.Counter(e[3] for e in edges)
    meta = {'utc': datetime.now(timezone.utc).isoformat(), 'binary': str(binary),
            'disk_sha256': hashlib.sha256(binary.read_bytes()).hexdigest(),
            'functions_sha256': hashlib.sha256(functions_raw).hexdigest(),
            'tool_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
            'capstone_version': capstone_version,
            'functions_json_entries': len(functions), 'function_starts_swept': len(starts),
            'instructions': instructions, 'callers': len(calls), 'call_edges': sum(len(v) for v in calls.values()),
            'functions_with_datarefs': len(datarefs), 'seconds': round(elapsed, 1),
            'edges_by_kind': dict(sorted(kinds.items())), 'eh_funcinfo_records': eh_records,
            'funclets_with_parent': len(funclet_parent), 'edges_from_undefined_code': orphan_edges, 'undefined_code_chunks': len(undefined),
            'undefined_chunks_16_aligned': sum(1 for a in undefined if a % 16 == 0),
            'gap_chunks_attached_to_their_function': attached_chunks,
            'edge_weight': 'callgraph.json: unique caller-target relationships, direct calls and out-of-range direct tail jumps; '
                           'callgraph_edges.json: one row per site and kind',
            'note': 'Linear Capstone sweep of disk bytes from each snapshot function start to the next start or int3 pad; '
                    'not validated function bodies. No register-indirect calls, jump tables or vtable dispatch.'}
    (args.output / 'callgraph_meta.json').write_text(json.dumps(meta, indent=1))
    print(json.dumps(meta, indent=1))


if __name__ == '__main__':
    main()
