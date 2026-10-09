# Pending registry owner binding readiness (CC12)

At Main `2fa724372622e47bb1b10a9feb5c25492e5402a1`, the actual section creator,
raw manager lookup/registration, guard cleanup, registry base cleanup and scalar
retirement services are available. **Production owner composition is still
held:** the surveyed relevant Source modules have no canonical `F878CC` cell,
`874BC0` constructor or `875280` getter, and the actual raw manager deletion
switch has no `D0DEA0` binding. Registering a new numeric-profile owner before
that binding exists would leave an unsupported object for manager drain.

This is a Source readiness audit. It reuses the accepted **69-byte constructor,
189-byte getter and independent EH reports**, plus the subsequently admitted
base/scalar helpers. No fresh native body, handler, table, caller or bootstrap
query is made. The earlier constructor/EH reports' historical “helper absent”
statements are superseded by the current primary Source admissions; their
encoded state/action evidence is unchanged.

## Concrete current Source interfaces

All declarations below are in namespace `bsp`, except the named game authority.

| Operation | Exact declaration / Source boundary |
| --- | --- |
| Actual section creation | `TrackedCriticalSection* create_native_tracked_critical_section_00bd1860();` Its definition is naked, takes no input and returns the raw section. No `noexcept`. |
| Raw allocation | `void* singleton_lifetime_allocate(const SingletonAllocationRequest&);` Requests retain native and host byte sizes. No `noexcept`. |
| Raw free | `void singleton_lifetime_free(void*) noexcept;` Real paired host `std::free`. |
| Manager lookup | `void* get_native_singleton_manager_00415350(void* volatile& actual_manager_publication_01090aa0);` Ordinary C++ reference interface; no `noexcept`. |
| Raw manager construction | `void* __fastcall construct_native_singleton_manager_00bd0960(void* owner, void* unused_edx);` Raw14h receiver, no `noexcept`. |
| Registration | `void __fastcall register_native_singleton_object_00bd0c30(void* owner, void* unused_edx, void* object);` Naked definition; no `noexcept`. |
| Getter guard cleanup | `void destroy_native_singleton_guard_00411ee0(void* actual_8byte_guard);` Captures actual guard +4, stamps `CE37FC`, decrements once then Leaves; preserves +4. No `noexcept`. |
| Constructor/base cleanup | `void cleanup_native_pending_registry_base_008748f0(void* actual_receiver, void* volatile& actual_registry_publication_00f878cc);` Clear actual cell, then concrete base profile helper. No `noexcept`. |
| Generic base store used by cleanup | `void destroy_native_generic_singleton_base_00412430(void* actual_receiver) noexcept;` Volatile DWORD `CE3818` store. |
| Actual section retirement | `void __fastcall release_native_tracked_critical_section_0041cc80(TrackedCriticalSection** actual_owner_slot);` Naked definition; no `noexcept`. |
| Registry scalar retirement | `void* retire_native_pending_registry_scalar_00875850(void* actual_receiver, const volatile std::uint8_t& actual_deleting_flags_byte, void* volatile& actual_publication_00f878cc);` Ordinary C++ interface; no `noexcept`. |
| Manager drain | `void __fastcall destroy_native_singleton_manager_00bd0400(void* owner, const NativeSingletonDeletionBindings& bindings);` No `noexcept`; caller retains manager allocation/publication ownership. |

The actual section creator calls its concrete cdecl `allocate_worker_lock`
with **1Ch**, which forwards `{critical_section, 1Ch, 1Ch}` to the existing
allocator. On a nonnull result it invokes real `InitializeCriticalSection`,
then writes zero to physical depth +18h and returns the same allocation. Its
null branch returns null. No projected/new-delete section is substituted, and
no cleanup is added if OS initialization exits nonlocally.

The base cleanup's current primary review covers the whole 25-byte Source leaf
and actual 14-byte generic child. The scalar's newer primary review covers all
**62 bytes / 24 instructions**, with actual 64-byte section release, 25-byte
base cleanup, 14-byte generic child and 6-byte host-free provider. It verified
the late byte load after section release, retention across base cleanup,
conditional free, returned receiver and unique core definitions/members. The
normal MSVC Win32 build and three existing checks passed. These are inherited
admissions, not a new build or execution claim by this audit.

## Accepted native construction, publication and cleanup schedule

The ordinary `874BC0` constructor captures its ECX raw8 receiver, arms state 0,
stores `D0DEA0` at +0, then makes its sole ordinary call to `BD1860`. Only after
the section service returns does it store returned EAX at receiver +4. It
returns the original captured receiver in EAX. It never pre-clears +4 and makes
no generic-base constructor call or initial `CE3818` store. It does not publish
`F878CC` on its normal path.

The independent constructor EH report encodes state 0 -> -1 through `C963E0`,
which loads the **current** receiver spill at `[EBP-10h]` and tail-transfers to
`8748F0`. That target is now an admitted ordinary Source clear-cell/base-reset
helper. The encoded route does not prove the shared runtime's frame anchor,
execution, nested-failure behavior or native fault handling. The constructor
has no separately armed section-free action after the provider returns.

The getter's ordinary schedule must remain ordered as follows:

1. Capture `F878CC`; a nonnull initial read is returned directly. Native still
   installs/removes its EH frame on this fast path. No caller register/stack
   argument is consumed by the original getter.
2. Resolve the first manager through `415350`, then dereference its actual
   +10h section without a manager-null check. Capture that section in the raw8
   `CE37FC` guard. If nonnull, Enter it, then increment unsigned physical depth
   +18h modulo 2^32. State 0 is armed **after** this sequence, including the
   null-section path.
3. Recheck `F878CC`. If absent, allocate exactly raw8 bytes while state 0 is
   active. Save the allocation, then arm state 1. For a nonnull allocation,
   call `874BC0`; the explicit null branch produces zero. The accepted
   constructor proves a normal nonnull result is the captured allocation.
4. Restore state 0 **before** publishing constructor return EAX to `F878CC`.
   Resolve the manager again, **then** reread the current `F878CC` and register
   that pointer with the second manager. Neither value is replaced with the
   first manager or an earlier captured owner.
5. Release the first captured manager section: decrement its unsigned depth
   once, then Leave. State 0 stays armed through normal release. Reload
   `F878CC` after release for the slow return.

The getter EH map is independently established: state 1 frees the **current
allocation spill**, then encoded state 0 invokes cleanup of the **current raw
guard**. Allocation failure occurs before state 1. Constructor failure has
the qualified inner clear-cell/base-reset route, followed by allocation free
and guard cleanup if the relevant actions are invoked and return normally.
After publication, the second manager lookup and registration run with only
guard state 0: no owner-free or publication-rollback action is armed there.
Do not restore an old publication or assume registration left the vector
unchanged after a provider failure. These reports establish encoded actions,
not original FH3/SEH compatibility or a general Source exception policy.

The registry owner's own +4 section is distinct from the manager +10h guard.
The admitted scalar captures the passed owner, restamps `D0DEA0`, retires its
actual +4 section, reads the borrowed flags byte after release, clears the
current actual publication then stamps `CE3818`, optionally frees the captured
owner for bit 0, and returns its address value. It never selects the owner
from the current publication.

## Canonical manager and actual Source binding mechanism

`game::GameNativeStringProcess` owns `void* volatile manager_{}` and exposes it
through `void* volatile& manager_01090aa0() noexcept`. The factory
`GameNativeStringProcess& game_native_string_process()` uses an intentionally
retained `new GameNativeStringProcess` behind a function-local static pointer.
The factory has no `noexcept` and can allocate on first access. The cell begins
null and survives application destruction and later CRT callbacks. This is the
accepted canonical **Source** authority, not an assertion that the host cell
occupies native absolute address `01090AA0`.

`GameSingletonHost` binds its manager reference to that same process cell in
its constructor. `get_native_singleton_manager_00415350` accepts the reference,
captures its first read and returns that capture when nonnull. Otherwise it
allocates raw14h, constructs the actual raw manager, publishes the result and
returns that captured result without a final reload or lookup lock. Its C++
construction-failure path frees the captured allocation and rethrows after the
constructor's own storage cleanup. The manager constructor zeros +4/+8/+0C,
reserves 256 pointer slots, obtains the real tracked section and publishes +10h.

`GameSingletonHost` owns `NativeSingletonDeletionBindings` and exposes it by
reference. Its normal/fallback `shutdown()` captures the current manager,
drains it using those bindings, retires dependent host services, frees that
captured manager and finally clears the canonical manager cell. The process
cell's permanent lifetime does not extend the lifetimes of host-owned bindings
or a future registry cell automatically. Those must survive every applicable
drain and any supported later owner recreation.

Registration validates raw begin/end bounds before checking null, then appends
the actual nonnull object. It performs no profile validation, lock, retention
or duplicate suppression. During drain the current last object is captured,
its slot is popped, and its **current** profile DWORD is loaded. Flags 1 and
the same stable bindings are passed to the internal ordinary Source function:

```cpp
void __fastcall delete_current_profile(
    void* owner, const NativeSingletonDeletionBindings& bindings,
    std::uint32_t profile, std::uint32_t flags);
```

That function switches over recovered numeric profiles and calls concrete
Source destructors. It does not call a native numeric profile as a Source
vtable. Unsupported profiles or missing required bindings throw
`std::logic_error("raw singleton deletion requires a recovered profile and its actual bindings")`.
The manager's C++ catch clears vector storage and rethrows. That is not a
registry scalar fallback and does not prove cleanup of the popped unsupported
owner or successful completion of the host shutdown.

There is currently no `D0DEA0` case, pending-registry scalar include/call, or
`F878CC` member in that deletion binding/header and host/process authority
scope. The existing `CE3818` case supplies only generic base deletion; changing
a live registry to that profile to obtain dispatch would lose its registry
section/publication retirement contract.

## Actual Source exception boundaries

The malloc allocator retries through `_callnewh` and throws `std::bad_alloc`
when the handler declines. Installed handler behavior is an additional actual
provider boundary; the caller cannot assume only one exception type. Neither
the allocator nor the naked section creator declares `noexcept`. OS section
initialization has no added compensation or SEH-to-C++ translation.

Registration growth has the concrete `NativeSingletonVectorLengthError`
transport for `BD0590`: a 28h owning legacy payload with a new Source C++ catch
type and RTTI. It is **not `std::length_error`**. Pointer-allocation overflow
also throws `std::bad_alloc`. The wrappers use actual `_invalid_parameter_noinfo`
and its current Source CRT handler domain; returning handlers are allowed, and
the original encoded handler/Watson and throw identities are not reproduced.

The current manager getter/constructor and manager drain contain C++
catch/cleanup/rethrow scopes. Similar scopes in another registry getter are
existing examples, not authority to copy them into this registry without the
accepted state boundaries and actual bindings. This audit adds no catch, RAII,
handler, `noexcept`, default cell or profile adapter. The available free/generic
base helper declarations are `noexcept`; the guard, base cleanup, scalar and
section release declarations are not. A potentially throwing declaration alone
does not establish that an operation actually throws or how native faults unwind.

## Minimal coherent next contracts and gates

A bounded ordinary constructor candidate can use an explicit actual raw8
receiver and borrowed actual `F878CC` reference for its Source failure cleanup.
Its normal path needs only the `D0DEA0` stamp, concrete `BD1860` call, delayed
+4 store and original receiver result. There is **no generic-base constructor
prerequisite**. A Source C++ failure policy must be stated and reviewed against
the accepted inner cleanup route using the already admitted base helper; it
cannot claim native frame-spill aliasing, FH3, SEH or nested-failure equivalence.

A subsequent ordinary getter contract needs two distinct, stable references:
the same canonical `01090AA0` manager cell and genuine `F878CC` registry cell.
It must retain the first actual manager section through release, allocate raw8
through the paired allocator, preserve the constructor/guard cleanup boundaries,
publish before the second lookup, register the current publication afterward,
and reload the publication after release. It adds an explicit Source reference
interface to the original no-input entry; no alternate manager or callback
provider is needed.

Before any production getter can register this owner, two concrete composition
gates remain:

1. Establish one canonical Source `F878CC` producer whose cell and associated
   bindings outlive all registry borrowers, manager drains and any admitted
   later recreation. Route constructor failure, getter publication and scalar
   retirement through that same cell. Existing `F899E4/F899FC/F899E8`, `F878FC`,
   `F8D41C` or observer cells are different authorities and cannot supply it.
2. Admit a concrete `D0DEA0` case in the actual Source deletion switch with that
   same retained cell binding. Its adapter must lend the low byte of the actual
   Source flags argument to the admitted scalar for the duration of the call,
   preserving its late read. A copied/default flags byte or current-publication
   receiver substitution does not meet that contract. This finite Source route
   still does not produce an arbitrary callable native method table.

The Source constructor can be reviewed independently as an explicit-binding
candidate. Production getter registration must wait for both composition gates;
the existing registration wrapper will not reject an unsupported profile early.
No further native body sweep is needed to identify these current Source gaps.

## Verification

The [report](../reports/cc12_pending_registry_owner_binding_readiness.json)
records exact scoped search commands/results, indexed metadata, provider and
accepted-report pins, current primary admissions and an actionable dependency
matrix. Searches cover the relevant pending/singleton/game/world modules and
the concrete deletion/authority files; their negative results are scoped, not
claims that an unrelated differently named implementation cannot exist.

Only this document and report change. JSON, current source/admission hashes and
diff checks pass. No live native queries, C++/CMake/ledger/Ghidra changes, build,
tests, probes, new reconstruction credit, ABI or runtime/game proof are added.
