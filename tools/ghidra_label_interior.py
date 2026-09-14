"""Name an interior code address of an existing function with a Ghidra LABEL.

`ghidra_annotate.py --apply` renames functions. When a packet establishes the
behaviour of a block that is an interior label of a larger function rather than
a function of its own - `00865284` and `008657A3` inside
`BSP_UnitGunneryAi_Tick 00864FE0`, for example - that path fails with
"No function at address". Defining a function there would be the wrong repair:
it fragments the enclosing body and corrupts the decompiler's view of it.

This applies a label instead, under the shared Ghidra write lock, and refuses
any address that is already a function start or that lies outside a function.

    python tools/ghidra_label_interior.py 00865284 BSP_UnitGunneryAi_InsertRankedCandidate \
                                          008657a3 BSP_UnitGunneryAi_WalkCandidatesForGun \
                                          [--record reports/<x>.json]
"""
import json
import re
import sys
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlencode
from urllib.request import Request, urlopen

sys.path.insert(0, str(Path(__file__).resolve().parent))

from coordination import check_writable, ghidra_lock  # noqa: E402
from ghidra_export import Client, ROOT  # noqa: E402


def post(client, endpoint, **body):
    client.verify()
    url = client.config['ghidra_url'] + '/' + endpoint + '?' + urlencode(
        {'program': client.config['program']})
    request = Request(url, data=json.dumps(body).encode(),
                      headers={'Content-Type': 'application/json'}, method='POST')
    with urlopen(request, timeout=90) as response:
        result = response.read().decode()
    decoded = json.loads(result)
    if decoded.get('error') or decoded.get('success') is False or decoded.get('status') == 'error':
        raise RuntimeError(result)
    return decoded


def main(argv):
    allow_data = False
    if '--data' in argv:
        allow_data = True
        argv = [a for a in argv if a != '--data']
    record = None
    if '--record' in argv:
        i = argv.index('--record')
        record = argv[i + 1]
        argv = argv[:i] + argv[i + 2:]
    if not argv or len(argv) % 2:
        print(__doc__)
        return 2
    pairs = [(argv[i].lower(), argv[i + 1]) for i in range(0, len(argv), 2)]

    client = Client(json.loads((ROOT / 'config/target.json').read_text()))
    client.verify()
    check_writable([a for a, _ in pairs])

    planned = []
    for address, name in pairs:
        # the bridge answers this one as plain text, e.g.
        #   Function: BSP_UnitGunneryAi_Tick at 00864fe0 / Entry: 00864fe0 / Body: ...
        info = client.get('get_function_by_address', address=address)
        text = info if isinstance(info, str) else json.dumps(info)
        entry = re.search(r'Entry:\s*([0-9a-fA-F]+)', text)
        named = re.search(r'Function:\s*(\S+)\s+at', text)
        if not entry:
            if allow_data:
                # An authored data row in .rdata - a gunnery preference table, a
                # name table - is legitimately in no function. Labelling one is
                # the right annotation; defining a function there would not be.
                planned.append({'address': address, 'name': name,
                                'enclosing': None, 'enclosing_address': None,
                                'kind': 'data'})
                continue
            raise SystemExit(
                f'{address}: not inside any function ({text[:80]}). '
                f'If this is an authored data row, pass --data.')
        start = entry.group(1).lower().rjust(8, '0')
        if start.lstrip('0') == address.lstrip('0'):
            raise SystemExit(
                f'{address}: this IS a function start ({named.group(1) if named else "?"}); '
                f'use tools/ghidra_annotate.py --apply instead')
        planned.append({'address': address, 'name': name,
                        'enclosing': named.group(1) if named else None,
                        'enclosing_address': start})

    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    with ghidra_lock(purpose='ghidra_label_interior'):
        for row in planned:
            post(client, 'create_label', address=row['address'], name=row['name'])
            where = ('data, in no function' if row.get('kind') == 'data'
                     else f'interior of {row["enclosing"]} at {row["enclosing_address"]}')
            print(f'{row["address"]} -> label {row["name"]}  ({where})')
        post(client, 'save_program')
    print('saved project')

    if record:
        out = ROOT / record
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(
            {'tool': 'ghidra_label_interior', 'applied_utc': stamp, 'labels': planned},
            indent=1) + '\n', encoding='utf-8')
        print(f'record {record}')
    return 0


if __name__ == '__main__':
    raise SystemExit(main(sys.argv[1:]))
