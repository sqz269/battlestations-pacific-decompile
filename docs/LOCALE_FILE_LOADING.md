# Locale file loading and reload correction

The locale manager now loads existing suffix variants in registration order and the
original table last. Base text therefore overwrites duplicate keys supplied by a
variant. The old `FileReader` abstraction omitted those variants and incorrectly
described `00bdef90` as collecting identical filenames across every mount.

Source: `src/locale_tables.cpp`, `include/bsp/locale_tables.hpp`. All descriptive names
are hypotheses, not recovered symbols. Analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, with live count/prototype/bytes checks and saved assembly.
See `reports/locale_file_loading.json` for validation and remaining boundaries.

## Native contracts and evidence

| Function | Original ABI | Evidence and behavior |
| --- | --- | --- |
| `00aa0020` | ECX manager; by-value native string at stack +4/+8, forwarded force at +C, skip-sidecar byte at +10; RET 10h | `00aa0077` obtains existing suffix paths, `00aa0088` appends original, `00aa00b0..0365` opens each mode 2 and parses in order. Force is forwarded by callers but unused here. |
| `00aa06d0` | ECX manager, force byte at stack +4; RET 4 | `00aa06f3..0704` returns when count is nonzero and force false. Otherwise loads without clearing map, without registration gate and without GUI refresh. |
| `00aa09d0` | ECX manager, native string pointer at stack +4; RET 4 | `00aa0a03..0a3b` compares case-insensitively, stores changed spelling and clears map; `00aa0a40..0a60` gates loading/refresh on registered names. `00aa0ca1/0ca8` obtains GUI manager and refreshes it after loading. |

Both outer loaders construct `lockit/<language>.lan`, load it with skip-sidecar false,
then probe decimal siblings `...lan0` through `...lan98`, stopping at the first failed
`00bdf4c0` resolution (`00aa083f`, `00aa0b90`). Resolution mutates a temporary; the
resolved result is discarded. Each successful probe reconstructs the original
numbered spelling for the actual load (`00aa0860`, `00aa0baf`) and passes skip-sidecar
true. Probe and read must remain separate operations. No sidecar suffix variants or
numbered sidecars are requested.

`00aa036b` gates sidecar loading. `00aa0379..0410` clears both ushort vectors before
opening original-name + `x`, mode 2. `00aa04cd..04d8` reads and discards two bytes,
then `00aa0502..05d0` appends pairs until cursor reaches size. The prefix has no
established meaning. The installed English sidecar is empty and is accepted. It does
not demonstrate a headerless format.

Native VFS `00bdf310` reports an unsuccessful mandatory open through manager +90 and
returns null if the diagnostic returns. Locale code immediately converts/dereferences
that stream (`00aa00d2/00aa00d8`, `00aa04c8`). Missing mandatory files are therefore
reported as errors by this host projection. They are not silently skipped. VFS
providers must also reject an opened but unusable stream rather than opening another
provider as fallback. Ordered suffix selection remains delegated to `00bdef90`.

## Parser and state corrections

Complete records are inserted before any later malformed record is reported. The old
loader discarded all records from a file if its tail was malformed. Native insertion
is immediate at `00aa01e9` within the record loop. At first-token EOF, native
`00aa02c0..0327` exits before interpreting the last read byte: an unfinished first
token is dropped, including a space that is itself the final byte. An unfinished
second token/value would continue unsafe reads natively; the port reports it.

Keys and stored values stop at embedded NULs as the native C-string copy loops do.
Scanner progress still consumes the full record through its marker. Bounds checks
remain deliberate host guards against overflowing the original stack buffers.
Partial sidecar pairs report an error while retaining preceding complete pairs.

Case-insensitive same-language requests perform no reads, clear or refresh and keep
the old spelling. Changed languages clear the map even when no table is registered.
Only successful registered loads call the required GUI host. Forced reload updates
and adds keys without removing ones absent from the new files. Map clear leaves
sidecar vectors alone; the next successful table phase clears them before its base
sidecar read. `loaded_files()` is diagnostic host state containing successfully opened
table paths in order, including a table whose parse later fails; probes/sidecars are
excluded.

## Validation and scope

Win32 Release compilation and the existing checks run through `scripts/build.ps1`.
The focused ignored fixture `local/locale_file_loading_fixture.cpp` loads the installed
999674-byte English table: 6864 records, 6856 distinct keys and `fe.BRIEFING` =
`Briefing`. It also checks variant/base overwrite order, numbered resolve/read order,
headered sidecar pairs, clear-before-failed-open, retained keys on force reload,
same-language and registration gates, required GUI callback order, per-record commits
on malformed input, EOF/NUL behavior and failed mandatory reads after a successful
probe. It is one local fixture, not a new permanent test suite.

These are reconstructed typed C++ interfaces with actual file-format validation.
The false `_free` no-return override at `00aa0626` was cleared and its 78-byte
cleanup tail decoded through `RET 10h` at `00aa0676` (exclusive end `00aa0679`).
The stored function body still ends at `00aa062a`; decoding did not extend that
metadata. `reports/locale_file_loading_flow_repair.json` preserves this limitation.
They are not native-layout/ABI replacements and do not establish game or GUI runtime
validation. The source and GUI services are required dependencies; this packet does
not implement the GUI tree refresh or own its singleton lifetime. Native diagnostics,
pool allocation, exception behavior and undefined malformed-input behavior are not
claimed. The sidecar values' consumers and meaning remain unproven.
