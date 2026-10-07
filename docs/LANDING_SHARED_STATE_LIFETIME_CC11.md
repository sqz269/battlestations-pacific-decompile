# Landing shared-state lifetime — CC11

The seven complete ordinary bodies are reconstructed through new Source APIs
in `native_land_state_lifetime.hpp/.cpp`. MoveTo cleanup now calls the complete
shared-state/vector closure; there is no zero-only resize or opaque growth
provider. These are conditional borrowed-storage providers, not an executable
class profile, constructor/arena adapter, native ABI replacement or game binding.

## Native coverage and flow limit

All end addresses below are **exclusive**. Complete disk/live bytes match,
754 bytes and 249 raw instructions in total, with thirteen direct calls and
two original vector virtual-call sites. The report records exact body hashes.

| Entry | Exclusive end | Bytes / disk instructions | Original interface |
| --- | --- | --- | --- |
| `007B3FC0` | `007B4024` | 100 / 31 | ECX destination, stacked source; EAX destination; RET4 |
| `007B4400` | `007B44F1` | 241 / 83 | ECX vector, stacked signed capacity; RET4 |
| `007B4500` | `007B4585` | 133 / 51 | ECX vector, stacked signed count; RET4 |
| `007B45F0` | `007B4644` | 84 / 26 | ECX state; RET, native SEH frame |
| `007B65E0` | `007B662E` | 78 / 23 | ECX nonnull MoveTo; RET, native SEH frame |
| `0064A610` | `0064A668` | 88 / 24 | ECX common prefix; RET, native SEH frame |
| `0064B5F0` | `0064B60E` | 30 / 11 | ECX element, stacked DWORD flags; EAX original identity; RET4 |

Primary repaired the returning-free continuation `007B44D0..007B44DF` and
decoded `007B462A..007B4644`. Its supported tools do **not** extend the stored
`007B45F0` body, which still ends at inclusive `007B4629` with one reported call
gap. The complete 84-byte Source flow therefore rests on independently matched
bytes and known raw instruction boundaries, separately from truncated saved
function metadata. No worker mutation or body-extension attempt occurred. See
`reports/shared_state_vector_flow_recovery_cc11.json`.

## Actual storage and profile admission

The state vector at `state+0Ch` contains actual `{data, signed count, signed
capacity}` fields and **24-byte** elements. Each element contains the actual
observer prefix at zero, byte `10h`, untouched bytes `11h..13h`, and FIRST-endpoint
identity `14h`. No 90h `NativeShipAiObstacleNodeStorage` is cast or reused. The
new views borrow the actual state profile/vector and MoveTo callback at `18h`;
callers must supply pure aliases, not translated caches or sidecars.

The earlier proposal incorrectly described CF5C20 slot zero as the ordinary
destructor. Exact disk/live tables and neighboring assembly correct that:

| Raw table | Actual slot zero | Protocol |
| --- | --- | --- |
| `00CF5C94` | `0064B5F0` | scalar wrapper, low-byte flag bit0, RET4 |
| `00CF5C20` | `0064A6A0` | separate scalar wrapper, low-byte flag bit0, RET4 |

Both wrappers call ordinary `0064A610`, which ends in RET. The neighboring
`0064A6A0..0064A6BE` body is 30 bytes/11 instructions, disk/live matched, and is
**read-only evidence**, not an eighth reconstruction. Primary corrected the
registered contract in `7b39bf48c`.

Only freshly read actual CF5C94 vector elements are admitted to scalar cleanup.
An unsupported profile raises an explicit Source admission error; there is no
CF5C20 substitution or default dispatcher. Raw CF5C20, CF5C94, D056D0 and state
profiles remain **uncallable** image identities. The packet supplies no complete
callable vtable or the unrelated callback/stream methods of those profiles.

## Complete ordering

The copy constructor clears destination `04/08/0C`, writes byte10 zero and the
CF5C20 stamp, then captures source14 once. It publishes destination14 and byte10
one before real `00694A60` registration. It retains destination padding and
does not copy source edge arrays, enabled state or any 90h tail. The growth
caller stamps each completed destination CF5C94 afterward.

Growth clamps requested capacity to at least one, compares signed capacity,
and uses native modulo32 `24*n` byte arithmetic. It copies/registers all new
elements **before** forward old-element scalar0 destruction. Fresh count and
old-array reads remain visible around the real calls. It frees the freshly
loaded old array, reloads the requested capacity, publishes new **data before
capacity**, and leaves count unchanged. There is no history-style saturation.

Resize includes all reserve, added-element and shrink branches. Added elements
clear exactly `04/08/0C/14`, set byte10 one, and stamp CF5C94; padding is retained.
Reverse shrink decrements the actual published count **before** reloading the
current index/data/profile and invoking scalar0 cleanup. It reloads count after
each cleanup and finally publishes the requested count.

Common element destruction stamps CF5C20, captures14 once, optionally invokes
real `006952A0`, then calls complete `00695870`. The final provider profile and
fields are retained; no first-cell clear, extra array reset or receiver free is
added. Field14 must already identify the actual FIRST endpoint; no whole-unit
pointer or assumed endpoint-offset translation is admitted.

Scalar cleanup captures the original identity, completes the ordinary helper,
then tests the low byte of flags for bit0 and optionally uses genuine CRT free.
The new C++ volatile flag observation/encoding is separately qualified from the
original TEST/RET4 ABI. Flags1 requires a **separate actual CRT element**, never
an interior vector slot or embedded callback; no post-free dereference occurs.

Shared-state destruction calls complete resize0, reloads/frees its array, then
stamps D056D0. It does not reset data/capacity. MoveTo destroys its actual
callback18 through `00695870` first, then calls that complete shared body.
Published backing pointers can dangle after cleanup and cannot be reused.

## Genuine services, callers and limits

The packet reuses complete Source observer registration, unregister and
callback-owner destruction with **mandatory** caller-supplied
`NativeObserverLifetime`. Its actual manager publication, current recursive
lock, pending dispatch, other-edge services and endpoint lifetimes must be
valid. Canonical CF7E64 registration/deletion is complete; other edge profiles
still need their actual deletion service. Existing canonical Source allocation
and free supply the BF55BE/BF6989/BF65AC boundaries, without a new arena.

Actual `009B2C80` calls shared cleanup for state offsets `254/228/200/1E0/1C0/1A0`
at `2CAE/2CBE/2CCE/2CDE/2CEE/2CFE`, Follow base `108` at `2D21`, and MoveTo `CC`
at `2D31`. Native `009C2980`/`009C2AC0` prefixes initialize the corresponding
vector and callback fields. Those bodies provide reachability/storage evidence;
the complete approach destructor, constructors, task arena, queue, death and
owner lifetimes are not newly admitted here.

Admit coherent live storage, nonnegative count/capacity/resize count with
count<=capacity, valid exact24B backing, disjoint growth/source/destination,
representable nonwrapping **total** byte/range arithmetic, ordinary successful
same-CRT allocation/free, and stable actual endpoints. Overflow, null allocation,
invalid placement, faults/private EH, structural reentry, concurrency and
aliased header mutation remain excluded. Fresh native observations are retained
despite that qualification. The Source fixture does not establish native reentry.

## Focused verification

Two fresh strict MSVC Win32 `/std:c++17 /EHsc /MD /O2 /Gy /DNDEBUG /W4 /WX` TUs
and one embedded-manifest probe passed **126 assertions**. One connected
actual-shaped case grows two elements to three, observes all new registrations
before forward old detach/free, preserves actual allocation padding, retires
two tails in reverse order after count publication, then destroys MoveTo
callback18 before the remaining vector element. Genuine pending suppression,
provider-final fields and captured recursive OS lock depth are checked.

Five full original direct-call bodies (copy/common/scalar/shared/MoveTo,
380 bytes) execute using **only nine natural CALL operand relocations** to
complete genuine Source observer/CRT/helper bridges. The original indirect
growth/resize bodies (374 bytes) are **not executed**: they have complete
byte/call evidence and connected Source branch checks, without a fabricated
callable image table. The original SEH frames run only normal successful paths;
Source bridges/this executable's CRT are not historical loader, private-EH,
native ABI or gameplay equivalence.

Ignored instrumentation temporarily traces this executable's own malloc/free
imports, always forwards the captured genuine operation, snapshots allocated
padding without modifying it, and inspects live fields only before real free.
It restores both imports and page protections. It is neither a production
provider nor behavior-changing reentry. No freed element/array is dereferenced.
The actual runtime manager lookup is unexercised under the fixture's prepublished
Source lock. No full constructor or state-entry invocation is claimed.

Source COFF confirms copy publication before registration, reserve free before
data/capacity publication, reverse decrement before providers, and shared free
before D056D0 stamp. Three support libraries were frozen from the current
`8ea38c617` build before root's later build; the report is the complete pre/post
input/support/artifact manifest. Probe source/executable are the ignored
`local/cc11_land_shared_state_lifetime_probe.cpp/.exe`. Root owns CMake source
registration, full main build, annotations, ledger additions and integration.

## Primary integration

Main `d670125fc8e3c969d6c111168f37fd8f645e5468` passed the full Win32 build and all three existing CTests. Root compiled3fresh TUs including the actual observer provider. All25current Source/provider/header/fixture inputs (19compiler includes),3current libraries and original PE stayed stable. The manifested independent fixture reproduced126checks. All754native bytes match disk/live and five380B original body literals match; the374B indirect callers remain Source/byte evidence. All21direct rows passed and two virtual sites remain excluded. Emitted free/requested/data/capacity and shared free/profile ordering were inspected. Existing90h ledger fields are retained, with a nested24B provider record. The PE32 asInvoker manifest was verified. All runtime/ABI/game qualifications above remain. No tracked tests were added.

Current Ghidra membership review (2026-10-07T21:19:00.969328+00:00): supported locked function recreation now stores the complete `007b45f0..007b4643` inclusive 84-byte original body, including the previously excluded decoded tail. Exact live/installed bytes and all26 decoded instructions agree. Existing descriptive name, undefined prototype and plate comments were recorded and preserved; definition receipt is `reports/cc11_shared_state_destructor_definition.json`. Earlier truncated-boundary/flow reports and fixture receipts remain historical. This changes stored membership only: Source remains unchanged and its prior successful fixture was not replayed. Additional CFG, private EH, native ABI, reentry and game validation remain unclaimed.
