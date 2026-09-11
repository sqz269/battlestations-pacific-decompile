# Native mesh draw section

`NativeMeshSectionStorage` reconstructs the actual 60h object published in
mesh+54's retained pointer array. `00533FA0` obtains a 64h physical slot from
canonical pool `010901D4` through `00B85EE0`/`00B85D30`, then constructs its
60h payload at `00B857F0`. The additional DWORD at slot+60 is the pool's slab
index. It is never a section field or a heap-allocation header.

The new C++ interfaces target MSVC Win32. They are not original calling-
convention replacements. Descriptive symbols remain hypotheses. Exact
instruction boundaries, disk-window SHA256 hashes, ABI descriptions and
the observed prior Ghidra names are in `reports/native_mesh_section.json`.
Every live read/export verified the configured `bsp.gpr` project and
`/battlestationspacific.exe`; this packet made no Ghidra mutations.

## Storage and ownership

| Offset | Meaning and lifetime |
| --- | --- |
| +00/+04 | Native D63194 profile and the actual atomic reference count |
| +08 | Primitive DWORD, initialized to4 |
| +0C..18 | Four raw draw-range DWORDs, initialized to0 |
| +1C | Instance-count DWORD, initialized to0 |
| +20 | Retained actual material identity |
| +24..30 | Four bounds words; constructor copies current CE4970 into fourth |
| +34 | Raw DWORD initialized to0 |
| +38 | Retained next-section identity |
| +3C..48/+4C | Four retained selected streams and signed active count |
| +50 | Retained actual vertex-layout identity |
| +54 | Uninterpreted copied DWORD; no retain/release established |
| +58 | Indexed-draw selection byte, initialized to1 |
| +59..5B | Untouched constructor padding |
| +5C | Retained actual instance-generator binding |

Unused stream slots are also untouched by construction. The constructor
loads CE4970 before any stores; its profile publication order is CEB130,
D63194, then material-null, then reference-count1. The original 104-byte
constructor was executed in an isolated probe with only its absolute MOVSS
data operand relocated to private copied data. All100 output bytes matched
the host constructor, including untouched payload and pool metadata bytes.

The section has no retained index-stream field. The mesh owns its index at
mesh+60. `00B85610` checks section material+20 and material effect+7C, then
forwards the supplied opaque argument and section to `00B451D0`. This is
instance-generator attachment, not an index assignment. This refines the
earlier wording in `MESH_SUBSET_LOD_FIELDS.md`. Likewise section+50 is the
layout; +54 must not be turned into an invented declaration/index owner.

## Canonical pool

The concrete section pool shares the existing `AllocatorListDomain` and
00E188B4 list. Its 38h storage contains the real Win32 critical section at
+0C, recursion+24, slab table+28/count+2C/capacity+30, and first-free+34.
Profile D6319C dispatches trim through B85C90. Construction prepends the
shared list and reserves32 slab pointers. Each1984h slab contains64 slots
of64h, a64-WORD free-index stack at1900h, and WORD count at1980h. Initial
free indices are63..0, so allocation initially returns ascending slots.

Allocation grows the table to2*capacity+2 and publishes values in native
order. A throwing real allocation leaves the lock/recursion as native code
does; no rollback or automatic unlock was added. Return reads current
slot+60, computes signed exact division by100, pushes the index, and updates
first-free under the actual lock. Trimming does not enter that lock: it
frees empty slabs, swaps the last slab, rewrites all64 moved slot IDs, and
retries the same index. Destruction frees slabs without invoking payload
destructors, frees the table, drains positive recursion, deletes the lock,
and unlinks the shared allocator list. Callers must finish section lifetimes
before destroying their canonical pool. Static CRT bootstrap is external.

These paths were independently checked in the section-pool assembly and
installed PE; the separate mesh pool's different dimensions were not used
as evidence. The shared implementation style reuses its allocator/list
boundary, without creating a second material owner or reference counter.

## Publication and destruction

Material B864C0, selected-stream B86500, layout B86650, and generator-binding
B417E0 setters all skip equal identities. Otherwise they publish and retain
the incoming actual+04 before releasing the captured old identity. B85B80
retains its nonnull stream before append/count publication. B86550 releases
then clears visited slots while reloading the live count after callbacks;
it sets count0 only after successful completion. B86390 releases material,
streams, then layout, and deliberately leaves the stream count unchanged.

B865A0 clears selected streams only when an existing layout is present,
then releases that layout. If no selection remains, it retains actual
mesh+64 stream0. It obtains each current stream virtual+24 descriptor and
passes a four-pointer-plus-count stack key to current renderer virtual+40;
the returned owned layout reference is published directly at+50, without
an extra retain. `NativeMeshSectionLayoutServices` requires those real
dispatch services; the existing B2F710 actual layout factory/tree/pool can
supply the renderer implementation. It supplies no dummy declaration,
COM object, material, effect, or invented stream.

B86420 publishes D63194, releases and clears +38 then +5C, clears resources,
and publishes base CEB130. Original state0 unwind at DFB918/CC24F0 performs
only BD30F0 base cleanup. Unvisited references are not retried on failure.
B86690 returns the original address and returns its slot to canonical
010901D4 only after successful destruction when flags&1.

`NativeMeshSectionReference` borrows the same actual atomic+04. The caller
registers one canonical companion in `NativeRenderActualOwners` before
publishing the section to its mesh. Terminal dispatch requires the CURRENT
D63194 profile with BD30E0/B86690 slots, performs scalar deletion/pool return,
then retires the companion through the explicit caller callback. Missing
profiles have no alternate destruction fallback.

## Analysis corrections and verification

Ghidra's `_free` no-return interpretation hid real continuations in pool
allocation, construction, destruction, trim and table cleanup, plus the
inspected table-growth helpers B857A0/B85A30. The report records each call,
continuation and true end. Notably B858F0 ends with RET at B85979 (one byte,
end B8597A), B85C90 at B85D2F (one byte, end B85D30), and B85D30 at B85E5F
(one byte, end B85E60). The actual B86690 destructor ends at B866AD RET4
(three bytes, end B866B0); the following B866B0 zero-return leaf is separate.

Missing functions found read-only: B85980 rewrites64 slab IDs (last B8599A
RET4, length3, end B8599D); B859A0 tests full-free count64 (last B859AD RET,
length1, end B859AE); B866B0 returns zero (last B866B2 RET4, length3, end
B866B5). They remain separate follow-up analysis candidates.

`scripts/build.ps1` passed after seed verification, including both existing
`reconstructed_math` and `native_math_differential` CTests. One ignored
focused probe checked the native constructor,2049 section-pool allocations,
32-to66 table growth,64 moved slab IDs, same+04 canonical terminal disposal,
retained release order, and selected-stream/layout rebuild publication.
Pool/destructor checks are host fixtures, not native differential execution.
No permanent test suite was added. No game execution, renderer output,
device-loss behavior, or original binary ABI replacement is claimed.
