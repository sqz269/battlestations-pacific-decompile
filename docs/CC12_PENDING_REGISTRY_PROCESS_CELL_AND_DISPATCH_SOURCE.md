# Pending registry process cell and Source dispatch (CC12)

This packet adds one canonical Source `F878CC` publication cell and connects
the existing raw singleton manager's finite `D0DEA0` route to the admitted
`retire_native_pending_registry_scalar_00875850` helper. It preserves the passed
owner and lends the low byte of the actual Source flags argument for the
helper's late read. No registry owner is constructed, published or registered
by this change. Primary integration and independent final admission remain
separate from this worker's build and artifact evidence.

## Retained publication and host binding

`GameNativeStringProcess` now appends
`void* volatile pending_registry_00f878cc_{}` after its established fields,
including both empty-string authorities. The default member initializer makes
the new cell null. Its accessor is:

```cpp
void* volatile& GameNativeStringProcess::pending_registry_00f878cc() noexcept;
```

The accessor returns a reference to that member. It performs no owner lookup,
allocation or registration. The existing `game_native_string_process()` factory
still retains one `new GameNativeStringProcess` behind its function-local static
pointer and installs no process-object destructor. Thus this cell shares the
canonical `01090AA0` cell's lifetime through application destruction and late
CRT callbacks. This establishes a Source storage authority; it does not claim
that the member occupies absolute native address `00F878CC` in the host process.

`GameSingletonHost` appends a borrowed reference to this same process cell and
exposes it as `pending_registry_publication_00f878cc() noexcept`. Its constructor
obtains the reference from `game_native_string_process().pending_registry_00f878cc()`
and installs its address in the actual deletion bindings. The reference creates
no shadow publication. The existing normal/fallback shutdown still uses the
host's retained bindings throughout the raw manager drain.

The existing process manager reference remains the canonical `01090AA0` one.
The process cell's retained lifetime does not extend unrelated host-owned
contexts. All existing binding-lifetime preconditions still apply. Construction
of the host must complete before a caller uses its new accessor or dispatch
binding; this packet adds no startup activation.

## Appended binding and actual dispatch

`NativeSingletonDeletionBindings` appends:

```cpp
void* volatile* actual_pending_registry_publication_00f878cc{};
```

Every previous field retains its offset. The new field is at **184 / B8h**;
the structure grows from **184 to 188 bytes**. Existing field assertions remain,
and new offset/size assertions cover the appended field. A separately constructed
binding record defaults the new pointer to null, preserving the existing
missing-binding error path until the caller supplies its actual cell.

The internal `delete_current_profile` function receives the popped owner,
stable binding record, current numeric profile and actual Source flags word.
Its new `case 0x00d0dea0` calls:

```cpp
retire_native_pending_registry_scalar_00875850(
    owner, *reinterpret_cast<const volatile std::uint8_t*>(&flags),
    *bindings.actual_pending_registry_publication_00f878cc);
```

The reference aliases the low byte of the actual `std::uint32_t flags`
parameter for the entire synchronous scalar call. It does not copy a byte,
substitute a constant, or evaluate the deletion decision before section
release. The function passes its owner argument unchanged, regardless of what
the publication currently contains. It passes the bound cell itself, with no
current-publication receiver lookup or identity check.

If the binding pointer is null, the case falls through to the existing
`std::logic_error` path for unsupported profiles or missing bindings. No new
exception type, catch, RAII cleanup, fallback owner or `noexcept` policy is
introduced. Existing manager behavior remains: pop before dispatch, pass flags
1, and perform its established C++ vector-storage cleanup/rethrow on failure.

`D0DEA0` continues to be a numeric native profile identity. The actual Source
operation is an explicit finite switch calling the concrete reconstructed
helper. This change supplies no callable native/Source method table and no
general virtual dispatch adapter.

## Emitted code and concrete providers

The worker report retains the whole affected COFF objects and physical
symbol-index relocation graphs under an ignored evidence directory. It records
the complete dispatcher extent and classifies its executable prefix, alignment
padding, relocated jump tables and byte selector maps separately. Embedded table
bytes are not counted as executed instructions.

In the reviewed dispatcher, `D0DEA0` compares equal at `+342h` and branches to
`+3E9h`. That path loads the binding pointer from `[EBX+B8h]`, keeps the existing
null-binding branch, then pushes the actual cell address. **`LEA EAX,[EBP+0Ch]`**
computes the actual flags argument address; the function pushes that address
and the unchanged captured owner in EDI before its concrete REL32 scalar call.
Its ordinary continuation uses the existing `ADD ESP,0Ch` and `RET 8` path.
The fastcall dispatcher frame places profile at `[EBP+8]` and flags at
`[EBP+0Ch]`; no intermediate copied flags byte is present.

The admitted scalar reads that byte only after its real section-release call,
captures the bit before base cleanup, conditionally frees the captured receiver,
and returns its address without a later dereference. The artifact review resolves
the scalar and its actual section/base/generic/free providers through physical
COFF symbol records and positive core archive definitions. No numeric native
address is used as a linker substitute.

Current build ownership is explicit: `native_singleton_destruction.cpp` and its
scalar/provider dependencies belong to `bsp_core`. Existing CMake routes
`game_native_string_process.cpp` and `game_hosts_singletons.cpp` directly to
`bsp_game`. Their whole object and application-link evidence is recorded in its
actual location; no core membership is claimed for those application objects.

## Scope and validation boundary

The worker's normal Win32 Release build passed `reconstructed_math` and
`tool_tests`. This fresh worktree had no ignored `local/seed_reference.hpp`, so
`native_math_differential` was not configured; no new native query was made to
create it. The primary's merged build remains responsible for its configured
checks. No test case was added and no new lifecycle execution is claimed.

The process accessor is four bytes / two instructions and returns member +30h.
The entire 216-byte factory clears that DWORD before publishing its retained
process pointer. The 710-byte host constructor stores the returned cell address
both in its appended reference and at binding base +B8h. The full 202-byte host
shutdown passes that actual binding record into the existing manager drain.

The [worker report](../reports/cc12_pending_registry_process_cell_and_dispatch_source.json)
records the normal MSVC Win32 build/check result, exact source and artifact pins,
complete affected object graphs, selected whole-function bodies and concrete
call/binding evidence. The primary integrator must independently review the
merged build and final archive/application placement before final admission.

This is finite Source composition with **zero new Original-function credit**.
It closes the missing canonical cell and explicit Source deletion-route gaps
identified in the prior owner-binding audit. It does not implement the pending
registry constructor/getter, invoke registration or bootstrap, or establish that
an owner was created or retired at runtime. The primary's separate constructor
packet is outside these edits.

The existing scalar's explicit receiver/reference C++ interface remains distinct
from the original native ECX/by-value flags/RET4 entry. Its extra Source helper
calls and current CRT/Win32 providers retain their prior qualification. Original
FH3/SEH, mutable frame aliases, hardware-fault behavior, arbitrary concurrency,
binary compatibility and game behavior are not established by this packet.
No original executable code is run or changed, and no Ghidra, CMake or ledger
mutation, new test or ad hoc probe is added.
