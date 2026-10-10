# CC12 allocation statistics: bounded Native frame gate

The actual saved wrapper and internal dispatcher now connect the previously
observed handler thunks to a concrete unwind loop and its call to
`__CallSettingFrame@12`. The admitted instructions do **not** establish the
funclet EBP coordinate. The frame-setting callee, the unwind routine's SEH
prologue and cleanup descendants, its excluded gap, and the allocation action's
physical epilogue remain unopened. This is evidence/readiness work only.

The worker stayed on `8139229e47aec77243caf9a092a7c616834e77f1`. The preceding
constructor and startup packets were awaiting primary review at packet start;
Root subsequently confirmed their integration. This packet changes only this
document and `reports/cc12_allocation_stats_native_frame_gate.json`. Library
names, signatures, AddressSets, listing, flow and comments were preserved.

## Exact admitted scopes

| Saved owner | Original/live span | Bytes | Decoded instructions |
| --- | --- | ---: | ---: |
| `BF6B43`, `FID_conflict:___CxxFrameHandler3` | `[BF6B43,BF6B79)` | 54 | 27 |
| `C07991`, `___InternalCxxFrameHandler` | `[C07991,C07A75)` | 228 | 84 |
| `C069A2`, `___FrameUnwindToState` | `[C069A2,C06A24)` and `[C06A3E,C06A62)` | 130 + 36 | 49 |

All four reads match their exact original PE bytes; all 160 instruction starts
and lengths agree with current typed metadata. The total is **448 admitted
instruction bytes**, with no new data read. The 26-byte gap
`[C06A24,C06A3E)` was neither read nor queried. The complete saved AddressSet for
`C069A2` has 166 bytes in two ranges. This does not make its 192-byte enclosing
interval a decoded function or establish complete physical/EH behavior.

## Wrapper argument and stack facts

Let `W` be ESP at `BF6B43` entry, `F` its incoming EAX, and `A1` through `A4`
the four DWORDs at `W+4` through `W+10h`. The prior actual handler instructions
load their descriptor address into EAX before jumping here. This wrapper:

1. Pushes old EBP, sets its own EBP to `W-4`, reserves eight local bytes,
   saves EBX/ESI/EDI, executes `CLD`, and stores incoming EAX at `[EBP-4]`.
2. Pushes three zero DWORDs, saved `F`, then `A4`, `A3`, `A2`, `A1`, and calls
   `C07991` at `BF6B64`. Its callee sees `(A1,A2,A3,A4,F,0,0,0)`.
3. Adds `20h` to ESP, saves the result in `[EBP-8]`, restores EDI/ESI/EBX,
   reloads that result into EAX, restores ESP/EBP and executes plain `RET`.

Under ordinary balanced call/return execution, internal-dispatcher entry ESP
is `W-3Ch`, and that function's explicit `PUSH EBP; MOV EBP,ESP` establishes
`H=W-40h`. The wrapper itself never assigns a parent frame pointer from `A2`,
adds a registration-node size, or invokes a funclet. Its own `W-4` frame must
not be substituted for the parent constructor/startup entry ESP `S`.

The metadata prototype is an undefined no-argument form. It is not an ABI
specification. The register input and eight outgoing arguments above are
recovered from the actual instructions, without replacing the library name.

## Internal-dispatcher paths

At `C07991`, EBP is explicitly established as `H`; saved EBX/ESI/EDI are
restored on the observed returns. Name the eight incoming words `A1`, `A2`,
`A3`, `A4`, `F`, `A6`, `A7`, `A8`, in order. The wrapper supplies zero for the
last three. `C079A3` loads `F` from `[H+18h]` and `C079A6` loads `A1` from
`[H+8]`. Names such as exception record, context and dispatcher remain
interpretations of these operands; the instruction-level argument order is
the evidence.

The preliminary path calls `C0522E`, tests its returned object's DWORD at
`+20Ch`, compares the record-like first DWORD against `E06D7363` and
`80000026`, masks the descriptor's first DWORD with `1FFFFFFF`, and tests
descriptor byte `+20h` with mask `01h`. The early return-one condition is: returned
DWORD `+20Ch` is zero, neither record constant matches, masked descriptor word
is unsigned-at-least `19930522`, and that bit is set. No callee implementation,
OS exception policy or runtime meaning for the returned object's field is
proved by these operations.

At the subsequent common path, a nonzero `byte[A1+4] & 66h` enters the unwind
branch. With descriptor DWORD `+4` nonzero and `A6` zero, the physical sequence
at `C079F0` pushes `-1`, `F`, `A4`, `A2` and calls `C069A2`. The outgoing order
is therefore `(A2,A4,F,-1)`; sixteen bytes are caller-cleaned. The common
return path produces EAX one. These instructions forward the second argument
unchanged; they do not establish funclet EBP.

When the `66h` test is zero, the search path requires either descriptor DWORD
`+0Ch` nonzero, or masked descriptor word unsigned-at-least `19930521` with
descriptor DWORD `+1Ch` nonzero. Its further record comparisons select one of
two calls:

- The computed call at `C07A4A` uses the nonzero pointer loaded through
  `[[A1+1Ch]+8]`, after matching record constant `E06D7363`, unsigned count
  `DWORD[A1+10h] >= 3`, and unsigned `DWORD[A1+14h] > 19930522`. Its argument
  order is `(A1,A2,A3,A4,F,A6,A7,zero_extend(byte A8))`; its result is returned.
- Otherwise `C07A65` calls saved-name `FindHandler` at `C07623` with physical
  argument order `(A1,A2,A3,A4,F,A8,A6,A7)`. Its result is discarded in favor
  of the common EAX-one result. Both paths caller-clean `20h` bytes.

No computed target, `FindHandler`, or `getptd` body was opened. The three
zero wrapper arguments do not justify generalizing the two distinct outgoing
orders into a common recovered prototype.

## Saved unwind-loop observations and remaining frame dependency

`C069A2` first pushes `10h`, pushes `E03398`, and calls saved-name
`__SEH_prolog4` at `C07C00`. Its implementation is unopened. The subsequent
instructions load EDI from `[EBP+10h]` and EBX from `[EBP+8]`; their association
with the incoming descriptor and registration argument remains conditional on
that prologue's actual stack/frame convention. The following facts use the
actual loaded EDI/EBX values rather than assuming the prologue.

The code reads the current state at `[EBX+8]`. A signed comparison of
`DWORD[EDI+4]` with `80h` selects a sign-extended byte read for values at most
128, and a full DWORD read for greater values. The two retained allocation
descriptors have small positive maxima, but their interpretation here also
depends on the argument/frame mapping. `C0522E` is called and its returned
DWORD at `+90h` is incremented. The meanings and matching release of that
field remain outside the opened scope.

The loop compares ESI with `[EBP+14h]`. Before indexing, signed ESI must be
greater than `-1` and less than `DWORD[EDI+4]`, or `_inconsistency` is called.
The existing metadata describes that callee as returning, and exposes ordinary
call fallthrough; no termination behavior is assumed or changed. The physical
index is `ESI << 3`, added to the map pointer at `[EDI+8]`. The entry's first
DWORD becomes the predecessor state in ESI and a local slot; its second DWORD
is the action pointer.

For a nonzero action, `C06A09` writes predecessor ESI to `[EBX+8]` **before**
the call. It then pushes `103h`, EBX and the current action pointer from the
map, and calls `C07B10`. Thus the physical outgoing argument order is
`(action,EBX,103h)`. A zero action skips this store/call; predecessor ESI still
drives the next iteration. The local state is reset, and `C06A22` jumps over
the excluded gap to `C06A3E`, which updates a local and returns to the loop
comparison. Continued use of nonvolatile values after unopened calls retains
the ordinary calling-convention qualification.

At the observed terminal branch, the routine records local `-2`, calls
`C06A68`, checks the target state, conditionally calls `_inconsistency`, stores
ESI to `[EBX+8]`, calls `__SEH_epilog4` at `C07C45`, and executes `RET`.
Neither helper nor the gap has been decoded. This is not proof of nested
exception policy, cleanup-failure behavior, notification behavior, successful
remaining actions, or full physical function coverage.

Metadata alone identifies `C07B10` as non-thunk, returning
`__CallSettingFrame@12`, with one complete 76-byte saved range
`[C07B10,C07B5C)`. The capped caller list contains this unwind loop and
`_CallCatchBlock2`; its one listed callee is `__NLG_Notify1` at `C16870`.
Neither the decorated name nor those identities prove stack cleanup, frame
normalization, notification effects or action ABI. No byte of either helper
was read. That 76-byte range is a concrete next body gate for separate Root
authorization; the prologue and notification dependencies remain distinct.

## Parent/action mapping and physical epilogue stay qualified

The preceding packets establish parent registration `R=S-0Ch`. The constructor
actions encode current saved receiver `[EBP-18h]` and actual guard address
`EBP-14h`; the startup action encodes current allocation spill `[EBP-130h]`.
These operands match the observed parent locals **if funclet EBP equals S**.
The newly opened wrapper, internal dispatcher and two unwind spans do not
close that condition. Installed EH headers and retained source-model
conventions remain context, not original runtime proof.

The separately queried `C86A3C`, immediately after the startup action's saved
call, is an unowned one-byte undefined DataDB unit with no instruction or flow.
No neighboring bytes or metadata were fetched. The preceding action's saved
range still ends at the returning-free call with a preserved `CALL_RETURN`
override. Its argument cleanup, actual return, extra effects and physical
continuation are a separate listing/flow repair gate; no repair is authorized
or performed here. The saved twelve-byte action and any current publication
cell must not be substituted for a proven full cleanup function.

## Current Source and verification limits

Current Source still supplies the ordinary receiver-profile reset and actual
eight-byte guard cleanup. The latter reads the guard's current `+4` section
before stamping its profile, then decrements and leaves that section while
preserving `+4`. The existing resource-reader `__finally` model advances a
local state before calling its actions; its header explicitly withholds Native
CRT/FH3 identity. A complete sorted Source address/name query found those
reader comments, not an implementation of the opened dispatcher/frame helpers.
This observation does not broaden the excerpt audit into a whole-program
absence proof. Existing stats owner, publication, aliasing and startup holds
remain as recorded in the preceding complete current inputs.

The bundle retains 76 current baseline inputs, including 38 Source files,
their full working/Git snapshots, nine bounded excerpts, one complete Source
query, 13 retained predecessor receipts including the immutable startup ZIP,
all local capture helpers and all raw replies. Seventy-five inputs match Git
byte-for-byte; the previously qualified older report differs only by CRLF/LF.
Full snapshots preserve provenance without adding whole-file semantic credit.

There are 167 accepted typed responses and 60 other retained raw GET replies.
The initial three metadata responses have epoch 26; subsequent accepted
batches have epoch 30 after Root's separate annotations. Each batch validates
its own identity and stable metadata epoch; no single frozen-program interval
is claimed. One 84-address request was rejected locally before any request or
capture because the limit is 64; the same starts were captured in two accepted
42-address batches. Loaded Java CodeSource remains unattested.

The exact original reads reuse previously admitted PE mapping and check original
size/mtime. They perform no fresh PE-header read or whole-image hash. Offline
verification checks raw reply hashes, typed schema/identity, exact byte windows,
all instruction boundaries, baseline and excerpt/query consistency, report pins
and every ZIP member/CRC. No GPR mutation, annotation, save, repair, Source or
CMake/ledger change, build, test, probe, ABI execution or gameplay check occurs.
