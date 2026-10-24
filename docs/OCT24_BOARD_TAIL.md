# October 24 board tail and wiring audit

Baseline: `515837c10`, refreshed index from the shared 2026-10-19 Ghidra snapshot.
`local/output/oct24_board_before.json` reproduces the requested board values:
target **20,726**, game done **20,714**, pool remaining **12**, game outside
**316**, and unreferenced **791**. The wire baseline in
`local/output/oct24_wire_before.json` is:

| State | Before |
| --- | ---: |
| wired | 8,217 |
| ready | 1,761 |
| callees_done_no_wired_caller | 10,445 |
| unknown_no_identifier | 1,744 |
| blocked_on_callees | 94 |

These are source-level, file-granular estimates, including disabled switches;
they do not establish runtime execution or ABI compatibility. The baseline
MSVC map was copied from the coordinator's build (2026-10-18); mappings are
validated against current source bodies, rather than trusting that older map.
No game runs are part of this lane.

## Counting corrections

`game_outside` means a target function has NULL `functions.segment`: it does
not mean missing or excluded. The board's target also includes reconstructed
game functions without incoming edges, so withdrawing unsupported completion
credit may move such a row into `unreferenced`.

The original wiring report included compiler/library exclusions that the board
already omits. It now shares the board's reviewed library-name/status rules.
An audited function record with status
`metadata_only_no_function_body_verified_oct24` retains its address, original
evidence and reason in the ledger, but no longer supplies reconstructed-body
credit to either report. This makes gaps visible without deleting provenance.

`config/wire_identifiers*.json` can identify a verified implementation without
editing busy source files. Each record cites source evidence and must name a
real C++ body in its implementation source. The resolver rejects missing
bodies, missing evidence, duplicate addresses and a symbol suffixed with a
different original address. Shared semantic algorithms retain the original
record's ABI/layout qualifications; a table accessor alone is not a body.

Work in progress: pool and outside audits, identifier mappings and final
counts will be added in subsequent landed batches.
