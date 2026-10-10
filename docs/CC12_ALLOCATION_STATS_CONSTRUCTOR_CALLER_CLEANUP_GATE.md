# Allocation statistics constructor and caller cleanup gate

The complete ordinary base and derived constructor schedules, a bounded caller
allocation/construction window and the exact constructor handler prefix are
established. An allocated owner API remains held on the required caller and
exceptional cleanup contract. Existing genuine allocation, manager,
registration, section and drain services do not establish those effects alone.

This packet starts at worker commit
`e5821c68b993926e9695c0d03d42f9d858ce0b00`; Root's primary integration of that
preceding base-profile gate was pending when this independent packet began.
The two owned files are this document and its paired report. No Source, CMake,
ledger, Ghidra, build, test, probe or runtime change is made here.

## Base constructor

`BE2750` is currently exactly `FUN_00be2750`, non-thunk, `no_return=false`, with
one complete saved range `[00BE2750,00BE27E1)`, 145 bytes. Its metadata prototype
is `undefined FUN_00be2750(void)` and does not recover the original ABI. The
exact original/live range is equal, fully decodes as forty instructions, and
matches all saved listing starts and typed lengths. SHA-256 is
`ef03c06e1bfb76ac89a9582e413b7d559adb99122dbfea3d2c184aa59d36adf0`.

The body establishes an FS exception frame with the actual operand `CC6A90`,
saves ESI/EDI, captures ECX receiver in EDI and saves that receiver at stack
local `ESP+8`. It sets state zero at `BE2770` **before** stamping the receiver's
profile `D685E0` at `BE2778`. The first manager getter at `BE277E` precedes
capturing that manager's `+10` section in ESI. The local guard contains profile
`CE37FC` and the same section pointer. A nonnull section is entered and its
actual `+18` recursion word is incremented.

State one is set at `BE27A1`, then the captured receiver is published to
`0109CEFC` at `BE27A6`. The second getter at `BE27AC` returns the manager used
for registration. Only after that getter does `BE27B1` reload the **current**
`0109CEFC`; `BD0C30` receives that current value rather than an assumed immutable
copy of the original receiver. On normal return, the originally captured ESI
section is decremented and left if nonnull. The function restores FS/ESI/EDI,
returns the original receiver in EAX and uses plain `RET`.

Typed call metadata identifies genuine `00415350` and `BD0C30`, plus imported
Enter/LeaveCriticalSection. No corresponding callee body or import-table word
is newly opened. State indices and the normal release sequence do not prove
the handler's cleanup order, publication clearing, unregistering, profile
reset, allocation release or native exception behavior.

## Derived constructor

`BE2900`, currently `BSP_AllocationStats_Construct`, has one complete saved
32-byte range `[00BE2900,00BE2920)`. All original/live bytes agree; all nine
instructions decode and match the saved listing and typed lengths. It saves
ESI, captures ECX receiver, and calls the actual `BE2750` at `BE2903`. Only
after that call returns does it stamp `D685F4`, write `40000000` to receiver
`+4`, and write zero to `+8`. It returns the captured receiver and restores ESI.

This body contains no allocation call and no separate local exception frame.
Neither it nor the current semantic three-field C++ structure proves which
caller owns a failed construction's allocation. Its capped caller identity
query returns `BSP_Application_Initialize` at `0073D410`.

## Bounded caller and handler evidence

The caller's current saved AddressSet totals 4,372 bytes in two ranges. Only
`[0073D45D,0073D480)`, 35 bytes, was separately admitted for original/live
comparison. Its ten decoded instruction starts and lengths match current
typed listing metadata. No enclosing caller listing or whole body was requested.
The window pushes `0Ch`, calls the established allocator `BF681B`, consumes
that argument and saves EAX to `[ESP+18h]`. It compares EAX with incoming EDI,
writes EDI to `[ESP+144h]`, branches to `73D47D` on equality, or otherwise passes
EAX in ECX to `BE2900` at `73D478`. The final instruction is `OR EBX,-1`.

This proves the twelve-byte allocator request and this conditional construction
schedule. It does **not** establish incoming EDI as zero, identify the saved
`ESP+144h` word as an exception state, show a post-call state store, or identify
the free-on-failure path. Calling the branch a proved null gate or the store a
proved state-zero transition would exceed this bounded evidence.

The actual constructor operand admits the separate ten-byte prefix
`[00CC6A90,00CC6A9A)`: `MOV EAX,00E01208; JMP 00BF6B43`. Original/live bytes agree
and both five-byte instruction boundaries match typed metadata. Neither start
has a function owner. `CC6A95` retains its `CALL_RETURN` override, with default
unconditional jump and effective call terminator. No function was created and
no override changed. The loaded `E01208` value was followed only after a separate
metadata and exact-data authorization. No CRT handler code was opened.

## Descriptor and map

Typed metadata identifies one defined 36-byte DataDB item
`[00E01208,00E0122C)` and a distinct following item; it does not expose the actual
data-type name. Exactly these original/live 36 bytes agree. Their nine raw words
are `19930522,2,E011F8,0,0,0,0,0,1`. Retained installed Ghidra source models
independently support the conditional x86 V3 FuncInfo interpretation: two unwind
entries at `E011F8`, with no try/IP/type-list table and EHFlags value one. The
flags are not further interpreted. These local layout sources are not an
attestation of the loaded plugin's Java CodeSource or of native CRT behavior.

After that gate, endpoint metadata identifies one defined 16-byte DataDB item
`[00E011F8,00E01208)`, ending exactly at the separate descriptor. The exact
original/live 16 bytes agree. Under the retained 32-bit unwind-entry layout,
entry zero has signed destination state minus one and action `CC6A80`; entry
one has destination state zero and action `CC6A88`. Both actions are separately
defined non-thunk functions with one complete eight-byte saved range. This
establishes the conditional table entries; it does not by itself establish
funclet frame restoration or any pointed-to cleanup effect.

The two separately authorized action bodies also match the original image:
`CC6A80` loads ECX from `[EBP-18h]` and jumps to `00412430`; `CC6A88` computes
ECX as `EBP-14h` and jumps to `00411EE0`. Both five-byte jumps retain their
`CALL_RETURN` overrides over default unconditional jumps. Their direct targets'
metadata currently names `BSP_SingletonBase_ResetProfile` and
`BSP_SystemSingletonGuard_Destroy`, with complete seven- and 25-byte saved
bodies respectively. Their accepted complete receipts and current ordinary
Source contracts are reused below; their original/live windows were not reread.

The constructor's observed receiver local is entry-ESP minus 18h and its guard
local is entry-ESP minus 14h. This aligns numerically with the two funclets if
their EBP equals the constructor's entry ESP. The packet has not established
that CRT frame restoration, so the association remains conditional. No
unregister, publication clear, allocation free or full destructor invocation is
present in either eight-byte action. The accepted targets perform only the
specific profile/guard operations below, not the full `BE27F0` destructor.

The relevant existing application-constructor and tick-reparent unwind
readiness reports also leave the original helper's EBP setup unproved. Current
metadata alone identifies `BF6B43` as a non-thunk, returning 54-byte function
with one complete range. Its provisional `FID_conflict:___CxxFrameHandler3`
name and undefined prototype do not establish its ABI. A capped 16-caller query
and its `___InternalCxxFrameHandler@C07991` callee identity add no frame proof.
Neither helper's body is opened here.

## Current Source prerequisites

The current canonical allocator uses the genuine `BF681B` allocation boundary:
malloc, current CRT new-handler retry, then bad allocation when the handler
declines. Its matching free service already exists. This establishes an
available Source service corresponding to the observed twelve-byte request;
it does not establish the caller's cleanup schedule for this owner.

`SoundLifetimeAccess` can borrow the actual canonical `01090AA0` publication.
Its manager view preserves resolving the manager before evaluating a current
registration argument, and dispatches actual `BD0C30` registration. The
existing captured-section service stores the first manager's section once,
enters/increments it, and decrements/leaves that same section during ordinary
destruction. None supplies an inferred allocation-statistics failure guard.

The raw shared drain pops an entry before reading its current profile and
dispatching flags one. Unbound profiles are rejected; the drain's existing
ordinary catch clears its remaining manager storage. Current Source has no
concrete `D685E0` or `D685F4` deletion binding. The separate startup semantic
object is stack-local with a null profile and does not perform base publication
or registration.

Any later heap owner must install its genuine base/derived dispatch before
publication or registration and retain the associated context through the same
canonical drain. Accepted normal destructor and scalar schedules are retained
as prerequisites. They do not establish `BE2750`'s failure cleanup or authorize
freeing, clearing or unregistering on an assumed exceptional path.

The accepted `00412430` leaf and its current
`destroy_native_generic_singleton_base_00412430` implementation write only
`CE3818` through the supplied receiver. Its existing complete seven-byte
receipt, listing and live-byte capture are preserved. It is not the scalar
deleter and does not free storage.

The accepted `00411EE0` leaf and current
`destroy_native_singleton_guard_00411ee0` take actual eight-byte guard storage.
They capture its current `+4` native section pointer **before** stamping
`CE37FC`, then decrement the section's current physical DWORD `+18h` and call
LeaveCriticalSection if nonnull. They preserve guard `+4`. The complete
25-byte historical preimage and its retained listing/live receipt are copied.
No historical fixture or build is rerun or treated as this constructor's EH
proof. A later ordinary Source composition can use these genuine leaves with
explicit valid-storage and exception-domain constraints; a captured-section
RAII object alone does not prove mutable guard/frame-spill alias behavior.

## Retention and limits

The fresh original/live scope is 238 instruction bytes and 52 descriptor/map
bytes, in eight separately admitted windows. All 65 decoded instruction starts
match current typed lengths. The original image's PE mapping and whole-image
integrity hash come from the preceding packet; no header or whole-image hash
is repeated here. Each exact read checks the original image's size and mtime.

The report retains 97 accepted typed responses and 91 other raw GET responses.
One local atomic-save failure retained 43 complete responses; it is separately
marked rejected. The identical scoped retry succeeded, and all 43 payloads are
byte-identical to that accepted retry. No analysis or repair was used to retry.
Accepted batches have distinct epochs 19, 21 and 24; Root confirmed separate
Root annotations caused the advances. This is not a single unchanged-program
interval claim.

The immutable bundle preserves 69 complete current inputs, including 36 Source
files, both working/Git forms where newline representation differs, the prior
base-profile bundle, selected accepted cleanup receipts, current bounded Source
excerpts and all local capture helpers. Whole Source files are provenance;
semantic review is limited to the named excerpts and complete bounded searches.
The original helper frame, startup incoming EDI/state identity, caller
allocation-failure release and concrete allocated-owner/provider composition
remain held. No new build, fixture, original ABI or game validation is claimed.
