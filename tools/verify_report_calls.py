"""Check a worker report's call-site rows against the live Ghidra function bodies and call graph.

Usage: python tools/verify_report_calls.py reports/<name>.json | docs/<name>.md [...]

A .md report is parsed for prose call claims instead (see check_markdown); claims it cannot
pair into a site or a caller are listed as unchecked, not failed.

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
ENTRY = re.compile(r'\bEntry:\s*([0-9a-fA-F]{8})', re.IGNORECASE)


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
        """Return (entry, body_end_inclusive, name) for the containing function.

        A backward branch can put body bytes below the ABI entry. In particular,
        _strchr enters at BF86F0 but owns a return block at BF86E0. Body minimum
        is therefore neither the callee entry nor the caller's function identity.
        """
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
        # The bridge reports a failure either as "No function ..." or as text this
        # client prefixed with "error:". Testing for the bare word "error" anywhere
        # in the reply rejects any function whose NAME contains it: the recovered
        # BSP_ShipAi_RudderFromHeadingError at 009da250 was reported as "not a
        # Ghidra function" for exactly that reason (packet cc_exe_2o).
        if 'No function' not in text and not text.lower().lstrip().startswith('error'):
            m = BODY.search(text)
            name = None
            fm = FN_AT.search(text)
            em = ENTRY.search(text)
            entry = int(fm.group(2), 16) if fm else int(em.group(1), 16) if em else None
            if fm:
                name = fm.group(1)
            if m and entry is not None:
                result = (entry, int(m.group(2), 16), name)
            else:
                # fall back to the prototype endpoint, which prints the body range
                try:
                    proto = self.client.get('get_function_signature', address=f'{addr:08x}')
                    proto = proto if isinstance(proto, str) else json.dumps(proto)
                    pm, pf, pe = BODY.search(proto), FN_AT.search(proto), ENTRY.search(proto)
                    if entry is None:
                        entry = int(pf.group(2), 16) if pf else int(pe.group(1), 16) if pe else None
                    if name is None and pf:
                        name = pf.group(1)
                    if pm and entry is not None:
                        result = (entry, int(pm.group(2), 16), name)
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
        if fn is None and row.get('kind') == 'no_ghidra_function':
            # The call sits in a routine Ghidra never defined, which the report
            # must also list under `no_ghidra_function` with its inclusive end.
            # The containment and listing checks cannot run, but the valuable
            # half - that the callee is a real function start - still did.
            print(f'  raw       {where}: {site:08x} is in a routine with no Ghidra '
                  f'function; callee {callee:08x} checked, containment not')
        elif fn is None:
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


# Markdown reports (docs/CC13_VER_*.md and the like) state their calls in prose. The parser
# below recognises four forms and checks them with the same live tests as a JSON row:
#   listing     "0090358D CALL 00903420" (also "0090358D: CALL", JMP)       site + callee
#   site        "calls 00903420 at 0090358D", "00955630 at 006DFAB3",
#               "to 00438AA0 (00862F58)"                                     site + callee
#   calls       "00901BA0(size, ...), which calls 008387B0(...)"            caller + callee
#   name suffix "torpedo_attack_range_00901ba0"                              function start
# A "calls" claim whose caller is not in the same sentence takes the nearest earlier address of
# the paragraph; it fails only when stated in one sentence, otherwise a missing edge is reported
# as unchecked. "site" forms fail only when the sentence says call; a site that is no CALL to the
# callee in a sentence without that word is unchecked (the "at" may name a read or a branch).
# Address ranges "A..B" / "A-B" are checked to lie in one function (a warning otherwise). Any
# other sentence that says call and names an address is listed as unchecked, never as a failure.
MD_ADDR = re.compile(r'(?<![0-9A-Za-z])(?:0x)?(00[0-9A-Fa-f]{6})(?![0-9A-Za-z])')
MD_SUFFIX = re.compile(r'\b[A-Za-z][A-Za-z0-9_:]*_(00[0-9A-Fa-f]{6})\b')
MD_RANGE = re.compile(r'(?<![0-9A-Za-z])(00[0-9A-Fa-f]{6})`?\s*(?:\.\.|-|–)\s*`?(00[0-9A-Fa-f]{6})(?![0-9A-Za-z])')
MD_LISTING = re.compile(r'(?<![0-9A-Za-z])(00[0-9A-Fa-f]{6}):?\s+(CALL|JMP)\s+(?:0x)?(00[0-9A-Fa-f]{6})(?![0-9A-Za-z])')
MD_CALLS = re.compile(r'\bcalls\b')
MD_SITE = re.compile(r'`?\s*(?:\([^()]*\))?`?\s*(?:,?\s+at\s+`?|\s*\(`?)(?:0x)?(00[0-9A-Fa-f]{6})(?![0-9A-Za-z])')
CALL_WORD = re.compile(r'\bcall(?:s|ed|ing)?\b|\bCALL\b', re.IGNORECASE)
SENTENCE_END = re.compile(r'(?<=[.;:!?])\s+(?=[A-Z`(*])')


def image_addr(text):
    value = int(text, 16)
    return value if 0x401000 <= value < 0x1000000 else None


def md_blocks(text):
    """Yield (first line number, block text) for paragraphs, list items and table rows."""
    block, start = [], 0
    for n, line in enumerate(text.splitlines(), 1):
        stripped = line.strip()
        new_item = re.match(r'([-*+]|\d+\.)\s|\|', stripped) is not None
        if not stripped or stripped.startswith('#') or stripped.startswith('```') or (new_item and block):
            if block:
                yield start, ' '.join(block)
            block = []
            if not stripped or stripped.startswith('#') or stripped.startswith('```'):
                continue
        if not block:
            start = n
        block.append(stripped)
    if block:
        yield start, ' '.join(block)


def function_calls(live, fn_start, callee):
    """True when the live listing of fn_start has a CALL or JMP to callee anywhere."""
    if live.listing_calls(fn_start, fn_start, callee) is None:
        return None
    text = live.listings[fn_start]
    text = text if isinstance(text, str) else json.dumps(text)
    return re.search(rf'\b(?:CALL|JMP)\b[^\n]*{callee:08x}', text, re.IGNORECASE) is not None


def check_site(live, site, callee, mnemonics=('CALL', 'JMP')):
    """Return (seen, function) where seen is 'direct', 'indirect', 'other' or None."""
    fn = live.containing(site)
    if fn is None:
        return None, None
    seen = 'other'
    for mnemonic in mnemonics:
        seen = live.listing_calls(fn[0], site, callee, mnemonic)
        if seen in ('direct', 'indirect', None):
            break
    return seen, fn


def check_markdown(path, live):
    text = Path(path).read_text(encoding='utf-8')
    failures, checked, unchecked, warnings = [], [], [], []
    done = set()

    def fail(line, message):
        failures.append(f'  FAIL      line {line}: {message}')

    def callee_start(line, callee):
        body = live.containing(callee)
        if body is None:
            fail(line, f'callee {callee:08x} is not a Ghidra function')
            return False
        if body[0] != callee:
            fail(line, f'callee {callee:08x} is inside {body[0]:08x} ({body[2]}), not a function start')
            return False
        return True

    cited = set()
    for line, block in md_blocks(text):
        sentences = SENTENCE_END.split(block)
        paragraph_addrs = []
        for sentence in sentences:
            addrs = [(m.start(), image_addr(m.group(1)), m.end()) for m in MD_ADDR.finditer(sentence)]
            addrs = [(pos, a, end) for pos, a, end in addrs if a is not None]
            cited.update(a for _, a, _ in addrs)
            says_call = CALL_WORD.search(sentence) is not None
            claimed = False

            for m in MD_LISTING.finditer(sentence):
                site, callee = image_addr(m.group(1)), image_addr(m.group(3))
                if site is None or callee is None or ('site', site, callee) in done:
                    continue
                done.add(('site', site, callee))
                claimed = True
                failed_before = len(failures)
                if callee_start(line, callee):
                    seen, fn = check_site(live, site, callee, (m.group(2).upper(),))
                    if fn is None:
                        fail(line, f'call site {site:08x} is in no Ghidra function')
                    elif seen != 'direct':
                        fail(line, f'{site:08x} in {fn[0]:08x} is not a {m.group(2).upper()} to {callee:08x} (live listing)')
                checked.append((line, None if len(failures) > failed_before
                                else f'{site:08x} {m.group(2).upper()} {callee:08x}'))

            for _, callee, end in addrs:
                m = MD_SITE.match(sentence, end)
                if not m:
                    continue
                site = image_addr(m.group(1))
                if site is None or site == callee or ('site', site, callee) in done:
                    continue
                seen, fn = check_site(live, site, callee)
                if seen == 'direct':
                    done.add(('site', site, callee))
                    claimed = True
                    good = callee_start(line, callee)
                    checked.append((line, f'{callee:08x} at {site:08x} (in {fn[0]:08x})' if good else None))
                elif says_call and fn is not None and live.containing(callee) is not None and seen == 'other':
                    done.add(('site', site, callee))
                    claimed = True
                    checked.append((line, None))
                    fail(line, f'{site:08x} in {fn[0]:08x} is not a CALL/JMP to {callee:08x} (live listing): '
                               f'"{sentence[:120]}"')
                elif says_call:
                    unchecked.append((line, f'{callee:08x} at {site:08x}: site is '
                                            f'{"in no function" if fn is None else seen}'))

            for vm in MD_CALLS.finditer(sentence):
                after = [(p, a) for p, a, _ in addrs if p > vm.end()]
                if not after or after[0][0] - vm.end() > 40:
                    continue
                callee = after[0][1]
                before = [a for p, a, _ in addrs if p < vm.start()]
                strong = bool(before)
                caller = before[-1] if before else (paragraph_addrs[-1] if paragraph_addrs else None)
                if caller is None or ('calls', caller, callee) in done:
                    continue
                done.add(('calls', caller, callee))
                fn = live.containing(caller)
                if fn is None:
                    unchecked.append((line, f'{caller:08x} calls {callee:08x}: caller is in no Ghidra function'))
                    continue
                edge = function_calls(live, fn[0], callee)
                if edge:
                    good = callee_start(line, callee)
                    checked.append((line, f'{caller:08x} calls {callee:08x} (in {fn[0]:08x}{"" if strong else ", caller from context"})' if good else None))
                    claimed = True
                elif strong and edge is False:
                    checked.append((line, None))
                    claimed = True
                    fail(line, f'{fn[0]:08x} ({fn[2]}) has no CALL/JMP to {callee:08x}, '
                               f'report says {caller:08x} calls it')
                else:
                    unchecked.append((line, f'{caller:08x} calls {callee:08x}: caller inferred from an '
                                            f'earlier sentence and {fn[0]:08x} has no direct call'))

            if says_call and not claimed and addrs:
                unchecked.append((line, sentence[:140]))
            paragraph_addrs.extend(a for _, a, _ in addrs)

        for m in MD_SUFFIX.finditer(block):
            addr = image_addr(m.group(1))
            if addr is None or ('suffix', addr) in done:
                continue
            done.add(('suffix', addr))
            body = live.containing(addr)
            if body is None:
                warnings.append(f'  warn      line {line}: {m.group(0)}: {addr:08x} is in no Ghidra function')
            elif body[0] != addr:
                warnings.append(f'  warn      line {line}: {m.group(0)}: {addr:08x} is inside {body[0]:08x} '
                                f'({body[2]}), not a function start')
        for m in MD_RANGE.finditer(block):
            lo, hi = image_addr(m.group(1)), image_addr(m.group(2))
            if lo is None or hi is None or hi <= lo or ('range', lo, hi) in done:
                continue
            done.add(('range', lo, hi))
            a, b = live.containing(lo), live.containing(hi)
            if a is None or b is None or a[0] != b[0]:
                where = lambda f: f'{f[0]:08x}' if f else 'no function'
                warnings.append(f'  warn      line {line}: range {lo:08x}-{hi:08x} spans '
                                f'{where(a)} and {where(b)}')

    for line, what in checked:
        if what:
            print(f'  ok        line {line}: {what}')
    for f in failures:
        print(f)
    for w in warnings:
        print(w)
    for line, what in unchecked:
        print(f'  unchecked line {line}: {what}')
    print(f'{path}: {len(cited)} addresses cited, {len(checked)} call claims checked, {len(failures)} failed, '
          f'{len(unchecked)} unchecked, {len(warnings)} warnings')
    return len(failures)


def main(argv):
    if not argv:
        print(__doc__)
        return 2
    index = Index(ROOT / 'local/bsp_index.sqlite')
    live = Live()
    total = 0
    for p in argv:
        if p.lower().endswith('.md'):
            total += check_markdown(p, live)
        else:
            total += check_report(p, index, live)
    return 1 if total else 0


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
