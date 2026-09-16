# Native renderer cache cleanup

Addresses: `00B30340`, `00B30410`, `00B316C0`, `00B31730`, `00B32090`, `00B32370`.

This packet adds actual-storage cleanup for the renderer's texture cache and
effect-record array, plus an actual-pool overload of the existing resource-record
resize. It borrows the same string pool, canonical reference owners, and immutable
numeric profile tables used by the application's existing providers. It creates
no parallel cache, ownership map, reference count, or string-pool projection.

| Routine | Inclusive original range | Bytes | Coverage |
| --- | --- | ---: | --- |
| Resource-record resize, actual-pool overload | B30340..B3040C | 205 | Complete existing body, shared with existing pool API |
| Effect-record resize | B30410..B304DB | 204 | Complete |
| Resource-cache clear | B316C0..B31729 | 106 | Complete in the verified texture profile domain |
| Resource-array destruction | B31730..B31746 | 23 | Complete |
| Resource-cache base destruction | B32090..B320EE | 95 | Complete in the verified texture profile domain |
| Texture-cache destruction | B32370..B32401 | 146 | Complete in the verified texture profile domain |

Descriptive names are hypotheses. The original resize methods use ECX = actual
12-byte header, one stack count DWORD, and `RET 4`. The other four use ECX =
actual owner and plain `RET`. These explicit-context C++ interfaces are not
original caller/register/private-frame or SEH ABI replacements.

## Raw records and array resize

The actual record is the existing producer-verified 2Ch
`NativeRenderResourceRecord`: name +00/+04, preserved +08, sentinel +0C, alias
count +10, five payload DWORDs +14..+24, resource +28. Both resize bodies compare
signed request/capacity and invoke their existing full reserve provider when
needed. Growth captures the current array base per row, zeroes name length/data,
allocates the actual 4C3020 sentinel, publishes sentinel then zero alias count,
and writes payload zeros from +24 down to +14. It preserves +08 and +28.

Failure during sentinel allocation cleans only the current partial name. The
state0 action then reads current array data for the verified no-op placement
delete 401130. Earlier completed rows are retained outside current count; no
new-array or completed-row rollback is added. Shrink publishes count-1 before
loading current data and destroying that current row. Finally count=request.

B30410 uses the existing B2FFE0 effect reserve and B2FA10 effect-record storage
destructor with the caller's actual allocation bindings. B30340 shares one
implementation between its existing `SizedStoragePool` interface and the new
`ActualNativeStringPoolStorage` overload. Only the string-storage adapter differs;
the native reads, stores, reserve and destruction order are shared.

## Cache clear and dispatch

The registry header holds profile +00, actual record data +04, count +08,
capacity +0C, and accounting +10. B316C0 uses a nonzero DWORD count condition:

1. Capture the current last row's resource, select its current profile and
   invoke its +0C accounting body.
2. Subtract returned size from current accounting with DWORD wrapping.
3. Reload current count/data/resource and current registry profile/+10 slot,
   then release that captured resource.
4. Reload current count. If nonzero, reload current data and destroy the
   current last record's storage, then decrement the post-destruction current
   count. This differs from the resize helper's decrement-before-destruction.
5. Repeat on current count, then call full B30340 on registry+4 with zero.

Verified child profiles are D61948 (2D, +0C B3CE30 reading actual +24), D61870
(cube, +0C A82250), and D618B0 (volume, +0C A82250). Cache profiles D5F088 and
D5F038 both have +10 B31DA0. The borrowed current table entry is verified before
dispatch. B31DA0's complete observed body decrements captured resource+4 and
calls its current virtual0 only on zero; the existing
`release_native_render_actual_owner` implements that operation with the same
actual atomic and canonical companion. No original executable address is called.
Unselected table views may be null; reached views must have the required extent.
Other profiles or entries have no implementation in this source domain and
produce an explicit provider error, not successful cleanup.

The parent B32920 direct clear site B32BBA supplies renderer+1A74. Parent and
outer-EH reconstruction remain integrator-owned. The parent EH audit separately
maps its texture-cache cleanup state to B32370 on the same +1A74 owner.

## Destruction and source unwinding

B31730 calls B30340(header,0), then frees the current data. It preserves the
stale pointer and capacity. B32090 publishes D5F038, arms state0 before B316C0,
then disarms before its normal resize0/free. State0's CBDCA0 action passes
owner+4 to B31730. If clear throws, array storage is destroyed and freed while
already performed reference/accounting effects remain.

B32370 publishes D5F088 and captures +20 before arming state1. If that reference
is nonnull, it decrements its actual +04, performs current-profile terminal
dispatch at zero, then clears owner+20. It captures name data +18, lowers state
to zero, and returns nonnull data using current length +14 plus one. Name bytes
are left intact. State is lowered to -1 before the normal B32090 call.

The verified DF66B8 unwind map is state1 -> CBDD08 (current string at owner+14),
then state0 -> CBDD00 (base B32090). The DF662C base map is state0 -> CBDCA0
(array B31730). Source guards preserve this armed/disarmed schedule. A throwing
cleanup during unwinding terminates under C++ rules. Existing actual string-pool
release and canonical terminal callbacks retain their noexcept obligations;
lazy pool recreation failure through that interface is not native SEH parity.

The saved B31730 body stops at B31741; its returning BF6989 call is B3173D and
its actual return is B31746. Saved B32090 stops at B320DB; its BF6989 call is
B320D7 and actual return B320EE. The other four spans already cover their return
instructions. No Ghidra mutation was performed by this worker; the integrator
owns returning-flow repairs, preserved-old-value comments, naming, save, and
refreshed exports after release of the lease.

## Validation boundary

The report records 779 guarded bytes matching the installed executable,
eight verified native seeds, strict Win32 build/CTest results, and the focused
fixture's exact scope and hashed artifacts. The fixture uses real raw records,
sentinel allocations, current storage, actual-pool resize, and actual reference
counters. It compares complete original B316C0/B31730 and normal B30340 paths
with main-archive source, including a terminal callback that changes current
data/count. Original indirect dispatch uses equivalent fixture vtable pointers;
source dispatch uses verified numeric profile identities. Source-only failure
checks establish base/derived armed cleanup and retained earlier effects.

The terminal callbacks are fixture instrumentation, not game texture destruction.
Nonnull pooled names, unmasked hardware faults, native SEH/CRT exception identity,
full effect-resize differential execution, and game validation are not claimed.

## R33 current-main reuse

The worker validation and saved-listing limitations above describe historical
commit `8e1bdea08`. R33 reuses the source unchanged with current main providers;
`reports/native_renderer_cache_cleanup_main_r33.json` records the fresh build,
fixture and live evidence. Current exports already cover the complete returns.
