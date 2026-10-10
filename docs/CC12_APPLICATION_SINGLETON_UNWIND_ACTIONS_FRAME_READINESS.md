# Application singleton unwind actions and frame requirements

**NO_CPP_GO; NO_FRAME_ACTIVATION_GO.** Both encoded actions now have exact
eight-byte bodies. One loads a pointer from `[EBP-18h]`; the other forms the
address `EBP-14h`. Each ends in a machine JMP with a saved CALL_RETURN flow
override. These observations constrain the required incoming frame but do not
establish FH3 invocation, cleanup semantics or return behavior.

Source is frozen at `91e874914248d3ec5d895d42f76a7fe48a05cb1f`: all 4,210 tracked
src/include files, Git blobs and working bytes, plus 30 supporting files.
Relative to the preceding map snapshot, only the accepted
`include/bsp/native_particle_model_lifetime.hpp` and corresponding `.cpp` differ.
All 433 prior pins are physically copied unchanged, including nested
355/287/230/184/124 proofs and qualified compiler-definition artifacts.

## Admission and exact instructions

The independently leased ranges are **`CC7230..CC7237`** and
**`CC7238..CC723F`**. Refreshed ownership and prototype metadata confirmed two
separate complete contiguous eight-byte functions before any code-byte read.
Before/after typed captures also cover all four instruction starts. Every raw
typed response and its base64/SHA-256 representation is retained and replayed.

The verified project is `bsp`, `C:\Users\sqz269\bsp.gpr`, program
`/battlestationspacific.exe`, x86/32-bit little-endian, 64,730 functions. All
fresh typed observations agree at modification **92**. Each CLI Native batch
verifies project/program; no mutation or repair occurred. Fresh code reads
total exactly 16 bytes and no fresh data reads were made.

| Address | Exact bytes | Machine instruction |
| --- | --- | --- |
| `CC7230` | `8B 4D E8` | `MOV ECX,DWORD PTR [EBP-18h]` |
| `CC7233` | `E9 F8 B1 74 FF` | `JMP 00412430` |
| `CC7238` | `8D 4D EC` | `LEA ECX,[EBP-14h]` |
| `CC723B` | `E9 A0 AC 74 FF` | `JMP 00411EE0` |

Independent decoding checks both complete hulls and both signed relative jump
displacements against the saved disassembly. No callee instruction text, bytes
or decompilation was fetched. No descriptor suffix, `BF6B43`, other funclet,
installed CRT, game or OS-provider inspection was performed.

## Machine effects and saved flow metadata

`MOV` reads four bytes at `EBP-18h` and replaces ECX with their value. An invalid
address could fault before the jump. `LEA` only computes `EBP-14h`, without
reading memory there. It passes a record address, conditionally on the frame
binding below, rather than loading the record's first DWORD.

Neither sequence modifies EBP, ESP or arithmetic flags before its target, and
neither contains a CALL or RET. The JMP pushes no return address and provides
no separate local return continuation. Target execution receives the existing
stack. A target might return using that stack if its contract permits, but this
packet establishes neither a return nor any target cleanup effect.

At both final instructions the typed **default** flow is
`UNCONDITIONAL_JUMP`; the saved override is **`CALL_RETURN`**, producing
**effective** `CALL_TERMINATOR`. Both have no fallthrough. The override remains
unchanged and does not convert either E9 opcode into a machine CALL. The
function flags remain `NoReturn=false`, and prototypes remain
`undefined Unwind@00cc7230(void)` / `undefined Unwind@00cc7238(void)`; these
stored signatures do not describe the implicit EBP/register contract.

The saved target names, `BSP_SingletonBase_ResetProfile` and
`BSP_SystemSingletonGuard_Destroy`, remain descriptive evidence only. Their
names and the prototype summary's call counts do not prove cleanup semantics.

## Conditional comparison with the retained constructor

This comparison uses only the already retained 145-byte `BEA810` constructor
capture at **epoch 79**, not a new constructor read. Its prologue saves three
DWORDs, reserves 12 bytes, and saves ESI/EDI, placing ESP 32 bytes below its
entry value at the first receiver store. The complete captured body has no
EBP operand and does not establish the incoming EBP required by either action.

| Retained constructor operation | Required action-frame relation |
| --- | --- |
| `BEA82C`: original receiver EDI stored at `[ESP_R+8]` | `EBP-18h = ESP_R+8`, hence `EBP = ESP_R+20h` |
| `BEA848`: guard profile `CE37FC` stored at `[ESP_G+0Ch]` | `EBP-14h = ESP_G+0Ch`, hence `EBP = ESP_G+20h` |
| `BEA850`: captured section stored at `[ESP+10h]` | Conditional guard's second DWORD, not the LEA result itself |

`ESP_R` and `ESP_G` denote the stack pointers at those respective stores.
If they are equal to a common `S`, a single `EBP=S+20h` satisfies both address
equalities. At the first receiver store that equals the constructor's entry
ESP. This is a necessary conditional match, not evidence that FH3 supplies
that EBP, that the two store-time ESP values coincide during actual execution,
or that unexamined callees obey the needed stack discipline.

Under that binding only, action 0 loads the **saved original receiver** while
action 1 passes the **guard record address**. The latter is neither the
profile DWORD nor the captured-section pointer. Current publication and the
selected registration argument remain separate identities. The retained
state stores at `BEA830` / `BEA861` still match the map's declared indices
0 / 1; the declared chain `1 -> 0 -> -1` is not a runtime unwind trace.

Epochs remain distinct: constructor 79, handler 85, descriptor 88, map 90,
actions 92. No newly atomic snapshot of their combined runtime contract is
claimed. Existing manager, guard and registration Source providers remain
insufficient to activate a callable Application owner or deletion profile.

## Custody, proof and next proposal

Keep raw storage, its actual +14 member reference, authority cells and borrowed
services alive while references and startup/shutdown/failure records escape.
No reset, unregister, free, retry or destructor policy follows from these
funclets. `E1AE78` remains a separate ownership gate, and normal callable
retirement remains unresolved.

Finite replay checks Source/Git closure, nine whole-Source lexical queries,
14 excerpts, all inherited proofs, compiler artifact identity, capture order,
typed metadata, exact bytes, independent decoding and the conditional frame
equalities. Physical relocation copies only declared files plus the report;
its guard denies undeclared reads, writes, subprocesses and network. Actual
Git replay additionally checks current/prior closed Source blobs and both
committed output files. No C++/CMake/ledger change, build, test, probe, Native
execution, ABI equivalence, frame activation or game validation is added.

Next single bounded proposal:
`cc12_application_singleton_unwind_targets_metadata_readiness`.
Independently claim exact addresses **`00412430`** and **`00411EE0`** and refresh
prototype plus complete typed ownership/listing metadata. The current packet
provides exact branch destinations and saved names, not fresh callee bounds.
The proposed gate admits **zero code or data bytes** and no body instruction
text or decompilation. Missing, shared, discontinuous, changed or larger
ownership must remain explicit; propose any subsequent body scope only from
those separate verified bounds. No descriptor suffix, other handler/funclet,
`BF6B43`, runtime read or listing/flow/prototype/no-return repair is included.

Evidence: `reports/cc12_application_singleton_unwind_actions_frame_readiness.json`
and its declared local finite bundle.
