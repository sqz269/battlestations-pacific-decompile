# Native PakRegistry FileBlock callbacks BO

Addresses: `00BB5770`, `00BB5910`. This packet reconstructs both complete ordinary
callback bodies over actual Win32 storage. It does not reconstruct the native
FH3 exception mechanism or wire the callbacks into the game runtime.

| Routine | Coverage | Original ABI | Evidence |
|---|---|---|---|
| `enter_native_pak_registry_block_00bb5770` | complete ordinary body | ECX captured registry; one stacked native name header; `BB5908 RET 4`; no uniform semantic EAX result | `BB5770..BB590A`, 411 bytes, 133 instructions, no gaps |
| `leave_native_pak_registry_block_00bb5910` | complete ordinary body | ECX captured registry; one stacked native name header; `BB5AAE RET 4`; no uniform semantic EAX result | `BB5910..BB5AB0`, 417 bytes, 133 instructions, no gaps |

Live queries verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, base `00400000`. Both bodies and the exception/literal/profile
spans were compared with the installed PE. No Ghidra writes were made. The only
incoming references to these entry points are profile data at `D64198` and
`D6419C`: the PakRegistry primary profile `D64190` has these callbacks at +8/+C.
The source interfaces explicitly pass services and retained invocation storage;
they are not replacement native stack interfaces.

## Actual owner, dependencies and publication

The existing `BB4FB0` producer establishes the registry's actual 1Ch layout:
primary profile +0, reference count +4, secondary profile +8, ordinal +C and a raw
pointer vector at +10 (`base +0`, `count +4`, `capacity +8`). `BB46F0` reserves the
actual backing allocation. The vector stores raw provider values and does not
own a reference to each value. No projected container or extra provider reference
is introduced. Provider names are actual length/data headers at provider +8/+C.

The callback context borrows the actual mutable `0109CEEC` publication, the
existing resolver, mount and unmount contexts, and the actual `CE3A70` dot
literal. These contexts must share one string/pool and VFS publication domain.
The caller retains its registry and original name storage. Concrete dependencies
are `BB5670`, `BDF4C0`, `BDD850`, `BE1890`, `BE0750`, `BB46F0`, the original
string/pool operations and the four counter/active helpers. The report enumerates
every direct call with its original owner and the separate EH tail edges.

## Entry ordering

`BB5795` constructs the owned `.mpak` header using the previously reconstructed
helper's first-occurrence, case-sensitive suffix rule. `BB57A7` clears active on
the current published manager. `BB57B7` independently reloads that publication
and resolves the mutable `.mpak` header. A false result skips all device, mount,
vector and ordinal work and proceeds to final recomputation and name release.

A true result copies the current resolved header into a distinct device header.
The pre-resize source length is captured; after `41DD40`, the source length gate,
destination count, source data and destination data are read in the listed order.
The copy excludes the terminator. `BB580E` reloads the current manager and selects
a device using the same actual device header as both native stacked arguments.
The selector's `-1` result is not rejected. The registry ordinal is incremented
with DWORD wrapping, then captured as priority before constructing the dot header.

`BB5842` reloads the current manager and calls `BE1890` with `.mpak`, dot, captured
priority, flags zero and the captured device. The returned provider, including a
normal null result, is appended to the actual vector. Equality of count/capacity
triggers reserve with wrapped doubled capacity when its signed value exceeds one,
otherwise one. Count and base are reread after reserve. The slot write has the
original null-slot gate. `BB5873` captures dot data before the current vector count
is incremented at `BB5877`; cleanup size is read afterward.

Dot cleanup precedes `BB589E`, which reloads `0109CEEC`, increments that manager's
count and sets active. Device-name cleanup follows. `BB58CC` reloads the current
manager again and recomputes active using signed count > 0, then releases the
`.mpak` header. There is no origin-manager capture across any of these calls.

## Exit ordering

`BB5934` constructs `.mpak`; `BB594C` resolves it on the current manager and ignores
the returned AL. The raw vector scan uses signed count comparisons. It removes
every entry whose provider name has equal stored length and case-insensitive
contents. Empty equal-length names bypass data dereferences and `stricmp`.
`BB595E` caches `.mpak` length before the loop. `BB5997` reloads it after each
reached `stricmp`, including mismatch; `BB59CB` reloads it only after a reached
shift loop. The implementation preserves these cached versus current reads.

A match shifts following raw DWORD values left, rereading the current base and
count during the shift, then decrements current count without advancing the outer
index. A mismatch advances the index. No removed value is released, retained or
zeroed, and the now-unused final slot is not cleared. The same provider can appear
multiple times; its destruction belongs to the single subsequent name unmount.

The exit constructs dot using `41DD40(1,true)` and a current-length-plus-one copy
from `CE3A70`, reloads the current manager, and invokes `BE0750` exactly once.
It saves AL and releases dot before testing the saved result. Only a true result
causes `BB5A64` to reload the current manager and decrement its count, followed by
decrementing the captured registry's ordinal. `BB5A73` always reloads publication
for final signed-count active recomputation, then the `.mpak` header is released.

## Exception evidence and source retention boundary

The native registration records point to decoded handlers `CC43F8` and `CC4420`.
The full decoded-only intervals `CC43F8..CC4401` and `CC4420..CC4429` have no
Ghidra function membership. Each loads its FuncInfo and tail-jumps to `BF6B43`.
No function creation or guessed handler naming is included in this worker packet.

| Callback | FuncInfo / unwind map | Native actions, all tail to `41DD20` |
|---|---|---|
| entry | `DFDCF8`, magic `19930522`, maxState 3, map `DFDCE0`, no try blocks, EHFlags 1 | state 0 -> -1 at `CC43E0` (mpak EBP-1C); state 1 -> 0 at `CC43E8` (device EBP-24); state 2 -> 1 at `CC43F0` (dot EBP-14) |
| exit | `DFDD2C`, magic `19930522`, maxState 2, map `DFDD1C`, no try blocks, EHFlags 1 | state 0 -> -1 at `CC4410` (mpak EBP-1C); state 1 -> 0 at `CC4418` (dot EBP-14) |

`NativePakRegistryBlockInvocation` owns immovable mutable outer headers, the
nested resolver/unmount acquired frames, captured raw values and the pending
pool-return arguments. A caller publishes it before entering either callback.
If a C++ exception escapes, the wrapper records the active original call site,
marks the invocation failed and rethrows without outer cleanup or mutation
rollback. Destroying an active/failed invocation terminates; replay is rejected.
Retain the frame and all its borrowed contexts, registry and original input until
the caller establishes a separate recovery policy. Completed pool returns are
not undone: retaining a header address does not resurrect already-freed backing.
This policy deliberately does not claim native FH3 equivalence.

| Exact escaping outer call sites | Known source behavior and borrowing boundary |
|---|---|
| `BB5795`, `BB5934` -> `BB5670` | String allocation/copy can throw after touching the outer output. The helper also owns a stack suffix header; its existing catch destroys a completed suffix. The outer frame preserves the output header, not that internal suffix. `NativeStringStorage::release` is noexcept in this domain. |
| `BB57B7`, `BB594C` -> `BDF4C0` | Context/replay validation can throw before binding the outer name. Later provider and logging dispatch can throw after borrowing names/output headers. The nested acquired object preserves its header addresses, but existing catches destroy completed original/direct/candidate names and visitor backing. Retained addresses are not a guarantee of retained internal allocations. |
| `BB57D9`, `BB59F5` -> `41DD40`; `BB5824` -> `41E870` | The actual outer headers remain stable through allocation failure. These calls keep the existing concrete string allocation/release behavior; they do not provide a generalized native unwind contract. |
| `BB580E` -> `BDD850` | The dependency constructs a stack 14h visitor and 8h copied-name header, normalizes the copy and visits actual mounts. Allocation can fail before provider dispatch; provider dispatch can reject an unsupported entry or throw after receiving internal copied-name/output storage. Existing catch cleanup destroys armed name/visitor storage before rethrow. Retaining the outer device header does **not** retain these internal header addresses or released backing. |
| `BB5842` -> `BE1890` / `BDB040` | An unsupported current manager selector throws before factory dispatch or creation of internal string locals. The concrete selector passes the stable outer `.mpak`/dot headers directly to current factory dispatch. Unknown factory/failure dispatch can throw after borrowing those outer headers. No provider rollback is added. |
| `BB5842` -> `BE1890` / `BE1740` | After a provider result and its device write, registration constructs stack canonical/payload/record/copied-record/iterator storage. String/canonicalizer/pool and tree allocation calls can throw after receiving this internal storage. Existing completed-state cleanup runs; the outer invocation does **not** preserve these internal locals. Initial canonicalization precedes registration's catch. Insertion may already have mutated the actual tree. |
| `BB5860` -> `BB46F0` | Actual vector reserve can throw. Preserve the returned provider capture and the actual registry/vector's current state; no append, mount rollback or extra retain is synthesized. |
| `BB5A2E` -> `BE0750` | The nested unmount acquired frame retains its own canonical header, iterator and captured provider/owner/node fields on escaping calls. It retains its documented concrete dependency limits, including noexcept string-release boundaries and unsupported native exception behavior. Completed raw-vector erasure is never rolled back. |
| entry getters/returns `BB588C/BB5893`, `BB58BA/BB58C1`, `BB58EB/BB58F2`; exit `BB5A4D/BB5A54`, `BB5A94/BB5A9B` | Captured data and wrapped length+1 survive in the outer frame. Getter can throw before a pool is obtained; return can throw after receiving captured pool/data/size. No retry or guessed release follows. A completed prior return remains completed. |

`stricmp` and the overlap-safe copy use the existing CRT domain. Raw pointer
faults, process corruption, concurrent data races, original stack/register-spill
aliasing and arbitrary SEH behavior are not caught or reconstructed by `/EHsc`.
Known source exceptions above are not evidence that original native callbacks
throw the same types or have the same nested lifetime behavior.

Production integration must provide the actual PakRegistry observer route and
publish/retain invocation and context lifetime before dispatch. This packet does
not supply that route. Retaining every unknown nested dependency failure would
also require separate dependency work on BDD850/BE1740 and a review of resolver
cleanup; this is not advertised as a completed production failure domain.

## Verification

Both new and unchanged pak-helper translation units pass MSVC Win32 with
`/std:c++17 /EHsc /W4 /WX /O2 /fp:strict /MD`. The ignored probe links those objects
against the frozen primary BM `bsp_core.lib`, SHA-256
`d8c2b7431a416c4c9266a2d179416505d506fcf8d9de0a5058e268fd3928b269`.
The receipt records identical source-before/source-after/frozen-copy hashes from
`J:/PROG/battlestations-pacific-decompile-orch4-20260910/build/win32/Release/bsp_core.lib`.
The library contains unmount but predates pak helpers, so the unchanged helper
source is compiled explicitly. All 73 source/header inputs are copied and hashed
in `local/pak_callbacks_bo/frozen_inputs.json`; the probe uses the frozen include
tree, `/MANIFEST:EMBED`, and an image base that permits actual native table maps.

One small actual-storage probe, in ordinary and controlled-failure process modes,
uses real pools, runtime bindings, Mpak device dispatch, FileStores, mount trees,
resident memory-stream storage and raw registry backing. It verifies:

- False exit resolution is ignored; duplicate raw values are erased without
  releases; one concrete zero-reference FileStore unmount destroys the provider.
- Real resolver/device selection obtains device 42. A normal null factory result
  still appends/reserves a slot and advances the ordinal. This does not exercise
  successful nonnull archive factory creation.
- Dot/device cleanup deliberately changes the actual manager publication. Later
  increment/decrement/recomputation reaches the new manager, while the previous
  manager's count stays unchanged. Negative signed counts recompute inactive.
- A false entry resolution leaves the ordinal/vector unchanged. Normal fixture
  cleanup balances the actual resident stream, backing, registry and pool.
- A controlled exception in outer getter `BB5A4D` occurs after all raw erasures
  and real unmount. The failed frame retains `.mpak` and dot headers; ordinal and
  counter are still unchanged. The process retains all borrowed context through
  termination. This is a source cleanup-exception test, not an unknown native
  provider or dependency-unwind test.

`verify_report_calls.py` checks all 29 callback CALLs and five native EH action
tail JMPs against live containing bodies/callees. The two decoded-only handler
JMPs are recorded separately with their full missing-function ranges. No
permanent tests, metadata, runtime or CMake edits are included. A full integration
build and production/gameplay validation remain with the primary integrator.
