# Raw singleton publication getters

`src/native_singleton_publication.cpp` reconstructs the complete source behavior
of `00415350[105]` and `00B1B730[200]`. Both native functions consume no input,
return a pointer in EAX and use plain RET. The new MSVC Win32 interfaces borrow
stable references to the actual mutable publication cells; this is an explicit
new source ABI. They allocate the original raw sizes and call the completed raw
construction and vector registration providers.

## Publication and locking order

The manager getter captures `01090AA0` once. Its fast path returns that value.
The slow path allocates `14h`, calls `BD0960` if nonnull, publishes the returned
owner or null and returns that result without rereading the cell. It installs
no lock. Allocation precedes the constructor cleanup boundary.

The registry getter captures `F8D41C` for its fast return. Otherwise it calls
the manager getter and captures that manager's section at raw offset `10h`.
An eight-byte guard holds profile `CE37FC` and the captured section. A nonnull
section is entered and its physical DWORD at `18h` incremented before the
guard cleanup becomes active. The registry publication is then rechecked.

If still absent, the getter allocates `10h`, calls base constructor `B1AA70`
and writes `D5E59C` to the captured owner. It does not call the separate derived
constructor wrapper. After constructor cleanup ends, it publishes the owner,
calls the manager getter again, rereads the registry publication and passes
that current value to raw registration `BD0C30`. The first captured section
is retained even if these calls change the manager publication. It decrements
that section's physical counter before LeaveCriticalSection; the slow return
rereads the registry publication after Leave. Null sections skip both pairs of
operations. Native null-allocation branches are retained, although the actual
source allocator normally retries or throws.

## Exception evidence

Saved Ghidra bytes were freshly compared with the original disk binary for
both complete bodies, their cleanup actions/handlers and FH3 data: six spans,
451 bytes, including 305 owned function bytes and 146 EH bytes.

| Entry | Native FuncInfo / unwind map | Cleanup |
| --- | --- | --- |
| `00415350` | `D8418C` / `D84184` | State 0 to -1 calls `C5E0D0`, freeing allocation spill EBP-10 through `BF65AC`. |
| `00B1B730` | `DF4B38` / `DF4B28` | State 1 to 0 calls `CBC7B8`, freeing allocation spill EBP-18. State 0 to -1 calls `CBC7B0`, tailing `411EE0` with guard EBP-14. |

Source C++ catch/cleanup/rethrow expresses these ownership transitions. The
registry base constructor performs its own reset before the getter frees the
captured allocation. A subsequent lookup or registration exception retains
the published registry allocation and runs only the guard cleanup. Enter and
counter increment precede the guard try; normal decrement/Leave remain inside
it. No publication clearing, owner rollback or destructor dispatch is added.

## Validation and limits

The strict Win32 build, both existing CTests and eight native seed comparisons
passed. The main archive, six exact object members, 44 unchanged prebuild
inputs and 215 compiler read dependencies are captured. The compiled getter
bodies, cleanup blocks, FH3 try/unwind tables, SafeSEH entries and provider
relocations are reviewed separately from the native instruction bodies. See
`reports/native_singleton_publication_audit.json` for hashes and proof paths.
No new tests were added; these getters were not executed by this packet.

The source CRT heap/new-handler/exception domain and Win32 synchronization
services are concrete dependencies. Original native FH3/SEH stack identity,
aliases to mutable EH spills or private source stack bindings, hardware-fault
cleanup and provider register/throw identities are outside this source ABI.
The borrowed publication cells must be distinct and refer to actual raw owners.
A typed `SingletonLifetimeDomain` publication cannot be cast to this interface.

These entries create no private global or second lifetime domain. `D5E59C` is
an identity DWORD, not a rebuilt callable vtable. The executable continues to
use its documented typed application-owned lifetime domain. Migrating it to
the raw manager requires evidence-backed destructor dispatch for every admitted
owner; in particular, raw registry destruction needs its added string-pool and
publication bindings. That mixed-owner destruction contract, original callable
ABI compatibility and full gameplay validation remain open. Descriptive names
are reconstruction hypotheses.
