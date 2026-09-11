# Native cube-texture raw allocation

The five complete original functions below operate on the actual cube pool
`0108DB70`. These new MSVC Win32 source interfaces borrow actual storage and
do not reproduce the original calling conventions.

| Address | Complete bytes | Native ABI | Source operation |
| --- | ---: | --- | --- |
| `B3D2A0..B3D2E0` | 64 | ECX slab; stack index; EAX same slab; RET4 | Initialize raw slab |
| `B3F170..B3F2A0` | 304 | ECX pool; EAX raw slot; RET | Allocate slot |
| `B3D940..B3D9A5` | 101 | ECX pool; stack slot; RET4 | Return raw slot |
| `B3F2C0..B3F2CA` | 10 | Replace ECX with `108DB70`; tailcall allocator | Canonical allocation adapter |
| `B3DCE0..B3DCEC` | 12 | ECX slot; select canonical pool; RET | Canonical raw return adapter |

The canonical allocation entry is currently misnamed
`CG_static_dtor_stub_00b3f2c0` in saved analysis. The return adapter is reached
by constructor unwind actions `CBD388` and `CBD530`; it does not destroy an
owner before returning the slot. The source canonical adapters take an
explicit actual pool pointer and add no private global pool.

## Storage and ordering

The pool is 38h bytes, with a real Win32 critical section at +0C, recursion
depth +24, slab-pointer table +28, count +2C, capacity +30 and earliest-free
index +34. A 6C4h slab has 32 slots of 34h bytes. Each slot preserves 30h owner
bytes and holds its DWORD slab index at +30. WORD free indices start at +680,
WORD free count is at +6C0, and the final WORD at +6C2 is padding.

Slab initialization writes free count 32 first, then writes each free index
`31-i` before that slot's slab-index DWORD. It preserves all owner bytes and
the final padding WORD. Assembly establishes the returned slab pointer despite
the incomplete saved signature.

Allocation enters the actual section before incrementing the current depth.
When earliest is `FFFFFFFF`, it publishes the current count as earliest and
allocates 6C4h through the existing actual CRT service. A nonnull raw block is
initialized with the then-current earliest index. If current count equals
captured capacity, publish unsigned wrapped `capacity*2+2`, allocate its wrapped
DWORD byte size, copy each current old-table word while reloading count, free
the current old table, and publish the captured replacement. `B3F206` is the
returning-free `ADD ESP,4` omitted by the current Ghidra flow; `B3F209` continues
with replacement publication. No capacity clamp, zero filling or size repair
is added.

Append the captured slab at the current table/count and increment the current
count after the store. Capture current table and earliest to select a slab,
decrement its WORD free count, reread that WORD, and use the selected WORD
index to compute the 34h slot address. If no slots remain, capture the later
count comparison before publishing `FFFFFFFF`, then scan the captured table
cursor against current count. Both the ordinary and exhausted-scan return
paths decrement current depth before leaving the real section.

Raw return enters the section, increments current depth, captures slot+30 and
the selected current table entry, and divides the signed low32-bit slot-minus-
slab displacement by 52 with truncation toward zero. Native assembly uses
`IMUL 4EC4EC4F`, `SAR EDX,4`, and sign correction. It writes the truncated WORD
index at `680 + old_free_count*2`, then reloads and increments the current
free-count WORD. The distinction matters when that store aliases free count.
It lowers earliest using an unsigned comparison, decrements depth and leaves.

The raw allocator has no exception frame. A real CRT allocation/new-handler
exception retains the entered section, depth and all preceding publications;
there is no raw-slab rollback. The new source preserves that behavior. The
input domain requires valid backing for every reached field/cell and a real
initialized section. A real new-handler that traverses the shared allocator
list requires the genuine `D61944/B3E690` cube binding before publication. Pool
initialization, that trim implementation, owner construction/destruction, and
list startup are separate packets.

## Verification

Eleven fresh guarded Ghidra ranges match the installed PE, including all
491 owned body bytes, three CRT service entry prefixes, actual lock IAT cells,
the `D61944 -> B3E690` profile word, and the canonical pool's image storage.
Every query verifies `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`; the installed PE is read-only.

One retained private fixture executes all five complete native bodies at their
original addresses in a new child process. Its original address range is
reserved before that child's loader initializes heaps. Three entry bridges
delegate actual CRT object allocation, table allocation and free; two IAT
bindings observe and forward real Win32 Enter/Leave operations. Source
observation aliases forward unchanged current allocation/free and real lock
operations. Runtime postimages verify that these five binding sites are the
only changes: all 491 owned instruction bytes remain unchanged.

The original/current traces match **39,664 DWORDs in 59 events**, including
all live allocation bytes with normalized pointer identities:

| Focused path | Matched words | Events |
| --- | ---: | ---: |
| Growth, returning free, both scan exits, canonical allocation and return | 30,786 | 30 |
| Actual CRT allocation failure and retained lock/publications | 2,872 | 10 |
| Signed negative return displacement and free-index/free-count alias | 5,796 | 12 |
| Allocation WORD free count aliases pool DWORD depth | 210 | 7 |

The failure is a real `malloc(FFFFFFF8h)` failure followed by the actual CRT
new handler returning zero and the existing service throwing `bad_alloc`.
The handler sees depth 1, count/earliest `1FFFFFFE`, and capacity `3FFFFFFE`.
A second thread fails `TryEnterCriticalSection` both inside the handler and
after the exception reaches the caller. The unpublished real slab is still
allocated at that point. The fixture releases retained resources only after
recording and checking the native failure state. It does not invoke shared
trim from the new handler or provide a substitute trim implementation.

The alias/negative-displacement inputs use valid backed raw storage to verify
unchecked native arithmetic; they are not claims about ordinary owner use.
The fixture supplies raw field postimages and real initialized sections; it
does not claim to execute the separate cube-pool initializer. A separate full
**1,732-byte slab comparison** checks the returned pointer, every owner byte,
all slot indices and both final padding bytes from identical nonzero preimages.

The strict Win32 whole build, both existing tests and all eight native math
seed checks pass. Final compiled source has no EH handler and reproduces the
native signed-division sequence. No new repository test suite was added.

The audit is `reports/native_cube_texture_pool_allocate_audit.json`. Retained
local reproduction uses `prepare_cube_allocate.py`, `build_cube_allocate.ps1`,
`build_cube_allocate_check.ps1` and `write_cube_allocate_audit.py` under `local/`.
Conditional build registration is private to `local/cube_allocate_sources.cmake`.
The primary integrator owns shared CMake, ledger, names, saved Ghidra flow
repair at `B3F201`, comments and export refresh. The canonical packet is
`native_cube_texture_pool_allocate5`; its existing lease identifier retains
the earlier suffix `allocate4` and covers all five addresses.

Status: complete source reconstruction, strict build tested and focused full
original-body fixture tested. Names remain descriptive hypotheses; original
binary ABI, full cube-owner/pool startup, and gameplay are not validated.
