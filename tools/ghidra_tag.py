"""Apply config/ghidra_tags.json to the existing bsp project; default is preview.

Each ledger entry renames a Ghidra-default `FUN_` function to an inventory tag or stock
library name and sets a bookmark (category "Inventory: <category>"). Entries flagged
`create` define the function first (table-referenced C entry points Ghidra missed).
Entries with action "bookmark" only set a bookmark. Existing non-default names are never
overwritten. Every change is logged with the prior state to local/ghidra-tags-<stamp>.json;
`--revert <log>` restores it. The server's dry_run flag is not honoured over HTTP, so
preview is done locally.
"""
import argparse
import json
import re
import time
from datetime import datetime, timezone
from urllib.error import HTTPError, URLError
from urllib.parse import urlencode
from urllib.request import Request, urlopen

from build_tag_ledger import PLATE_CATEGORIES
from ghidra_export import Client, ROOT, write
from ledger import load_tags


def make_post(client):
    def post(endpoint, **body):
        url = client.config['ghidra_url'] + '/' + endpoint + '?' + urlencode({'program': client.config['program']})
        request = Request(url, data=json.dumps(body).encode(), headers={'Content-Type': 'application/json'}, method='POST')
        for attempt in range(3):
            try:
                with urlopen(request, timeout=90) as response:
                    result = response.read().decode()
                break
            except (TimeoutError, URLError, HTTPError):
                if attempt == 2:
                    raise
                time.sleep(1 + attempt)
        decoded = json.loads(result)
        if decoded.get('error') or decoded.get('success') is False or decoded.get('status') == 'error':
            raise RuntimeError(result)
        return decoded
    return post


def current_function(client, address):
    text = client.get('get_function_by_address', address=address)
    if not isinstance(text, str):
        return None
    match = re.match(r'Function:\s+(\S+)\s+at\s+([0-9a-fA-F]+)', text)
    if not match or match.group(2).lower() != address:
        return None
    return match.group(1)


def apply(client, post, entries, log, save):
    changes = []
    errors = 0
    marker_fmt = '[BSP inventory {address}]'
    for index, entry in enumerate(entries, 1):
        address = entry['address']
        record = {'address': address, 'entry': entry, 'status': 'pending'}
        changes.append(record)
        try:
            before = current_function(client, address)
            record['before_name'] = before
            if entry['action'] == 'bookmark':
                if before is None:
                    record['status'] = 'missing'
                else:
                    post('set_bookmark', address=address, category=f"Inventory: {entry['category']}",
                         comment=f"[{entry['confidence']}] {entry['evidence']}")
                    record['status'] = 'bookmarked'
            elif before is None:
                if entry['create']:
                    created = post('create_function', address=address, name=entry['name'])
                    record['created'] = created
                    if created.get('function_name') != entry['name']:
                        post('rename_function_by_address', function_address=address, new_name=entry['name'])
                    record['status'] = 'created'
                else:
                    record['status'] = 'missing'
            elif before == entry['name']:
                record['status'] = 'already'
            elif not before.startswith('FUN_'):
                record['status'] = 'kept_existing'
            else:
                post('rename_function_by_address', function_address=address, new_name=entry['name'])
                record['status'] = 'renamed'
            if record['status'] in ('created', 'renamed', 'already'):
                post('set_bookmark', address=address, category=f"Inventory: {entry['category']}",
                     comment=f"{entry['name']} [{entry['confidence']}] {entry['evidence']}")
                if entry['category'] in PLATE_CATEGORIES:
                    old = client.get('get_plate_comment', address=address)
                    previous = old.get('comment', '') if isinstance(old, dict) else ''
                    record['before_plate'] = previous
                    marker = marker_fmt.format(address=address)
                    if marker not in previous:
                        text = f"{marker} {entry['category']} ({entry['confidence']}): {entry['evidence']}"
                        comment = (previous + '\n\n' if previous else '') + text
                        post('set_plate_comment', address=address, comment=comment)
                        record['plate_set'] = True
            errors = 0
        except Exception as exc:  # noqa: BLE001 - log and continue, abort on a run of failures
            record['status'] = 'error'
            record['error'] = str(exc)[:500]
            errors += 1
            if errors >= 25:
                write(log, changes)
                raise RuntimeError(f'{errors} consecutive errors, last at {address}: {exc}')
        if index % 50 == 0 or index == len(entries):
            write(log, changes)
        if index % 200 == 0 or index == len(entries):
            print(f'{index}/{len(entries)} {address} {record["status"]}', flush=True)
    if save:
        post('save_program')
    write(log, changes)
    return changes


def revert(client, post, log_path, save):
    changes = json.loads(log_path.read_text())
    count = 0
    for record in reversed(changes):
        entry = record['entry']
        address = record['address']
        status = record['status']
        try:
            if status in ('renamed', 'created', 'already', 'bookmarked'):
                post('delete_bookmark', address=address, category=f"Inventory: {entry['category']}")
            if record.get('plate_set'):
                post('batch_set_comments', address=address, plate_comment=record.get('before_plate', ''))
            if status == 'renamed':
                post('rename_function_by_address', function_address=address, new_name=record['before_name'])
            elif status == 'created':
                post('delete_function', address=address)
            count += 1
        except Exception as exc:  # noqa: BLE001
            print(f'revert failed at {address}: {exc}', flush=True)
    if save:
        post('save_program')
    print(f'reverted {count} records from {log_path}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--apply', action='store_true')
    parser.add_argument('--category', action='append', help='Only these ledger categories')
    parser.add_argument('--limit', type=int, help='Apply at most N entries (after filters)')
    parser.add_argument('--resume', help='Skip entries already applied in this log file')
    parser.add_argument('--revert', help='Revert the changes recorded in this log file')
    parser.add_argument('--no-save', action='store_true')
    args = parser.parse_args()
    client = Client(json.loads((ROOT / 'config/target.json').read_text()))
    client.verify()
    post = make_post(client)
    if args.revert:
        revert(client, post, ROOT / args.revert if not args.revert.startswith(('/', 'C:', 'J:')) else __import__('pathlib').Path(args.revert), not args.no_save)
        return
    entries = load_tags()  # sharded config/tags/*.jsonl plus any legacy config/ghidra_tags.json
    if args.category:
        entries = [e for e in entries if e['category'] in set(args.category)]
    if args.resume:
        done = {r['address'] for r in json.loads((ROOT / args.resume).read_text()) if r['status'] != 'error'}
        entries = [e for e in entries if e['address'] not in done]
    if args.limit:
        entries = entries[:args.limit]
    counts = {}
    for e in entries:
        counts[e['category']] = counts.get(e['category'], 0) + 1
    print(f'{len(entries)} entries selected: {counts}')
    if not args.apply:
        for e in entries[:20]:
            print(f"  {e['address']} {e['action']:8s} {e['name'] or '-':40s} {e['category']}")
        return
    stamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
    log = ROOT / 'local' / f'ghidra-tags-{stamp}.json'
    changes = apply(client, post, entries, log, not args.no_save)
    summary = {}
    for r in changes:
        summary[r['status']] = summary.get(r['status'], 0) + 1
    print(f'done: {summary}; log {log}')


if __name__ == '__main__':
    main()
