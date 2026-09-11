"""Define a function in Ghidra at an address that a worker read from the raw listing.

Usage: python tools/ghidra_define_function.py <start> <end_exclusive> [<start> <end_exclusive> ...] [--record reports/x.json]

For each pair: verify the bytes in Ghidra match the disk image, disassemble the byte range
explicitly, then create the function from the existing instructions without letting Ghidra
re-run its flow discovery (which re-marks _free call sites). The function takes the reviewed
ledger name when one exists. Every event is appended to the record file. Runs under the
Ghidra write lock; refuses addresses leased to another owner.
"""
import json
import sys
from pathlib import Path
from urllib.parse import urlencode
from urllib.request import Request, urlopen

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))
import ledger  # noqa: E402
from coordination import check_writable, ghidra_lock, owner_name  # noqa: E402
from ghidra_export import Client  # noqa: E402


def main(argv):
    argv = list(argv)
    record_path = ROOT / 'reports/defined_functions.json'
    if '--record' in argv:
        i = argv.index('--record')
        record_path = ROOT / argv[i + 1]
        del argv[i:i + 2]
    recreate = '--recreate' in argv  # delete a truncated existing function first, keeping its name and plate comment
    force = '--force' in argv  # proceed over another owner's lease (a body repair changes no name; record why)
    args = [a for a in argv if not a.startswith('--')]
    if len(args) < 2 or len(args) % 2:
        sys.exit(__doc__)
    pairs = [(args[i].lower().replace('0x', '').zfill(8), args[i + 1].lower().replace('0x', '').zfill(8)) for i in range(0, len(args), 2)]
    cfg = json.loads((ROOT / 'config/target.json').read_text(encoding='utf-8'))
    c = Client(cfg)
    c.verify()
    names = ledger.load_names()
    record = json.loads(record_path.read_text(encoding='utf-8')) if record_path.exists() else {'events': []}

    def save():
        record_path.write_text(json.dumps(record, indent=1) + '\n', encoding='utf-8', newline='\n')

    def post(endpoint, body):
        req = Request(cfg['ghidra_url'] + '/' + endpoint + '?' + urlencode({'program': cfg['program']}),
                      data=json.dumps(body).encode(), headers={'Content-Type': 'application/json'}, method='POST')
        with urlopen(req, timeout=300) as response:
            result = json.loads(response.read())
        record['events'].append({'endpoint': endpoint, 'body': body, 'result': result})
        save()
        if result.get('error') or result.get('success') is False or result.get('status') == 'error':
            raise RuntimeError(result)
        return result

    import pefile
    pe = pefile.PE(cfg['binary'], fast_load=True)
    base = pe.OPTIONAL_HEADER.ImageBase

    def disk_bytes(start, length):
        rva = int(start, 16) - base
        return pe.get_data(rva, length)

    owner = owner_name()
    with ghidra_lock(owner=owner, wait_seconds=60, purpose='define functions read from the raw listing'):
        conflicts = check_writable([p[0] for p in pairs], owner=owner, force=force)
        if conflicts:
            record['events'].append({'forced_over_leases': [str(x) for x in conflicts]})
            save()
        for start, end in pairs:
            length = int(end, 16) - int(start, 16)
            assert 0 < length < 0x4000, f'{start}-{end}: implausible length {length}'
            existing = c.get('get_function_by_address', address=start)
            plate = None
            if isinstance(existing, str) and 'No function' not in existing and 'error' not in existing.lower():
                if not recreate:
                    print(f'{start}: already a function: {existing.splitlines()[0]}')
                    continue
                try:
                    plate = c.get('get_plate_comment', address=start)
                    if isinstance(plate, dict):
                        plate = plate.get('comment') or plate.get('plate_comment')
                    if isinstance(plate, str) and ('No plate' in plate or 'error' in plate.lower()):
                        plate = None
                except Exception:
                    plate = None
                record['events'].append({'recreate': start, 'previous': existing, 'plate': plate})
                save()
                post('delete_function', {'address': start})
                print(f'{start}: deleted the truncated function ({existing.splitlines()[0]}) for re-creation')
            mem = c.get('read_memory', address=start, length=length)
            ghidra_hex = mem.get('hex') if isinstance(mem, dict) else ''.join(ch for ch in str(mem) if ch in '0123456789abcdefABCDEF')
            if bytes.fromhex(ghidra_hex)[:length] != disk_bytes(start, length):
                sys.exit(f'{start}: Ghidra bytes differ from the disk image; not defining')
            if isinstance(names, dict):
                entry = names.get(start) or names.get(int(start, 16)) or {}
            else:
                entry = next((n for n in names if str(n.get('address', '')).lower().replace('0x', '').zfill(8) == start), {})
            name = entry.get('name') if isinstance(entry, dict) else None
            dis = post('disassemble_bytes', {'start_address': start, 'end_address': end})
            body = {'address': start, 'disassemble_first': False}
            if name:
                body['name'] = name
            created = post('create_function', body)
            if plate:
                post('set_plate_comment', {'address': start, 'comment': plate})
            after = c.get('get_function_by_address', address=start)
            record['events'].append({'defined': start, 'end': end, 'name': name, 'after': after})
            save()
            print(f"{start}: defined {name or '(unnamed)'}; disassembled {dis.get('bytes_disassembled', '?')} bytes; "
                  f"{str(after).splitlines()[0] if after else created}")
        import time
        for attempt in range(12):
            try:
                post('save_program', {})
                break
            except RuntimeError as exc:
                if 'active transaction' not in str(exc) or attempt == 11:
                    raise
                time.sleep(5)  # Ghidra refuses to save while its analysis transaction is open
    print(f'saved; record {record_path.relative_to(ROOT)}')


if __name__ == '__main__':
    main(sys.argv[1:])
