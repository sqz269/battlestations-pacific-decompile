"""Export static vtable membership, candidate virtual dispatch, and Lua bindings.

Read-only disk analysis; no game or Ghidra connection. See docs/CALLGRAPH_INDIRECT.md.
The question mark in vcall? is intentional: slot compatibility is not a resolved call.
"""
import argparse
import collections
import functools
import hashlib
import json
import os
import random
import re
import struct
import tempfile
import time
from datetime import datetime, timezone
from pathlib import Path

import pefile
from capstone import CS_ARCH_X86, CS_MODE_32, Cs, __version__ as capstone_version

from workspace import exports_dir

ROOT = Path(__file__).resolve().parents[1]
REG = re.compile(r'^(?:eax|ebx|ecx|edx|esi|edi|ebp|esp)$')
PARTIAL = {part: whole for whole, parts in (
    ('eax', ('ax', 'al', 'ah')), ('ebx', ('bx', 'bl', 'bh')),
    ('ecx', ('cx', 'cl', 'ch')), ('edx', ('dx', 'dl', 'dh')),
    ('esi', ('si',)), ('edi', ('di',)), ('ebp', ('bp',)), ('esp', ('sp',))) for part in parts}
MEM = re.compile(r'^(?:dword ptr )?\[(eax|ebx|ecx|edx|esi|edi|ebp|esp)(?: \+ (0x[0-9a-f]+|[0-9]+))?\]$')
IMM = re.compile(r'^(?:0x[0-9a-f]+|[0-9]+)$')
NAME = re.compile(r'^[A-Za-z_][A-Za-z0-9_.:]*$')


@functools.lru_cache(maxsize=None)
def demangle_type(name):
    if os.name == 'nt':
        import ctypes
        undecorate = ctypes.WinDLL('dbghelp').UnDecorateSymbolName
        undecorate.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_uint32, ctypes.c_uint32]
        undecorate.restype = ctypes.c_uint32
        buffer = ctypes.create_string_buffer(8192)
        # RTTI's leading '.' is not part of the type encoding consumed by DbgHelp.
        if undecorate(name[1:].encode('ascii'), buffer, len(buffer), 0x2000):
            result = buffer.value.decode('ascii')
            if not result.startswith('?'):
                return re.sub(r'^(?:class|struct) ', '', result)
    body = name[4:-2]
    return name if '?' in body else '::'.join(reversed(body.split('@')))


def number(value):
    return int(value, 16) if isinstance(value, str) else value


def memory(operand):
    match = MEM.fullmatch(operand)
    return (match[1], int(match[2] or '0', 0)) if match else None


def sha256(data):
    return hashlib.sha256(data).hexdigest()


class Image:
    def __init__(self, binary):
        self.raw = binary.read_bytes()
        pe = pefile.PE(data=self.raw, fast_load=True)
        if pe.FILE_HEADER.Machine != 0x14c or pe.OPTIONAL_HEADER.Magic != 0x10b:
            raise ValueError('expected an x86 PE32 image')
        self.base = pe.OPTIONAL_HEADER.ImageBase
        self.sections = {s.Name.rstrip(b'\0').decode('ascii'): (
            self.base + s.VirtualAddress, s.get_data()) for s in pe.sections}
        pe.close()

    def read(self, address, size):
        for lo, data in self.sections.values():
            off = address - lo
            if 0 <= off and off + size <= len(data):
                return data[off:off + size]
        return None

    def dword(self, address):
        data = self.read(address, 4)
        return struct.unpack('<I', data)[0] if data is not None else None

    def string(self, address, limit=256):
        for lo, data in self.sections.values():
            off = address - lo
            if not 0 <= off < len(data):
                continue
            end = data.find(b'\0', off, min(off + limit + 1, len(data)))
            if end < 0:
                return None
            raw = data[off:end]
            return raw.decode('ascii') if all(32 <= b < 127 for b in raw) else None
        return None

    def typename(self, descriptor):
        name = self.string(descriptor + 8) if descriptor else None
        if not name or not name.startswith(('.?AV', '.?AU')) or not name.endswith('@@'):
            return None
        return demangle_type(name)


def crt_initializer_ranges(image, caller, instructions, functions):
    """Exclude proven CRT callback arrays, rather than all unanchored arrays."""
    rlo, rdata = image.sections['.rdata']
    rhi = rlo + len(rdata)

    def interval(lo, hi, site):
        if lo % 4 == hi % 4 == 0 and rlo <= lo < hi <= rhi:
            return {'start': hex(lo), 'end_exclusive': hex(hi),
                    'consumer': hex(caller), 'site': hex(site), 'kind': 'crt-initializers'}
        return None

    for i, (site, _, mnemonic, operands) in enumerate(instructions):
        if mnemonic == 'call' and IMM.fullmatch(operands):
            target = functions.get(int(operands, 0), {}).get('name')
            previous = instructions[max(0, i - 2):i]
            if target in ('_initterm', '__initterm', '_initterm_e', '__initterm_e') and len(previous) == 2:
                if all(m == 'push' and IMM.fullmatch(op) for _, _, m, op in previous):
                    found = interval(int(previous[1][3], 0), int(previous[0][3], 0), site)
                    if found:
                        yield found
        # MSVC can inline the C++ initializer traversal in __cinit. Require the
        # iterator/end constants, indirect array load+call, +4 stride and compare.
        if functions[caller]['name'] not in ('_cinit', '__cinit') or mnemonic != 'mov':
            continue
        parts = operands.split(', ')
        if len(parts) != 2 or not REG.fullmatch(parts[0]) or not IMM.fullmatch(parts[1]):
            continue
        iterator, lo = parts[0], int(parts[1], 0)
        for j in range(i + 1, min(i + 5, len(instructions))):
            _, _, m, op = instructions[j]
            endparts = op.split(', ')
            if m != 'mov' or len(endparts) != 2 or not REG.fullmatch(endparts[0]) or not IMM.fullmatch(endparts[1]):
                continue
            endreg, hi = endparts[0], int(endparts[1], 0)
            window = instructions[j + 1:j + 18]
            for k, (_, _, lm, lop) in enumerate(window):
                loaded = lop.split(', ')
                if lm != 'mov' or len(loaded) != 2 or not REG.fullmatch(loaded[0]) or memory(loaded[1]) != (iterator, 0):
                    continue
                tail = window[k + 1:]
                callsite = next((a for a, _, cm, cop in tail if cm == 'call' and cop == loaded[0]), None)
                stride = any(am == 'add' and aop == f'{iterator}, 4' for _, _, am, aop in tail)
                compare = any(cm == 'cmp' and cop == f'{iterator}, {endreg}' for _, _, cm, cop in tail)
                found = interval(lo, hi, callsite) if callsite is not None and stride and compare else None
                if found:
                    yield found


def discover_vtables(image, starts, anchors, excluded):
    lo, data = image.sections['.rdata']
    text_lo, text = image.sections['.text']
    text_hi = text_lo + len(text)
    runs, run = [], []
    for off in range(0, len(data) - 3, 4):
        target = struct.unpack_from('<I', data, off)[0]
        # Keep executable-pointer holes so a missing snapshot start cannot shift
        # every later slot. Only exact starts will become graph targets.
        if text_lo <= target < text_hi and not any(a <= lo + off < b for a, b in excluded):
            run.append(target)
        else:
            if sum(v in starts for v in run) >= 2:
                runs.append((lo + off - 4 * len(run), run))
            run = []
    if sum(v in starts for v in run) >= 2:
        runs.append((lo + (len(data) // 4 - len(run)) * 4, run))
    tables, bases = {}, collections.defaultdict(set)
    split_runs = []
    for address, slots in runs:
        end = address + 4 * len(slots)
        local_anchors = {a for a in anchors if address <= a < end}
        locator = image.dword(address - 4)
        col = image.read(locator, 20) if locator else None
        rtti_at_start = col and struct.unpack_from('<I', col)[0] == 0 and image.typename(struct.unpack_from('<I', col, 12)[0])
        if not local_anchors and not rtti_at_start:
            # An unanchored executable-pointer array can be a switch jump table.
            # For it, retain only the requested strict exact-start runs.
            begin = 0
            for i in range(len(slots) + 1):
                if i == len(slots) or slots[i] not in starts:
                    if i - begin >= 2:
                        split_runs.append((address + begin * 4, slots[begin:i]))
                    begin = i + 1
            continue
        boundaries = sorted({address} | local_anchors) + [end]
        for a, b in zip(boundaries, boundaries[1:]):
            segment = slots[(a - address) // 4:(b - address) // 4]
            if any(x in starts and y in starts for x, y in zip(segment, segment[1:])):
                split_runs.append((a, segment))
    for address, slots in split_runs:
        name, type_id = None, None
        locator = image.dword(address - 4)
        raw = image.read(locator, 20) if locator else None
        if raw:
            signature, _, _, descriptor, hierarchy = struct.unpack('<5I', raw)
            if signature == 0:
                name = image.typename(descriptor)
                if name:
                    type_id = descriptor
                    chd = image.read(hierarchy, 16)
                    if chd:
                        sig, _, count, array = struct.unpack('<4I', chd)
                        if sig == 0 and 0 < count <= 1024:
                            for i in range(count):
                                bcd = image.dword(array + i * 4)
                                parent = image.dword(bcd) if bcd else None
                                if parent and image.typename(parent):
                                    bases[descriptor].add(parent)
        tables[address] = {'address': hex(address), 'class': name,
                           'slots': [hex(s) for s in slots],
                           '_slots': slots, '_type': type_id}
    return tables, bases


def compatible_tables(address, tables, bases):
    """Exact table plus RTTI-derived classes, including secondary base tables."""
    root = tables[address]['_type']
    if root is None:
        return {address}
    return {a for a, t in tables.items() if t['_type'] == root or root in bases[t['_type']]}


class ReceiverState:
    """Small basic-block value tracker; discard evidence at joins and calls.

    A candidate is narrowed only after a visible immediate vtable store and a
    subsequent load from the SAME tracked object location. A constructor elsewhere
    in the function, or a dead register's previous value, cannot narrow a call.
    """
    def __init__(self):
        self.reset()

    def reset(self):
        self.regs = {r: ('object', r) for r in ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp')}
        self.stores = {}

    def key(self, mem):
        reg, disp = mem
        return (self.regs.get(reg), disp) if reg in self.regs else None

    def table(self, reg):
        value = self.regs.get(reg)
        return value[1] if value and value[0] == 'vtable' else None

    def method(self, reg):
        value = self.regs.get(reg)
        return value if value and value[0] == 'method' else None

    def step(self, site, mnemonic, operands, tables):
        parts = operands.split(', ')
        if mnemonic == 'mov' and len(parts) == 2:
            dest, src = parts
            dm, sm = memory(dest), memory(src)
            if dm:
                key = self.key(dm)
                if key is not None:
                    self.stores.pop(key, None)
                    if IMM.fullmatch(src) and int(src, 0) in tables:
                        self.stores[key] = int(src, 0)
            elif REG.fullmatch(dest):
                value = self.regs.get(src) if REG.fullmatch(src) else None
                if sm:
                    table = self.stores.get(self.key(sm))
                    base = self.regs.get(sm[0])
                    if table is not None:
                        value = ('vtable', table)
                    elif base and base[0] == 'vtable':
                        value = ('method', base[1], sm[1], site, src)
                    elif sm[1] == 0 and sm[0] not in ('esp', 'ebp'):
                        value = ('vtable', None, site)
                    else:
                        value = None
                self.regs.pop(dest, None)
                if value is not None:
                    self.regs[dest] = value
        elif parts and REG.fullmatch(parts[0]) and mnemonic not in ('cmp', 'test', 'push'):
            # Includes LEA, POP and arithmetic: no stale receiver identities.
            self.regs.pop(parts[0], None)
        if parts and parts[0] in PARTIAL and mnemonic not in ('cmp', 'test', 'push'):
            self.regs.pop(PARTIAL[parts[0]], None)
        if (mnemonic in ('xchg', 'xadd', 'cmpxchg', 'cmpxchg8b', 'mul', 'div', 'idiv', 'cdq', 'popal', 'popad') or
                mnemonic.startswith(('rep', 'lods', 'stos', 'movs', 'scas', 'cmps'))):
            self.reset()
        if '[' in operands and mnemonic not in ('mov', 'cmp', 'test', 'push', 'lea', 'call', 'jmp'):
            self.stores.clear()
        if mnemonic == 'call' or mnemonic.startswith('j') or mnemonic.startswith('ret') or mnemonic.startswith('loop'):
            self.reset()


def function_instructions(image, starts, functions):
    """Use snapshot body ranges when available, else mark linear sweep explicitly."""
    md = Cs(CS_ARCH_X86, CS_MODE_32)
    text_lo, text = image.sections['.text']
    text_hi = text_lo + len(text)
    for i, start in enumerate(starts):
        if not text_lo <= start < text_hi:
            continue
        record = functions[start]
        ranges = record.get('body_ranges')
        if ranges:
            spans = [(number(r[0]), number(r[1]) + 1) for r in ranges]
        else:
            spans = [(start, min(starts[i + 1] if i + 1 < len(starts) else text_hi, text_hi))]
        instructions = []
        for lo, hi in spans:
            if not text_lo <= lo < hi <= text_hi:
                raise ValueError(f'invalid function range for {hex(start)}: {hex(lo)}..{hex(hi)}')
            for insn in md.disasm_lite(text[lo - text_lo:hi - text_lo], lo):
                if insn[2] == 'int3' and not ranges:
                    break
                instructions.append(insn)
        yield start, instructions, bool(ranges)


def discover_lua(image, starts, functions):
    """Candidate luaL_Reg arrays; registration evidence is checked after sweeping."""
    tables = {}
    for section in ('.rdata', '.data'):
        lo, data = image.sections.get(section, (0, b''))
        pairs = {}
        for off in range(0, len(data) - 7, 4):
            nameptr, target = struct.unpack_from('<2I', data, off)
            if target not in starts:
                continue
            name = image.string(nameptr)
            if name is not None and (NAME.fullmatch(name) or
                                     (name == '' and functions[target]['name'].startswith('luaopen_'))):
                pairs[lo + off] = (name, target)
        for address in sorted(pairs):
            if address - 8 in pairs:
                continue
            entries, pos = [], address
            while pos in pairs:
                name, target = pairs[pos]
                entries.append((pos, name, target))
                pos += 8
            if image.dword(pos + 4) == 0:
                tables[address] = entries
    return tables


def inline_lua(image, instructions, starts, closure_api, field_api, register_api):
    """EDX handler ABI and adjacent closure/setfield, or pushed helper arguments."""
    constants, pushes, pending = {}, [], None
    joins = {int(op, 0) for _, _, m, op in instructions
             if m.startswith(('j', 'loop')) and IMM.fullmatch(op)}
    joins.update(current[0] for previous, current in zip(instructions, instructions[1:])
                 if previous[0] + previous[1] != current[0])
    for site, _, mnemonic, operands in instructions:
        if site in joins:
            constants.clear()
            pushes.clear()
            pending = None
        parts = operands.split(', ')
        if mnemonic == 'mov' and len(parts) == 2 and REG.fullmatch(parts[0]):
            constants.pop(parts[0], None)
            if IMM.fullmatch(parts[1]):
                constants[parts[0]] = int(parts[1], 0)
        elif parts and REG.fullmatch(parts[0]) and mnemonic not in ('push', 'test', 'cmp', 'call'):
            constants.pop(parts[0], None)
        if parts and parts[0] in PARTIAL and mnemonic not in ('push', 'test', 'cmp'):
            constants.pop(PARTIAL[parts[0]], None)
        if mnemonic == 'push':
            value = int(operands, 0) if IMM.fullmatch(operands) else constants.get(operands)
            pushes.append(value)
        if mnemonic == 'call':
            target = int(operands, 0) if IMM.fullmatch(operands) else None
            if target in closure_api:
                handler = constants.get('edx')
                pending = (handler, site) if handler in starts and pushes else None
            elif target in field_api and pending and site - pending[1] <= 96 and len(pushes) == 1:
                name = image.string(pushes[-1]) if pushes[-1] is not None else None
                if name and NAME.fullmatch(name):
                    yield pending[0], site, name
                pending = None
            elif target in register_api and len(pushes) >= 2:
                name = image.string(pushes[-1]) if pushes[-1] is not None else None
                if name and NAME.fullmatch(name) and pushes[-2] in starts:
                    yield pushes[-2], site, name
                pending = None
            else:
                pending = None
            constants.clear()
            pushes.clear()
        if mnemonic.startswith(('j', 'ret', 'loop')) or mnemonic in ('pop', 'enter', 'leave'):
            constants.clear()
            pushes.clear()
            pending = None


def export(args):
    began = time.monotonic()
    config_raw = args.config.read_bytes()
    config = json.loads(config_raw)
    image = Image(Path(config['binary']))
    functions_path = args.output / 'functions.json'
    functions_raw = functions_path.read_bytes()
    records = json.loads(functions_raw)
    functions = {number(f['address']): f for f in records if not f.get('isExternal')}
    starts = sorted(functions)
    start_set = set(starts)
    # Code Ghidra never made a function (tools/callgraph_sweep.py lists it under "undefined") can still fill
    # vtable slots; accept those starts as slot targets too, so a vtable holding one is neither split nor dropped.
    edges_path = args.output / 'callgraph_edges.json'
    undefined_starts = {int(k) for k in json.loads(edges_path.read_text()).get('undefined', {})} \
        if edges_path.exists() else set()
    slot_targets = start_set | undefined_starts
    snapshot_path = args.output / 'snapshot.json'
    snapshot_raw = snapshot_path.read_bytes() if snapshot_path.exists() else None
    if snapshot_raw:
        snapshot = json.loads(snapshot_raw)
        if snapshot.get('project') != config['project'] or snapshot.get('program') != config['program_path']:
            raise ValueError('snapshot project/program does not match target configuration')
    # An RTTI-less maximal run may concatenate several classes. Constructors
    # provide real slot-zero boundaries even when the compiler omitted RTTI.
    anchors = set()
    initializer_ranges = []
    rdata_lo, rdata = image.sections['.rdata']
    for caller, instructions, _ in function_instructions(image, starts, functions):
        initializer_ranges.extend(crt_initializer_ranges(image, caller, instructions, functions))
        for _, _, mnemonic, operands in instructions:
            parts = operands.split(', ')
            if mnemonic == 'mov' and len(parts) == 2 and memory(parts[0]) and IMM.fullmatch(parts[1]):
                value = int(parts[1], 0)
                if value % 4 == 0 and rdata_lo <= value < rdata_lo + len(rdata):
                    anchors.add(value)
    excluded = {(number(r['start']), number(r['end_exclusive'])) for r in initializer_ranges}
    tables, bases = discover_vtables(image, slot_targets, anchors, excluded)
    slots = collections.defaultdict(set)
    edges = []
    for address, table in sorted(tables.items()):
        for slot, target in enumerate(table['_slots']):
            if target not in slot_targets:
                continue
            slots[slot].add(target)
            edges.append({'caller': hex(address), 'callee': hex(target), 'site': hex(address + 4 * slot),
                          'kind': 'vslot', 'detail': f"{table['class'] or '<unknown>'}::slot {slot}"})
    lua_tables = discover_lua(image, start_set, functions)
    counters = collections.Counter()
    instructions_at = {}
    lua_refs = collections.defaultdict(set)
    direct_calls = collections.defaultdict(set)
    # Known symbols come from the snapshot, not fixed addresses in this tool.
    lua_register = {a for a, f in functions.items() if f['name'] in ('luaL_register', 'lua_register',
                    'BSP_LuaMachine_RegisterGlobalFunction')}
    lua_closure = {a for a, f in functions.items() if f['name'] == 'lua_pushcclosure'}
    lua_setfield = {a for a, f in functions.items() if f['name'] == 'lua_setfield'}
    lua_open = {a for a, f in functions.items() if f['name'] == 'lua_call'}
    lua_reader = {a for a, f in functions.items() if f['name'] == 'BSP_LuaReader_ReadField'}
    # Array-field references can point at either member, including indexed loads.
    lua_members = {row + offset: address for address, entries in lua_tables.items()
                   for row, _, _ in entries for offset in (0, 4)}
    for caller, instructions, has_ranges in function_instructions(image, starts, functions):
        counters['functions_swept'] += 1
        counters['functions_with_body_ranges' if has_ranges else 'functions_with_linear_ranges'] += 1
        counters['instructions'] += len(instructions)
        for target, site, name in inline_lua(image, instructions, start_set, lua_closure, lua_setfield, lua_register):
            edges.append({'caller': hex(caller), 'callee': hex(target), 'site': hex(site), 'kind': 'lua', 'detail': name})
            counters['lua_inline_bindings'] += 1
        # A taken branch joins a basic block even when the source is outside it.
        joins = {int(op, 0) for _, _, m, op in instructions
                 if (m.startswith('j') or m.startswith('loop')) and IMM.fullmatch(op)}
        joins.update(current[0] for previous, current in zip(instructions, instructions[1:])
                     if previous[0] + previous[1] != current[0])
        state = ReceiverState()
        for site, _, mnemonic, operands in instructions:
            if site in joins:
                state.reset()
            if mnemonic in ('call', 'jmp') and IMM.fullmatch(operands):
                direct_calls[caller].add(int(operands, 0))
                if int(operands, 0) in lua_reader:
                    counters['lua_property_reader_sites_ignored'] += 1
            if mnemonic in ('push', 'mov', 'lea'):
                for match in re.finditer(r'0x[0-9a-f]+', operands):
                    address = lua_members.get(int(match[0], 0))
                    if address is not None:
                        lua_refs[address].add(caller)
            mem = memory(operands) if mnemonic in ('call', 'jmp') else None
            method = state.method(operands) if mnemonic in ('call', 'jmp') and REG.fullmatch(operands) else None
            if mem or method:
                counters['indirect_memory_sites' if mem else 'indirect_loaded_register_sites'] += 1
                table_address = method[1] if method else state.table(mem[0])
                disp = method[2] if method else mem[1]
                reg = mem[0] if mem else None
                if disp % 4 == 0 and reg not in ('esp', 'ebp'):
                    slot = disp // 4
                    candidates = slots.get(slot, set())
                    narrowed = table_address is not None
                    if narrowed:
                        allowed = compatible_tables(table_address, tables, bases)
                        candidates = {tables[a]['_slots'][slot] for a in allowed
                                      if slot < len(tables[a]['_slots']) and tables[a]['_slots'][slot] in slot_targets}
                    if len(candidates) > args.max_candidates:
                        counters['vcall_sites_skipped_fanout'] += 1
                        counters['vcall_edges_skipped_fanout'] += len(candidates)
                    elif candidates:
                        counters['vcall_sites'] += 1
                        counters['vcall_sites_narrowed' if narrowed else 'vcall_sites_slot_only'] += 1
                        instructions_at[site] = f'{mnemonic} {operands}'
                        detail = f'slot {slot}; {mnemonic}; ' + (
                            f"visible vtable {hex(table_address)} and RTTI descendants" if narrowed else 'slot-only candidate')
                        if method:
                            detail += f'; method load {hex(method[3])} {method[4]}'
                        for target in sorted(candidates):
                            edges.append({'caller': hex(caller), 'callee': hex(target), 'site': hex(site),
                                          'kind': 'vcall?', 'detail': detail})
                    else:
                        counters['vcall_sites_without_candidates'] += 1
            state.step(site, mnemonic, operands, tables)
    # Recognize one-level game helpers by their actual Lua API call combination.
    # Do not walk arbitrary transitive callers: an unrelated array in a high-level
    # initializer must not become a binding just because it eventually invokes Lua.
    lua_register |= {a for a, calls in direct_calls.items()
                     if calls & lua_closure and calls & lua_setfield}
    lua_evidence = []
    for address, entries in sorted(lua_tables.items()):
        refs = lua_refs.get(address, set())
        registering = {a for a in refs if direct_calls[a] & lua_register or
                       (direct_calls[a] & lua_closure and direct_calls[a] & lua_open)}
        if not registering:
            counters['lua_tables_unconfirmed'] += 1
            continue
        counters['lua_tables'] += 1
        lua_evidence.append({'table': hex(address), 'bindings': len(entries),
                             'registering_functions': [hex(a) for a in sorted(registering)]})
        for row, name, target in entries:
            edges.append({'caller': hex(address), 'callee': hex(target), 'site': hex(row + 4),
                          'kind': 'lua', 'detail': name})
    counts = collections.Counter(e['kind'] for e in edges)
    for key in ('vcall_sites_narrowed', 'vcall_sites_slot_only', 'vcall_sites_skipped_fanout',
                'vcall_edges_skipped_fanout', 'vcall_sites_without_candidates', 'lua_inline_bindings'):
        counters.setdefault(key, 0)
    rng = random.Random(0x18bc)
    virtual = [e for e in edges if e['kind'] == 'vcall?']
    samples = [{**e, 'instruction': instructions_at[number(e['site'])]} for e in
               rng.sample(virtual, min(10, len(virtual)))]
    meta = {'utc': datetime.now(timezone.utc).isoformat(), 'binary': config['binary'],
            'disk_sha256': sha256(image.raw), 'config': str(args.config), 'config_sha256': sha256(config_raw),
            'functions': str(functions_path), 'functions_sha256': sha256(functions_raw),
            'snapshot_sha256': sha256(snapshot_raw) if snapshot_raw else None,
            'tool_sha256': sha256(Path(__file__).read_bytes()), 'capstone_version': capstone_version,
            'pefile_version': pefile.__version__,
            'functions_json_entries': len(records), 'max_candidates': args.max_candidates,
            'counts': {kind: counts[kind] for kind in ('vslot', 'vcall?', 'lua')},
            'vtables': len(tables), 'vtables_with_rtti': sum(t['class'] is not None for t in tables.values()),
            'vtables_without_rtti': sum(t['class'] is None for t in tables.values()),
            'vtables_at_visible_store_anchors': sum(a in anchors for a in tables),
            'vtable_unindexed_executable_slots': sum(s not in slot_targets for t in tables.values() for s in t['_slots']),
            'vtable_slots_in_undefined_code': sum(s in undefined_starts for t in tables.values() for s in t['_slots']),
            'non_vtable_pointer_ranges': initializer_ranges,
            'crt_initializer_slots_excluded': sum(image.dword(a) in start_set for lo, hi in excluded for a in range(lo, hi, 4)),
            **dict(counters), 'lua_table_candidates': len(lua_tables),
            'lua_validation': 'ASCII names (empty only for luaopen_*), exact snapshot starts; function-null tables referenced by registration/helper callers, or immediate handlers with adjacent pushcclosure/setfield',
            'lua_tables_evidence': lua_evidence,
            'vcall_validation_samples_seed': '0x18bc', 'vcall_validation_samples': samples,
            'limitations': ['vcall? is slot compatibility, not receiver-type or runtime proof',
                            'RTTI-less function-pointer runs can include non-vtables',
                            'unindexed executable slots preserve offsets but receive no edges',
                            'linear ranges used when snapshot body_ranges is absent'],
            'seconds': round(time.monotonic() - began, 2)}
    public_tables = [{k: v for k, v in t.items() if not k.startswith('_')} for t in tables.values()]
    payload = {'meta': meta, 'edges': edges, 'vtables': public_tables}
    args.output.mkdir(parents=True, exist_ok=True)
    destination = args.output / 'callgraph_indirect.json'
    # Readers see either the complete previous export or the complete new one.
    with tempfile.NamedTemporaryFile(mode='w', encoding='utf-8', dir=args.output,
                                     prefix='.callgraph_indirect-', suffix='.tmp', delete=False) as stream:
        temporary = Path(stream.name)
        try:
            json.dump(payload, stream, separators=(',', ':'))
        except BaseException:
            stream.close()
            temporary.unlink(missing_ok=True)
            raise
    try:
        os.replace(temporary, destination)
    finally:
        temporary.unlink(missing_ok=True)
    print(json.dumps({k: v for k, v in meta.items() if k != 'vcall_validation_samples'}, indent=2))
    print(f'Wrote {destination}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--config', type=Path, default=ROOT / 'config/target.json')
    parser.add_argument('--output', type=Path, default=exports_dir(), help='shared exports directory containing functions.json')
    parser.add_argument('--max-candidates', type=int, default=64, help='skip a dispatch site above this unique-target count')
    args = parser.parse_args()
    if args.max_candidates < 1:
        parser.error('--max-candidates must be positive')
    export(args)


if __name__ == '__main__':
    main()
