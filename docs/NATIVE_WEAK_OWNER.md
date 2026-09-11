# Actual weak-reference base and pool

`NativeWeakOwnerDomain` supplies the concrete `NativeGuiSceneWeakBase` required by
`gui_camera_store_owner.hpp`. Construction and destruction use the scene's actual
`+04` reference count and `+08` handle field. `NativeWeakOwnerView` supports another
real native prefix without casting between unrelated C++ storage classes; it
checks that all three borrowed fields are at `identity + 0/4/8`.

The code is a new MSVC Win32 C++ interface. It is reconstructed, build-tested and
host-fixture-tested. It is not a drop-in binary replacement, a native weak-owner
differential result, or a game/render validation. Names below are hypotheses.
`reports/native_weak_owner.json` records original names/comments, exact extents,
complete-range byte comparisons, ABI details and pending annotation work.

## Storage and lifetime contract

The caller supplies one `SingletonLifetimeDomain`, the actual `0109CE90` mutex
publication by reference, the actual `0109CE94` pool storage, and the shared
`AllocatorListDomain` backed by `00E188B4`. There are no default owners or fallback
locks. `NativeWeakHandlePool` holds no independent slab table or free stack.
Its host virtual-zero binding points at the concrete `B003A0` trim implementation.

`NativeWeakOwnerDomain` is directly usable as the `weak_base` member of
`NativeGuiSceneEnvironment`. Its singleton manager's `destroy_registered` callback
must recognize the mutex profile `D190B4` and route the original owner and flags
to `delete_lock_owner_00925430`. This is the same manager used for registration,
not a second shutdown registry. The domain, pool companion and supplied storage
must survive the registered mutex and all live weak handles.

The static adapter requires an explicit binding of that same pool via
`bind_static_native_weak_pool_0109ce94`. `initialize_static_native_weak_pool_00cd8a60`
calls the shared constructor and registers `destroy_static_native_weak_pool_00ce1040`
with the supplied atexit function. As in other actual native pool companions,
explicit destruction performs the native cleanup; the C++ companion destructor
does not destroy storage or unregister the native owner.

| Actual storage | Layout / behavior |
|---|---|
| Weak-capable object prefix | `+00 D190F4`, `+04 atomic references`, `+08 weak handle` |
| Mutex object at `0109CE90` | 8 bytes: `+00 D190B4`, `+04 TrackedCriticalSection*` |
| Weak object | 12 bytes: `+00 D190B8`, `+04 atomic references`, `+08 target identity` |
| Pool slot | 16 bytes, with slab ID at `+0C` outside the weak object |
| Pool at `0109CE94` | 56 bytes: shared allocator element, actual Win32 section at `+0C`, recursion `+24`, slab table/count/capacity/first-free `+28/+2C/+30/+34` |
| Slab | `0x904` bytes; 128 slots; reverse WORD free stack at `+800`; WORD free count at `+900`; final two bytes left untouched |

`925490` writes the base `CEB130` phase, seeds the actual owner count to one, then
writes `D190F4`. Under the actual mutex it obtains a slot from `0109CE94`, seeds
the slot's own count to one, sets `D190B8`, publishes it in owner `+08` and writes
that same owner's identity into slot `+08`. Owner and handle have different,
required native counts; the implementation adds neither a companion count nor
a second owner object.

`925540` restores the weak-owner phase, captures the actual mutex, clears the
same handle's target and atomically releases its count. Zero follows the verified
`BD30E0 -> vtable+4(flags=1) -> 925470` path: write the `CEB130` base phase and return
the raw slot to the same pool. The outer owner ends in `CEB130`. Its count and
handle pointer remain unchanged, including the stale pointer after a terminal
handle release. An externally retained handle survives with its target null until
its own final release. `retain_handle`/`release_handle` are host equivalents of
the established reference-count operations, not invented implementations of the
other weak virtual methods.

## Address evidence

All ranges below are half-open, include the complete final instruction, and were
compared against the installed executable through the verified saved Ghidra
program. The shared profile functions also serve a second static pool; their
proposed names are `Pool128x16`, not weak-only names.

| Address range | Reconstructed operation | Native ABI |
|---|---|---|
| `[00925490,0092553D)` | weak base construct | ECX object; EAX identity; RET |
| `[00925540,009255E6)` | weak base destroy | ECX object; RET |
| `[00924480,0092453D)` | lazy mutex getter | no inputs; EAX publication; RET |
| `[00924050,00924095)` | mutex construct | ECX raw 8 bytes; EAX identity; RET |
| `[00925430,00925467)` | mutex scalar delete | ECX object; stack flags; EAX identity; RET4 |
| `[009242F0,00924420)` | allocate raw weak slot | ECX pool; EAX slot; RET |
| `[009236A0,009236E3)` | initialize slab | ECX slab; stack index; EAX slab; RET4 |
| `[00924420,00924475)` | return raw slot | ECX pool; stack slot; RET4 |
| `[00925470,00925490)` | weak handle scalar delete | ECX handle; stack flags; EAX identity; RET4 |
| `[00CD8A60,00CD8A76)` | weak static init/atexit | no inputs; EAX atexit result; RET |
| `[00CE1040,00CE104A)` | weak static destruction | load ECX `0109CE94`; tail JMP `B002C0` |
| `[00B004B0,00B00583)` | shared pool constructor | ECX pool; EAX identity; RET |
| `[00B003A0,00B00440)` | shared pool trim | ECX pool; RET; no internal lock |
| `[00B002C0,00B0034A)` | shared pool destroy | ECX pool; RET |

The mutex getter performs the observed double check under the singleton manager's
captured section, allocates 8 bytes, calls the real existing
`critical_section_create_00bd1860`, publishes and registers that owner, unlocks,
then reloads the publication. Complete mutex deletion destroys the owned section,
clears that actual publication, writes `CE3818`, and conditionally frees. Constructor
unwind instead follows `CA6AA0 -> 923620` and only clears publication/base phase.

Pool construction prepends the actual allocator element, initializes the actual
section and reserves 32 table entries. Allocation publishes the first-free index
before slab allocation; table growth is `2*n+2`. There is no internal allocation
unwind guard. The outer weak-owner lock has a separate guard and base unwind.
Pool constructor EH map `DF33E0` points to table cleanup `CBB3B3/AFFA20`, section
cleanup `CBB3A8/402F70`, then allocator unlink `CBB3A0/403970`.

Trim frees fully empty slabs, swaps the last slab into the hole, decrements the
count, rewrites all 128 moved slot IDs, rechecks the moved slab and recomputes
first-free. Destruction frees every slab and the table, drains positive explicit
section recursion, deletes the real section, restores the allocator base phase
and unlinks the real element. It does not call weak-object destructors or clear
the stale table/count/index and list-link fields.

## Pending Ghidra repair

Ghidra was read-only for this packet. `CD8A60` has no function; its full 22 bytes
are the independently verified static initializer, ending in RET at `CD8A75`.
Do not accept the index's enclosing `CD8460` candidate as its function ownership.

Incorrect no-return assumptions after free hide live continuation bytes:

| Free call | Missing range | Restored behavior |
|---|---|---|
| `924381` | `[924386,924389)` | stack cleanup; replacement publication follows at `924389` |
| `925459` | `[92545E,925461)` | stack cleanup; return original identity/RET4 |
| `B002D8` | `[B002DD,B002E8)` | slab free-loop continuation |
| `B002F0` | `[B002F5,B002F8)` | stack cleanup before section teardown |
| `B003C2` | `[B003C7,B003FD)` | swap-last slab removal, count/ID repair and recheck |
| `B00564` | `[B00569,B0056C)` | stack cleanup before replacement publication |

The report preserves all old names and the original `CE1040` inventory comment.
The name ledger contains proposed annotations; native comments/names and repaired
exports require the integrator's coordinated annotation batch. The remaining
`D190B8` entries at `925660/9256D0/925740` are outside this implementation; no weak
promotion or observer semantics are inferred from them.

## Verification limits

`scripts/build.ps1` passed under MSVC 19.51 Win32 `/W4 /WX`. After `verify-seeds`,
both existing tests passed: `reconstructed_math` and `native_math_differential`.
Those differential tests cover their existing seed routines, not this owner.

One ignored fixture, `local/native_weak_owner_probe.cpp`, passed. It used the
concrete GUI interface, checked same-identity/ref/handle storage and unchanged
tail bytes, retained the handle through target destruction, allocated 4,097 raw
slots, observed capacity `32 -> 66`, trimmed an early empty slab with the last
slab still live, verified its repaired ID, returned all slots, exercised real
singleton registration/shutdown, and verified actual allocator unlink and stale
table fields. The probe was built with an embedded manifest and added no permanent
test target. Its hashes and build log hash are in the report.

No exception-allocation injection, concurrent races, cross-thread promotion,
native weak-owner execution, live CRT timing, or game/render behavior was tested.
The existing critical-section creator's host nothrow allocation policy remains
an explicit dependency, not a claim of native allocation-failure equivalence.
