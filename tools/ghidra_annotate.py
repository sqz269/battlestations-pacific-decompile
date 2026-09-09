"""Apply the reviewed naming ledger to the existing bsp project; default is preview."""
import argparse
import json
from datetime import datetime, timezone
from urllib.parse import urlencode
from urllib.request import Request, urlopen

from ghidra_export import Client, ROOT, write


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--addresses', nargs='+', help='Apply only selected ledger addresses')
    args = parser.parse_args()
    client = Client(json.loads((ROOT / 'config/target.json').read_text()))
    client.verify()
    entries = json.loads((ROOT / 'config/ghidra_names.json').read_text())
    if args.addresses:
        selected = {f'{int(address, 16):08x}' for address in args.addresses}
        unknown = selected - {row['address'] for row in entries}
        if unknown:
            parser.error(f'Addresses not present in naming ledger: {sorted(unknown)}')
        entries = [row for row in entries if row['address'] in selected]
    if not args.apply:
        print(json.dumps(entries, indent=2))
        return
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
