"""Build config/ghidra_tags.json from the library inventory reports.

The ledger tags functions that are library code, compiler-generated helpers or template
instantiations so they can be excluded from reconstruction target selection and named
systematically in Ghidra by tools/ghidra_tag.py. Names produced here are inventory tags or
stock library names, not recovered symbols; medium/low confidence library names carry a
`__prov` suffix. Only functions still carrying Ghidra default `FUN_` names are tagged.
"""
import argparse
import json
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from ledger import load_names, load_reconstruction, load_tags, write_tags  # noqa: E402

ROOT = Path(__file__).resolve().parents[1]
INV = ROOT / 'reports/library_inventory'

# Precedence is the order of this list: the first category that claims an address wins.
CATEGORIES = [
    'lua511', 'zlib121', 'rtti_vslot', 'crt_unmatched', 'static_init',
    'block_dyn', 'block_telemetry', 'block_telemetry_tmpl', 'block_metrics_wrapper', 'block_pipe_ipc',
    'stl_throw_site', 'cg_scalar_deleting_dtor', 'cg_vector_deleting_dtor', 'cg_array_ctor_helper',
    'cg_adjustor_thunk', 'cg_static_dtor_stub', 'stl_instantiation', 'trivial_body',
    'stl_probable',
    # Shape-review categories (docs/CC13_TEMPLATE_COPY_SWEEP.md); never derived here, kept as applied rows.
    'stl_shape', 'tmpl_shape',
]
PLATE_CATEGORIES = {'lua511', 'zlib121', 'rtti_vslot', 'crt_unmatched', 'static_init',
                    'block_dyn', 'block_telemetry', 'block_telemetry_tmpl', 'block_metrics_wrapper',
                    'block_pipe_ipc'}


def addr(value):
    return f'{int(value, 16):08x}'


PROVISIONAL = '__prov'


def sanitize(name, limit=64):
    """Ghidra-safe identifier; keeps a leading underscore (stock `_tr_*` zlib names) and the provisional suffix."""
    provisional = name.endswith(PROVISIONAL)
    if provisional:
        name = name[:-len(PROVISIONAL)]
    lead = '_' if name.startswith('_') else ''
    name = re.sub(r'[^A-Za-z0-9_]+', '_', name).strip('_')
    name = re.sub(r'_+', '_', name)
    name = lead + name[:limit].rstrip('_')
    return name + PROVISIONAL if provisional else name


EH_VECTOR_DTOR_ITERATOR = '00bf7c6e'
CTOR_ITERATORS = ('00403560', '00453ed0', '00bf7cd1', '00bf9a96')   # vector / copy / eh variants
ARRAY_CTOR_HELPER_MAX_INSTRUCTIONS = 20


def has_shape(category, lines):
    """The compiler-generated shape the category's name claims, read from the exported listing.

    The inventory lists tagged anything that CALLED an iterator, so real class destructors and
    constructors were named CG_vector_deleting_dtor / CG_array_ctor_helper (cc13-cgretag census:
    none of the 38 / 15 still so named had the shape). A deleting destructor tests its flag
    argument: bit 1 (delete) for scalar, bit 2 (array) for vector, and the vector form calls the
    eh vector destructor iterator; an array ctor helper is a short body around a ctor iterator.
    """
    def test(bit):
        return any(re.search(rf'\bTEST\b.*,0x{bit}\s*$', line) for line in lines)

    def calls(*targets):
        return any(re.search(rf'\bCALL 0x{t}\b', line) for line in lines for t in targets)

    if category == 'cg_scalar_deleting_dtor':
        return 1 in flag_bits_tested(lines)
    if category == 'cg_vector_deleting_dtor':
        return test(2) and calls(EH_VECTOR_DTOR_ITERATOR)
    if category == 'cg_array_ctor_helper':
        return len(lines) <= ARRAY_CTOR_HELPER_MAX_INSTRUCTIONS and calls(*CTOR_ITERATORS)
    return True


_REG_FAMILY = {r: f for f, regs in {
    'A': ('EAX', 'AX', 'AL', 'AH'), 'B': ('EBX', 'BX', 'BL', 'BH'), 'C': ('ECX', 'CX', 'CL', 'CH'),
    'D': ('EDX', 'DX', 'DL', 'DH'), 'SI': ('ESI', 'SI'), 'DI': ('EDI', 'DI'), 'BP': ('EBP', 'BP')}.items() for r in regs}


def flag_bits_tested(lines):
    """The bits a body tests on its incoming flag argument (the first stack argument, [ESP+4] at entry).

    A bit test anywhere in the listing is not enough: 004D9830 tests bits 1 and 2 of a local EH-cleanup
    byte (cc13-scalarshape). The listing is walked in address order. The frame depth is the bytes the
    prologue pushes or reserves (register saves, the EH prolog, SUB ESP); later PUSHes are call arguments,
    dropped at the CALL (a following ADD ESP is the same cleanup) and at RET the frame is restored for
    the next block. A TEST of [ESP + frame + arguments + 4] (or [EBP + 8] style after MOV EBP,ESP), or of
    a register loaded from that slot before it is overwritten, counts.
    """
    frame, args, ebp_depth, flag_regs, bits = 0, 0, None, set(), set()
    in_prologue, cleanup, prologue_frame = True, 0, 0
    imm = re.compile(r'-?0x[0-9a-f]+')

    def slot(operand):
        m = re.fullmatch(r'(?:byte|dword) ptr \[(ESP|EBP)(?: \+ 0x([0-9a-f]+))?\]', operand.strip())
        if not m:
            return False
        off = int(m.group(2) or '0', 16)
        if m.group(1) == 'ESP':
            return off == frame + args + 4
        return ebp_depth is not None and off == ebp_depth + 4  # EBP = ESP after ebp_depth bytes pushed

    parsed = []
    for line in lines:
        text = line.split(':', 1)[1].strip() if ':' in line[:10] else line.strip()
        op, _, rest = text.partition(' ')
        operands = [o.strip() for o in rest.split(',')] if rest else []
        parsed.append((op, operands, operands[0] if operands else ''))

    def saved_register(i, reg):
        """A PUSH of a callee-saved register is a save, not an argument, when the register is
        overwritten before the next CALL (00419F30 saves EDI in a loop block, then loads it)."""
        for op, operands, dest in parsed[i + 1:]:
            if op == 'CALL' or op.startswith('RET'):
                return False
            if dest == reg and op not in ('CMP', 'TEST', 'PUSH'):
                return True
        return False

    previous, written = '', set()
    for i, (op, operands, dest) in enumerate(parsed):
        if op == 'TEST' and len(operands) == 2 and imm.fullmatch(operands[1]):
            value = int(operands[1], 16)
            if slot(dest) or _REG_FAMILY.get(dest) in flag_regs:
                bits.update(b for b in (1, 2) if value & b)
            in_prologue = False
            continue
        if op == 'MOV' and len(operands) == 2 and dest in _REG_FAMILY and slot(operands[1]):
            flag_regs.add(_REG_FAMILY[dest])
            continue
        if op == 'MOV' and dest == 'EBP' and operands[1:] == ['ESP']:
            ebp_depth = frame + args
            continue
        # prologue saves: callee-saved registers, and the EH prolog's -1, handler and old FS:[0] (EAX)
        # (a register already written is an argument: 006C23D0 is MOV ESI,ECX; PUSH ESI; CALL)
        prologue_push = op == 'PUSH' and ((dest in ('EBX', 'ESI', 'EDI', 'EBP') and _REG_FAMILY[dest] not in written)
                                          or dest == '-0x1'
                                          or (dest == 'EAX' and 'FS:' in previous)
                                          or (dest == 'ECX' and parsed[i + 1:i + 2] and parsed[i + 1][0] != 'CALL')  # slot reservation
                                          or imm.fullmatch(dest) and int(dest, 16) >= 0x00c00000)
        previous = ','.join(operands)
        if op == 'PUSH':
            if (in_prologue and prologue_push) or (dest in ('EBX', 'ESI', 'EDI', 'EBP') and saved_register(i, dest)):
                frame += 4
            else:
                args += 4
                in_prologue = False
        elif op == 'POP':
            if args:
                args -= 4
            else:
                frame -= 4
            flag_regs.discard(_REG_FAMILY.get(dest))
        elif op in ('SUB', 'ADD') and dest == 'ESP' and len(operands) == 2 and imm.fullmatch(operands[1]):
            n = int(operands[1], 16)
            if op == 'SUB':
                if in_prologue:
                    frame += n
                else:
                    args += n
            elif n <= cleanup:
                cleanup -= n  # cdecl cleanup of arguments already dropped at the CALL
            elif args >= n:
                args -= n
            else:
                frame -= n - args
                args = 0
        elif op == 'CALL':
            args, cleanup, in_prologue = 0, cleanup + args, False
            flag_regs.discard('A'); flag_regs.discard('C'); flag_regs.discard('D')
        elif op.startswith('RET'):
            frame, args, cleanup = prologue_frame, 0, 0
        elif dest in _REG_FAMILY and op not in ('CMP', 'TEST', 'PUSH'):
            flag_regs.discard(_REG_FAMILY[dest])
        if dest in _REG_FAMILY and op not in ('CMP', 'TEST', 'PUSH'):
            written.add(_REG_FAMILY[dest])
        if op not in ('PUSH', 'MOV', 'SUB', 'LEA', 'XOR') and not op.startswith('RET'):
            in_prologue = False
        if in_prologue:
            prologue_frame = frame
    return bits


SHAPE_CATEGORIES = ('cg_scalar_deleting_dtor', 'cg_vector_deleting_dtor', 'cg_array_ctor_helper')


def shape_ok(category, address, exports):
    """False when a CG_ shape category lacks its shape, or its listing is not exported."""
    if category not in SHAPE_CATEGORIES:
        return True
    listing = exports / address / 'assembly.txt'
    return listing.exists() and has_shape(category, listing.read_text().splitlines())


def in_range(a, start, end):
    return int(start, 16) <= int(a, 16) < int(end, 16)


def source_function(source):
    """Turn 'file.c:func (+ notes)' from an inventory map into a bare function identifier."""
    text = source.split(':', 1)[1] if ':' in source else source
    outside = re.sub(r'\([^)]*\)', ' ', text)
    tokens = re.findall(r'[A-Za-z_][A-Za-z0-9_]*', outside)
    if not tokens:
        # Only a parenthetical note such as "(errorlimit inlined)": keep its words as the name.
        tokens = re.findall(r'[A-Za-z_][A-Za-z0-9_]*', text)
        return '_'.join(tokens[:2]) if tokens else 'unnamed'
    if len(tokens[0]) < 6 and len(tokens) > 1:
        return f'{tokens[0]}_{tokens[1]}'
    return tokens[0]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--legacy-json', help='Also write the flat legacy JSON list to this path')
    args = parser.parse_args()

    functions = {row['address']: row for row in json.loads((__import__('workspace').exports_dir() / 'functions.json').read_text())}
    name_addresses = {}
    for row in functions.values():
        name_addresses.setdefault(row['name'], set()).add(row['address'])
    reviewed_names = {row['address']: row['name'] for row in load_names()}
    reviewed = set(reviewed_names)
    ledger = load_reconstruction()
    reviewed |= {row['address'] for row in ledger['functions']} | {row['address'] for row in ledger.get('fragments', [])}
    # Existing tag records: a function already renamed to its tag (or created by the tag run) stays taggable,
    # and records the reports no longer derive are kept so the ledger remains the record of applied tags.
    existing_tags = {row['address']: row for row in load_tags()}

    lua = json.loads((INV / 'lua.json').read_text())
    zlib = json.loads((INV / 'zlib_and_borrowed.json').read_text())
    crt = json.loads((INV / 'crt_stl.json').read_text())
    rtti = json.loads((INV / 'rtti_middleware.json').read_text())
    templates = json.loads((INV / 'templates_and_generated.json').read_text())
    rtti_classes = json.loads((INV / 'rtti_classes.json').read_text())

    claimed = {}
    entries = []
    skipped = {'not_fun': 0, 'thunk': 0, 'reviewed': 0, 'duplicate': 0, 'no_function': 0, 'no_shape': 0}
    exports = __import__('workspace').exports_dir() / 'functions'

    def claim(address, name, category, confidence, evidence, source, create=False, action='rename'):
        address = addr(address)
        if address in claimed:
            skipped['duplicate'] += 1
            return
        if not shape_ok(category, address, exports):
            skipped['no_shape'] += 1
            return
        if address in reviewed:
            skipped['reviewed'] += 1
            return
        row = functions.get(address)
        previous = existing_tags.get(address)
        if row is None:
            if not create and previous is None:
                skipped['no_function'] += 1
                return
        else:
            if row.get('isThunk'):
                skipped['thunk'] += 1
                return
            already_tagged = previous is not None and row['name'] in (previous.get('name'), f'FUN_{address}')
            if not row['name'].startswith('FUN_') and not already_tagged:
                skipped['not_fun'] += 1
                return
            create = False
        if action == 'rename':
            name = sanitize(name)
            # Preserve an applied name even when snapshot thunks/aliases share it.
            # Only a new assignment must avoid names reserved by other addresses.
            already_named = row is not None and row['name'] == name
            taken_elsewhere = name_addresses.get(name, set()) - {address}
            if not already_named and (taken_elsewhere or any(e['name'] == name for e in entries[-2000:])):
                name = f'{name}_{address}'
        claimed[address] = category
        entries.append({'address': address, 'name': name if action == 'rename' else '', 'category': category,
                        'confidence': confidence, 'action': action, 'create': bool(create),
                        'evidence': evidence[:400], 'source': source})

    # 1. Lua 5.1.1: stock source names; medium/low confidence marked provisional.
    for m in lua['map']:
        func = source_function(m['source'])
        name = func if m['confidence'] == 'high' else f'{func}__prov'
        claim(m['address'], name, 'lua511', m['confidence'],
              f"{m['source']}; {m.get('evidence', '')}", 'reports/library_inventory/lua.json',
              create=not m.get('ghidra_defined', True))

    # 2. zlib 1.2.1: stock source names.
    zl = next(lib for lib in zlib['libraries'] if lib['name'].startswith('zlib'))
    for m in zl['map']:
        func = source_function(m['source'])
        name = func if m['confidence'] == 'high' else f'{func}__prov'
        claim(m['address'], name, 'zlib121', m['confidence'],
              f"{m['source']}; {m.get('evidence', '')}", 'reports/library_inventory/zlib_and_borrowed.json',
              create=m.get('ghidra_defined') is False)

    # 3. RTTI vtable slots for the in-house/middleware namespaces (not std/undname).
    for cls in rtti_classes:
        ns = cls.get('ns', '')
        if ns not in ('Dyn', 'iostdnet', 'iometrics', 'Mit'):
            continue
        nice = sanitize(cls['nice'], limit=48)
        for col in cls.get('cols', []):
            for vt in col.get('vtables', []):
                for slot, method in enumerate(vt.get('methods', [])):
                    a = f'{method:08x}'
                    claim(a, f'{nice}_vslot{slot}', 'rtti_vslot', 'high',
                          f"vtable {vt['vtable']:08x} slot {slot} of {cls['nice']} (RTTI TypeDescriptor {cls['td']:08x})",
                          'reports/library_inventory/rtti_classes.json')

    # 4. CRT cluster leftovers and .text$yc/$yd initializers.
    for rng in crt['ranges']:
        if rng.get('reconstruct', True):
            continue
        label = rng['label']
        if label.startswith('MSVC 2005 CRT'):
            cat, prefix, conf = 'crt_unmatched', 'LIBCRT_unmatched', 'high'
        elif label.startswith('.text$yc'):
            cat, prefix, conf = 'static_init', 'CG_static_init', 'high'
        else:
            continue
        for a, row in functions.items():
            if in_range(a, rng['start'], rng['end']):
                claim(a, f'{prefix}_{a}', cat, conf, f"inside {label} [{rng['start']},{rng['end']})",
                      'reports/library_inventory/crt_stl.json')

    # 5. Block ranges from the RTTI/middleware inventory.
    block_specs = [
        ('Dyn physics', 'block_dyn', 'DYN_physics', 'high', lambda r: r['start'] == '00c30930'),
        ('iostdnet + iometrics', 'block_telemetry', 'TELEMETRY_iostdnet', 'high', lambda r: r['start'] == '00a4d610'),
        ('iostdnet + iometrics', 'block_telemetry_tmpl', 'TELEMETRY_tmpl', 'high', lambda r: r['start'] == '0094cab0'),
        ('iostdnet + iometrics', 'block_metrics_wrapper', 'METRICS_wrapper', 'medium', lambda r: r['start'] == '00750820'),
        ('named-pipe', 'block_pipe_ipc', 'PIPEIPC', 'medium', lambda r: r['start'] == '00a5de34'),
    ]
    for block in rtti['blocks']:
        for key, cat, prefix, conf, pick in block_specs:
            if key.lower() not in block['name'].lower():
                continue
            for rng in block.get('ranges', []):
                if not pick(rng):
                    continue
                for a in functions:
                    if in_range(a, rng['start'], rng['end']):
                        claim(a, f'{prefix}_{a}', cat, conf,
                              f"inside {block['name']} range [{rng['start']},{rng['end']}): {rng.get('note', '')}",
                              'reports/library_inventory/rtti_middleware.json')

    # 6. Template / compiler-generated address lists (precision per tagging rules).
    lists = templates['address_lists']
    mech = [
        ('throw_sites', 'stl_throw_site', 'STL_xlen_throw', 'high', 'pushes one of the six Dinkumware STL throw strings (precision ~1.0)'),
        ('deleting_dtors', 'cg_scalar_deleting_dtor', 'CG_scalar_deleting_dtor', 'high', 'calls operator delete, tests [esp+4|8]&1, ret 4 (scalar deleting destructor shape)'),
        ('vector_deleting_dtors', 'cg_vector_deleting_dtor', 'CG_vector_deleting_dtor', 'high', 'tests its flag &2 and calls eh_vector_destructor_iterator 00bf7c6e (vector deleting destructor shape)'),
        ('array_ctor_helpers', 'cg_array_ctor_helper', 'CG_array_ctor_helper', 'high', 'body <= 20 instructions around a vector/eh_vector constructor iterator call'),
        ('adjustor_thunks', 'cg_adjustor_thunk', 'CG_adjustor_thunk', 'high', 'this-adjustor / vcall thunk shape'),
        ('static_dtor_stubs', 'cg_static_dtor_stub', 'CG_static_dtor_stub', 'high', 'atexit-registered static destructor stub'),
        ('scl_callers_le96', 'stl_instantiation', 'STL_inst', 'high', 'calls _invalid_parameter_noinfo 00bf6713 (_SECURE_SCL hook) and body <= 96 bytes (precision ~0.95)'),
        ('trivial_pattern', 'trivial_body', 'TRIV_body', 'high', 'trivial body pattern (ret / getter / setter / jmp) <= 32 bytes'),
    ]
    for key, cat, prefix, conf, why in mech:
        for a in lists.get(key, []):
            a = addr(a)
            claim(a, f'{prefix}_{a}', cat, conf, why, 'reports/library_inventory/templates_and_generated.json')
    for a in lists.get('scl_callers_97_256', []):
        claim(a, '', 'stl_probable', 'medium',
              'calls _invalid_parameter_noinfo 00bf6713 with body 97-256 bytes (precision ~0.6-0.8); bookmark only',
              'reports/library_inventory/templates_and_generated.json', action='bookmark')

    derived = {e['address'] for e in entries}
    # Keep applied tags the reports no longer derive, but never for an address that has since
    # received a reviewed name or a reconstruction record: the review supersedes the tag.
    # A review that adopted the tag's name (library review passes such as CC13_EMBEDDED_LUA_LIBRARY and
    # CC13_DYN_LIBRARY, or a ledger name copied from an inventory tag) supersedes nothing: keep that tag
    # while the reviewed name is still the tag's name.
    def review_pass_row(address, row):
        return bool(row.get('name')) and reviewed_names.get(address) == row['name']
    # A kept CG_ shape row must still have its shape, so the old iterator-caller tags do not return.
    kept = [row for address, row in existing_tags.items()
            if address not in derived and (address not in reviewed or review_pass_row(address, row))
            and shape_ok(row['category'], address, exports)]
    entries.extend(kept)
    entries.sort(key=lambda e: int(e['address'], 16))
    shards, evidence_ids = write_tags(entries)
    if args.legacy_json:
        Path(args.legacy_json).write_text(json.dumps(entries, indent=1) + '\n')
    counts = {}
    for e in entries:
        counts[e['category']] = counts.get(e['category'], 0) + 1
    print(f'wrote {len(entries)} entries to config/tags/ ({shards} shards, {evidence_ids} evidence texts)')
    for cat in CATEGORIES:
        if cat in counts:
            print(f'  {counts[cat]:6d}  {cat}')
    print('skipped:', skipped)
    print('create_function needed:', sum(e['create'] for e in entries))


if __name__ == '__main__':
    main()
