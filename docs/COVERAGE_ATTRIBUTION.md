# Execution-coverage attribution

`tools/coverage_census.py` attributes supplied coverage observations. It never
launches a game, attaches a debugger, downloads a tracer, or writes to an
installation/profile. Outputs belong under ignored `local/`; the primary
coverage lane owns run receipts, committed board summaries, and the final
mission report.

Coverage means reach reported in the supplied run logs. It establishes
neither correctness nor parity. Repeated drcov records are not call counts.

## Inputs and invocation

First refresh the index with `python tools/bsp.py index --if-stale`. Supply the
`wire_readiness.json` corresponding to the coverage build; do not assume a
different checkout's source-level wiring analysis describes that build.

```powershell
python tools/coverage_census.py --mission USN04 --index local/bsp_index.sqlite --wire local/output/wire_readiness.json --original-log local/coverage/original.log --original-image local/coverage/original.exe --bsp-log local/coverage/bsp.log --bsp-image local/coverage/bsp_game.exe --pdb local/coverage/bsp_game.pdb --output local/output/coverage_census.json
```

`--original-log` and `--bsp-log` repeat to union supplied process/thread logs.
The same path cannot be supplied twice. Each side is optional: an unavailable
original trace, for example, leaves original execution cells **null**. Module
basenames default to `battlestationspacific.exe` and `bsp_game.exe`; override
them explicitly for a differently named copied image. The command above uses
placeholder paths, not evidence that either mission was traced.

The tool refuses to overwrite its output. Exit zero means the requested
available attribution completed without a top-level error; two means BSP
symbolization was unavailable and the JSON preserves that unknown state;
one means input/parsing failed. Inspect `summary` and completeness fields:
an exhausted inline scan is explicitly incomplete even when positive hits
have already been found.

## Drcov observations

The reader supports drcov file versions 2/3, module tables 2 through 5, and binary
or text basic-block tables. The declared column names drive module parsing;
module zero is not assumed to be the executable. Paths containing commas are
preserved. Custom module fields and older/unknown layouts fail explicitly.

The binary record is eight little-endian bytes: a uint32 offset, uint16 size,
and uint16 module/segment ID. Version 2 offsets use the containing module's
base; version 3 offsets use the named segment's base. RVA normalization
preserves that distinction. Bounds, absent module IDs, zero-sized blocks, and truncated
or extra table bytes are checked. Unknown-module records and other-module
records remain counted in input receipts. Distinct selected `(RVA,size)`
observations are deduplicated across supplied logs.

Drcov block sets have no per-block game timestamps. Clock start/end receipts
describe a run window; they cannot trim startup/menu blocks from an existing
log. The mission label is supplied by the operator, not recovered from drcov.
Different start paths or collection windows remain a separate explanation
for a cross-table difference. Select the logs containing the target module;
an unrelated helper-process log is rejected rather than treated as an empty
mission trace.

These definitions were checked against the official DynamoRIO
[drcovlib record declaration](https://github.com/DynamoRIO/dynamorio/blob/master/ext/drcovlib/drcovlib.h),
[record producer](https://github.com/DynamoRIO/dynamorio/blob/master/ext/drcovlib/drcovlib.c),
and [module-table producer](https://github.com/DynamoRIO/dynamorio/blob/master/ext/drcovlib/modules.c).
The installed release header inspected by this lane is in the primary
coverage tree's `local/tools/dynamorio/DynamoRIO-Windows-11.91.20749/`.
Acquisition/version/hash receipts belong to the primary lane.

For the original, every selected containing-module base must be `00400000`.
When an image is supplied, its PE timestamp and image extent must match the
module record. Image and log SHA-256 values are recorded separately: drcov
does not store a cryptographic hash of the loaded executable, so the guarded
run receipt remains the binding evidence. The JSON records supplied image
hashes, PE sections, PDB CodeView identity, tool/index/wire provenance, and Git
HEAD. Missing image input must not be described as a verified executable hash.

## Original attribution and scope

Each selected block is intersected with `ranges` in `local/bsp_index.sqlite`.
These are contiguous swept ranges, not arbitrary Ghidra function-body unions.
Any overlap establishes a reported function hit, and each function retains
its distinct observed blocks. A function without an indexed body can be
positively identified at its exact start; absence elsewhere stays unknown.

EH funclets use `funclet_parent` and the repository's compiler-name prefixes.
Swept `undefined` chunks are reported separately, with their distinct blocks.
Bytes covered by neither function ranges nor undefined chunks remain explicit
unattributed spans. They are not silently discarded or invented as functions.
An executed named function lacking a function reconstruction ledger record is
also counted separately. Block-overlap buckets are not additive partitions:
one block can intersect more than one category/range.

The board-game flag reuses `progress_board.is_game`, its library-name,
compiler/funclet, and out-of-scope-status exclusions. Segment categories reuse
`progress_board.category_for`. This is the board's **categorical boundary**;
its historical body/reference target-membership gate is a separate
denominator. EH and vendor functions receive no game-function credit. Unknown
code receives no invented game-function identity. The output exposes both
all identified board-game hits and their function-ledger intersection.

## BSP PDB attribution

Symbolization is offline in a private DbgHelp session with a unique nonzero
session token. It does not use a game process handle. The explicit local PDB
search path ignores the environment symbol path and uses exact-symbol
matching. Loading must establish a full PDB with the PE's CodeView GUID/age,
and the loaded PDB path must equal the supplied path.

Outer function records come from `SymEnumSymbolsEx` and `SymFromAddr`.
Inline inventory uses `SYMENUM_OPTIONS_INLINE`; observed inline frames use
`SymAddrIncludeInlineTrace`, `SymQueryInlineTrace`, and
`SymFromInlineContext`, with source filenames from the corresponding line
APIs. This follows Microsoft's
[session requirements](https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-syminitialize),
[symbol enumeration options](https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symenumsymbolsex),
[inline query contract](https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symqueryinlinetrace),
and [inline symbol lookup](https://learn.microsoft.com/en-us/windows/win32/api/dbghelp/nf-dbghelp-symfrominlinecontext).
ctypes layouts were checked against the installed Windows SDK `DbgHelp.h`.

Every covered byte in the union of block spans is queried for inline records,
because an inline site can begin inside a basic block. Adjacent equal contexts
are compressed afterward. Inline context numbers are interpreted relative to
the queried address; they are not cached as persistent site identities.
`--max-pdb-addresses` defaults to five million covered bytes. Exhaustion or an
API/inventory failure leaves inline completeness false and prevents negative
BSP execution claims for unobserved functions. Positive mapped hits remain
usable as a lower bound. A block-coverage log is still not an instruction-
retirement trace or an exact tick/window boundary.

C++ names resolve through the wire file's `ident` values. A bare leaf name
alone is insufficient: require an original-address suffix, an exact qualified
wire identifier, or a matching PDB source/implementation filename. Inventory
records needing that source check are queried before establishing presence;
unavailable source identity remains unknown. In particular, MSVC `std::next`
or `std::to_string` records must not establish execution of a game class's
similarly named method. Rejected leaf mappings and source-unverified records
are counted separately. Per-address rows
retain `ident_via`, wiring state, source, and implementation source. A unique
heuristic mapping remains a mapping approximation, not an ABI proof. A source
use behind an OFF switch remains source wiring; it establishes no execution.
Unresolved names and multiple candidate identifiers stay separate.

Legacy wire files omit `ident`/`ident_via` for ordinary address-suffix rows.
Only their `unknown_no_identifier` state establishes an unresolved source
identifier; a missing optional field alone does not. The census distinguishes
that state count from missing explicit fields and includes all source-resolved
rows when checking absent PDB records. Executed PDB symbol/site-record counts
also remain separate from unique C++ names and original-address counts.

Different PDB function records sharing an RVA make the logical caller
ambiguous. Observing that code does not credit every possible alias. Likewise,
a wire function with no matching outer symbol or inline record is reported as
such; it is not definitely labelled optimized away, removed, or folded by ICF.
The output separately lists/counts missing outer block symbols, unresolved
executed C++ symbols, ambiguous executed original candidates, unresolved wire
identifiers, and absent outer/inline records. An unavailable inventory leaves
the absent-record count null.

## Tables and future count sources

`per_original_address` has tri-state `orig_executed` and `bsp_executed`. True
is an identified hit. False means not observed in the supplied usable logs
and sufficiently resolved attribution. Null means missing trace, unresolved
identity/symbol data, shared-RVA ambiguity, or incomplete attribution. No
trace is never converted to a false/zero execution claim.

Wire rows absent from the function index remain in `unindexed_original_rows`,
with unknown native identity/scope and any positive BSP mapping preserved.
They do not silently disappear or receive invented board-game credit.

The definite `orig yes / bsp no` frontier ranks distinct original blocks,
then static original caller count, and groups addresses by board category and
textual `game_hosts*.cpp` mention. A host text mention can be a comment and is
not a call edge. `orig yes / bsp unknown` remains a separate unresolved list.
The other cross-cells remain explicit; `orig no / bsp no` means not observed
in this supplied mission window, not dead code.

The shared `Observation` model carries normalized RVA/size and an optional
positive `entry_count`. Drcov always leaves that field null. A future entry
counter can provide observations to the same original/PDB attribution layers;
zero count rows must be omitted. Counts attach only to exact function starts,
and inline/shared-RVA ambiguity is retained. No production entry-count source
or inferred call-count total is supplied in this packet.

## Focused checks

Python compilation and whitespace checks passed. Synthetic binary/text logs
checked table versions 2/5, nonzero module IDs, comma paths, duplicate blocks,
truncation rejection, EH/undefined/unattributed separation, and missing-side
nulls. The end-to-end synthetic census is labelled `SYNTHETIC` and is not a
mission observation.

The first real BSP smoke used file version 3 with module-table version 5.
Its strict eight-byte tables were accepted after checking the installed
release header's v2 module-relative/v3 segment-relative distinction. A focused
two-segment fixture checks that both versions normalize to the same image RVA.

Two small optimized x86 `/Zi /Zo /DEBUG:FULL` probes were compiled under
ignored `local/coverage-probe/` with `/MANIFEST:EMBED`; neither executable was
launched. Offline PDB checks established GUID/age matching, nested inline
sites beginning inside a supplied block, address-relative inline contexts,
the scan-budget unknown state, and logical ambiguity when two probe functions
share an `/OPT:ICF` RVA. Receipts include `pdb-attribution-check.json` and
`shared-rva-check.json` in that folder. These establish parser/API behavior,
not original or `bsp_game` mission coverage totals.
