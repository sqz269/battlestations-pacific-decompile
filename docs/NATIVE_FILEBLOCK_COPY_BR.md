# FileBlock copy semantics correction (BR)

Address: `00BE0A30`, call site `00BE0A9F` -> `00BF7680`.

The BQ constructor's direct copy now uses `std::memmove`. The native callee
retains its correct `_memcpy` library name, but its full body includes backward
overlap handling, including STD / REP MOVSD / CLD at the BF7844 branch. The
source operation must permit that behavior; its name alone is insufficient
evidence for a non-overlap restriction. The caller's current field reads and
zero-count behavior remain as reconstructed.

The BQ normal and deliberate observer-failure probes were copied and rebuilt
against the corrected source in a new BR artifact directory. Both passed;
the historical BQ reports and binaries remain intact. The new queue fixture
also creates and retires a FileBlock through the corrected constructor. These
checks exercise ordinary construction and retained failure behavior; they do
not independently establish arbitrary-overlap behavior or native unwind parity.

`reports/native_fileblock_copy_br.json` records the complete constructor span,
copy-call evidence, source hash and BR fixture artifacts. The original BQ
FileBlock retained-frame and native ABI/gameplay limits still apply.
