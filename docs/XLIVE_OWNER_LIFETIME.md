# XLive allocation identity and shared lifetime

Addresses: 00A3F530, 00A3F5D0, 00A3F670, 00A3FDC0.

This packet connects the existing XLive owner bodies to the application's raw
singleton manager. It is a source allocation/projection boundary, not the native
3F0h manager layout or a binary ABI replacement. Ghidra was read-only, verified
as project `bsp` (`C:/Users/sqz269/bsp.gpr`), program
`/battlestationspacific.exe`, x86 little-endian, image base 00400000.

## Identity and ownership

The legacy `XLiveManagerOwner` begins with a reference to its pump context. Its
native vtable word is elsewhere, in `owner.storage.vtable_00`. Registering that
projection directly in raw BD0400 would read the context pointer as a deletion
profile. A second vtable word maintained beside the canonical storage would also
be incorrect: constructor/destructor writes and unwind must affect the word the
drain reads.

`XLiveOwnerAllocation` owns its canonical `XLiveManagerOwnerStorage` as the first
member, followed by a pointer to its owned typed projection. Compile-time standard
layout and offset checks establish that `identity()` addresses the same DWORD as
`owner().storage.vtable_00`. The projection's context, sign-in state, lifetime
access and services remain borrowed. The projection allocation is source metadata;
neither allocation claims original native size or field offsets beyond that first
vtable word. There is no proxy registration token or duplicate field storage.

Construction copies the supplied defined scalar preimages and binds the projection.
It performs no native construction, registration, SDK operation or initialization
of borrowed fields. In particular the initial pump's unwritten +12C/+14C inputs
are still the caller's responsibility. `XLiveOwnerAllocation`'s destructor frees
only this source storage and its projection. Native teardown must already have
finished, or explicit failed-construction host recovery must have removed any
surviving publication/registration before storage is discarded.

`XLiveManagerLifetimeAccess(SoundLifetimeAccess, published)` borrows the same raw
01090AA0 domain as sound. The original `(SingletonLifetimeDomain&, published)`
constructor and `manager_00415350()` semantic API remain available. The new
`manager_view_00415350()` and `lifetime_access()` expose the existing shared view
and captured-section implementation; no private manager is created.

F8ABE8 remains one `XLiveManagerOwner* volatile&` slot. After the native second
manager getter, `published_registration_identity()` reads that slot once. In raw
mode it validates and returns that owner's allocation identity; in semantic mode
it retains the legacy projection pointer. Null publication maps to null. It never
caches a separately published pump context or assumes the current owner is `this`.
An unbound projection is not admitted to raw registration.

## Recovered ordering

| Routine | Coverage | Native inputs and return |
|---|---|---|
| A3F530 | Complete existing base body and C++ unwind; shared lifetime/identity binding added | ECX owner; RET; EAX captured owner |
| A3F5D0 | Complete existing base body and C++ unwind; shared lifetime/identity binding added | ECX owner; RET |
| A3F670 | Complete existing scalar body; allocation overload added | ECX owner; flag byte in DWORD stack slot; RET4; EAX captured owner |
| A3FDC0 | Complete existing scalar body; allocation overload added | ECX owner; flag byte in DWORD stack slot; RET4; EAX captured owner |

A3F530 writes D24138, captures the first 00415350 result's +10 section, enters it
and increments +18, publishes F8ABE8 at A3F586, re-gets the manager at A3F58C,
reloads the published owner at A3F591, and registers through BD0C30 at A3F59A.
A3F5D0 similarly re-gets at A3F626, reloads at A3F62B and unregisters the current
identity at A3F634, then clears publication at A3F639. The originally captured
section is released even if callbacks change a publication.

Both catches surround the captured-section lifetime. Consequently its destructor
runs before the catch writes CE3818. Constructor map DE9D90 is
`{-1,CB4120},{0,CB4128}`; destructor map DE9DC4 is
`{-1,CB4140},{0,CB4148}`. The state1 lock funclets tail-jump to 411EE0; state0
tail-jumps to 412430, whose body is the root-vtable store. Publication and partial
registration are not rolled back. These are C++ unwind semantics, not arbitrary
native FH3/SEH stack behavior.

The scalar wrappers capture incoming ECX in ESI, call the complete destructor,
test only flags bit0, optionally free captured ESI, then return it. The existing
projection overloads keep their source return value `&owner`. The allocation
overloads capture `identity()` before invoking them and return that address even
after free. They do not add a second free or call another destructor. Supply
`XLiveOwnerAllocation::free_owner_storage` as the existing
`RecoveredXLiveManagerOwnerServices::FreeOwner` callback; it deletes the captured
allocation and projection once, without following the current global or destroying
borrowed runtime state. A custom owning service must provide the equivalent actual
deleter. Flags1 requires allocation by `new`, and the caller must relinquish any
owning pointer after that scalar call. Flags without bit0 retain the allocation.

All four functions and terminal instructions already exist. Current listings have
no gaps in A3F670/A3FDC0; their returning-free ADD ESP,4 instructions at A3F685 and
A3FDD5 are present. No function definitions, tail repairs or no-return changes are
requested. Exact ends, call rows, vtable data and EH tail sites are in the report.

## Integration and validation boundaries

The primary integrator owns finite D24138/D2413C dispatch in BD0400 and the
GameSingletonHost binding. It must check the exact retained allocation identity,
bind before registration, retain all services through raw drain, and avoid a
second owning delete after scalar flags1. A failed constructor can leave CE3818
and a publication/registration behind: explicit host recovery must retain the
allocation and remove that registration before freeing it. Generic root-profile
free cannot dispose of this source projection metadata on its own.

This packet does not enable A40DF0 in GameStartupHost, implement SDK shutdown,
alter IPC's native timeout behavior, initialize an input owner, or supply missing
game profile/renderer bindings. Existing native derived cleanup omissions remain
unchanged. See XLIVE_MANAGER_OWNER.md and XLIVE_STARTUP_SERVICES.md.

Standalone MSVC Win32 Release `scripts/build.ps1` passed, with both existing
CTests and all eight verified seeds. `verify_report_calls.py` checked 12 direct
CALL rows with zero failures; four Win32 indirect calls and six EH tail sites
are qualified separately. Logs are `local/xlive_owner_ac_build.log`,
`local/xlive_owner_ac_seeds.json` and `local/xlive_owner_ac_calls.log`.

The ignored `local/run_xlive_owner_ac_fixture.cmd` passed with `/W4 /WX` and
`/MANIFEST:EMBED`, linking this worktree's `bsp_core.lib`; its output is in
`local/xlive_owner_ac_fixture.log`. It retains the existing semantic owner cases and adds real raw
registration/current-owner removal, canonical first-word checks, an actual CRT
validation throw while the section is locked, and raw-vector pop followed by the
real scalar overload with actual allocation free. Its vector pop is a fixture
operation; full BD0400 XLive dispatch belongs to primary integration. The existing
raw BD0400 drains the remaining null holes. The fixture records three actual
scalar frees, one CRT invalid callback, and a retained borrowed achievement batch.
No live SDK or application startup claim is made by this packet.

## Correction from docs/NATIVE_SINGLETON_INPUT_ONLINE.md

The shared raw manager now admits the documented online/input profiles. Primary
archive-only fixtures exercised actual BD0400 drain, including nonempty backend
and action storage. See that document and reports/native_singleton_input_online.json
for the executed profiles, artifact hashes and provider/application boundaries.
Original packet validation above describes its earlier standalone state.
