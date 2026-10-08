# Locked observer-pair query readiness

`00694AF0` (`BSP_Observer_IsPairRegistered`, descriptive hypothesis) is ready
for a **complete normal-return Source implementation under the existing raw
storage/lifetime admission**. Both direct native dependencies already have
genuine current providers; the existing outer guard uses the real Win32
critical section and the physical tracked-depth word. No new classifier,
dispatcher, semantic endpoint, lock mock, or lifetime domain is needed.

This packet adds only an audit, not the implementation or its verification.
The original entry ABI/private FH3-SEH behavior, invalid storage, concurrent
teardown, original enclosing classes and gameplay remain unproved. The proposed
function is a new C++ interface over actual storage, not a binary replacement.

The [report](../reports/cc12_observer_query_readiness.json) pins Source revision
`5b6a854c8a297043ccfa51f8260ab95d2f3caa9a`, complete bytes and Source hashes.
Current provider files also matched `main` at inspection. Read-only Ghidra
queries verified the existing `C:/Users/sqz269/bsp.gpr` project and
`/battlestationspacific.exe`; no other native body was expanded.

| Routine | Coverage | Static evidence |
| --- | --- | --- |
| `00694AF0..00694B8C` inclusive | Complete | 157 bytes, 52 instructions, zero listing gaps, both normal exits |
| Native callees and exception handler | Existing ledger/Source only | No new native body expansion |

All 157 bytes match installed PE offset `294AF0`, SHA-256
`01cfbb1d6cb08ff91919ca2d6606d3bb7cade7e62ca79caabfc83d0fd661e968`.
The installed PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Independent PE decoding also checks both `E8` calls and all three `FF 15`
import calls, including their import-table identities.

## Whole scalar and control-flow contract

Entry is `ECX = actual first endpoint`, `EDX = actual callback-owner base`, no
stack arguments. The routine saves EBX/ESI/EDI, captures first in EBX and owner
in EDI, and uses ESI for one captured critical-section pointer. EBP is not
modified. Both normal exits restore EBX/ESI/EDI and use plain `RET`.

1. `00694AF0..00694B0A` installs a stack exception registration with handler
   `00C7E9B8` and initial state -1; eight bytes are reserved for a guard.
2. `00694B0F` calls current lock-owner getter `00694280`. `00694B14` reads its
   `+4` section into ESI. There is no null-owner fallback before this read.
   The local guard is stamped `00CE37FC`, with the captured section at guard+4.
3. If the section is nonnull, `00694B28` calls imported
   `KERNEL32.dll!EnterCriticalSection(section)`, then `00694B2E` adds one to
   the physical DWORD at section+18, modulo 32 bits. A null section skips both
   operations, but does **not** skip lookup or force a false result.
4. `00694B32..00694B3E` restores the two original endpoint identities to EDX/ECX,
   sets the exception state to 0, and calls `006949D0`. That existing Source
   lookup independently gets the current lock owner and takes its own guard;
   the outer captured section must not be substituted for this second lookup.
5. `00694B43` tests the complete returned edge pointer in EAX. Nonzero chooses
   the true arm; zero chooses the false arm. This decision happens before the
   outer unlock, and no edge is dereferenced after lookup.
6. Both arms, if the captured ESI is nonnull, subtract one from section+18
   **before** calling `LeaveCriticalSection` on that same captured section.
   The true arm sets `AL=1` at `00694B58`; the false arm clears AL at `00694B7F`.
   Both restore the prior FS:[0] exception link, remove the 14h-byte local/SEH
   frame, and return at `00694B69` or `00694B8C`.

Only AL is the boolean result. The true null-section arm retains the edge
pointer's upper EAX bits; a nonnull-section arm leaves upper EAX dependent on
the void Win32 leave call. The decompiler's `undefined4 return 1/0` is therefore
not a full-EAX contract. ECX ends as the saved exception-list link on normal
return; other volatile results are not a new C++ interface guarantee. There
are no x87 or SSE instructions in this body. No float conversion or profile
dispatch is involved.

| Site | Native call | Arguments and return |
| --- | --- | --- |
| `00694B0F` | `00694280` | No native input; EAX=current published lock owner |
| `00694B28` | IAT `00CE2218`, EnterCriticalSection | Captured ESI pushed; Win32 callee consumes one DWORD |
| `00694B3E` | `006949D0` | ECX=original first; EDX=original callback owner; EAX=edge/null |
| `00694B50` | IAT `00CE2210`, LeaveCriticalSection | Captured ESI pushed on true path; one DWORD consumed |
| `00694B73` | Same leave import | Same captured ESI on false path |

Every site belongs to this exact live function body. The call-row verifier
checks the direct calls; the report separately retains PE import resolution
and exact instruction bytes for imports, which that verifier skips.

## Existing concrete provider chain

`NativeObserverLifetime::lock_owner_00694280` in `observer_lifetime.cpp` already
borrows the application publication, returns the existing owner or uses the
existing manager lock, allocates eight bytes, constructs the lock owner,
publishes/registers it and performs the final reload after unlock. Its
constructor uses `create_native_tracked_critical_section_00bd1860`, which
allocates actual 1Ch storage, calls real `InitializeCriticalSection`, and
initializes the depth at +18. This is not the older convenience/projection lock.

`NativeObserverLifetime::find_pair_006949d0` already operates on actual endpoint
arrays and actual edge pointers. It takes a separate nested guard, chooses the
smaller unsigned endpoint count (equality selects the callback owner's array),
and compares the opposite endpoint's actual address. It returns the edge or
null without synthesizing a semantic pair set. The current query wrapper does
not add references, change arrays, or dispatch the edge's profile.

`ObserverEdgeGuard` in `observer_edges.cpp` already performs precisely the
required normal enter/increment and decrement/leave order on a captured
`TrackedCriticalSection*`, using unsigned DWORD arithmetic for depth wrap.
Reusing it preserves a separate outer guard around the lookup's own guard.

The current application binding is also concrete: `GameSingletonHost` owns
`GameObserverRuntime`; its `SoundLifetimeAccess` borrows the one actual manager
publication cell, and `GameObserverRuntime` supplies the actual observer
lock/dispatch publication cells to `NativeObserverLifetime`. The deleting
context is bound before registration and retained through manager drain.
Existing `CF7E70` singleton destruction reaches `delete_lock_owner_00694ea0`,
which uses the matching real tracked-section release provider. The query does
not require unknown edge callback/deleting dispatch.

Admit the same actual aligned 10h-byte endpoint prefixes, valid live allocated
pointer spans (count <= capacity), and valid reached 10h-byte edges with the
actual endpoint identities at +4/+8. Null data with zero count is an empty
span; positive count does not acquire a default/null-edge recovery. The
existing lifetime context and its publication cells, manager, required dispatch
owner, lock owner and any reached section must remain valid through the query;
cleanup must precede their drain. A nonnull section is a real initialized 18h
Win32 CRITICAL_SECTION followed by the writable DWORD+18, not a copied OS lock.
The native null-section path is preserved. No endpoint profile is called or
validated, so this admission does not establish a unit or group constructor,
class lifetime, observer callback, world registration, or game behavior.

## Proposed bounded implementation and one connected fixture

Add the following declaration to `include/bsp/observer_edges.hpp` and definition
to `src/observer_edges.cpp`, under a new lease for those files and `00694AF0`:

```cpp
bool observer_pair_registered_00694af0(
    NativeObserverOwnerStorage& first,
    NativeObserverOwnerStorage& callback_owner,
    NativeObserverLifetime& lifetime);
```

The full normal-return body is a captured
`ObserverEdgeGuard(lifetime.lock_owner_00694280()->section_04)` followed by
`return lifetime.find_pair_006949d0(first, callback_owner) != nullptr;`.
C++ evaluates the boolean before releasing the local guard. Do not mark it
`noexcept`; the existing getter/allocation and C++ unwinding contracts remain.
No new CMake source, lock abstraction, global mapping or virtual service is
required. This interface intentionally adds the explicit existing lifetime
context; it is not the original ECX/EDX entry.

After implementation and the required Win32 build, one temporary connected
Source fixture should use the existing `GameSingletonHost`/`GameObserverRuntime`
and initialize its dispatch owner, retaining that same actual manager context.
Use two explicitly admitted raw endpoint prefixes with empty actual arrays
(borrowed storage, not copied unit/group objects). Query absent; register the
pair through current `register_observer_pair_00694a60`; query present; verify
query leaves the same edge pointer, reference count and endpoint arrays
unchanged; unregister through the existing lifetime provider; query absent.
Check real section identity and tracked depth before/after queries. Clean up
endpoint arrays with the current owner teardown paths before the raw manager
drain. This is one connected storage/lifetime case, not a new broad suite.
Any ad hoc Win32 executable must link `/MANIFEST:EMBED`; a proposed safe name is
`cc12_observer_query_probe.exe`. No such fixture was created or run here.

The handler at `00C7E9B8` remains named and unexpanded. Existing
`destroy_native_singleton_guard_00411ee0` is a real raw-guard cleanup provider,
but its existence alone is not proof of this handler's original unwind table
or asynchronous fault behavior. C++ RAII supplies its existing Source unwind
behavior; original FH3/SEH identity and exact volatile/FS entry ABI require a
separate explicit packet if those become acceptance requirements. They are not
silently promoted by this readiness result.

No normal-return provider is missing for the proposed bounded Source wrapper.
Completion still requires implementation, build, the connected Source check,
and review. No native execution or gameplay evidence is claimed by this audit.

Primary review independently read all52 live instructions, checked the whole
157-byte installed body and allfive call-site bytes, and rehashed all18 provider
Source pins against the integration checkout. The accepted scope is normal
return Source readiness; implementation/fixture evidence belongs to a separate
packet. Physical Original SEH/FH3 identity and gameplay remain unvalidated.
