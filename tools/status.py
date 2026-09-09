"""Report local export and reconstruction coverage without a live Ghidra connection."""
import json
import sys
from collections import Counter
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from ledger import load_reconstruction, load_tags  # noqa: E402

root = Path(__file__).resolve().parents[1]
ledger = load_reconstruction()  # sharded config/reconstruction/*.jsonl plus any legacy file
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
        print(f"Compiler EH funclet names (Unwind@/Catch_All@): {funclets}")
        print(f"Other named internal functions: {other_named} (includes libraries, generated and in-house code)")
        print(f"FUN_ names: {default_names} (may still carry provisional bookmarks)")
        print("Naming/tagging does not establish a reduced reconstruction denominator or completed behavior.")
    tags = load_tags()  # sharded config/tags/*.jsonl plus any legacy file
    if tags:
        renames = sum(tag['action'] == 'rename' for tag in tags)
        print(f"Inventory tag ledger: {renames} names + {len(tags) - renames} bookmark-only entries (tools/ghidra_tag.py)")
        counts = Counter(tag['category'] for tag in tags)
        print("Inventory categories: " + ', '.join(f'{key}={value}' for key, value in sorted(counts.items())))
        print("Tag confidence concerns the recorded evidence; STL-shaped, trivial and RTTI tags are not skip approvals.")
else:
    print('No local export snapshot. Run ghidra_export.py snapshot first.')
print('Game rebuild: incomplete; no game executable target')
print('ABI compatibility and gameplay equivalence: not established')
