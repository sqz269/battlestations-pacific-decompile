Primary review: Root independently read the selected18B/four-op handler/action and44B descriptor/map, replayed live/PE bytes, and read/replayed the already admitted25B/eight-op cleanup provider. Root also read all31B/12 selected Source provider operations and verified its whole object unchanged in the Source109 build. Normal cleanup uses captured K, while unwind reads current actual guard+4; cleanup remains armed through normal release. New Source composition must state its own guard-storage and C++ failure policy. No new Native discovery or implementation credit is supplied.

The current registered build snapshot is Source109:109 inputs, four artifacts,30 captured/replayed objects,34 positive selected Core roots and three existing checks passed. The worker section below is an immutable Source107 snapshot; its frozen Core/provider captures remain verified historical evidence. Other provider bodies/EH, Original interpreter/frame/CRT ABI, startup and gameplay remain unproved.

# Tick subnode reparent failure-cleanup readiness refresh

This refresh confirms the existing encoded unwind edge for Native
`00876020..00876112` and identifies its actual current Source cleanup provider.
It adds no Native discovery, reconstruction, Source function, ABI or runtime
credit. The original selected edge is already recorded in
`CC12_TICK_SUBNODE_REPARENT_UNWIND_CONTRACT_READINESS.md` and its JSON report.
This packet independently replays that edge and checks the provider against
the Source107 build snapshot captured during this work.

## Selected bytes and prior evidence

The bounded code is handler `00C964E8..00C964F1` (10 bytes, two operations) and
action `00C964E0..00C964E7` (eight bytes, two operations). The handler loads
EAX=`00DC864C` and tail-jumps to `00BF6B43`. The 36-byte descriptor at
`00DC864C..00DC866F` contains DWORDs
`19930522,1,00DC8644,0,0,0,0,0,1`. Its selected eight-byte map at
`00DC8644..00DC864B` contains `FFFFFFFF,00C964E0`: previous state -1 and the
action address under the accepted x86 FuncInfo interpretation.

The action executes `LEA ECX,[EBP-14h]; JMP 00411EE0`. It does not load a
pointer from that frame address, push an argument, change arithmetic flags,
reset the state word or restore FS. The runtime must supply the appropriate
EBP. The handler has both saved instruction starts but no containing saved
Ghidra function; no listing repair or function creation is needed or performed.
All 18 code bytes/four operations and 44 data bytes match the Original PE and
live analysis. Root independently read and replayed these exact ranges.

The prior document's statement that shared `00BF6B43` is unopened is
historical. The later squadron-task EH report records Root's accepted complete
54-byte/27-operation review. Its older embedded capture flag remains false;
the report's final `validation.full_Root_Astra_assembly_read` is true. This
packet pins that accepted report without reopening the body. Its downstream
`00C07991` interpreter remains unopened. Neither metadata nor the shared
wrapper establishes Original dispatch, frame-register setup or exception policy.

## Current guard contents and active state

The accepted 243-byte/80-operation caller audit and Root's fresh caller gate
supply this context; this packet does not repeat the full caller audit. With
S=caller entry ESP, the registration is at S-0Ch and the raw eight-byte guard
is at S-14h. The guard contains profile `00CE37FC` and a section pointer at +4.
Under the accepted current x86 comparison convention EBP=S, the action's LEA
addresses that guard. Actual Original interpreter delivery of that EBP remains
unproved; the unconditional physical action input is the address EBP-14h.

State -1 remains through the singleton getter, optional Enter/depth increment,
late incoming-node read and old-parent read/TEST. The explicit state-zero
DWORD write follows those operations. No later explicit state reset occurs
before normal depth decrement, LeaveCriticalSection and FS restoration.
Arbitrary aliases can still change the state or guard backing.

Normal cleanup uses the section captured in EBX. Selected unwind cleanup
instead reads the current stored guard+4. Those pointers need not agree after
alias writes. Using an automatic cleanup destructor on every normal return,
or substituting a saved pointer for the unwind provider's current guard read,
would lose this distinction. Disarming before normal Leave would also narrow
the visible state-zero interval. The descriptor does not prove whether a
particular fault/exception invokes the action, how the interpreter advances
state, or that an action/release executes exactly once.

## Actual cleanup provider and current compiled call

The actual Source API is
`void bsp::destroy_native_singleton_guard_00411ee0(void* actual_8byte_guard)`
in `include/bsp/native_diagnostic_sink_lifetime.hpp` and
`src/native_diagnostic_sink_lifetime.cpp`. It is an ordinary C++ function, with
no explicit `noexcept`, local catch or automatic cleanup object. No differently
named lock-cleanup interface or replacement service is introduced.

It borrows the actual eight guard bytes and copies the pointer from +4 before
copying profile `00CE37FC` to +0. On a nonnull captured pointer, it decrements
the current physical unsigned DWORD at section+18h and calls
`LeaveCriticalSection(captured)`. It preserves guard+4, does not reload it
after profile publication, and performs no lookup, pointer validation,
rollback, count repair, extra release or free. Prior writes remain visible if
later work fails. The non-atomic unsigned decrement wraps modulo 32 bits.

The pointer must address actual Win32 section storage: a 24-byte
`CRITICAL_SECTION` prefix followed by the current tracked DWORD, requiring
at least 1Ch bytes. Existing `TrackedCriticalSection` asserts that layout.
`SystemSingletonCriticalSection` is a separate C++ projection containing a
native pointer and counter reference; passing that projection's address as
the raw section pointer does not satisfy this provider's contract. Retain the
actual guard, section and relevant owner/publication lifetimes through release.

The already admitted Native provider `00411EE0..00411EF8` was also replayed:
25 bytes/eight operations match saved starts and PE/live bytes. Native captures
guard+4 in EAX, tests it, writes the profile, conditionally adds -1 to current
section+18h, pushes the captured section and calls IAT `00CE2210`, then RET.
The four-byte IAT cell matches PE/live bytes; the verified Original PE import
metadata names `KERNEL32.dll!LeaveCriticalSection`. No OS implementation was
opened or executed. Root also read/replayed the provider and replayed the IAT.

The captured Source107 Core contains the actual existing 12,242-byte provider
object. Its selected public COMDAT section is 31 bytes/12 operations and has
one relocation to `__imp__LeaveCriticalSection@4`. All 31 selected bytes were
decoded and read; other object bodies/EH were not reviewed here. This section
loads the guard from a stack argument, captures +4, writes the profile, tests
the pointer, conditionally DEC/push/calls the SDK import, and returns through
its own EBP frame. It has no local EH registration. This is evidence for the
actual Source call, not the Original ECX receiver ABI, registers or flags;
Native ADD and Source DEC also have different carry-flag behavior.

The generated Release Win32 project selects `ExceptionHandling=Sync`, current
MSVC 19.51.36244.0, and Windows SDK 10.0.26100.0. Its pinned `synchapi.h`
declares `VOID WINAPI LeaveCriticalSection(_Inout_ LPCRITICAL_SECTION)` without
an explicit C++ exception specification. A void result supplies no success
status. The Source provider adds no catch or terminate rule; a future owning
guard must state its own C++ failure policy. Current synchronous C++ settings
do not establish Original SEH/hardware-fault delivery or successful OS release.

## Readiness and validation boundary

Qualified Source243 composition now has a verified selected cleanup target and
the real callable provider. It still needs explicit actual storage/publication
bindings, the late reads and active interval, captured-normal versus current-
guard cleanup selection, and a stated C++ cleanup-failure policy. An explicit
noexcept owning guard would terminate if its cleanup threw; that would be the
new Source guard's policy, not an existing provider declaration or proof of
Original interpreter behavior. No fake Native frame, guard, global, callback,
lock object, blanket rollback or automatic registration substitution is added.

The Source107 receipt is
`reports/cc12_native_lua_variant_header_initializer_primary_review.json`:
normal build 2026-10-09 18:56:24..18:56:40 UTC, 107 inputs, four artifacts,
29 whole objects, 33 positive public Core definitions and three passed checks.
The full prior 28-object replay is Root-reported. This packet checks five
relevant input pins and its selected provider object/Core member; that object
is outside the whole-29 review. Source107 is a snapshot at this readiness
capture, not a promise that subsequent Root builds leave artifacts unchanged.
Source105, Source97 and the caller audit's Source57 artifacts are historical.

Validation includes bounded Native PE/live/saved-start comparisons, Root's
independent selected-edge gate, actual Source/provider/SDK inspection, selected
COFF relocation and archive-member equality, pinned receipts, JSON and diff
checks. No Source, CMake, ledger or Ghidra mutation, build, test, probe, new
consumer, fixture or runtime execution occurs in this two-file packet.
