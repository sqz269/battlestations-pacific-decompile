# CC12 allocation statistics: actual SEH prologue coordinates

The actual `__SEH_prolog4` body establishes EBP at the unwind routine's entry
ESP minus four. The previously observed `[EBP+8]`, `[EBP+10h]` and `[EBP+14h]`
operands therefore select that routine's first, third and fourth arguments.
This closes the internal prologue-coordinate condition: the action receives
EBP equal to the raw second argument forwarded from `BF6B43`, plus `0Ch`.
The OS association of that raw argument with the parent's registration node,
fault behavior and the action's physical epilogue remain separate holds.

The worker fast-forwarded to accepted main
`ea02ab2c493ce26f7fcc8892e75c70ddedf340f4`. Root first authorized metadata only
for `C07C00`, then admitted its exact complete saved 69-byte body after the
shape review. Only this document and
`reports/cc12_allocation_stats_seh_prolog_metadata_gate.json` are changed.
Correct library names and all saved listing, flow, AddressSet, prototype and
ownership metadata are preserved.

## Metadata gate and admitted bytes

Initial typed metadata identifies exact entry/owner `C07C00` as
`__SEH_prolog4`, non-thunk and returning, with one complete saved range
`[C07C00,C07C45)`, 69 bytes. The first instruction is five bytes with ordinary
fallthrough to `C07C05`, no override and no direct target at that start. The
brief prototype is `undefined __SEH_prolog4(void)`. It is not a recovered ABI:
the body explicitly rearranges its caller's stack and return word.

After separate authorization and a range lease, exactly 69 live bytes and the
complete saved listing were retained. The original PE window at file offset
`00807C00` matches every byte. Independent decoding covers all 69 bytes in 21
instructions; every start and length matches fresh typed metadata. There is
no new data, neighboring instruction, handler target, caller or callee body
read. In particular, neither `C07C90`, the encoded data operand `E15590`, nor
the pointed-to `E03398` table was queried. The prior unwind gap and action
epilogue remain outside the fresh scope.

## Caller-relative prologue derivation

Let `U` be ESP on entry to the already retained `C069A2` unwind routine,
pointing at its caller's return address. Its retained prefix is:

```text
C069A2  PUSH 10h
C069A4  PUSH E03398
C069A9  CALL C07C00       ; copied return address is C069AE
```

Thus this helper enters with ESP=`U-0Ch`. Its first two instructions push
handler constant `C07C90` and the previous `FS:[0]` word, leaving ESP=`U-14h`.
At `C07C0C`, `[ESP+10h]` is `[U-4]`, the caller's pushed `10h` local-size
word. The next instruction replaces that slot with incoming EBP, and
`C07C14` executes `LEA EBP,[ESP+10h]`. Therefore the new EBP is exactly
**`U-4`**, without invoking another helper or assuming a standard prologue.

The subsequent `SUB ESP,EAX` reserves the actual pushed `10h` bytes. The code
saves EBX, ESI and EDI, then reads the DWORD at encoded address `E15590` into
EAX. Call that unread value `K`. It XORs the scope word at `[EBP-4]` with `K`,
XORs EAX with the new EBP, pushes that result, and records current ESP at
`[EBP-18h]`. Those are physical arithmetic and access operands; this packet
does not read or validate the actual value of K or the data owner's state.

The helper pushes the return word from `[EBP-8]`, which is still `C069AE` at
that point. It loads the XORed scope word, writes `-2` to `[EBP-4]`, then stores
the saved XORed word at `[EBP-8]`. It computes `EBP-10h` and publishes that
address to `FS:[0]`. Its plain `RET` consumes the copied `C069AE` word. The result on
ordinary return to `C069AE` is:

| Quantity | Actual coordinate/value |
| --- | --- |
| EBP | `U-4` |
| ESP | `U-34h` |
| EAX and newly published `FS:[0]` | `U-14h` |
| `[U-4]` | Saved incoming EBP |
| `[U-8]` | State `-2` |
| `[U-0Ch]` | `E03398 XOR K`; formerly the helper return word |
| `[U-10h]` | Handler constant `C07C90` |
| `[U-14h]` | Previous `FS:[0]` word |
| `[U-1Ch]` | Saved ESP value `U-34h` |
| `[U-28h]`, `[U-2Ch]`, `[U-30h]` | Saved EBX, ESI and EDI |
| `[U-34h]` | `K XOR (U-4)` |
| `[U-38h]` | Copied `C069AE`, consumed by the final RET |

The original unwind-routine return word at `[U]` and its incoming argument
words are not stack destinations of these observed prologue stores. EBX,
ESI and EDI are saved without being redefined in this helper; ECX and EDX are
untouched. EBP, ESP and EAX change as above. Arithmetic flags are affected by
SUB and XOR; no flags-preservation claim is made. Normal execution and valid
required memory/segment accesses are conditions, not runtime observations.

## Argument coordinates now follow from actual instructions

The retained dispatcher pushes `-1`, its descriptor F, its fourth raw
argument and its second raw argument before calling `C069A2`. Consequently:

| Observed post-prologue slot | Same address at C069A2 entry | Incoming value |
| --- | --- | --- |
| `[EBP+8]`, loaded into EBX | `[U+4]` | First argument: forwarded raw second argument |
| `[EBP+0Ch]` | `[U+8]` | Second argument: forwarded raw fourth argument |
| `[EBP+10h]`, loaded into EDI | `[U+0Ch]` | Third argument: descriptor F |
| `[EBP+14h]`, compared with ESI | `[U+10h]` | Fourth argument: target state `-1` |

This removes the previous assumption that a generic or installed prologue
model supplies those coordinates. The saved wrapper at `BF6B43`, internal
dispatcher at `C07991`, current prologue and the retained post-prologue loads
now form a concrete argument chain. The first two bodies are reused immutable
evidence; no wrapper/dispatcher window is reread.

The loaded EBX is passed unchanged as the second argument to the accepted
`C07B10` frame-setting helper. Its actual notification path preserves the
computed frame, so action-entry EBP is **that original raw second argument
plus `0Ch`** on the ordinary observed unwind path.

The new `FS:[0] = U-14h` node is the unwind routine's **own** registration.
It is distinct from the forwarded argument loaded into EBX. Substituting the
new current FS node for that argument would lose the actual parent frame;
the code explicitly loads `[U+4]` instead.

## Remaining parent, return and fault boundaries

The retained constructor/startup prologues place their registration nodes at
parent entry ESP minus `0Ch`. If the OS-supplied raw second argument is that
parent node, the newly established arithmetic gives action EBP equal to
parent entry ESP. The constructor's current saved receiver/actual guard and
startup's current allocation-spill operands then align with their observed
parent locals. This packet does not prove that OS entry association, replace
the actual saved-frame operands with publication cells, or infer a generic
Native FH3 interpreter from library names.

The stored handler constant `C07C90`, prior `C069A2` gap
`[C06A24,C06A3E)`, unwind cleanup descendants, epilogue and nonlocal/fault
paths remain unopened here. The prologue's ordered writes and FS publication
do not establish rollback, exception filtering, cookie validation or successful
cleanup after a fault.

The startup allocation action still ends its saved body at the returning-free
call under a preserved `CALL_RETURN` override. Its undefined `C86A3C`
continuation, physical return and stack cleanup are not reread or repaired.
The established action-entry coordinate therefore does not imply a balanced
action return or complete failure ownership. Source startup composition,
Native exception execution, ABI execution and gameplay receive no new credit.

## Current context and evidence verification

The packet freezes 92 current baseline inputs, including 40 Source files,
with full working/Git snapshots; 91 match exactly and one previously qualified
older report differs only by CRLF/LF. Ten bounded excerpts, a complete sorted
Source query for the exact prologue/frame addresses and names, twelve retained
predecessor receipts, and the immutable previous ZIP accompany the captures.
The current Source query has no matches. That bounded result is not a
whole-program absence proof, and no Source prologue/dispatcher is added.

An offline algebra receipt derives all ESP offsets and argument coordinates
from the retained three-instruction caller prefix and admitted prologue. It
executes no Native code and is not a runtime or exception test. Whole-file
snapshots retain provenance without whole-file semantic credit. Existing
notification Source, canonical-data preconditions and prior ownership limits
remain context; their older artifacts and runtime checks are not rerun.

All 22 accepted typed responses independently validate epoch 32. Thirteen
other raw GET replies are retained. Loaded Java CodeSource remains unattested.
The exact original window uses retained PE mapping and stable size/mtime;
no fresh PE header or whole-image hash is read. Offline verification checks
raw hashes, identity/schema, exact bytes/listing/boundaries, current baseline,
excerpts/query, coordinate receipt, report pins and every ZIP member/CRC.
There is no GPR mutation, save, annotation, repair, C++/CMake/ledger change,
build, test, probe, OS exception dispatch or game execution.
