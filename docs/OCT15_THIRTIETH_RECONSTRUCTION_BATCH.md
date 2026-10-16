# Controller-name pair destruction integration

Whole `006A2BB0` covers 143 Native bytes/46 instructions. Logical key-cleanup
action `00C7F200` covers eight bytes/two instructions. Read-only handler and
44-byte EH evidence remain separate. No global coverage increment is claimed.

The pair uses the published whole embedded-tree range. Key cleanup stays armed
through current-head release, head/count reset and current key-data capture.
It then ends its EH scope before normal length/getter work. A normal getter
failure cannot retry key cleanup; an armed failure cleans the current key.

Root accepted frozen `84b56af9d4ca1427beda72a0adc90094e50ab8bc` after 14 independent checks, fresh PE/live bytes,
50 Native starts, six transfers and 38 provider LF comparisons. All actual
CMake code was reviewed: 299 bytes/115 physical instructions, 12 code and five
data relocations. Combined and worker code bytes match; the only code relocation
name difference is an explicitly checked private translation-unit symbol.
The 44-byte generated EH data and its relocations also match.

Combined Source `dedb993659b9a0c8c39efc9af9c44e88e2e3ff34` passed default MSVC Win32 Release and the three
existing checks (8.65s). Those checks do not execute this pair against
Native state. Two annotations were saved with preimages and exports refreshed.
Snapshot count remained 76774; the index was rebuilt. No new tests/probes ran.

Source adds RawCtx and uses RET4; Native uses plain RET. The logical action has
an explicit Source API instead of reading the Native runtime EBP frame.
Nested cleanup failure, current CRT/provider ownership, private frames, flags,
faults, FH3/SEH/RTTI, heap, binary ABI and game behavior remain unproved.

Machine-readable receipt: `reports/oct15_thirtieth_reconstruction_batch.json`.
