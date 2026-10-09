# Header13 native query dependency TEXT correction (CC12)

The r02 candidate removes two nonexistent Python-package requirements from
`native_query_helper_files`. The supported repository route uses
`ghidra_export.Client` and standard-library `urllib`. This is **Source credit0**:
Root still owns the independent review, acceptance receipt and fresh qualification.

Root reported that its metadata preparation stopped at package discovery after
materializing the unaccepted r01 family. This worker neither inspected that Root
family nor repeated the package lookup. No selected generator or helper ran.

The correction lives in `local/h13gen_depfix/header13_final_generator.r02.py.txt`:
`c30a8e242ec0432bacbf38d76199dfffc244220970620cb81b4b1b5a1b7dbbc4` (130936 bytes).
`Root_generation_selection.proposed.r02.json` identifies that candidate and remains
`accepted:false`. The old r01 candidate and proposal are preserved in a complete,
byte-identical frozen copy of the selected88-file family.

Only the dependency-discovery function changes. It retains the existing recursive
`tools/**/*.py` file set and stable ordering. The `ghidra_bridge` and `jfx_bridge`
lookup loop is removed because neither package is imported by the query path.
Whole-module AST and raw prefix/suffix comparisons independently verify that every
other byte is unchanged. There are no optimizable Python `assert` statements.

The capstone, pefile and ordlookup analysis-package pins remain intact, as do the
interpreter, toolchain, environment, compiler/linker, Root receipt, exact launch,
six-key ordering, complete helper closure and strict trailing-only CFG guards.
No linked gate, import/provider promise or process permission is broadened.

The proposed selection retains every old67 reviewed-artifact row and appends r02,
giving68. Its materialization plan remains78 unique rows: only the `recipe.py`
origin changes. The fixed Main `local/h13p2` identity, draft39, Q15, currentSource10,
context and pattern pins are unchanged. The old r01 candidate remains reviewed.

Static inspection covers the following actual route:

- `tools/bsp.py:18-38` imports four local modules and resolves shared exports.
- `tools/bsp.py:280-287` opens the existing SQLite index or returns None; it never rebuilds.
- `tools/bsp.py:675-691` creates and verifies `ghidra_export.Client`.
- `tools/bsp.py:756-787`, `837-844` and `845-873` implement count/proto, bytes and disasm.
- `tools/ghidra_export.py:6-15` and `72-111` use standard-library imports and loopback HTTP.

The conservative local import closure is `bsp`, `branch_sync`, `coordination`,
`ledger`, `workspace`, `ghidra_export` and `ghidra_launch`. With the unchanged
`BSP_GHIDRA_AUTOSTART=0`, the error path imports the last module but rethrows before
probe/ensure. Unrelated bsp commands lazily import pefile/capstone; their real
analysis pins remain required. No external Python package belongs to the selected
count/proto/bytes/disasm path in the frozen current source.

All46 Main `tools/**/*.py` files are frozen and checked before/after. Five have
Main/worktree line-ending differences; both hashes are recorded and the frozen
copies preserve exact Main bytes. No tool source was changed. All88 selected
input files, including their seal and proposal, are likewise unchanged.

This records the declared repository Python frontier. It does not establish
interpreter/site/stdlib hermeticity or loaded child dependencies. In particular,
`workspace.exports_dir` can invoke `git rev-parse`; its source is pinned, while
runtime executable resolution remains a Root environment concern.

Two stopped versions of this worker's metadata utility are retained: an ordering
assumption in the seal comparison, then an overly strict Main/worktree newline
comparison. Both were corrected in separate utility versions; neither stop came
from executing selected code. Every utility, log, stop, frozen input and metadata
snapshot belongs to the final exact seal.

Root must independently review r02, retain the existing unaccepted r01 recipe and
selection under explicit historical names, and author a real new TEXT receipt.
The receipt must bind all68 reviewed artifacts and the actual tool-source pins.
The separate linked review, sole native attempt and recorded-reader verification
remain pending. No Root receipt, runtime capture or provider result was authored.

Only this doc and its paired JSON report are committed. No Source, CMake, ledger,
Ghidra or tests changed; no compiler, linker, Native, provider or target operations
ran. The final handoff supplies the exact `local/h13gen_depfix/seal.json` count and
hash after committing the metadata and releasing its lease.
