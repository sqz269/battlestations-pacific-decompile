# Native string-pool ownership

Packet `orch3_native_string_pool_owner_m` reconstructs the lazy getter, embedded
ring constructor, complete owner constructor, destructor and scalar deleting
destructor against their actual Win32 storage. The source is
`src/native_string_pool_owner.cpp`; its public layout and explicit lifetime
binding are in `include/bsp/native_string_pool_owner.hpp`.

This packet does **not** make `SizedStoragePool` actual native storage. That older
class owns a separate arena, vector ring and cached return gate, and adds bounds
and failure behavior. No getter, allocator or string consumer here calls it.

## Evidence and coverage

All live queries used `python tools/bsp.py ghidra ...`, whose client verifies the
configured `bsp` project, `/battlestationspacific.exe`, x86 language and image
base before each query. Configured saved project: `C:/Users/sqz269/bsp.gpr`.
Ghidra and the installed game were read-only. New exports for the ring and
destructors are under the shared ignored `exports/bsp/functions/` tree.

| Native span (exclusive end) | Original ABI | Rebuilt operation |
| --- | --- | --- |
| `00419CC0..00419D80` | No arguments; EAX current publication; RET | `native_string_pool_get_or_create_00419cc0` |
| `00BD0F50..00BD0F93` | ECX embedded ring; EAX same ring; RET | `construct_native_string_pool_ring_00bd0f50` |
| `00BD1480..00BD14B2` | ECX raw owner; EAX same owner; RET | `construct_native_string_pool_00bd1480` |
| `00BD14C0..00BD150F` | ECX owner; RET; no specified return | `destroy_native_string_pool_00bd14c0` |
| `00BD1730..00BD174E` | ECX owner; stack flags; EAX original owner; RET4 | `delete_native_string_pool_00bd1730` |

The saved signatures omit register inputs. Constructor `00BD0F50` initializes
the embedded ring, not a complete-owner base. At `00BD1745` Ghidra's destructor
listing omits `83 C4 04` after a wrongly classified no-return `_free` call.
Direct bytes prove that stack repair followed by `MOV EAX,ESI; POP ESI; RET4`
ends at `00BD174E`. The rebuilt deleting destructor includes that return path.

The getter's handler at `00C5E123` is not a Ghidra function start. Bytes decode
`MOV EAX,00D841EC; JMP 00BF6B43`. FuncInfo `00D841EC` has two unwind states and
no catch map: `00D841DC` maps state0 to `00C5E110` (guard teardown through
`00411EE0`) and state1 to `00C5E118` (free captured allocation, then state0).
The latter's Ghidra extent omits `POP ECX; RET` at `00C5E121..00C5E123`.
These support the getter's cleanup; they are not separately reconstructed or
renamed functions. MSVC `__try/__finally` retains cleanup on C++/Win32 unwind;
the new interface does not reproduce the original exception-handler ABI.

## Actual layout and writes

`NativeStringPoolStorage` is a trivial, uninitialized `0x8AD4A0`-byte aggregate.
Win32 static assertions establish every interpreted offset. The constructor
writes profile DWORD `00D68200`, constructs the embedded ring, initializes the
actual `CRITICAL_SECTION`, zeros its depth, then zeros the bump offset.

| Offset | Actual field | Constructor behavior |
| --- | --- | --- |
| `+00` | Native vtable identity DWORD | Write `00D68200`; never callable host data |
| `+04` | Four uninterpreted bytes | Preserve |
| `+08` | Embedded 7,000,000-byte arena | Preserve all bytes |
| `+6ACFC8` | DWORD bump offset | Zero, after section/depth initialization |
| `+6ACFCC` | Embedded `NativeStringPoolRingStorage` | Initialize metadata only |
| `+8AD484` | Embedded 24-byte Win32 critical section | `InitializeCriticalSection` |
| `+8AD49C` | DWORD tracked depth | Zero |

Ring relative offsets: `+000000` holds `0x80000` actual pointer cells;
`+200000` holds 150 heads; `+200258` holds 150 tails; `+2004B0/+2004B4`
hold live/peak DWORDs. For each class, store `head=i*0xDA7`, then `tail=head-1`;
after all classes zero live, zero peak, then overwrite tail0 with `0x7FFFF`.
The pointer cells retain their allocation preimage. There is no vector or
secondary arena, and construction does not modify the real `01090AA4` gate.

## Publication and lifetime

The getter borrows the application's actual volatile `01090AA8` slot and its
canonical `SingletonLifetimeDomain` for `01090AA0`. A cached nonnull value
returns immediately. On a miss it captures manager+10h, enters that section
if nonnull, increments its tracked depth, then tests the slot again. It allocates
exactly `0x8AD4A0` bytes through canonical `singleton_lifetime_allocate`
(`00BF681B` malloc/new-handler/retry/throw contract), constructs if nonnull,
publishes, gets the manager again and registers the **current slot pointer**.
The allocator currently throws on exhaustion; the native null branch remains.

Constructor failure frees only the raw allocation, then releases the captured
manager section. Registration failure leaves the owner published and releases
only the captured section. Unlock precedes the final publication reload. There
is no additional publication mutex, ownership registry, or implicit destructor.

Destruction publishes `00D68200`, sets the caller's real volatile `01090AA4`
gate to one, decrements/releases the embedded section while its DWORD depth is
positive under a signed comparison, deletes that section, unconditionally
clears the caller's `01090AA8` slot, then writes base profile `00CE3818`.
Zero or negative signed depth skips the leave loop. It neither unregisters
from the singleton manager nor releases individual arena blocks. Deleting
destruction additionally calls canonical `singleton_lifetime_free` only for
`flags & 1`; native `00BF65AC` jumps to `_free 00BF9DC8`. EAX returns the
original address even when that address has been freed.

`NativeStringPoolLifetimeBinding` composes a real `destroy_registered` callback
for the application's one shared domain. Its references identify the same actual
slot and shutdown gate. Construct the binding first, construct that canonical
domain using `binding.callbacks()`, and retain the binding until shutdown.
For identity `00D68200` it directly calls the rebuilt `00BD1730` implementation;
all other owner profiles and invalid-parameter calls go to required downstream
callbacks. There is no literal-vtable invocation or silent unknown-owner path.
The canonical manager pops an entry before dispatching with flag1, so the
native pool's lack of unregister is intentional. No private domain is created.

## Mandatory next boundary

This packet supplies owner storage and lifetime only. Startup's narrow/wide
string consumers still require an actual-state allocation/release adapter, with
these functions independently owned by the integrator:

| Address / exclusive end | Contract required over this exact owner |
| --- | --- |
| `00BD1120..00BD11D2` | ECX pool; stack `(DWORD size, unused)`; RET8. `size>=0x96` calls nullable `_malloc 00BF9F1A` without a lock. Otherwise enter the embedded section and increment depth; pop actual `slots[tail[size]]` while decrementing live/tail, or return `arena+bump` and add exact size to bump. Preserve DWORD wrapping, no alignment and no arena bounds check. Unlock before returning the captured pointer. |
| `00BD1510..00BD156F` | ECX pool; stack `(block, DWORD size, unused)`; RET12. Large blocks call `_free 00BF9DC8`. Small blocks reread actual `01090AA4` before entering the embedded section; nonzero drops the return. Otherwise call `00BD12A0` with the address of the captured stack block and its exact size. Never cache the gate. |
| `00BD12A0..00BD1375` | ECX actual embedded ring; stack `(void* const* block, DWORD size_class)`; RET8. Preserve native circular displacement/collision behavior over actual cells, head/tail arrays, live and peak counters. No host vector or bounds recovery. |

The actual-state adapter must run this getter for each allocation/release just
as callers do, retain the same publication/domain/gate references, and respect
the nullable large-allocation path. `NativeStringStorage` currently documents a
nonnull allocation contract; an adapter must resolve that documented mismatch
without adding throwing behavior to native `00BD1120`. This packet neither
changes consumer APIs nor substitutes `crt_string_storage()` to claim closure.

## Validation and limits

`scripts/build.ps1` passed, including both existing CTests (`reconstructed_math`
and `native_math_differential`). `cmake/startup.cmake` was leased by another
worker, so this packet leaves source registration to the integrator. The new
source was compiled separately with MSVC Win32 `/W4 /WX /fp:strict /EHsc /MD`.
The ignored `local/native_string_pool_owner_probe.cpp` fixture linked the source
object and existing core library using `/MANIFEST:EMBED` and passed.

The fixture checks preserved bytes across the full arena and pointer-cell
region, all ring metadata, actual-pointer registration/cached identity, manager
depth restoration, unchanged shutdown gate during construction/recreation,
flag2 destruction retaining storage, depth2 release, and the canonical manager
dispatching the real deleting destructor/free while forwarding another owner.
It does not inject constructor/registration exceptions or execute original
pool machine code. No permanent tests, game entry execution, or installation
changes were made. Descriptive names remain hypotheses; the C++ interfaces are
not binary replacements, and neither startup nor gameplay is validated.
