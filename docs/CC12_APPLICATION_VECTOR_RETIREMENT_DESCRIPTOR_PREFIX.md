# Application retirement descriptor candidate: twelve-byte prefix

Two exact reads of `00DB57B8..00DB57C3` returned
`22 05 93 19 01 00 00 00 B0 57 DB 00`. Their SHA-256 is
`f739968ba1ecaa1310f01fdbc561487a62ea03c389bc22582ca82714940100ec`.
Independent little-endian decoders agree on these raw DWORDs:

| Offset | Numeric hexadecimal value | Unsigned value |
| --- | --- | --- |
| +0 | `19930522` | 429065506 |
| +4 | `00000001` | 1 |
| +8 | `00DB57B0` | 14374832 |

No field meaning is accepted here. In particular, the second word is not
classified as maxstate or a state count, and the third word is not established
as a map pointer or dereferenced. The requested classic absolute FH3 prefix
remains a conditional candidate; neither these numbers nor saved data-unit
metadata authenticates its layout or Original FH3 semantics.

## Positive carrier and metadata gate

The accepted `00C86208` instruction loads `EAX=00DB57B8` and falls through to
the accepted physical `E9 JMP` at `00C8620D`, reaching numeric `00BF6B43`
without pushing a return address. Saved `CALL_RETURN` remains a separate
database override. The accepted shared54B wrapper preserves incoming `EAX=D`,
stores it at `S-8` and carries it at target stack `T+14h` through its physical
`CALL 00C07991`. That wrapper does not dereference D. Its external-return ESP,
EBP, EAX and memory remain unknown; normal restoration is conditional, and
the handler's own EBP does not bind an Original action frame.

Before any prefix bytes were read, paired typed queries covered exactly twelve
address units `00DB57B8..00DB57C3`. Every address is contained by one defined
`DataDB` beginning at `00DB57B8`, saved length36. Exact `code_unit_at` exists
only at the first address; the other eleven exact-at values are null.
Both instruction aliases and both function references are null throughout,
all function lists are empty and instruction flow is null. The defined data
unit was not replaced with an undefined or instruction interpretation.

Root independently accepted this metadata gate and admitted only two12B reads.
The retained gate SHA-256 is
`e6114bc914dc88929c6d39703d751c802471088103c74c033a8dddc9335c9c8d`;
its producer is retained opaque and pinned. The saved36B length is metadata
only: the remaining24B at `00DB57C4..00DB57DB` were not read. All three typed
batches agree at epoch98, and four count bookends remain64730 in
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`.

## Proof and limits

The [report](../reports/cc12_application_vector_retirement_descriptor_prefix.json)
pins ten finite immediate Git inputs: three accepted Root doc/report pairs,
target configuration and three CLI producer files. The local proof retains
every Native response and CLI stream, actual frozen/staged/final Git objects,
exactly two output blobs, full physical replay/producer/helper bodies and
preimages, and the Root gate. Original and distinct-copy strict ZIP replays
allow one supplied archive read and one declared receipt write. Observed final
pins and receipts are delivered separately, avoiding self-reference.

An offline formatting guard initially rejected the earlier derived decoder
JSON's CRLF bytes against canonical LF. Numeric values were identical. The
original JSON, helper and complete failure record are retained; the correction
compares parsed values and canonicalizes only the derived result. No Native
evidence was lost, overwritten or reacquired for that correction.

Earlier proofs remain immutable. Source subtree IDs are identity only: no
Source census or future Root equality condition is introduced. No tail, map,
action, pointer dereference, target body, stack expansion, Source/Ghidra/ledger
mutation, build, test, probe, game or OS/audio execution occurred. No Original,
FH3, ABI, state-0 action, ownership, activation or runtime credit is earned.
Parent Source remains blocked on positive actual state-0 action evidence and
its other outstanding contracts; any map/action gate needs separate scope.
