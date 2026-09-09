"""Report local export and reconstruction coverage without a live Ghidra connection."""
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
ledger = json.loads((root / 'config/reconstruction.json').read_text())
snapshot_path = root / 'exports/bsp/snapshot.json'
if snapshot_path.exists():
    snapshot = json.loads(snapshot_path.read_text())
    count = snapshot['internal_function_count']
    complete = sum(all((folder / name).exists() for name in ('decompiled.c', 'assembly.txt', 'metadata.json'))
                   for folder in (root / 'exports/bsp/functions').glob('*') if folder.is_dir())
    print(f"Snapshot: {snapshot['project']} {snapshot['program']} ({snapshot['utc']})")
    print(f"Internal functions: {count}; total including externals: {snapshot['total_function_count']}")
    print(f"Pseudocode + assembly exported: {complete}/{count}")
    print(f"Reconstructed: {len(ledger['functions'])}/{count} ({100 * len(ledger['functions']) / count:.4f}%)")
    print(f"Partial routine fragments: {len(ledger.get('fragments', []))} (excluded from reconstructed count)")
    functions_path = root / 'exports/bsp/functions.json'
    if functions_path.exists():
        rows = [row for row in json.loads(functions_path.read_text()) if not row.get('isExternal')]
        reviewed = {row['address'] for row in ledger['functions']}
        funclets = sum(row['name'].startswith(('Unwind@', 'Catch_All@')) for row in rows)
        default_names = sum(row['name'].startswith('FUN_') for row in rows)
        reviewed_named = sum(row['address'] in reviewed and not row['name'].startswith('FUN_') for row in rows)
        other_named = len(rows) - funclets - default_names - reviewed_named
        candidates = default_names + len(reviewed)
        print(f"Compiler EH funclets (Unwind@/Catch_All@): {funclets} (not reconstruction targets)")
        print(f"Library, thunk, FID and inventory-tagged names: {other_named} (see reports/library_inventory/SUMMARY.md)")
        print(f"Untagged FUN_ candidates: {default_names}")
        print(f"Reconstructed against candidates: {len(ledger['functions'])}/{candidates} "
              f"({100 * len(ledger['functions']) / candidates:.3f}%)")
    tags_path = root / 'config/ghidra_tags.json'
    if tags_path.exists():
        tags = json.loads(tags_path.read_text())
        renames = sum(tag['action'] == 'rename' for tag in tags)
        print(f"Inventory tag ledger: {renames} names + {len(tags) - renames} bookmark-only entries (tools/ghidra_tag.py)")
else:
    print('No local export snapshot. Run ghidra_export.py snapshot first.')
print('Game rebuild: incomplete; no game executable target')
print('ABI compatibility and gameplay equivalence: not established')
