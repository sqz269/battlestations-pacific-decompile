# Observer pair release and callback-owner destruction: ABI readiness

The complete Native bodies at `006952A0` and `00695870` now have fresh live/PE,
listing, raw control-flow, register, stack and field-access evidence. Existing
Source `NativeObserverLifetime` methods cover their principal normal operations,
but require a real retained lifetime service and actual owner storage. This audit
adds no binding, Source implementation or ABI credit. Native subordinate behavior
and exception cleanup remain separate obligations.

## Scope and whole-body gate

Baseline is published `main` / `origin/main`
`572b6612cf6c2c8092922f75a8b104d408a8e374`. Only `006952A0` and `00695870` were leased,
and only this document and its report are changed. Correct saved BSP/library names
remain unchanged; descriptive BSP names are existing hypotheses.

| Entry | Verified extent | Physical bytes / instructions | Calls / RET | Raw CFG nodes / edges |
| --- | --- | --- | --- | --- |
| `BSP_Observer_UnregisterPair` | `006952A0..006953BF` | 288 / 92 | 13 / 1 | 92 / 103 |
| `BSP_CallbackOwner_Destroy` | `00695870..00695930` | 193 / **61** | 8 / 1 | 61 / 66 |

The saved destroy metadata reports 60 instructions; fresh listing and independent
PE decode both contain all 61 starts. There is no missing listing instruction or
extra decoded padding. Unregister's saved call metric is 10 direct calls, whereas
its physical count includes two IAT calls and one virtual call. Destroy's metric
is four direct calls plus four physical IAT calls. Both raw CFGs cover every owned
instruction when conditional edges and compatible call returns are allowed.
Neither body has an external jump, tail transfer or unowned normal continuation.
Actual branch feasibility, provider returns and EH paths are not thereby proved.

The original PE SHA-256 remains
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Owned body hashes are `d59a5120603745ae7ec3337a2890a3ee67cb1a95bb9ed43fdab8e61c9671bb84`
and `ac2f2126dc573849014eec43e7f052e45ac0e9116643b42cdc8f1d5aab3ff032` respectively.
Each CLI batch verified configured project `bsp`, program
`/battlestationspacific.exe`, x86 LE32 and image base `00400000`; configured
`C:/Users/sqz269/bsp.gpr` exists. No stronger server absolute-project-path claim is
made. Function count remains 64729.

A read-only exact flow-property request returned no rows because bridge script
execution is disabled. Its raw response is retained. No setting or mutation
fallback was attempted. The complete listing and raw-byte gate establishes no
gap; exact saved no-return/flow-override properties remain unavailable.

## `006952A0`: two register inputs and a live dispatch loop

Entry ECX is first endpoint `F`; EDX is callback owner `M`. ESI captures M and EDI
captures F before `00694280`. No owned instruction reads a caller stack argument.
Incoming EAX is discarded by the FS-chain read. The first getter receives the
original ECX/EDX physically, so its hidden-input independence is not proved by
the owned caller. The returned lock owner is dereferenced at `+4` without a null
owner guard. Only its section pointer is tested for null.

Let `S` be entry ESP and `B=S-44` the ordinary body ESP. State, handler and prior
FS-chain words are at `S-4`, `S-8`, `S-12`. Locals hold guard section/profile at
`S-16/S-20`, captured end-field address at `S-24`, captured dispatch owner at
`S-28`, and saved outer section at `S-32`. Entry EBX/ESI/EDI backing occupies
`S-36/S-40/S-44`. On the selected-edge path, a later PUSH EBP makes loop ESP
`S-48`; maximum owned depth including call return slots is `S-52`.

For nonnull section, the body pushes it, calls operand `[00CE2218]`, then adds one
to its current DWORD at `+18h`. Owned pseudocode labels this IAT edge
`EnterCriticalSection`; the slot value and provider body remain unread. A
four-byte callee cleanup is required by the surrounding stack schedule.
Full DWORD EH state zero is written after acquisition, immediately before
`006949D0` with ECX=F and EDX=M. Its result is captured in EDI as selected edge E.
Null E skips the entire global dispatch walk.

For nonnull E, the exact loop is:

1. Load `D0=[00E198E4]`, capture cursor `[D0+4]`, save D0 and the **address**
   `D0+8`. Validate unsigned cursor <= the current DWORD through that address.
2. Push current EBP. The listed `MOV EDI,EDI` at `0069531E` is a two-byte no-op.
   Each iteration reloads current owner Dc from `00E198E4` and captures end Ec
   from `[Dc+8]` before validating `[Dc+4] <= Ec` and saved D0 == Dc.
3. Returning validation callbacks do not restart the loop or refresh those
   captured owner/end values. Cursor == Ec exits; otherwise the saved original
   end-field address is reloaded from its stack local.
4. Validate cursor < the current DWORD through that original end field, then
   compare the cursor's DWORD with E. A match causes another end check and a
   DWORD zero store. The matched decision is not rerun after validation returns.
5. Check the original end field again before adding four to cursor modulo32,
   then reload current global owner/end at the next iteration.

There are six physical validation call sites to `00BF6713`: one initial site and
five in the loop. They push no explicit arguments. Slot checks use unsigned
comparisons and reread the original owner's current end; iteration termination
uses the current owner's end captured before callbacks. The global word itself
was not read during this audit. No null, restart, rollback or repaired-default
policy is inferred for returning validation.

On loop exit, ADD -1 updates the selected edge's DWORD `+0Ch` modulo32. POP EBP
preserves those arithmetic flags, and JNZ tests that result without reloading
the count. Initial count zero therefore wraps to `FFFFFFFF` and skips deletion.
On resulting zero, the body captures both `[E+4]` and `[E+8]` before either erase,
then calls `00694F60` with ECX=first+4 / EDX=E and subsequently
ECX=captured callback+4 / EDX=E. Both require zero stack-argument cleanup.

After both erases, it reloads the current table at `[E]`, then current slot zero
from that table, pushes flag 1, sets ECX=E and calls through EDX. EAX holds the
loaded table and EDX the loaded target. The virtual call must clean four argument
bytes. There is no table identity check or fallback, and no target/table word was
opened. Selected-edge lifetime through both erases and the call remains required.

Selected-edge paths reload the release section from the **current** saved local
at `S-32`; the null-edge path instead retains current EBX. For nonzero release
pointer, current DWORD depth `+18h` is decremented before pushing the pointer and
calling `[00CE2210]`, labeled `LeaveCriticalSection` by owned pseudocode. It also
requires cleanup4. The exit reads current saved prior FS into ECX, restores the
three saved registers, writes FS:[0], adds `20h` to ESP and performs plain RET.
Compatible return gives ESP `S+4`; final arithmetic flags come from that ADD
`(S-32)+32`, while EAX/EDX remain provider/residue values. EBP preservation depends
on its conditional saved backing and all relevant providers.

## `00695870`: owner prefix, two captured sections and late free

Entry ECX is owner M, captured in EBP. No owned caller stack argument is read.
The direct owner accesses establish a minimum 12-byte range: DWORD write at `+0`,
DWORD read at `+8`, and later DWORD read at `+4`. Capacity at `+0Ch` is not accessed
by this body; the full 16-byte Source owner type is a wider provider/storage
contract rather than an owned extent proof.

Ordinary body ESP is `B=S-40`. EH state/handler/prior-chain occupy `S-4/-8/-12`;
guard section/profile occupy `S-16/-20`; owner is saved at `S-24`. Entry
EBX/EBP/ESI/EDI backing occupies `S-28/-32/-36/-40`. Maximum owned depth including
call return slots is `S-48`.

The function writes DWORD `00CE3CD4` to `[M]` before publishing full DWORD state
zero and calling `00694280`. Its returned `+4` supplies outer section O, retained
in EDI and also written to a guard local. EBX becomes one. If O is nonzero, Enter
is followed by current depth += EBX. Then **only the low state byte** is written
from BL, normally making state one; the high three bytes are not rewritten.

A second getter runs even if O is null. Its result supplies independently
captured inner section I in ESI. The two section pointers need not be equal.
If I is nonzero, Enter and depth += EBX follow. TEST I is then followed by a MOV
capturing DWORD `[M+8]` into EBX; that MOV preserves the section-test flags.
Inner depth is decremented and I unlocked before testing the captured count.
A nonzero captured count delivers ECX=current EBP to `00695530`; changes to the
owner count during unlock are not reread for this branch.

After optional detach, outer release uses retained EDI, decrements current depth
and calls Leave. It does not reload the guard local or global. Only then does
`0069590E` reload current `[M+4]` into EBP. Nonzero pointer is pushed to `00BF6989`
(saved correct `_free` label); the callee must return without argument cleanup,
after which ADD ESP,4 discards the argument. No owner allocation is freed.
Pointer/count/capacity fields are not cleared by owned code.

The epilogue loads current saved prior-chain into ECX, pops all four nonvolatiles,
restores FS:[0], adds `18h` to ESP and uses plain RET. Compatible return gives
`S+4`, with final arithmetic flags from `(S-24)+24`. Neither this body nor
unregister supplies a deliberate EAX result, an x87/SSE operation, a guard against
misbalanced providers, or a local repair of changed saved backing.

## Exception and caller composition boundaries

The actual handler immediates are `00C7E9F8` and `00C7EAA3`; guard profile literal
is `00CE37FC`. These targets/data are unopened. State widths and writes are
observed, including the destroy byte write; neither function resets state to
`FFFFFFFF` on its normal exit. This does not establish handler cleanup, FH3
identity, throwing behavior or async-fault policy. Source RAII/catch is separate
evidence. All frame arithmetic assumes the per-call cleanup and live nonvolatile
requirements recorded in the report, with valid current saved backing.

The pinned [accepted endpoint audit](CC12_OBSERVER_ENDPOINT_CLEANUP_ABI_READINESS.md)
establishes `00653390` delivering ECX=late `[M+14h]`, EDX=current M to unregister,
then ECX=current M to destroy; both children require plain-return cleanup. Accepted
member92 registration delivers captured endpoint/M to **`00694A60`**, whose
135-byte / 43-op saved metadata was checked without opening its body. Pair identity
requires the same actual member/endpoint and lifetimes across intervening work.
Under the accepted parent placement M=task+20h, the late endpoint is task+34h.
The generic 16-byte owner prefix does not cover that separate member `+14h` field.
No caller, registration body, parent or EH-handler body was reopened.

## Actual Source and current CRT evidence

`NativeObserverOwnerStorage` is exactly `10h` bytes: profile, edge pointer, count,
capacity. `NativeObserverEdgeStorage` is `10h` bytes with the four directly observed
selected-edge fields. `NativeObserverLifetime` retains real `SoundLifetimeAccess`,
mutable lock/dispatch publication references and `ObserverLifetimeServices`.
`GameObserverRuntime` owns the publication cells and retained lifetime, and binds
it to `GameSingletonHost`. An endpoint cannot serve as this service object.

Current Source unregister preserves the normal sequence through `ObserverGuard`,
nested `find_pair`, captured dispatch owner/end checks, unsigned reference decrement,
both captured endpoints and a fresh table read. Its deletion helper directly uses
the Source canonical CF7E64 implementation when that table matches; otherwise it
uses a virtual service. Current `GameObserverRuntime` throws for unsupported edge
profiles. Native always calls the freshly read slot zero, so this Source routing
is a qualified policy boundary, not generic Native virtual-target equivalence.

Current Source destroy uses separate outer/nested guards, captures count before
inner unlock, optionally detaches, then reloads/frees current array. Its catch also
frees the current array and rethrows. `ObserverGuard` adjusts the Source section's
DWORD depth around Win32 calls; `singleton_lifetime_free` calls `std::free`.
Those Source contracts do not prove the unopened Native providers or EH handlers.

The current validation service is the existing local naked
`observer_crt_invalid_parameter() { jmp _invalid_parameter_noinfo }`.
The newly admitted standalone Source17 wrapper remains separate and unbound.
Its [primary receipt](../reports/cc12_native_invalid_parameter_noinfo_call_primary_review.json)
records 17 bytes / 9 operations and DIR32 at offset 9 to the actual import, while
Native16 retains its five physical pushes and direct Native36 call. The accepted
Native36 global `0109DD64` remains unread; its handler selection and tail delivery
are not reproduced merely by borrowing the Source zero-argument API.

All 80 current Source input hashes, eight accepted receipt/document pins, seven
actual provider Source files and four current build artifacts were replayed.
The prior normal build at `16:28:10.509150Z..16:28:29.144081Z` passed three existing
checks; its receipt records 16 whole objects and 18 positive Core public roots.
Those object/import records are inherited, not re-parsed here. SDK `10.0.26100.0`
still declares `_ACRTIMP_ALT void __cdecl _invalid_parameter_noinfo(void);` at line
371, without noexcept/noreturn. The selected project remains `v145`, Win32 Release,
`MultiThreadedDLL`, `ExceptionHandling=Sync`. Source import IAT `103CC620` is Source
metadata, never evidence for either Native IAT operand or a handler policy.

The remaining binding needs actual member/endpoint storage and the retained
service, plus explicitly qualified subordinate, CRT and EH policies. This audit
supplies no raw entry adapter or production binding and grants no new Source,
Original ABI, runtime, startup or gameplay credit. The
[report](../reports/cc12_observer_pair_release_owner_ABI_readiness.json) contains all
153 decoded operations, 21 call contracts, 57 explicit-memory operand mappings,
frame/flag contracts, receipt pins and the unavailable flow-property response.
