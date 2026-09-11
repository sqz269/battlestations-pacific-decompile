# Retained backing and memory-stream ownership

Seven complete functions operate on actual intrusive owners, reference slots
and shared accounting DWORDs. Descriptive names describe recovered behavior;
they are not recovered C++ symbols. The implementation uses new MSVC Win32
interfaces and the existing CRT allocation domain.

| Entry and exclusive end | Original ABI | Behavior |
| --- | --- | --- |
| `008D43C0..008D4437` | ECX backing, stack signed request, EAX owner, RET4 | Construct 10h backing and account original request |
| `008D4440..008D446D` | ECX backing, base tail | Free data, update current counters, destroy base |
| `008D4470..008D44B2` | ECX backing, stack flags, EAX owner, RET4 | Full destruction then optional scalar free |
| `00BEF6D0..00BEF74C` | ECX backing, EAX stream, RET | Allocate and initialize 14h retaining stream |
| `00BEF9C0..00BEFA36` | ECX stream, RET | Release backing and destroy stream/reference bases |
| `00BB8F90..00BB8FAE` | ECX stream, stack flags, EAX owner, RET4 | Full destruction then optional scalar free |
| `00B23640..00B2367B` | ECX destination slot, EDX source slot, EAX destination, RET | Publish/retain replacement before old release |

These complete spans total 561 bytes. The backing contains profile `+00`,
intrusive count `+04`, data `+08` and stored length `+0C`. The stream contains
profile/count, backing `+08`, end `+0C` and cursor `+10`. Storage is borrowed;
there are no replacement objects, copied headers or shadow reference counts.

The context borrows actual non-atomic globals `0109DB98` (object count) and
`0109DB9C` (requested bytes), plus immutable views of the supported native
profiles. Backing `D15AD8` contains `BD30E0/8D4470`; stream `D642C0` contains
`BD30E0/BB8F90`. A zero-count call reads the current profile for slot 0, then
the complete `BD30E0` path rereads it for the flag-1 deleting slot. Original
address words are data, not callable host tables. Other current profiles are
outside this interface's domain.

## Backing construction and disposal

Construction writes `CEB130`, count 1, then arms its sole cleanup state and
publishes `D15AD8`. A positive signed request is stored and allocated exactly;
zero/negative requests store zero but still allocate one byte. After publishing
data, the routine increments the actual object counter and adds the original
request bits to the byte counter with DWORD wrap. A negative request therefore
creates an intentional accounting residual: destruction subtracts stored zero,
not the original negative request. This behavior is preserved.

Allocation uses the existing ordinary/array `SingletonLifetimeDomain` service.
Allocation failure restores only profile `CEB130`: count 1, stored length and
stale data remain, and neither global counter is updated. Handler `CA2EF8`,
FuncInfo `DD5090`, map `DD5088` and action `CA2EF0 -> BD30F0` establish this
cleanup-only scope. There is no catch map or synthetic catch/rethrow.

Destruction captures data before publishing `D15AD8`, frees that capture,
decrements the current object counter, rereads stored length after free and
subtracts it from the current byte counter, then writes `CEB130`. It leaves
count, data and length fields stale. Scalar deletion repeats the full behavior
and frees the original owner only for flag bit 0. Both original returning-free
tails were restored in Ghidra, with prior comments preserved and project saved.

## Stream and reference-slot ordering

`BEF6D0` allocates 14h bytes, writes base/count/profile and zeros all three
pointer fields. It still reloads old backing `+08`, then publishes its input
backing before the real `InterlockedIncrement`. It reads the backing data,
publishes cursor, reads the current backing length, then publishes wrapped end.
The original allocation-null branch proceeds to a null-owner load; the new
interface adds neither a safe-null return nor rollback. Its ordinary allocator
normally throws before this branch. The freshly zeroed old-backing release
branch is retained without inventing a reachable reentrant mutation.

Stream destruction writes `D642C0`, captures backing before arming state 0,
then atomically decrements it. Only a zero count invokes the current profile's
two-stage deleting dispatch. Successful return clears the current stream slot.
Both normal and unwind base cleanup write `D5C104` followed by `CEB130`, leaving
cursor/end untouched. Native handler `CC76B8`, FuncInfo `E022AC`, map `E022A4`
and action `CC76B0 -> BB86E0` contain no catch map. The guard is disarmed before
the normal base tail. Scalar deletion frees the owner only after full success.

`B23640` reads source first and old destination second. Pointer identity skips
all stores and atomics, including when the slots differ. Otherwise it publishes
the replacement, retains it if nonnull, then releases the captured previous
owner. A failing old terminal would leave the new pointer already published and
retained; the admitted nonthrowing CRT/free terminals do not manufacture such a
failure for testing. Actual atomics and current profile reads remain observable.

## Evidence and validation

[The audit](../reports/native_retained_memory_owners_audit.json) records complete
native bytes, source/library pins, the repaired function extents and focused
original-code comparison. Build and fixture results establish only the listed
storage/profile and service domains. Native ABI compatibility, original pool
construction, complete texture ownership and game execution are separate work.
No tracked tests are added.

The final private fixture verifies 22 original live-PE spans (761 bytes), all
34 permitted patches and postimages, and seven actual primary-library entry
providers. Eight paired cases match 3,123 DWORDs. The real malloc-null/new-handler
path raises `std::bad_alloc` and executes original constructor EH search and
unwind once each; the rebuilt base-only cleanup matches. Cascading assignment
executes the original current-profile invoker and both scalar deleters.

The stream release exception edge is captured but unexercised for valid acyclic
owners with the supported profiles: their terminals ultimately use nonthrowing
free. No throwing free, fabricated terminal, null-success allocator or mutation
of the freshly initialized old-backing field was injected. Boundary snapshots
also do not establish concurrent mutation or instruction-by-instruction write
observation. Both native and compiled owner FH3 maps have no catch entries.
