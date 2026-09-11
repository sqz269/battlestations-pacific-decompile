# Native node raw allocation and return

Packet `orch3_node_pool_allocation_o_recovery`, 2026-09-11. Implements five
complete bodies in `src/native_node_pool_allocation.cpp`. The worker left an
uncommitted header/source when its turn stopped at a usage limit. The integrator
adopted those files, corrected an MSVC assembler-keyword collision, independently
reviewed the assembly, and completed compilation and differential verification.

These functions allocate and return raw node slots through the caller's actual
constructed pool. They neither construct a node nor supply a replacement pool
owner. Descriptive names are hypotheses, not recovered symbols.

## Native interfaces and extent

| Address | Inclusive end | Original ABI |
|---|---|---|
| `00B6DD10` | `00B6DD52` | ECX actual slab; stack slab index; EAX same slab; RET4 |
| `00B6EB00` | `00B6EC3B` | ECX actual pool; EAX raw slot; RET |
| `00B6ED70` | `00B6ED79` | Incoming ECX ignored; replace with `0108FF58`; tail JMP B6EB00 |
| `00B6E490` | `00B6E4FB` | ECX actual pool; stack raw slot; RET4; no established result |
| `00B6E670` | `00B6E67B` | ECX raw slot; select `0108FF58`; CALL B6E490; RET |

The preexisting `CG_static_dtor_stub_00b6ed70` inventory label was incorrect.
The two instructions are `MOV ECX,0108FF58` and `JMP B6EB00`. Callers including
`868193` supply size `174h` in ECX, but the wrapper overwrites it. The new C++
wrapper instead receives the actual global-pool address explicitly; it does not
pretend to reproduce that binary entry convention.

Live Ghidra and installed-PE bytes agree across all five complete spans. The
report records lengths and SHA256 values. Ghidra's omitted `B6EB96` bytes are
`83 C4 04`, the stack repair after returning free. The flow tool restored them
and reported zero remaining call gaps. `B6EBF9..B6EBFF` is an unreachable
seven-byte LEA alignment instruction after `JMP B6EC00`; it was left alone.

## Physical storage

The caller supplies an initialized `38h` owner. Allocation/return use its
critical section at `+0C`, DWORD tracked depth at `+24`, slab pointer table at
`+28`, unsigned count/capacity at `+2C/+30`, and earliest nonfull slab index at
`+34`. `FFFFFFFF` means that a new slab is needed. Fields `+00..+0B` remain
untouched by these five bodies. Access helpers preserve native DWORD address
arithmetic and explicit field reloads without a copied vector or shadow count.

Each slab is `2F44h` bytes. It contains 32 slots of `178h` bytes, each with a
`174h` payload followed by its DWORD slab-table index. The last `44h` bytes hold
32 unsigned WORD free indices, a WORD free count at `+2F40`, and two untouched
padding bytes. `B6DD10` sets count32, writes free indices31 down to0 and every
slot's slab-table index. It leaves payloads and final padding untouched.

## Allocate and return

`B6EB00` enters the embedded section and increments tracked depth. When `+34`
is `FFFFFFFF`, it publishes the current slab count there before allocating
`2F44h` through the canonical ordinary-new service and initializing that slab.
If count equals capacity, it first publishes `2*capacity+2`, then allocates a
pointer table of wrapped `4*new_capacity` bytes. The copy loop rechecks current
count and backing storage. Returning free precedes replacement-table publication.
The new slab is appended and count increments.

The selected slab's WORD count decrements before the free-index read. The
returned address is slab base plus `178h*index`. When the slab becomes full,
the function publishes the sentinel and scans later slabs for a nonzero WORD
free count. It decrements tracked depth before leaving the captured section and
returns the captured raw slot. It neither zeroes the payload nor invokes node
construction. There is no native local EH guard: allocation failure preserves
the held section, depth, and already-written fields. No rollback was added.

`B6E490` enters the same section, increments depth, reads the slot's stored
slab-table index, and resolves that slab through the current table. Native
`IMUL`/shift/correction at `B6E4B8..B6E4C9` computes signed pointer-delta division
by `178h`, truncated toward zero. Its low WORD is pushed at the slab's current
free count, then that WORD count increments. An unsigned comparison lowers
the earliest index when necessary. Depth decrements before unlock. Native code
contains no null, ownership, double-return or capacity validation and reclaims
no slab here. Those remain caller obligations.

## Verification and remaining dependencies

The registered sources passed the strict MSVC Win32 build and both existing
CTests. One ignored fixture uses five copied original function spans, rebinding
only allocation/free calls, constructor/allocator calls, Windows import cells
and the two global-pool immediates. It compares complete slab initialization
bytes and preserved payload/padding; allocation/return compare pointer identity
as slab/index, full free-index metadata, slab IDs and owner counters.

The fixture performs 1,057 allocations, growing the table from32 to66 entries,
then tests scanning across31 full slabs, lowering the earliest index, LIFO
reuse, ignored wrapper-size values, both raw-return entry points, and nested
critical-section depth. Allocation-failure execution and native exception ABI
were not tested. Current CRT/Windows services are explicit rebindings.

The fixture prepares valid low-level input state with a real critical section
and allocated table; this is not production pool-owner reconstruction.
`B6E980` construction, `B6E3D0` destruction, and their `CD7D10`/`CE0E20` static
lifetime chain remain separate dependencies. The worker's read-only discovery
of that chain is a starting point to reverify before implementation. `B6F5A0`
must construct the actual `174h` node prefix, and its retained terminal and
scene companion must bind to that physical owner. `NativeNodeBinding` and
`SceneNodeAttachment` cannot substitute for the allocated storage.

## Correction from docs/NATIVE_NODE_POOL_OWNER.md

Packet `orch3_node_pool_owner_p` reverified and reconstructed B6E980/B6E3D0,
B6EA60 compaction, B6DDF0 table cleanup, and the CD7D10/CE0E20 static lifetime
chain. Its original-byte fixture uses the reconstructed owner with these raw
allocation/return routines, including returns after moved slab IDs are rewritten.
The owner-lifetime dependency above is now implemented. The shared checkout
already supplies B6F5A0 in `native_node_construction.cpp`; composition with
this pool, the actual native string-pool bridge and retained terminal binding
still remains work.

No game entry was executed, installed file changed, or native vtable/exception
ABI compatibility established. The full point-effect constructor, deferred
release dispatch and application integration remain unfinished.
