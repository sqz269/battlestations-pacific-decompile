# Type8 local library-edge audit

The sealed Root family `local/t8p2` covers all 29 distinct retained TU bodies
and its four declared additional helpers. Its 33 packed spans match the linked
PE exactly. Raw relocation operands nevertheless reach **14 local library
helper spans outside that gate**, including nine normal verification and
initialization support thunks. This is a byte-coverage finding, not evidence
that the recorded fixture failed.

This review contributes **Source0 and two metadata files**. Root decides any
hold, correction, admission or integration. The published Source1 report,
earlier review, constructor, capture ABI and accepted domain are unchanged.
No compiler, fixture, Native executable, provider check or Ghidra operation ran.

## Independent raw proof and counting

This audit parses actual `source.obj`, `canonical.obj`, `probe.obj`,
`t8_probe.map`, `t8_probe.exe` and `gate.bin`. Its self-contained raw PE/COFF/map
decoder reuses source text from this reviewer's earlier independent callback
audit; no old module or top-level checker executes. Accepted recipes, saved
COFF/static-gate results and prior review results are not proof oracles.
Capstone only decodes the linked instructions.

There are **31 retained symbol proofs, 29 distinct physical TU bodies,
286 symbol relocation checks and 280 unique physical operands**. The earlier
review's 280 count correctly describes unique operands. Two actual weak
aliases duplicate six relocation checks; they do not create extra code.
All DIR32/REL32 operands resolve against the actual map. Every other byte of
each entire COFF body equals the linked PE, and every body is wholly gated.
All executable public/static map entries owned by the three TUs are covered.

Both canonical weak externals use auxiliary mode **1,
IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY**, not mode 3 SEARCH_ALIAS:

| Weak symbol / raw index | Raw 18-byte auxiliary record | Selected fallback / index | Actual body |
| --- | --- | --- | --- |
| `??_Eexception@std@@UAEPAXI@Z` / 293 | `240100000100000000000000000000000000` | `??_Gexception@std@@UAEPAXI@Z` / 292 | `37001180`, 45 bytes |
| `??_Ebad_alloc@std@@UAEPAXI@Z` / 299 | `2a0100000100000000000000000000000000` | `??_Gbad_alloc@std@@UAEPAXI@Z` / 298 | `37001150`, 45 bytes |

Selection is proved by the actual fallback index, defined COFF code symbol,
same map address, full extent and every linked byte; search mode alone does
not prove which fallback was selected.

The 40-byte gate header, 33 code records and four 104-byte CRT specifications
consume all 11,031 bytes, with no overlap or trailing bytes. Additional spans
are cookie `370038F0` (14 code bytes), memcpy `3700462C` (6), cold throw
`3700463E` (6), and chkstk `37003940` (48). Chkstk has **43 reachable code
bytes plus five alignment INT3 bytes**, with its backward branch included.
The cookie's failure tail points to separate `___report_gsfailure` at
`37003C50`; that external failure body remains outside admission.

## Actual omitted library edges

All following direct edges are actual TU **REL32 CALL operands**, not guessed
dependencies. The operand VA names the four-byte displacement after CALL's
opcode. Each 6-byte import thunk is decoded as `FF 25 <actual IAT VA>` and its
slot is matched to the PE import table. Imported DLL implementations are not
part of this code-gate claim.

| Missing destination | TU operand VA(s) | Normal support owner |
| --- | --- | --- |
| `370038BF` BCryptOpenAlgorithmProvider | `3700173B` | hash_bytes |
| `370038C5` BCryptGetProperty | `37001775` | hash_bytes |
| `370038CB` BCryptCloseAlgorithmProvider | `37001803` | hash_bytes |
| `370038D1` BCryptCreateHash | `370017A5` | hash_bytes |
| `370038D7` BCryptHashData | `370017C4` | hash_bytes |
| `370038DD` BCryptFinishHash | `370017DE` | hash_bytes |
| `370038E3` BCryptDestroyHash | `370017F8` | hash_bytes |
| `370038E9` K32GetMappedFileNameW | `37001C80` | verify_module |
| `37004644` memset | `37001715`, `37001FF5`, `3700205B`, `370020DE`, `3700239F`, `37002605`, `37002621`, `37002645`, `3700311F` | hash_bytes / main |

These nine distinct normal destinations account for 17 physical operands and
17 symbol checks. “Normal” identifies verifier/initialization support; it
does not assert that every call site ran in every captured case.

Cold support adds three direct destinations, seven physical operands and
11 symbol checks:

| Missing destination | TU operand VA(s) | Qualification |
| --- | --- | --- |
| `37004632` __std_exception_copy | `370010CD`, `3700111D` | bad_alloc / exception copy constructors |
| `37004638` __std_exception_destroy | `3700113B`, `3700115E`, `3700118E` | destructor / deleting-destructor bodies; two weak aliases duplicate checks |
| `37003900` sized scalar delete | `37001170`, `370011A0` | deleting destructors; both weak aliases duplicate checks |

The sized-delete body at `37003900` is 16 bytes. Its CALL at `37003906`
(operand `37003907`) reaches unsized delete at `37003C60`. That five-byte
body is `E9F7090000`, targeting the six-byte `_free` import thunk at
`3700465C`, which is `FF25A4500037` and resolves to actual IAT `370050A4`.
All three spans lie outside the 33 packed ranges. Eleven INT3 bytes after
unsized delete are alignment, not its code body.

In total, 24 unique direct TU operands / 28 symbol checks reach 12 missing
direct destinations. Following the actual delete chain adds two more local
helpers, for **14 omitted spans**. There are no unresolved indirect edges in
those decoded helper bodies. This exact image could gate those 14 spans;
future link layouts must be derived afresh rather than inheriting counts or
addresses. The cold EH/OOM/deleting-destructor paths remain unadmitted.

## Preserved scope and evidence

The constructor remains Native `008EF2F0`, 112 bytes, SHA256
`4c3786af3a642703dc38315ae63d9dee0468552f47a03bc1405fc0e90ab5c928`.
Actual Source CALL operands `[47,51)` and `[62,66)` bind the adapter at
`37001000` and memcpy at `3700462C`; all 104 other bytes agree with Original
and bound Original. The first instruction tests the low flag byte at entry
ESP+C and both exits retain RET0C. This audit preserves the Type8 physical
order **data / byte count / full flag DWORD**, all full captures/storage/input
files and the prior interpretation of retained versus copied ownership.

Recorded stdout still has Source2/Original2, two branches each, six allocations
and six frees. This audit checks those recorded counts and literal constructor
binding only; it does not repeat the earlier full capture/runtime/provider
review. No float experiment, exception/private-CRT ownership, class behavior,
startup or gameplay claim is added.

Before/after inventories verify **20,386 unique pins** unchanged, including
Root's exact 404-file family and 18,537 prior pins, the 42-file callback review,
the prior 40-file Type8 peer family and its six-file publication family. Earlier
failed artifacts and draft families retained by those pins remain unchanged.

The first new parser stopped after a valid TU proof because an internal chkstk
map label (`cs10` at `37003954`) was mistaken for its next function boundary.
That parser, log and partial proof remain intact. The separate `a02` parser
uses the next actual function row and passes the complete artifact audit.
This parser correction does not modify Root's immutable artifacts.

The new family is
`local/cc12_type8_complete_helper_peer_review20261008a`, with complete raw
proofs, classification, preservation inventories, stopped attempt, handoff and
an exact recursive `artifact_manifest.json`. No file is added after sealing.
