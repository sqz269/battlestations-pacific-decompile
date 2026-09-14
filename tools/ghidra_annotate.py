"""Apply the reviewed naming ledger to the existing bsp project; default is preview."""
import argparse
import json
from datetime import datetime, timezone
from urllib.parse import urlencode
from urllib.request import Request, urlopen

from coordination import check_writable, ghidra_lock
from ghidra_export import Client, ROOT, write
from ledger import load_names


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--addresses', nargs='+', help='Apply only selected ledger addresses')
    args = parser.parse_args()
    client = Client(json.loads((ROOT / 'config/target.json').read_text()))
    client.verify()
    entries = load_names()  # sharded config/names/*.jsonl plus any legacy config/ghidra_names.json
    if args.addresses:
        selected = {f'{int(address, 16):08x}' for address in args.addresses}
        unknown = selected - {row['address'] for row in entries}
        if unknown:
            parser.error(f'Addresses not present in naming ledger: {sorted(unknown)}')
        entries = [row for row in entries if row['address'] in selected]
    if not args.apply:
        print(json.dumps(entries, indent=2))
        return
    check_writable([row['address'] for row in entries])  # refuse addresses leased to another agent
    # the lock must be released on every exit path: a failed plate-comment read once left it
    # behind for its full TTL and blocked the next writer
    with ghidra_lock(purpose='ghidra_annotate --apply'):
        apply_entries(client, entries)



def _function_entry(previous):
    """The entry address of the function a plate-comment lookup landed in, or None.

    `get_plate_comment` answers for the enclosing function, so its reply carries
    that function's own address rather than the address asked about.
    """
    if isinstance(previous, dict):
        found = previous.get('address') or previous.get('entry_point')
        if found:
            return str(found).lower().replace('0x', '').rjust(8, '0')
    return None

def apply_entries(client, entries):
    changes = []
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    log = ROOT / 'local' / f'ghidra-annotations-{stamp}.json'
    def post(endpoint, **body):
        client.verify()
        url = client.config['ghidra_url'] + '/' + endpoint + '?' + urlencode({'program': client.config['program']})
        request = Request(url, data=json.dumps(body).encode(), headers={'Content-Type': 'application/json'}, method='POST')
        with urlopen(request, timeout=90) as response:
            result = response.read().decode()
        decoded = json.loads(result)
        if decoded.get('error') or decoded.get('success') is False or decoded.get('status') == 'error':
            raise RuntimeError(result)
        return decoded
    for entry in entries:
        address = entry['address']
        old = client.get('get_plate_comment', address=address)
        previous = old.get('plate_comment', old.get('comment', '')) or ''
        marker = f"[BSP reconstruction {address}]"
        comment = previous
        if marker not in previous:
            comment = (previous + '\n\n' if previous else '') + marker + '\n' + entry['evidence']
        elif entry['evidence'] not in previous:
            comment = previous + '\n\n' + marker + ' update\n' + entry['evidence']
        # rename_function_by_address resolves an interior address to its ENCLOSING
        # function and renames that, so a ledger record for a block inside a larger
        # body silently clobbers the enclosing function's established name. This
        # happened once to BSP_UnitGunneryAi_Tick 00864FE0, which was overwritten by
        # a record for its interior label 00865284. Skip those and say so; interior
        # blocks are named with tools/ghidra_label_interior.py, which applies a label.
        entry_address = _function_entry(old)
        if entry_address and entry_address != address:
            print(f"{address}: SKIPPED, interior of {entry_address} "
                  f"({old.get('function_name') if isinstance(old, dict) else ''}). "
                  f"Use tools/ghidra_label_interior.py {address} {entry['name']}", flush=True)
            changes.append({'address': address, 'before': old, 'after': entry,
                            'status': 'skipped_interior', 'enclosing': entry_address})
            write(log, changes)
            continue
        changes.append({'address': address, 'before': old, 'after': entry, 'status': 'pending'})
        write(log, changes)
        post('rename_function_by_address', function_address=address, new_name=entry['name'])
        post('set_plate_comment', address=address, comment=comment)
        changes[-1]['status'] = 'applied'
        write(log, changes)
        print(f"{address} -> {entry['name']}", flush=True)
    post('save_program')
    print(f'Saved project; prior annotations recorded in {log}')


if __name__ == '__main__':
    main()
