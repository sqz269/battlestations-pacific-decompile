# SkinModel writer ownership: metadata audit

`00CD8524` is still an exact six-byte instruction with **no exact or containing
Ghidra function**. The accepted earlier packet established only its store to
`01090344`; this packet adds current typed ownership/flow metadata without
opening another Native byte. No initializer, incoming ECX value, entrypoint,
ABI, parent/counter chain or production backing is admitted.

Baseline main is `8ab093935e70c6da1918861eebe0d03228cd9a2f`; the worker
inspection revision is `5d6b7cbc3ec508da92bfaa8e53b6a53d7239c068`. The completed
camera Source packet and its frozen receipts remain unchanged. This packet
owns only this document, its report and ignored evidence.

## Complete typed responses

The initial lease covered `01090344` and the already observed store `CD8524`.
After the named xref query returned four other exact source addresses, the
lease was refreshed before querying those addresses. Two typed-flow batches
retain all eight full raw responses. Every response passed the strict validator
with complete ranges, explicit nulls, exact query identity and modification
number `5` before/after, also unchanged across the two batches.

Both batches verify project `bsp`, actual marker
`C:\Users\sqz269\bsp.gpr`, program `/battlestationspacific.exe`,
language `x86:LE:32:default`, image base `00400000` and address space `ram`.
They also retain before/after program verification. This improves the older
discovery packet's project-directory qualification; it does not attest the
loaded Java CodeSource or refresh any original bytes.

| Address | Current code unit | Exact / containing function | Complete body |
| --- | --- | --- | --- |
| `00CD8524` | Exact instruction, length 6 | Both null | No function records |
| `01090344` | Defined data, length 4; both instruction fields null | Both null | No function records |
| `00B8F920` | Exact instruction, length 5 | Both `00B8F920` | One complete range `00B8F920..00B8F925`, 6 addresses |
| `00B8F940` | Exact instruction, length 5 | Both null | No function records |
| `00B91630` | Exact instruction, length 2 | Both null | No function records |
| `00B91624` | Exact instruction, length 5 | Both null | No function records |

The one function record is currently named `BSP_SkinModel_GetCurrentType`.
Its independent flags are NoReturn false, nonthunk, direct thunk target null.
Unavailable flags at the other addresses are not treated as false.

Every queried instruction has flow override `NONE`, default/effective
`FALL_THROUGH`, no overridden fallthrough and empty default/effective target
arrays. Their exact fallthrough addresses are respectively `CD852A`, `B8F925`,
`B8F945`, `B91632` and `B91629`. The data cell has explicit null instruction
flow. Empty direct-call arrays describe only these selected instructions;
they do not characterize any uninspected body.

`lookup CD8524` still offers nearest indexed candidate
`CD8460 BSP_GroupPool_InitializeStatic`. This is not function membership.
The actual typed containing-function result at the store is null; the nearest
candidate supplies no owner, full-body extent or usable calling contract.

## Exact incoming references

The existing BSP `ghidra xrefs 01090344 --limit 100` command returned all five
records below. An ignored transport recorder preserved response bytes before
the existing client parsed them or the CLI annotated them. It permits only
GETs for that named xref and the two normal identity endpoints; autostart was
disabled. The complete xref response and four identity HTTP responses are
retained as text/base64 with SHA-256 and HTTP status, with no tool/source edit.

| Reference source | Returned reference category |
| --- | --- |
| `00B8F920` | READ |
| `00B8F940` | READ |
| `00B91630` | READ |
| `00B91624` | DATA |
| `00CD8524` | WRITE |

This agrees with the accepted discovery packet and identifies one recorded
direct write. It does not exclude indirect or unresolved writers. The DATA
reference from `B91624` is a reference category: current typed listing metadata
at that address is an instruction, not a data code unit. No opcode, table,
pointer role or initializer semantics is inferred from the category.

## Current Source context

The complete current Source files are pinned against baseline Git and copied
into the evidence bundle. `native_skin_model_type_00b8f920` is an existing
one-cell read through a borrowed `const volatile uint32_t&`.
`NativeResourcePostprocessContext` borrows `skin_model_type_01090344`; the
point-light population context separately borrows `token_01090344` for type
dispatch. These are current consumers, not publication or backing owners.

Exact identifier searches found no `01090344` backing, guard or `CD8524`
initializer in `GameNativeTypeStorage`, its implementation, or the retained
VFS/resource application composition. No value, substitute guard or descriptor
was introduced. SkinModel `01090344`, skined-mesh resource `01090454`, the
excluded `01090370` family, camera resource `01090288` and compact `0109042C`
remain separate.

## Precise next scope, not opened

The store's incoming ECX definition and preceding control flow remain unknown.
Before any byte extension, a separately approved typed query at **`CD8523`**
(the byte immediately preceding the known store) can establish whether that
byte belongs to an instruction and supply its actual start/length. It must
not be promoted to an entrypoint by alignment or proximity.

The proposed physical lookbehind is exactly **`[00CD8515,00CD8524)`, 15 bytes**,
solely to inspect the immediate predecessor using verified instruction starts
and seek the current ECX definition. It is a bounded inspection envelope, not
a function extent or an assertion that its lower edge is an instruction.
Do not decode from `CD8515` merely because it is the envelope boundary. If the
verified predecessor or necessary value origin needs different scope, stop
and request a separate exact scope; no automatic expansion is authorized.
The already accepted six-byte store need not be reopened for this step.

This proposed metadata/byte work was not performed or leased here. Even a
resolved immediate predecessor would not by itself establish a complete
initializer, guard/name/layout, counter owner, ABI, EH, caller or CRT placement.
No Source initializer packet is ready from the current evidence.

There were no new Native byte/disassembly/decompile/callee/data/handler/table
openings, physical scope extensions, C++/CMake/ledger/GPR changes, POSTs,
Ghidra scripts, restarts, builds, tests or runtime execution. The only live
analysis was the explicitly leased metadata GETs and normal identity checks.

Report: [full raw metadata, Source pins and held gates](../reports/cc12_skin_model_writer_owner_metadata.json).

## Primary review

Root independently retained 52 pin occurrences and verified 8 baseline/current tracked files. 8 complete raw typed responses passed the current strict validator at modification 5 with actual GPR/program identity. All five raw xref/identity HTTP hashes replayed. This review adds no C++, live Native inspection, Ghidra mutation, build, test or runtime credit. The stated ownership/application/ABI gates remain held.
