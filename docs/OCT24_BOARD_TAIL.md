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
different original address unless the mapping explicitly records verified
`shared_implementation` evidence. Shared semantic algorithms retain the original
record's ABI/layout qualifications; a table accessor alone is not a body.

## Landed audit batches

The twelve original pool rows are exhaustively listed in
[OCT24_TAIL_POOL.md](OCT24_TAIL_POOL.md). Four independent Mit lifecycle
bodies were added and pass MSVC Win32 Release and the worker's three existing
CTest targets. The primary integrated build also passes its two configured
CTest targets. These interfaces retain actual pool storage and explicit CRT
services; native ABI and FH3/SEH transport are not claimed. No switches or
gameplay runs were added. The five segment-67 rows and the cc9 surface-target
forwarder remain with their owners; Listbox remains explicitly partial.

`00BF3ACC` is now `non_callable_continuation_out_of_scope`: it is an arm inside
the already reconstructed 399-byte `00BF3A80`, entered after a shared SEH
prologue. Its saved Ghidra function split, parent evidence and fragment record
are preserved. It supplies neither an independent callable ABI nor another
target function.

[OCT24_TAIL_OUTSIDE.md](OCT24_TAIL_OUTSIDE.md) categorizes all 316 original
outside rows, samples the 791 unreferenced rows and records the undefined-code
check. Its original breakdown was 256 implementation records, 53 recorded-only
gaps, four partial bodies and three compiler boundary projections. The 53
records explicitly saying "Not reconstructed" have lost completion credit.
The render audit subsequently identified another table-only outside record,
`00B4B480`. The three compiler starts `00CB89AD`, `00CC1A01`, `00CC1B36` are now
`compiler_generated_out_of_scope`: each is exactly ten bytes, `MOV EAX,FuncInfo;
JMP __CxxFrameHandler3`, with parent-prologue/EH references only. Their source
cleanup projections remain qualifications of their parents.

The original unreferenced set has 728 rows with no indexed incoming edges and
63 with only unwind/thunk tail transfers. Samples cover Dyn, game code,
untagged cleanup/renderer and trivial bodies. No omitted ordinary call, data,
vslot or Lua edge was found. This is an incoming-edge census, not proof that
EH cleanup cannot run. Undefined vtable chunk `007B3AA0` is a credible separate
missing-body candidate outside the board denominator; it was not reconstructed.

Identifier evidence is in [OCT24_TAIL_IDENTIFIERS_LOW.md](OCT24_TAIL_IDENTIFIERS_LOW.md)
and [OCT24_TAIL_RENDER_POOLS.md](OCT24_TAIL_RENDER_POOLS.md). Low-address auditing
accounts for every one of its 513 original rows: 152 explicit body mappings,
316 metadata/partial-body corrections, six evidenced stock STL exclusions,
25 existing library exclusions, eleven parser-supported definitions and three
pool bodies belonging to the pool packet. Render/pool auditing adds 219 body
mappings and exposes 31 table-only rows. The primary currently adds 123
evidenced header/shared-template/robot mappings and 54 no-body corrections.
Further GUI and host identifier batches and final counts are still pending.

Validation includes source-definition checks for mappings, exact preservation
of original fields/ABI qualifications in corrected records, Python compilation,
and a focused isolated batch-ledger check (invalid input and duplicate keys
write nothing; valid records check all leases before mutation). Common plain
method names such as `dispatch` and `initialize` count uses only in their own
implementation file when their names collide across files. Destructor bodies
can resolve, but implicit destructor execution is not inferred from a type-name
mention.
