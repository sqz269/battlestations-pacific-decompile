"""Export the mission Lua binding table at 00E0B7B8 from the disk image.

The table is an array of `{const char *name; lua_CFunction handler;}` rows, eight bytes each,
registered one row at a time by `006B8610` (`docs/MISSION_LUA_HOST.md`). It is terminated by a
row whose name points at the empty string, at 00E0C938. Reading it from the PE rather than from
a Ghidra query means `config/lua_bindings.json` can be regenerated without the bridge running.

    python tools/lua_bindings_export.py                      # rewrite config/lua_bindings.json
    python tools/lua_bindings_export.py --check              # fail if the file is stale
    python tools/lua_bindings_export.py --table 00E0B7B8     # another table of the same shape

Each row is annotated from local/bsp_index.sqlite (built by `python tools/bsp.py index`) with the
Ghidra function that contains the handler, its ledger name and any reconstruction record, so the
survey shows which handlers are already recovered. The annotation is skipped when the index is
absent; the addresses and names come from the image alone.
"""
from __future__ import annotations

import argparse
import json
import sqlite3
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'tools'))

DEFAULT_TABLE = 0x00E0B7B8
DEFAULT_OUTPUT = ROOT / 'config' / 'lua_bindings.json'
MAX_ROWS = 4096


def load_target(config_path: Path) -> dict:
    return json.loads(config_path.read_text(encoding='utf-8'))


class Image:
    """Minimal VA reader over the mapped sections of the PE."""

    def __init__(self, path: Path):
        import pefile  # imported lazily so the module is optional for --check on a cached file

        self.pe = pefile.PE(str(path), fast_load=True)
        self.image_base = self.pe.OPTIONAL_HEADER.ImageBase
        self.sections = []
        data = path.read_bytes()
        for section in self.pe.sections:
            start = self.image_base + section.VirtualAddress
            raw = data[section.PointerToRawData:section.PointerToRawData + section.SizeOfRawData]
            self.sections.append((start, start + max(section.Misc_VirtualSize, len(raw)), raw))

    def read(self, va: int, length: int) -> bytes:
        for start, end, raw in self.sections:
            if start <= va < end:
                offset = va - start
                chunk = raw[offset:offset + length]
                if len(chunk) < length:  # virtual tail (.bss-like) reads as zero
                    chunk = chunk + b'\0' * (length - len(chunk))
                return chunk
        raise ValueError(f'VA {va:08X} is outside every mapped section')

    def u32(self, va: int) -> int:
        return int.from_bytes(self.read(va, 4), 'little')

    def cstring(self, va: int, limit: int = 256) -> str:
        raw = self.read(va, limit)
        end = raw.find(b'\0')
        return raw[:end if end >= 0 else limit].decode('latin-1')


def read_rows(image: Image, table_va: int) -> list[dict]:
    """Rows until the `{"", NULL}` sentinel; the sentinel itself is not returned."""
    rows = []
    va = table_va
    for _ in range(MAX_ROWS):
        name_ptr = image.u32(va)
        handler = image.u32(va + 4)
        if name_ptr == 0:
            break
        name = image.cstring(name_ptr)
        if name == '' or handler == 0:
            break
        rows.append({
            'name': name,
            'handler': f'{handler:08x}',
            'row': f'{va:08x}',
            'name_pointer': f'{name_ptr:08x}',
        })
        va += 8
    return rows, va


def annotate(rows: list[dict], index_path: Path) -> dict:
    stats = {'indexed': 0, 'ledger_named': 0, 'reconstructed': 0, 'no_function': 0}
    if not index_path.exists():
        return stats
    db = sqlite3.connect(index_path)
    for row in rows:
        handler = int(row['handler'], 16)
        hit = db.execute(
            'SELECT address, name, ledger_name, recon_kind, recon_name, recon_status '
            'FROM functions WHERE address = ?', (handler,)).fetchone()
        if hit is None:
            row['ghidra_function'] = None
            stats['no_function'] += 1
            continue
        stats['indexed'] += 1
        row['ghidra_function'] = f'{hit[0]:08x}'
        row['ghidra_name'] = hit[1]
        if hit[2]:
            row['ledger_name'] = hit[2]
            stats['ledger_named'] += 1
        if hit[3]:
            row['reconstruction'] = {'kind': hit[3], 'name': hit[4], 'status': hit[5]}
            stats['reconstructed'] += 1
    db.close()
    return stats


def build(table_va: int, config_path: Path, index_path: Path) -> dict:
    target = load_target(config_path)
    image = Image(Path(target['binary']))
    rows, sentinel = read_rows(image, table_va)
    stats = annotate(rows, index_path)
    by_handler: dict[str, list[str]] = {}
    for row in rows:
        by_handler.setdefault(row['handler'], []).append(row['name'])
    shared = {handler: names for handler, names in by_handler.items() if len(names) > 1}
    return {
        'source': 'tools/lua_bindings_export.py over the installed battlestationspacific.exe',
        'table': f'{table_va:08x}',
        'sentinel_row': f'{sentinel:08x}',
        'row_bytes': 8,
        'count': len(rows),
        'registered_by': '006b8610',
        'distinct_handlers': len(by_handler),
        'handlers_shared_by_several_names': shared,
        'index_stats': stats,
        'bindings': rows,
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('--table', default=f'{DEFAULT_TABLE:08x}', help='table VA (hex)')
    parser.add_argument('--output', type=Path, default=DEFAULT_OUTPUT)
    parser.add_argument('--config', type=Path, default=ROOT / 'config' / 'target.json')
    parser.add_argument('--index', type=Path, default=ROOT / 'local' / 'bsp_index.sqlite')
    parser.add_argument('--check', action='store_true',
                        help='compare against the existing file instead of writing it')
    args = parser.parse_args()

    data = build(int(args.table, 16), args.config, args.index)
    text = json.dumps(data, indent=2) + '\n'
    if args.check:
        if not args.output.exists():
            print(f'{args.output} is missing')
            return 1
        if args.output.read_text(encoding='utf-8') != text:
            print(f'{args.output} is stale')
            return 1
        print(f'{args.output} matches the image: {data["count"]} bindings')
        return 0
    args.output.write_text(text, encoding='utf-8')
    print(f'{args.output}: {data["count"]} bindings, {data["distinct_handlers"]} distinct handlers, '
          f'{data["index_stats"]} ')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
