# CC12 fallback classifier type writer discovery

**The writer is still unestablished.** This metadata-only packet identifies an
exact candidate at `00CD82C0`, derived from a recorded `DATA` reference to
`0109021C`. No recorded `WRITE` reference was returned for the three queried
fallback cells. Neither result proves how the cells are initialized or updated.
Production classifier attachment remains held.

Baseline: `8ab093935e70c6da1918861eebe0d03228cd9a2f`. Tracked ownership is limited
to this document and the matching JSON report. The approved live scope was named
`get_xrefs_to` at `0109021C`, `01090220`, `01090224`, typed-flow at the known
consumer `00B86950`, and typed-flow at an exact xref-derived candidate after its
lease was extended. No Native instruction bytes, disassembly, pseudocode, data,
table or handler contents were read. Function extent metadata is not a body read.

## Identity and retained queries

The first typed-flow capture queried `00B86950`. The three named xref requests
followed, with exact program `/battlestationspacific.exe`, `offset=0`, `limit=80`.
A final typed-flow batch queried `00B86950` and `00CD82C0`, with `max_ranges=16`.
All three typed responses were complete and passed the existing validator.

Every typed response reports the actual project marker
`C:\Users\sqz269\bsp.gpr`, project `bsp`, location `/C:/Users/sqz269/`, program
`/battlestationspacific.exe`, language `x86:LE:32:default`, default address space
`ram` and image base `00400000`. Each response reports modification number `5`
both before and after. The surrounding `Client.verify()` checks agree on project,
program, language and base; the captured program has 64,730 functions.

The xref tool itself supplies text and an MCP wrapper, without a runtime project
locator or modification number. Its exact arguments and complete returned wrapper
are retained; the matching typed responses bracket the xref calls. They are not
represented as raw HTTP xref receipts. Typed-flow HTTP status, raw response bytes
as Base64, raw SHA-256, exact URL and CLI stdout/stderr/exit receipts are preserved.

## Recorded references

| Queried cell | Recorded source | Kind | Recorded owner name |
| --- | --- | --- | --- |
| `0109021C` | `00B86860` | `READ` | `BSP_UnknownResourceItem_GetCurrentTypeToken` |
| `0109021C` | `00B86960` | `READ` | `BSP_UnknownResourceItem_MatchesCurrentTypeToken` |
| `0109021C` | `00B86954` | `DATA` | `BSP_UnknownResourceItem_MatchesCurrentTypeToken` |
| `0109021C` | `00CD82C0` | `DATA` | `BSP_UnknownResourceTypeTokens_StartupEntry` |
| `01090220` | `00B86960` | `READ` | `BSP_UnknownResourceItem_MatchesCurrentTypeToken` |
| `01090224` | None returned | — | Tool reports no references found. |

The five returned rows fit below the requested limit. This is a report of recorded
xrefs, not an exhaustive proof about indirect references, pointer-based access or
runtime writes. In particular, no reference at `01090224` does not disprove the
existing three-cell Source consumer. `DATA` must not be promoted to `WRITE`.
Recorded names are descriptive metadata and do not establish startup order,
storage layout, native ABI or behavior. Ownership at read sites other than the
queried entry is reported by the xref text; it was not separately re-proved.

## Exact owner and flow metadata

At `00B86950`, both exact and containing function entries are `00B86950`.
The complete one-range extent is `00B86950..00B86977` (40 bytes). At the queried
instruction, the listing reports an exact instruction of length 4, ordinary
fallthrough to `00B86954`, no flow override, and no direct call targets.

At `00CD82C0`, both exact and containing function entries are `00CD82C0`.
The complete one-range extent is `00CD82C0..00CD82C9` (10 bytes). At the queried
instruction, the listing reports an exact instruction of length 5, ordinary
fallthrough to `00CD82C5`, no flow override, and no direct call targets. The
following instruction was not queried. These flow observations apply only at
`00CD82C0`; they do not exclude a transfer elsewhere in the function.

Both functions are recorded as non-thunks with `no_return=false`. Their names and
extents are Ghidra metadata. This packet establishes no opcode, operand, register
input, store, delegation target or true writer. The candidate is selected by its
exact xref source and exact function ownership, never by nearest-symbol matching.

## Current Source context

The existing `NativeGameResourceClassificationContext` borrows exactly three
current fallback cells. Its `00B86950` Source predicate compares them in order;
the classifier's captured target dispatches that special case locally. Those
facts are current Source evidence, not a fresh Native byte comparison.

A bounded case-insensitive search for the three exact cell strings under `include`
and `src` finds only the classifier context pointer and its use. A separate search
for `00CD82C0` and its recorded owner name returns no matches. These explicit
identifier searches do not rule out unnamed, indirect or external implementations.
The existing fallback item constructor stamps its profile and reference count;
that Source object is not a proved owner or initializer for the type cells.

The accepted consumer review remains the attachment contract. No retained fallback
backing, descriptor name, descriptor extent, guard, allocation counter, initializer
target, CRT position or publication lifetime is supplied by this discovery.
Fallback identity `0109021C/01090220/01090224` remains separate from SceneResource
`01090210` and the selected mesh identities. No copied IDs or substitute storage
were introduced.

## Next bounded evidence gate

Request a separate Root/Astra instruction-evidence packet for exactly
`00CD82C0`, length **10** (`00CD82C0..00CD82C9`), the complete extent reported by
metadata. Its purpose is to decode the data-referencing instruction and subsequent
transfer, if any, so that an actual writer or delegated initializer can be selected
from evidence. The first instruction alone is length 5; a stricter staged variant
can inspect those 5 bytes and query typed-flow at `00CD82C5` before further reads.
Neither variant is executed or authorized by this packet.

That future packet needs explicit address/file ownership and comparison against
the original executable where available. Any newly discovered target, data or
handler needs its own reviewed scope. Do not infer a writer from the provisional
startup name, copy a nearby initializer, or assign a CRT slot. Source implementation
and consumer attachment remain dependent on the true writer plus actual retained
owner/publication and other outstanding loading/forwarding/deletion contracts.

## Verification limits

The report pins ten Source/config/tool/context files against baseline Git blobs,
normalizing CRLF only for the equality check. Exact working-file hashes and copies
of baseline blobs are retained separately. Five complete CLI/search receipts
include the two successful typed-flow batches and three Source searches; the
candidate search's exit status 1 is a legitimate no-match result. All stderr files
are empty. Raw metadata response hashes and all evidence pins are checked offline.

There are zero new Native windows, byte comparisons, C++ changes, Ghidra mutations,
analysis/repair operations, ledger changes, builds, tests or runtime actions.
Metadata acceptance does not establish original ABI compatibility, current game
behavior, startup execution, CRT completeness, gameplay or rendering parity.
