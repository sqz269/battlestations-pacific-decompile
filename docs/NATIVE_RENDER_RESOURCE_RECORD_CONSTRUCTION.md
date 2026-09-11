# Actual renderer resource-record copy construction

This packet reconstructs the full sentinel, alias-list and `2Ch` resource-record
copy constructors, plus the list destructor required by the constructor's catch.
All operations use the existing actual node/record layouts, real allocations,
and concrete alias-range/string helpers. They do not substitute a host vector,
shadow header, resource lifetime policy, or whole renderer container.

| Body | Complete native extent, end exclusive | Original ABI | C++ entry |
| --- | --- | --- | --- |
| Sentinel allocation | `004C3020..004C303A`, 26 bytes | No register inputs; EAX sentinel; RET | `allocate_native_render_alias_sentinel_004c3020()` |
| List copy construction | `004D48A0..004D490C`, 108-byte main; catch `004D490C..004D491D`, 17 bytes | ECX destination owner, stack source owner; EAX destination; RET4 | `copy_construct_native_render_alias_list_004d48a0(...)` |
| List destruction | `004D0A10..004D0A2D`, 29 bytes | ECX actual owner; RET; no semantic result | `destroy_native_render_alias_list_004d0a10(...)` |
| Record copy construction | `00B2FC60..00B2FD02`, 162 bytes | ECX destination record, stack source record; EAX destination; RET4 | `copy_construct_native_render_resource_record_00b2fc60(...)` |

The list owner remains the actual three-word prefix at record `+08`: preserved
word `+00`, sentinel pointer `+04`, count `+08`. The sentinel/node is the existing
`10h` `NativeRenderResourceAliasNode`; the record remains the existing `2Ch`
`NativeRenderResourceRecord`. List APIs take raw actual owners, and record copy
takes references to that existing storage. `SizedStoragePool` and the existing
invalid-parameter callbacks preserve the established allocation/exception domains.
These source interfaces are not drop-in original ABI replacements. Names are
descriptive hypotheses, not recovered original symbols.

## Concrete dependency closure

The main Ghidra listing of `004D48A0` omitted its catch. The full native FuncInfo
points to `004D490C`, which calls previously unreconstructed `004D0A10` and then
the native rethrow service. The primary agent extended this packet's ownership
to those two addresses after reviewing that concrete frontier. The cleanup
owner composes existing full `004D05E0` clear and the existing lifetime free
service; there is no unresolved operation replaced by a callback or stub.

Current assembly also confirms the sentinel's direct call is `00BF681B`, the
existing malloc/new-handler/retry allocator service. `00BF55BE` is its jump
thunk, used at other allocation sites. Sentinel and ordinary alias nodes use
the same host lifetime allocation/free domain. Pooled string storage remains
the separate actual `SizedStoragePool` domain.

## Native ordering retained

`004C3020` allocates exactly `10h`. It writes next=self only if the allocation
pointer is nonzero. It separately computes wrapped `pointer+4` and tests that
address before writing previous=self. The second check is not changed into
another allocation-pointer test. The string length/data words at `+08/+0C`
are left uninitialized. The existing allocator service throws on exhaustion;
the odd null/wrapped-address branches are retained without claiming they are
usable successful allocations. Default placement construction does not value
initialize the node or write those untouched words.

`004D48A0` first obtains the new sentinel. It then stores that pointer at the
actual destination `+04` and zeroes destination count `+08`. Only afterward does
it read the current source sentinel and its first node (`004D48DC/DF`). The
insertion position comes from the captured allocation's next pointer at
`004D48E1`, rather than a later destination-sentinel reload. It passes actual
destination/source owner pairs to the existing full `004D26A0` range insertion.
The native seventh stack word repeats the source owner and is unread by the
range body; the C++ interface does not invent a semantic argument for it.

There is no list identity branch. If source and destination are the same owner,
source reads see the newly published empty list, leaving the old graph abandoned.
The preserved `+00` owner word is never initialized or copied. Construction does
not free any previous sentinel, aliases, or storage in that owner.

The catch calls actual `004D0A10`, which clears aliases, reloads the current
sentinel after clear callbacks, frees it, then nulls the actual owner `+04`.
The stores after `_free` are present in the installed bytes despite the saved
Ghidra listing stopping at that call. Callback changes to count and the preserved
owner word survive the final sentinel-null store. The destructor does not free
the owner itself, its enclosing name, or any resource pointer. It adds no new
rollback around the existing range insertion's own rollback behavior.

`00B2FC60` captures record identity, then clears destination name length/data
before taking its identity branch. Distinct records resize the actual name with
the then-current source length, using preserve=1. After that allocation boundary
the source length guard is reread. The copy captures current destination length,
current destination data, and current source data in that order
(`00B2FCA2/A4/A8`); the destination length is the copy count.

Only completed initial name construction arms the name cleanup. The record then
constructs aliases on the two actual record `+08` owners. After list success it
performs five sequential current source DWORD loads/stores at `+14..+24`, then
copies current `resource_28`. It does not copy `unknown_08` or retain/release the
resource. Identity still executes list construction and all six tail stores,
while the cleared name and old alias graph remain abandoned. Allocation callbacks
can change later source fields, and the new body does not snapshot the tail early.

## Exception ownership

| Owner | Complete metadata | Behavior |
| --- | --- | --- |
| Alias list | Handler `00C66030`; 36-byte FuncInfo `00D8EE1C`; 16-byte unwind map `00D8EDF8`; 20-byte try map `00D8EE08`; 16-byte catch descriptor `00D8EDE8` | Two states with null unwind actions; try state 0, catch state 1; catch-all at `004D490C` destroys current list then rethrows |
| Record | Handler `00CBD908`; 36-byte FuncInfo `00DF6208`; 8-byte unwind map `00DF6200`; 8-byte action `00CBD900` | State `0 -> -1` calls `0041DD20` on the saved actual record/name pointer at frame `-10h` |

Both FuncInfo records carry magic `19930522`, a null exception-specification
list and EHFlags `1`. List allocation/publication/source-link capture precede
arming state zero at `004D48EB`. Sentinel allocation failure therefore does not
invoke list cleanup. The list catch calls `004D0A10` at `004D490F`, then
`00BF6885(nullptr, nullptr)` at `004D4918`; no success continuation or rollback is
invented after rethrow. An exception from cleanup can replace the initial one in
the native control flow. Existing host pool/lifetime releases are `noexcept`,
which remains the previously documented narrower service boundary.

The record is state `-1` during its initial name allocation/copy. Failure there
leaves partial callback-mutated name fields untouched, without a synthetic free.
State zero begins at `00B2FCBC` before the alias constructor call. A sentinel
allocation or alias-copy failure then releases the current name pointer and
current length plus one. That cleanup leaves the name fields unchanged. A list
copy failure first runs the list's current-storage cleanup; only then does the
record name unwind execute. Payload/resource tail stores have not yet occurred.

## Verification and limits

All 23 selected initialized spans matched fresh guarded live Ghidra reads and
the installed PE, including complete owner/catch/free tails and full exception
records. Every query used the repository CLI to verify
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, Win32 x86 and image base.
Old stored names, prototypes and comments are retained in the audit. No Ghidra
mutation or function export was made by this worker; the primary integrator owns
flow repair, annotations and ledger updates. Selected-span comparison does not
establish whole-image or running-process identity.

One private focused original-caller fixture passed seven variants, comparing
720 normalized words across 40 events. All four complete owner bodies execute
original machine code, including the catch at `004D490C` and the destructor's
post-free stores. Ten checked pointer relocations preserve their EH records;
the two owner handler immediates use in-module bridges to actual host
`__CxxFrameHandler3`. The original catch calls actual host `_CxxThrowException`
through a verified entry hook. It executed one native list catch/rethrow and two
native record-name unwind actions. Four original list calls composed the full
actual C++ range helper, and three original memcpy calls used host memcpy.

The fixture exercises sentinel-only initialization, allocation-mutated source
name and source-sentinel capture, current tail reads after alias allocation,
initial name failure, a partially inserted node whose checked advance throws,
current-name mutation during list cleanup, and self-construction. Its direct
list-destruction case lets a clear callback publish a replacement sentinel, then
lets that sentinel's free callback change count to 17: the current sentinel is
freed and nulled, while count 17 remains. Another case makes sentinel allocation
throw after successful name construction, verifying name-only cleanup before
any list publication. Static tail-store inspection supplements the self-case;
byte-identical final fields alone cannot prove redundant identity stores.

Actual existing lifetime allocation/free and sized-pool bodies back the fixture.
Thin private wrappers record allocations, fill newly returned raw bytes with a
known pattern, and apply the stated mutations/failures; they do not emulate the
constructors, clear, node allocation, checked iteration, count growth or range
insertion. Successful allocations use real malloc/free. Deliberately abandoned
buffers from native construction behavior are cleaned outside the observation
window. This is original-owner composition evidence, with host CRT, pool and
exception boundaries; it is not native CRT/pool binary execution, exhaustive
failure/SEH equivalence, a binary-compatible replacement, or game validation.

Strict MSVC Win32 `/std:c++17 /O2 /Oy- /EHsc /fp:strict /MD /Gy /Gw /W4 /WX`
compilation passed. `scripts/build.ps1` passed with this source privately
registered, all eight native seed comparisons matched, and both existing CTests
passed. No tracked tests or shared CMake registry were changed. The existing
zero-byte memcpy omission and host allocation/exception domains are explicit.
Container reserve/resize/default construction remains separate work.

The audit is `reports/native_render_resource_record_construction_audit.json`;
it contains raw bytes, hashes, ABI contracts, current dependency source pins,
old annotations, focused-fixture reproduction paths and validation results.
