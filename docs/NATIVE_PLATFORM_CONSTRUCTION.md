# Raw platform construction and retirement

This module adopts the six raw source bodies from commit
`0ca920c74f87df949340d01fbb66025973cd9959` onto Main
`9228d13fe2594cb12405733fcdd5ec0160770b66`. The original six source bodies are
preserved; the follow-up retirement packet starts from `e7d28388b`.
It provides source-callable construction for actual **0x184-byte (388-byte)**
platform storage, publication through a borrowed raw singleton domain, and
window-base and derived retirement. The D68CC4 canonical singleton deletion
dispatch now borrows the same raw creation context. Application activation
remains absent. Evidence: `reports/native_platform_construction.json`.

| Native body | Source operation | Original ABI |
| --- | --- | --- |
| BE2960..BE29F0, 145 bytes | Publish/register platform base | ECX owner; EAX original owner; RET |
| BE2A00..BE2A98, 153 bytes | Unregister platform base | ECX owner; no specified result; RET |
| BE2AC0..BE2B09, 74 bytes | Construct window base | ECX owner; EAX original owner; RET |
| BE2B10..BE2B76, 103 bytes | Destroy window base | ECX owner; no specified result; RET |
| BEC710..BEC729, 26 bytes | Allocate/self-link queue sentinel | ECX unused; EAX node; RET |
| BECDA0..BECE23, 132 bytes | Construct derived Win32 platform | ECX owner; EAX original owner; RET |
| BEC730..BEC777, 72 bytes | Destroy raw text queue | ECX queue header; RET |
| BECA00..BECA29, 42 bytes | Retire derived Win32 platform | ECX owner; tail jump BE2B10 |
| BECE30..BECE6E, 63 bytes | Retire/free derived Win32 platform | ECX owner; stack flags; EAX original owner; RET 4 |

The first eight bodies consume no original stack arguments. The added C++ context
parameters are new interfaces, not replacements for the original register/FH3
ABI. Names are behavioral hypotheses, not recovered symbols. Numeric profiles
are identity DWORDs and must never be called as host vtables.

## Preservation and fresh evidence

The previous author's checkout, local fixture and immutable-closure path are
absent. Before building, the exact four committed candidate files were frozen
under `local/platform_owner_adoption/frozen_candidate`; `freeze.json` records
their hashes and missing old paths. The old report is available from its commit
with SHA-256 `7e03412a1bab726478979c0004580b4c32b77cc23332db7ff36dbd0ce7c1cd2f`.
Its renderer/device/installed-effect run and historical build hashes are not
fresh evidence and are not adopted as current results.

Current live Ghidra, the installed PE and the frozen candidate's byte digests
agree for all 633 function bytes and 300 bytes of EH handlers, FuncInfo records,
unwind maps, cleanup funclets and profiles. Thirteen direct calls and four
named Enter/LeaveCriticalSection imports were rechecked. Each Ghidra wrapper
verifies project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Installed PE SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

## Publication, cleanup and provider contracts

BE2960 arms generic-base cleanup before storing D68604. It resolves the actual
raw manager, captures section +10h, enters it and increments section+18h.
Only then is guard cleanup armed and the original receiver published to the
borrowed 0109CF04 cell. The second manager getter precedes the current
publication read used by BD0C30. Normal decrement/Leave retains the first
captured section, with guard cleanup armed through Leave. Return is the
original receiver even if a provider changes the publication.

BE2A00 writes D68604 before arming generic-base cleanup. It calls BCFCA0 on the
current publication after the second getter, clears publication only after
removal returns, releases the captured section and resets the original owner
to CE3818. BCFCA0 clears only the first matching registry slot; its raw vector
length and later slots remain unchanged. E01270/E01260 and E012A4/E01294 provide two cleanup states each:
state1 calls 411EE0 on the eight-byte guard, then state0 calls 412430 on the
original receiver. First-enter failure reaches only state0.

BE2B10 writes D68608 and captures title data +8h before arming base cleanup.
A nonnull title captures DWORD-wrapped length+1 before the actual 419CC0 pool
getter and BD1510 return. The title header remains stale. The getter may throw;
this path does not pass through the semantic noexcept string-release adapter.
E012D0/E012C8 and CC6B00 establish the cleanup call to BE2A00. Normal BE2A00 is
called after disarming that cleanup and is not retried on failure.

The current providers are `get_native_singleton_manager_00415350`, raw
BD0C30/BCFCA0 registration/removal, the raw-publication overload of
`native_string_pool_get_or_create_00419cc0`, and
`return_native_string_pool_00bd1510`. The supplied string context and platform
context must borrow the same canonical 01090AA0 manager, 01090AA8 pool,
01090AA4 return gate and 0109CF04 platform cell. This module owns no second
manager or private publication. Allocation/free uses the current source CRT;
original CRT exception identity and original hardware-fault unwind are unproved.

## Incoming bytes and derived construction

BE2AC0 completes base publication, clears the native title header and executes
**FILD signed DWORD +24h / FIDIV signed DWORD +28h**. It writes 640 and 480
only afterward, before FSTP float +10h. The inline assembly keeps every
interleaved store and the caller's x87 environment. No default aspect or
incoming dimension preimage is inferred from the later constants.

BECDA0 arms window-base cleanup after that constructor returns, writes D68CC4,
allocates a real 0Ch queue sentinel through BEC710 and writes it to +178h with
zero count at +17Ch. Its subsequent byte/DWORD clears retain original order.
Queue +174h, requested dimensions, power-policy storage and tail +182h/+183h
remain untouched. E01F94/E01F8C and CC7460 establish the exception cleanup to
BE2B10. BEC710 self-links +0/+4 and leaves +8..+Bh untouched, preserving both
pointer tests and DWORD wrapping; it does not normalize null allocation.

## Construction adoption validation

The fresh validation section in the report records the strict Win32 build,
standalone manifested probe, exact library/object/executable hashes and results.
The fresh retained artifact manifest is
`local/checkpoints/cc14-native-platform-owner/final/validation.json`.
The candidate is compiled separately with `/O2 /MD /EHsc /W4 /WX /fp:strict`
and linked against this worktree's freshly built Core. CMake registration is
left to the primary integrator; the unregistered Core build alone does not
validate this new translation unit.

The focused probe uses explicit adversarial incoming bytes
(-2147483647 / 16777217), not a claimed game allocation preimage. Its original
execution is limited to unchanged BE2AC8..BE2B07 (64 bytes), with only a receiver
prelude and return suffix. The source/fragment comparison checks all 388 bytes
and x87 control/status. Source checks additionally cover real raw registration,
tracked-section balance, shared-pool title return/reuse, stale headers, base
retirement, derived construction and sentinel links. Fixture cleanup frees its
sentinel explicitly; it is not derived-destructor execution. No permanent test
framework or game/window/renderer launch is introduced. The initial probe
incorrectly expected unregister to compact the registry; the assertion was
corrected against the existing BCFCA0 raw contract and preserved null slots.
The standalone link also needed the application's advapi32/shell32 imports.
Failed logs/binaries are retained; neither correction changed the candidate.

## Raw derived retirement and current validation

The raw queue header at platform+174h is 12 bytes: +0 is untouched, +4 is the
current sentinel, and +8 is count. BEC730 resets the sentinel's two links and
count, captures each node's next pointer before freeing it, reloads the current
sentinel after each free, then frees and clears the sentinel. The existing
`TextInputQueue` owns a different private typed allocation and cannot provide
this contract. The naked Source wrapper retains the complete original 72-byte
instruction sequence, including the comparison before the count store and the
alignment instruction. Only its two free call operands change to the canonical
current Source CRT provider. Both COFF and linked bytes verify this equality.

BECA00 and BECE30 store D68CC4, call the actual
`SystemParametersInfoA(0x11, 1, nullptr, 0)` and ignore its result, retire the raw
queue, then run BE2B10. The scalar form frees the original owner only when flags
bit 0 is set, after successful base cleanup. Neither path destroys a window,
restores the captured power scheme, or introduces a platform-stop call.
The canonical singleton dispatcher requires an explicit `platform` context,
appended at offset 200 in its Source binding structure; every prior field offset
is preserved. Missing binding follows the existing fail-closed path.

The retirement packet rechecks all 177 body bytes and the 4-byte D68CC4 slot
against saved listings, live Ghidra and the installed PE. Its strict Win32
candidate object and manifested probe use current Core and actual Win32 APIs.
The focused fixture compares copied Original72 queue code (only the two free
operands rebound) against Source for a populated queue. It also checks a
bit0-clear scalar retirement across all 388 bytes, title return/reuse, cleared
publication and preserved registry holes, then drains a second empty-queue owner
through the actual D68CC4 binding and the same manager/string-pool cells. This
second case owns the flags1 platform free, pool free, vector and section cleanup.
The fixture saves, restores and reads back the real screensaver setting; it was
0 before, after both retirements, and after restoration. API success or an
effective system-setting transition is not asserted by the candidate.

The permanent CMake row is coordinated separately. This packet's combined
`scripts/build.ps1` qualification uses an ignored local
`CMAKE_PROJECT_INCLUDE_BEFORE` overlay that adds the actual source to Core.
The exact overlay, cache entry, build result, current libraries, probe and
emitted code are retained in
`local/checkpoints/cc14-native-platform-owner-retirement/final/validation.json`.
The earlier construction-adoption artifact set remains unchanged. No permanent
test framework, host binding or game/window launch is added.

## Remaining full-owner integration

1. Preserve a genuine canonical 0x184-byte allocation and its incoming +24h/+28h
   bytes. Bind the one actual platform publication into every consumer, including
   Text's signed DWORD +28h read at AB8CE0. The existing
   `NativePlatformWindowFocusStorage` is only 0x44 bytes, with recovered HWND
   +30h and active +41h; its opaque +28h bytes are not configuration height.
   Neither that receiver nor `Win32PlatformState` may be cast or cloned into a
   full raw owner.
2. Bind actual window/configuration BECEE0 and HWND/message ownership through
   BEC3B0 -> BED3B0, including the original stack ABIs, window-extra receiver,
   renderer/input/UI/audio dependencies and shared backing fields. Retain the
   real render-entry-cache and power-policy owners. Do not invent a normal-exit
   +8 stop call: the inspected BECA00/BECE30 paths enable the screensaver but
   do not restore the captured power scheme. See
   `docs/PLATFORM_STOP_SCHEDULE_CC10.md` and `docs/STARTUP_WINDOW_HANDOFF.md`.

Allocation/registration/Leave exceptions, second-failure termination, original
FH3/SEH ABI, concurrent publication mutation and invalid-pointer behavior have
not been differentially exercised. The source cleanup schedule is supported by
assembly/EH evidence. The populated copied queue case does not exercise a free
provider that mutates the sentinel; exact instruction equality preserves those
reloads. Fresh build and fixture results do not prove Original whole-derived-body
execution, native ABI replacement, active frame, draw or gameplay.
