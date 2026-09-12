"""Check a worker report's call-site rows against the live Ghidra function bodies and call graph.

Usage: python tools/verify_report_calls.py reports/<name>.json [...]

For every row that carries an `address` (call site) and a `native` (callee) in any list of the
report (`host_steps`, `path_steps`, `steps`, `host_methods`, ...), the script checks:

  1. the callee is the start of a Ghidra function (or a thunk that is one);
  2. the call site lies inside some Ghidra function F (live `get_function_by_address`);
  3. the live listing contains `CALL <callee>` at that exact site, or `JMP <callee>` when the
     row explicitly declares `kind: "tail_jump"`. A caller/callee graph edge
     alone cannot validate an instruction address elsewhere in the same function.

Rows that name a `function` (or `caller`) are also checked to be that F. Vtable slots written as
`<addr>+vtableNN` or `<addr>+<hex>` are reported as `indirect` and skipped. The script is read-only
for Ghidra; it exits 1 when any row fails so an integration can gate on it.

docs/WORKER_VERIFICATION_CHECKLIST.md rules 1 and 2 are the ones this enforces.
"""
import json
import re
import sqlite3
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
from ghidra_export import Client  # noqa: E402

HEX = re.compile(r'^(?:0x)?([0-9a-fA-F]{6,8})$')
BODY = re.compile(r'body:?\s*([0-9a-fA-F]{8})\s*-\s*([0-9a-fA-F]{8})', re.IGNORECASE)
FN_AT = re.compile(r'Function:\s*(\S+)\s+at\s+([0-9a-fA-F]{8})')


def parse_addr(value):
    if isinstance(value, int):
        return value
    if not isinstance(value, str):
        return None
    m = HEX.match(value.strip())
    return int(m.group(1), 16) if m else None


def rows_with_calls(node, path='report'):
    if isinstance(node, dict):
        if 'address' in node and 'native' in node:
            yield path, node
        for k, v in node.items():
            yield from rows_with_calls(v, f'{path}.{k}')
    elif isinstance(node, list):
        for i, v in enumerate(node):
            yield from rows_with_calls(v, f'{path}[{i}]')


class Index:
    def __init__(self, db):
        self.c = sqlite3.connect(str(db)) if db.exists() else None

    def is_function(self, addr):
        if not self.c:
            return None
        return self.c.execute('select 1 from functions where address=?', (addr,)).fetchone() is not None

    def calls(self, caller, callee):
        if not self.c:
            return None
        return self.c.execute('select 1 from calls where caller=? and callee=?', (caller, callee)).fetchone() is not None


class Live:
    def __init__(self):
        cfg = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
        self.client = Client(cfg)
        self.client.verify()
        self.cache = {}
        self.listings = {}

    def containing(self, addr):
        """Return (start, end_inclusive, name) of the function containing addr, or None."""
        key = addr
        if key in self.cache:
            return self.cache[key]
        try:
            text = self.client.get('get_function_by_address', address=f'{addr:08x}')
        except Exception as exc:  # bridge errors are reported, not fatal
            text = f'error: {exc}'
        result = None
        if isinstance(text, dict):
            text = json.dumps(text)
        if 'No function' not in text and 'error' not in text.lower():
            m = BODY.search(text)
            name = None
            fm = FN_AT.search(text)
            if fm:
                name = fm.group(1)
            if m:
                result = (int(m.group(1), 16), int(m.group(2), 16), name)
            else:
                # fall back to the prototype endpoint, which prints the body range
                try:
                    proto = self.client.get('get_function_signature', address=f'{addr:08x}')
                    pm = BODY.search(proto if isinstance(proto, str) else json.dumps(proto))
                    if pm:
                        result = (int(pm.group(1), 16), int(pm.group(2), 16), name)
                except Exception:
                    result = None
        self.cache[key] = result
        return result

    def listing_calls(self, fn_start, site, callee, mnemonic='CALL'):
        """'direct' when the site is the requested CALL/JMP <callee>, 'indirect' for a transfer through a
        register or memory operand (a virtual the report resolved), 'other' when the site is a
        CALL to a different immediate or not a CALL, None when the listing is unavailable."""
        if fn_start not in self.listings:
            try:
                self.listings[fn_start] = self.client.get(
                    'disassemble_function', address=f'{fn_start:08x}')
            except Exception:
                self.listings[fn_start] = None
        text = self.listings[fn_start]
        if text is None:
            return None
        if not isinstance(text, str):
            text = json.dumps(text)
        line = re.search(rf'^\s*0?x?{site:08x}\b.*$', text, re.IGNORECASE | re.MULTILINE)
        if not line:
            return 'other'
        ins = line.group(0)
        if not re.search(rf'\b{mnemonic}\b', ins, re.IGNORECASE):
            return 'other'
        if re.search(rf'{callee:08x}', ins, re.IGNORECASE):
            return 'direct'
        if re.search(rf'\b{mnemonic}\b\s+(?:dword\s+ptr\s+)?\[|\b{mnemonic}\b\s+E[A-D]X|\b{mnemonic}\b\s+E[SD]I|\b{mnemonic}\b\s+E[BS]P', ins, re.IGNORECASE):
            return 'indirect'
        return 'other'


def check_report(path, index, live):
    data = json.loads(Path(path).read_text(encoding='utf-8'))
    failures = 0
    checked = 0
    for where, row in rows_with_calls(data):
        site = parse_addr(row.get('address'))
        native_raw = row.get('native')
        callee = parse_addr(native_raw)
        if site is None:
            continue
        if callee is None:
            print(f'  indirect  {where}: native={native_raw!r} (not checked)')
            continue
        if site == callee:
            continue  # an entry row (the routine itself), not a call site
        checked += 1
        problems = []
        body = live.containing(callee)
        if body is None:
            problems.append(f'callee {callee:08x} is not a Ghidra function')
        elif body[0] != callee:
            problems.append(f'callee {callee:08x} is inside {body[0]:08x}, not a function start')
        fn = live.containing(site)
        if fn is None:
            problems.append(f'call site {site:08x} is in no Ghidra function')
        else:
            start, end, name = fn
            claimed = parse_addr(row.get('function') or row.get('caller'))
            if claimed is not None and claimed != start:
                problems.append(f'call site {site:08x} is inside {start:08x} ({name}), row claims {claimed:08x}')
            mnemonic = 'JMP' if row.get('kind') == 'tail_jump' else 'CALL'
            seen = live.listing_calls(start, site, callee, mnemonic)
            if seen == 'indirect':
                print(f'  indirect  {where}: {site:08x} uses {mnemonic} through a register or memory operand; '
                      f'the report resolves it to {callee:08x} (not verifiable here)')
            elif seen == 'other':
                problems.append(f'{site:08x} in {start:08x} is not a {mnemonic} to {callee:08x} (live listing)')
            elif seen is None:
                problems.append(f'{site:08x} -> {callee:08x}: live listing unavailable')
        if problems:
            failures += 1
            print(f'  FAIL      {where}: ' + '; '.join(problems))
    print(f'{path}: {checked} call rows checked, {failures} failed')
    return failures


def main(argv):
    if not argv:
        print(__doc__)
        return 2
    index = Index(ROOT / 'local/bsp_index.sqlite')
    live = Live()
    total = 0
    for p in argv:
        total += check_report(p, index, live)
    return 1 if total else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
