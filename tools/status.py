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
else:
    print('No local export snapshot. Run ghidra_export.py snapshot first.')
print('Game rebuild: incomplete; no game executable target')
print('ABI compatibility and gameplay equivalence: not established')
