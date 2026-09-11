# Native hardware-layout pool trimming

`trim_native_hardware_layout_pool_00b60350` reconstructs the complete native
`[00B60350,00B603F0)` body: **160 bytes**, ECX actual pool, RET with no semantic
result. Native vtable `D62AF0` points its virtual slot zero at this routine.
The implementation passed strict MSVC Win32 compilation and a bounded
original-instruction fixture: **62,291 recorded DWORD values and six actual
slab frees match on both sides**.

## Storage and behavior

The function borrows the actual 38h pool used by hardware-layout allocation.
The first 0Ch bytes are its allocator-list element; critical-section storage
is at +0Ch, depth at +24h, slab table at +28h, current slab count at +2Ch,
capacity at +30h, and earliest slab with available slots at +34h. Each 944h
slab contains 32 slots of 48h. A slot's final DWORD at +44h is its table index;
the slab's free-count WORD is at +940h.

For each current unsigned table index, a free-count WORD of exactly 32 causes
an actual shared-domain free. After that service returns, the routine reloads
the **current** table and count, copies its current final pointer into the
current index, and then decrements the current count. If the index is still
inside the new count, it reloads the moved slab and writes the index into all
32 tokens. It rescans that index, so a moved slab that is also wholly free is
removed in the same pass.

The initial Ghidra listing/decompile omitted **55 bytes at B60376..B603AC** after
the free call. Fresh live/installed-byte comparison and full x86 decode recover
this returning continuation. Primary integration restored it, preserved existing
comments, and saved the corrected body. Stopping at the early return would
lose table compaction, token rewriting, count updates, and replacement rescan.

After the deletion loop, the native nonempty comparison precedes writing
earliest to `FFFFFFFFh`. If that captured comparison was nonempty, the routine
captures a table cursor once and searches for the first slab with a nonzero
free-count WORD. Each loop test rereads current count. An empty pool or one
whose surviving slabs have no free slots retains `FFFFFFFFh`.

The routine leaves capacity, inactive table tail entries, owner payloads,
free-index stacks, slab padding, list links, critical-section storage, and
depth alone. It performs no lock acquisition, table shrink, or owner
destruction. The caller must provide valid backed storage and the supported
free domain; the reconstruction adds no safety behavior for malformed data or
concurrent mutation.

## Allocator-list setup

The new host setup API is:

```cpp
bind_native_hardware_layout_pool_trim_00d62af0(actual_pool, actual_list);
```

It binds the pool's actual first 12 bytes as an `AllocatorListElement`, with
native profile `{D62AF0, B60350}`, the actual pool as context, and the complete
trim implementation as the callable target. Repeating the same binding is
idempotent. Setup neither writes raw pool storage nor publishes an element in
the shared E188B4 list. It must run **before the separate pool initializer
publishes the element**, including before initialization-time allocation could
invoke the global new-handler traversal. No placeholder target or substitute
pool is introduced.

## Verification

The ignored fixture under `local/pool_trim_diff` executes all 160 original trim
bytes without patching them. It also executes original `004B46B0` allocator-list
traversal through the caller-supplied E188B4 head, the actual relocated D62AF0
vtable slot, and original B60350. The host side uses the real setup API and
existing `AllocatorListDomain::trim_all_004b46b0`.

Four spans total 202 bytes: original trim, list traversal, the five-byte free
entry, and the virtual slot. Every span freshly matched live Ghidra and the
installed executable. Complete runtime postimages match two address
relocations and one entry bridge only. The bridge preserves the native cdecl
free boundary and calls the actual x86 host CRT free; original relative call
distances and the caller's post-free stack cleanup remain intact.

The single fixture checks:

- Multiple wholly free slabs, including a wholly free replacement that must
  be rescanned; every moved token and surviving payload/index-stack/padding
  byte, stale inactive table entries, and unchanged capacity.
- A controlled free-boundary perturbation that performs actual free and then
  changes the pool's table/count before returning. Both implementations use
  those current values. This is a deliberate observation experiment, not a
  claim that ordinary CRT free changes pool fields.
- Removal to an empty pool, surviving slabs with zero availability, and an
  initially empty pool whose unused table value is deliberately invalid.
- Setup before publication, idempotent real binding, native virtual dispatch,
  and unchanged list, critical-section, and depth fields.

`./scripts/build.ps1` and both existing tests (`reconstructed_math` and
`native_math_differential`) passed after seed verification. The integrator
owns shared source registration: the worker baseline did not include
this new translation unit. Its ignored fixture separately compiled the owned
production source with `/O2 /Oy- /MD /EHsc /fp:strict /W4 /WX` and links the
baseline library's actual free and allocator-list implementations. No new
tracked test target was added.

Primary integration registered this source and reran the unchanged fixture
against a frozen current primary library containing the trim, real allocator
list, and shared free implementations. It again matches all 62,291 DWORDs
and six frees. All 25 worker pins, four fresh live/PE ranges and four complete
runtime postimages were checked. The complete original trim body remains
unpatched. Saved Ghidra annotations, ledger registration and refreshed exports
close the reconstruction evidence; real CRT free is pinned as the x86 provider.

The [audit report](../reports/native_hardware_layout_pool_trim_audit.json)
contains all original and patched bytes, complete instruction decode, source
and artifact hashes, free events, map providers, and the actual CRT DLL pin.
The local proof SHA-256 is
`14edbe5463a764fb0d9a7569603ccf40d9eb90b4c1a96a73d66ef52698347567`.
Installed executable SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

Names remain descriptive reconstruction hypotheses. These are new Win32 C++
interfaces and explicit host bindings, not drop-in native ABI replacements.
The fixture establishes bounded behavior and storage agreement, not complete
process startup or game validation.
