# Native logical-buffer device reset discovery

Read-only packet `native_buffer_device_reset_next`, based on `b1febd2`.
Names below are reconstruction hypotheses. The accompanying
`reports/native_buffer_device_reset_next.json` pins 40 fresh live/installed-PE
spans (4,101 bytes), including all 1,238 bytes of the six assigned routines.
Every live query verified project `bsp`, program `/battlestationspacific.exe`,
x86 language and image base `00400000`. No Ghidra mutation, production source,
ledger change, compilation or execution of these candidates occurred here.

## Assigned entries and actual storage

| Address | End exclusive | Bytes | Proposed behavior |
| --- | --- | ---: | --- |
| `00B49D00` | `00B49DBE` | 190 | Save logical vertex-buffer bytes before reset |
| `00B49DC0` | `00B49F70` | 432 | Recreate logical vertex buffer and restore saved bytes |
| `00B49F80` | `00B4A03E` | 190 | Save logical index-buffer bytes before reset |
| `00B4A040` | `00B4A182` | 322 | Recreate logical index buffer and restore saved bytes |
| `00B23270` | `00B232A4` | 52 | Drop current physical vertex COM buffer for reset |
| `00B23180` | `00B231B4` | 52 | Drop current physical index COM buffer for reset |

All six consume the actual owner in ECX, have no stack arguments, and end in
plain RET. They have no local FH3 registration or cleanup state and no stable
documented EAX result. Exceptions propagate with completed mutations intact.

Vertex logical storage uses physical owner `+58`, flags `+60`, element count
`+64`, declaration pointer `+68`, and CPU shadow pointer `+6C`. The declaration's
stride is its actual DWORD `+CC`. Index logical storage uses physical owner
`+08`, flags `+10`, element count `+14`, format `+18`, and shadow pointer `+1C`.
Neither routine constructs or destroys either logical owner.

Physical owner storage is the existing 2Ch layout: flags `+14`, capacity `+18`,
cursor `+1C`, lock depth `+20`, dynamic-lock count `+24`, COM pointer `+28`.
The current owner table, rather than a semantic binding structure, supplies:

| Table offset | Index target | Vertex target | Original ABI |
| --- | --- | --- | --- |
| `+08` | `B4B850` | `B4BA00` | Lock: ECX owner; five stack args; EAX data; RET 14h |
| `+0C` | `B4B820` | `B4B9D0` | Unlock: ECX owner; RET |
| `+14` | `B4C250` | `B4C370` | Attach: ECX owner; COM/flags/capacity; RET Ch |
| `+18` | `B4B800` | `B4B9B0` | Capacity: ECX owner; EAX DWORD `+18`; RET |
| `+1C` | `B4B840` | `B4B9F0` | COM getter: ECX owner; EAX pointer `+28`; RET |

The four complete original profiles are `D61E10` (private index), `D61E34`
(private vertex), `D61E58` (pooled index) and `D61E7C` (pooled vertex). Their
listed five targets agree, while their lifetime and recreation slots differ.
Compiled host tables must point to the real implementations and dispatch the
actual selected owner. Numeric original profile addresses are not callable host
tables; unrelated test callbacks do not close these dependencies.

## Save and direct release ordering

Save skips when `(logical.flags & F000h) == 1000h`, physical owner is null, or
the first current physical COM getter returns null. Otherwise it calls the
getter a second time on the freshly reloaded physical owner/table. If that
second result is nonnull, it captures that COM object, calls its current AddRef
slot `+04`, reloads the same captured object's table, and calls Release `+08`.
It continues even if the second getter returned null.

It then reloads physical owner/table and obtains capacity for array-new
`BF55BE -> BF681B`. It publishes the returned pointer to the current logical
shadow field without releasing a previous shadow. A fresh physical table is
captured for the next capacity call. After capacity returns, the physical owner
is reloaded but Lock is selected from that previously captured table. Lock gets
`(capacity, 0, 1, &zero_initialized_base_offset, 1)`. The third argument is unused;
the final byte is true in both save and restore, including the write-back copy.

After Lock returns, save reloads owner/table and capacity again. It copies that
current byte count from the captured Lock result to the current shadow pointer,
then reloads owner/table for Unlock and reloads owner once more for the direct
release helper. Allocation size, Lock size and copy size are three separate
capacity observations. No HRESULT, pointer, capacity, overflow or shadow-state
guard may be added as an alleged reconstruction of this native behavior.

The two 52-byte release helpers are identical apart from address. Capture
`physical+28`; if nonnull, perform AddRef followed by Release on that captured
object, selecting its current table independently for each call. Then reload
the current `physical+28`. If nonnull, call that object's current Release and
clear `physical+28` only after it returns. A callback can therefore change which
object receives the final Release. A throwing Release prevents the later clear;
there is no retry, owner destruction, pool return or metadata reset.

## Restore ordering and incomplete listing tails

Restore uses the same flags/physical-owner gate but proceeds only when the
current physical COM getter returns null. Vertex resource flags are decoded
inline; index calls the distinct register-output helper `B20A80` with kind 7.
The current renderer publication `F8D394` supplies ECX to `B1FEF0`, which borrows
the device at renderer `+1A10` without AddRef. The device's current table selects
CreateVertexBuffer `+68` or CreateIndexBuffer `+6C`.

The temporary COM output starts null. Vertex Create receives wrapped DWORD
`current declaration+CC * current logical+64`, decoded usage, FVF 0, decoded
pool, output address and null shared handle. Index Create receives current
logical format and count times stride 2 for format 65h, 4 for 66h, or 0 otherwise.
The HRESULT is ignored. After Create returns, capacity is computed again from
current logical fields, and current flags, physical owner and table select
Attach. Thus creation arguments and attached metadata can differ after a
callback. Attach's existing complete actual-storage provider also performs its
diagnostic string/support work; a simplified COM assignment is insufficient.

Next, restore reloads the temporary output pointer and unconditionally calls
its current Release slot. It does not release the borrowed renderer device.
If Create fails and leaves the temporary null, native behavior reaches a null
dereference after Attach; it does not return an HRESULT or skip the copy.

The capacity/Lock/current-capacity sequence matches save. Copy direction is
reversed: captured Lock result is destination, current logical shadow is source.
Unlock dispatch reloads current physical owner/table, then current shadow is
passed to `BF6989 -> BF65AC`. On return it clears the logical shadow field.
Ghidra currently omits these returning-free continuations from its listing:

| Gap | Complete native instructions |
| --- | --- |
| `B49F5F..B49F6B` | ADD ESP,4; POP EDI; MOV `[ESI+6C]`,0; POP EBX |
| `B4A171..B4A17D` | ADD ESP,4; POP EDI; MOV `[ESI+1C]`,0; POP EBP |

The 12-byte gaps are included in fresh live/PE spans. No source should inherit
the decompiler's false terminal-free path. If any earlier call throws there is
no automatic temporary COM release, Unlock, shadow free or field rollback.

For flags whose low nibble exceeds 3, native pool selection remains untouched:
vertex reads an uninitialized local DWORD at entry ESP-4, and index's helper
leaves its pool output at entry ESP-12 unwritten. A complete native entry must
retain that behavior; a C++ interface must explicitly bound the supported
input/stack-state domain. Zero initialization, DEFAULT fallback or INVALIDCALL
would change behavior. `B20A80` itself is ready for all flags because its explicit
output pointer can retain its incoming value on those cases.

## SDK slots and remaining physical Lock behavior

Windows SDK 10.0.26100.0 `d3d9.h` pins the x86 stdcall COM layout: AddRef `+04`,
Release `+08`, Lock `+2C`, Unlock `+30`, GetDesc `+34`. Vertex GetDesc takes a
24-byte D3DVERTEXBUFFER_DESC; index takes a 20-byte D3DINDEXBUFFER_DESC. Size is
at `+10` in both, and vertex FVF is `+14`. These six reset functions and their
physical Lock/Unlock methods do not call GetDesc. Their byte counts come from
physical metadata or current logical count/stride, not queried COM descriptors.

`B4B850` and `B4BA00` are each 172 bytes. Their capacity diagnostic uses a signed
comparison of wrapped `(cursor + extra_offset)` against capacity, then calls
`4C14C0` if greater. Requested byte count is not part of that check. They choose
NOSYSLOCK 800h, optionally READONLY 10h, or dynamic flags DISCARD 2000h at cursor
zero / NOOVERWRITE 1000h otherwise. Dynamic nonzero extra offset calls the same
singleton again. Flags and cursor are read after the first diagnostic; COM and
cursor are reloaded after the second. Dynamic count `+24` increments before Lock.

Nonnull COM invokes its captured current table's Lock with current cursor plus
extra offset, captured requested byte count, and a null-initialized local data
pointer. The HRESULT is discarded. Null COM returns address `F8D4B8` and clears
cursor. Afterward it writes current cursor to the caller's output pointer;
that output can alias physical fields, so the later flags test must reread
`+14`. If then dynamic, it adds wrapped bytes plus offset to current cursor.
Finally it increments lock depth. Unlock is 24 bytes: capture current COM,
skip entirely if null, otherwise call its current Unlock then decrement current
depth even for a failed HRESULT. A throw prevents the decrement.

Existing `D3D9BufferBinding` in `d3d9_buffers.hpp/.cpp` is a semantic structure
with a vector prefix and different failure/domain rules. Its Lock fragments and
even the ledger's function-labelled Unlock entries do not implement this raw
storage path. Existing GUI diagnostic singleton is also a semantic substitute:
it sets booleans and returns its state object rather than allocating/registering
the actual four-byte singleton. These names do not make the larger packet ready.

## Ready implementation packets and proposed interfaces

Packet file stems below each mean a new header under `include/bsp`, source under
`src`, matching uppercase documentation and an audit JSON under `reports`.
The integrator owns shared ledger/CMake/annotation work. Claim exact addresses
and files before implementation; these are readiness proposals, not new leases.

| Packet | Explicit addresses | File stem | Readiness |
| --- | --- | --- | --- |
| `native_physical_buffer_reset_release2` | B23270, B23180 | `native_physical_buffer_reset_release` | Ready: actual owner and COM ABI only |
| `native_physical_buffer_access4` | B4B800, B4B9B0, B4B820, B4B9D0 | `native_physical_buffer_access` | Ready: raw capacity and Unlock |
| `native_renderer_device_getter1` | B1FEF0 | `native_renderer_device_getter` | Ready: seven-byte borrowed device leaf |
| `native_resource_creation_flags1` | B20A80 | `native_resource_creation_flags` | Ready; assigned separately: 245-byte pure helper plus 16-byte jump table |
| `native_diagnostic_sink_owner3` | 4C14C0, 4BBCA0, 411EE0 | `native_diagnostic_sink_owner` | Ready: actual lifetime/CRT/Win32 providers exist |
| `native_physical_buffer_lock2` | B4B850, B4BA00 | `native_physical_buffer_lock` | Blocked until actual diagnostic owner3 |
| `native_logical_buffer_device_save2` | B49D00, B49F80 | `native_logical_buffer_device_save` | Blocked until access4, lock2, release2 |
| `native_logical_buffer_device_restore2` | B49DC0, B4A040 | `native_logical_buffer_device_restore` | Blocked until access4, lock2, device getter1, flags1; retain unsupported-pool stack domain |

Release APIs should take `void* actual_physical_owner` and expose separate
address-suffixed vertex/index entry names. Capacity returns uint32; Unlock takes
the same raw owner. Device getter takes the actual renderer and returns borrowed
`IDirect3DDevice9*`. Flags helper takes actual uint32 output pointers for usage
and pool plus engine flags and resource kind; native ECX is usage output, EDX
is pool output, stack arguments are flags/kind and RET 8. Preserve aliasing:
pool writes occur before the final usage write; invalid pool nibbles leave pool
untouched unless it aliases usage. Do not replace it with the guarded semantic
creation decoder.

Diagnostic owner3 uses a four-byte `NativeDiagnosticSinkStorage`, a reference to
the actual volatile publication `0109CF14`, and the application's existing
`SingletonLifetimeDomain`. `4C14C0` is 170 bytes, `4BBCA0` is 41 bytes and
`411EE0` is 25 bytes. The getter captures manager's optional actual section,
enters it, increments actual section `+18`, then arms its one unwind state.
After a second publication check it allocates four bytes, writes CE752C on
nonnull allocation, publishes even null, reacquires the actual manager, reloads
publication and registers it. It decrements the captured counter before Leave,
then returns the latest publication. Allocation/registration exceptions unwind
the captured section without unpublishing or freeing a published object.

The scalar deleter tests incoming flags bit 0, always clears the actual global,
writes base CE3818, optionally frees and returns original address with RET 4.
It does not unregister. `4BBCA0` has no Ghidra function yet; fresh raw bytes reach
the complete RET through `4BBCC9`. The raw guard API can take its actual eight-byte
record, capture section pointer `+04`, write profile CE37FC, decrement section
`+18` and call actual LeaveCriticalSection, leaving the pointer intact.

Getter FH3 handler C64F68 selects FuncInfo D8D5B8: one unwind-map entry at D8D5B0,
previous state -1, no catch/try maps, EHFlags 1. Thunk C64F60 passes `[EBP-14]` to
411EE0. The current actual lifetime provider has a real CRITICAL_SECTION at +0
and the real recursion DWORD at +18 (asserted in `singleton_lifetime.cpp`). Its
allocation, registration and OS operations can support this owner without
inventing a callback. The same actual lifetime instance must register/delete it.

Lock's proposed C++ context borrows that actual diagnostic publication/domain
and actual sentinel storage for F8D4B8. Save/restore context additionally borrows
actual string/resource-support/lifetime services required by physical Attach;
restore also borrows current renderer publication. Those contexts must never
cache physical owner, table, flags, capacity or shadow across the observed calls.

The report pins current complete actual Attach, COM getter, native string,
resource support and lifetime source files, alongside the partial semantic
providers used to establish blockers. It contains explicit files and APIs for
each packet. This is source/disassembly readiness evidence only, not a build,
fixture, ABI-replacement, device-reset, GPU or game-validation claim.
