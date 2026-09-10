"""Read-only exporter for the HTTP backend used by the installed Ghidra MCP bridge.

Standard library only. A named bsp project must already be open in Ghidra.
Never imports, analyzes, renames, or saves a Ghidra program.
"""
import argparse
import hashlib
import json
import struct
import sys
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlencode, urlparse
from urllib.request import urlopen

ROOT = Path(__file__).resolve().parents[1]


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    text = value if isinstance(value, str) else json.dumps(value, indent=2) + '\n'
    temporary = path.with_suffix(path.suffix + '.tmp')
    with temporary.open('w', encoding='utf-8', newline='\n') as stream:
        stream.write(text.replace('\r\n', '\n'))
    temporary.replace(path)


def pe_summary(path):
    data = path.read_bytes()
    if data[:2] != b'MZ':
        raise ValueError('Not a DOS/PE executable')
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    if data[pe:pe + 4] != b'PE\0\0':
        raise ValueError('Missing PE signature')
    machine, count, timestamp = struct.unpack_from('<HHI', data, pe + 4)
    optional_size = struct.unpack_from('<H', data, pe + 20)[0]
    opt = pe + 24
    if machine != 0x14c or struct.unpack_from('<H', data, opt)[0] != 0x10b:
        raise ValueError('Expected x86 PE32')
    entry, base = struct.unpack_from('<I', data, opt + 16)[0], struct.unpack_from('<I', data, opt + 28)[0]
    sections = []
    for index in range(count):
        offset = opt + optional_size + index * 40
        name = data[offset:offset + 8].split(b'\0')[0].decode('ascii', errors='replace')
        virtual_size, rva, raw_size, raw_offset = struct.unpack_from('<IIII', data, offset + 8)
        raw = data[raw_offset:raw_offset + raw_size]
        sections.append(dict(name=name, rva=f'{rva:08x}', virtual_size=virtual_size,
                             raw_size=raw_size, raw_offset=raw_offset,
                             raw_sha256=hashlib.sha256(raw).hexdigest()))
    def raw_offset(rva):
        for section in sections:
            delta = rva - int(section['rva'], 16)
            if 0 <= delta < section['raw_size']:
                return section['raw_offset'] + delta
        raise ValueError(f'RVA {rva:08x} has no disk backing')
    dlls = []
    import_rva = struct.unpack_from('<I', data, opt + 104)[0]
    if import_rva:
        descriptor = raw_offset(import_rva)
        while any(data[descriptor:descriptor + 20]):
            name_rva = struct.unpack_from('<I', data, descriptor + 12)[0]
            start = raw_offset(name_rva)
            dlls.append(data[start:data.index(b'\0', start)].decode('ascii'))
            descriptor += 20
    return dict(path=str(path), size=len(data), sha256=hashlib.sha256(data).hexdigest(),
                machine='x86', timestamp=timestamp, image_base=f'{base:08x}',
                entry_point=f'{base + entry:08x}', sections=sections, imported_dlls=dlls,
                note='Current disk file only; identity with the saved Ghidra image is unverified.')


class Client:
    def __init__(self, config):
        self.config = config
        parsed = urlparse(config['ghidra_url'])
        if parsed.scheme != 'http' or parsed.hostname not in ('127.0.0.1', 'localhost', '::1'):
            raise ValueError('Expected a loopback HTTP Ghidra server')

    def get(self, endpoint, **params):
        params.setdefault('program', self.config['program'])
        url = self.config['ghidra_url'].rstrip('/') + '/' + endpoint + '?' + urlencode(params)
        for attempt in range(3):
            try:
                with urlopen(url, timeout=90) as response:
                    text = response.read().decode('utf-8')
                break
            except (TimeoutError, OSError):
                if attempt == 2:
                    raise
                time.sleep(1 + attempt)
        try:
            result = json.loads(text)
        except json.JSONDecodeError:
            result = text
        if isinstance(result, dict) and (result.get('error') or result.get('success') is False):
            raise RuntimeError(f'{endpoint}: {result}')
        if isinstance(result, str) and result.lstrip().lower().startswith(('error', 'failed', 'no program')):
            raise RuntimeError(f'{endpoint}: {result}')
        return result

    def verify(self):
        project = self.get('list_project_files', folder='/')
        info = self.get('get_current_program_info')
        expected = self.config
        if project['project_name'] != expected['project']:
            raise RuntimeError(f"Wrong Ghidra project: {project['project_name']}")
        for key, actual in [('program_path', info['path']), ('language', info['language']), ('image_base', info['image_base'])]:
            if actual != expected[key]:
                raise RuntimeError(f'Wrong target {key}: {actual}')
        return project, info


def snapshot(client, output):
    project, info = client.verify()
    functions = []
    while True:
        page = client.get('list_functions_enhanced', offset=len(functions), limit=10000)['functions']
        if not page:
            break
        functions.extend(page)
        print(f"Inventory {len(functions)}/{info['function_count']}", flush=True)
    if not functions or len({row['address'] for row in functions}) != len(functions):
        raise RuntimeError('Empty inventory or duplicate addresses; retry snapshot')
    final_project, final_info = client.verify()
    if final_info['function_count'] != info['function_count']:
        raise RuntimeError('Analysis changed during export; retry when idle')
    write(output / 'functions.json', functions)
    write(output / 'program.json', info)
    write(output / 'project.json', final_project)
    write(output / 'disk_binary.json', pe_summary(Path(client.config['binary'])))
    imports = []
    while True:
        page = client.get('list_imports', offset=len(imports), limit=1000)
        if not isinstance(page, list):
            raise RuntimeError('Unexpected list_imports response schema')
        if not page:
            break
        imports.extend(page)
    write(output / 'imports.json', imports)
    for endpoint in ('list_segments', 'list_exports'):
        # Text endpoints are paginated; continue until an empty / no-results page.
        pages = []
        offset = 0
        while True:
            page = client.get(endpoint, offset=offset, limit=1000)
            if not isinstance(page, str):
                raise RuntimeError(f'Unexpected {endpoint} response schema')
            if not page.strip() or page.strip().lower().startswith('no '):
                break
            pages.append(page)
            lines = page.strip().splitlines()
            if len(lines) < 1000:
                break
            offset += len(lines)
        write(output / (endpoint + '.txt'), '\n'.join(pages))
    write(output / 'snapshot.json', dict(utc=datetime.now(timezone.utc).isoformat(),
          project=project['project_name'], program=info['path'],
          total_function_count=info['function_count'], internal_function_count=len(functions),
          external_symbol_count=len(imports),
          note='getFunctions(true) enumerates internal functions; total count also includes external functions.'))


def decompile(client, output, addresses, force=False):
    client.verify()
    inventory = {row['address']: row for row in json.loads((output / 'functions.json').read_text())}
    failures = []
    for index, address in enumerate(addresses):
        address = f'{int(address, 16):08x}'
        if address not in inventory:
            # defined after the snapshot (integrator definitions, worker-found leaves): accept it when
            # Ghidra has a function there now, and note the provenance in the metadata
            info = str(client.get('get_function_by_address', address=address))
            if 'No function' in info or 'error' in info.lower():
                raise ValueError(f'{address} is not a function in this snapshot or in Ghidra')
            name = info.splitlines()[0].split('Function:', 1)[-1].split(' at ')[0].strip() or f'FUN_{address}'
            inventory[address] = {'address': address, 'name': name, 'snapshot': 'live (defined after the last snapshot)'}
        folder = output / 'functions' / address
        if not force and all((folder / file).exists() for file in ('decompiled.c', 'assembly.txt', 'metadata.json')):
            continue
        try:
            client.verify()  # Prevent cross-project evidence if the user switches projects.
            code = client.get('force_decompile' if force else 'decompile_function', address=address, timeout=60)
            if force and isinstance(code, str) and code.startswith('Success: Forced redecompilation of '):
                code = code.partition('\n\n')[2]
            assembly = client.get('disassemble_function', address=address)
            if not isinstance(code, str) or '{' not in code or not isinstance(assembly, str) or not assembly.strip():
                raise RuntimeError('Empty or unsuccessful function export')
            write(folder / 'decompiled.c', code)
            write(folder / 'assembly.txt', assembly)
            write(folder / 'metadata.json', {**inventory[address], 'project': client.config['project'],
                  'program_path': client.config['program_path'], 'utc': datetime.now(timezone.utc).isoformat()})
            print(f'Exported {index + 1}/{len(addresses)} {address}', flush=True)
        except (OSError, RuntimeError) as error:
            failures.append(dict(address=address, error=str(error)))
            print(f'FAILED {address}: {error}', file=sys.stderr, flush=True)
    write(output / 'last_export_failures.json', failures)
    if failures:
        raise RuntimeError(f'{len(failures)} function exports failed; rerun to retry')


def verify_seeds(client, output):
    client.verify()
    disk = Path(client.config['binary'])
    summary = pe_summary(disk)
    data = disk.read_bytes()
    # Lengths from assembly through the final RET (including its immediate).
    lengths = {'00401170': 0x1c, '00401c20': 0x2f, '00401cb0': 0x19,
               '00401cd0': 0x1d, '00401cf0': 0x19,
               '00bf0cf0': 0x2e, '00bf0d20': 0xcf, '00ba2c20': 0x5e}
    results = []
    for address, length in lengths.items():
        rva = int(address, 16) - int(summary['image_base'], 16)
        section = next(s for s in summary['sections'] if int(s['rva'], 16) <= rva
                       and rva + length <= int(s['rva'], 16) + s['raw_size'])
        offset = section['raw_offset'] + rva - int(section['rva'], 16)
        original = data[offset:offset + length]
        saved = bytes.fromhex(client.get('read_memory', address=address, length=length)['hex'])
        results.append(dict(address=address, length=length, matches_disk=original == saved,
                            ghidra_hex=saved.hex(), disk_hex=original.hex()))
    write(output / 'seed_byte_comparison.json', dict(disk_sha256=summary['sha256'], ranges=results,
          note='Only these byte ranges were compared. This does not establish whole-image identity.'))
    print(json.dumps(results, indent=2))
    if not all(r['matches_disk'] for r in results):
        raise RuntimeError('Saved Ghidra code differs from current disk file; use saved-project evidence explicitly.')
    header = '// Generated from byte ranges verified against the local executable.\n#pragma once\n'
    for row in results:
        values = ','.join(f'0x{byte:02x}' for byte in bytes.fromhex(row['ghidra_hex']))
        header += f"inline constexpr unsigned char ref_{row['address']}[] = {{{values}}};\n"
    write(ROOT / 'local/seed_reference.hpp', header)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['snapshot', 'seed', 'decompile', 'all', 'verify-seeds'])
    parser.add_argument('--config', type=Path, default=ROOT / 'config/target.json')
    parser.add_argument('--output', type=Path, default=__import__('workspace').exports_dir(),
                        help='defaults to the main checkout\'s exports/bsp, shared by every worktree')
    parser.add_argument('--addresses', nargs='+')
    parser.add_argument('--force', action='store_true')
    args = parser.parse_args()
    client = Client(json.loads(args.config.read_text(encoding='utf-8')))
    if args.command == 'snapshot':
        snapshot(client, args.output)
        return
    if args.command == 'verify-seeds':
        verify_seeds(client, args.output)
        return
    if args.command == 'seed':
        addresses = json.loads((ROOT / 'config/seed_functions.json').read_text())
    elif args.command == 'all':
        addresses = [r['address'] for r in json.loads((args.output / 'functions.json').read_text()) if not r['isExternal'] and not r['isThunk']]
    else:
        if not args.addresses:
            parser.error('decompile requires --addresses')
        addresses = args.addresses
    decompile(client, args.output, addresses, args.force)


if __name__ == '__main__':
    main()
