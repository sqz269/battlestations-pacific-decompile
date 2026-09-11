# Cube texture pool static lifetime fixture

The independent review found no defect in the static startup/shutdown source.
The complete original `CD7B80` initializer and `CE0CB0` shutdown callback match
the final production library in one composed scenario through real process
exit: 18,931 matching DWORDs and 33 observation events on each side.

This packet owns only this document and
`reports/native_cube_texture_pool_static_fixture.json`. It changes no
production source, build metadata, shared ledger, Ghidra annotation, permanent
test, or installed game file.

## Reference and production code

| Original function | Exact span, inclusive | Bytes | Native ABI |
| --- | --- | ---: | --- |
| Static initializer | `CD7B80..CD7B95` | 22 | No arguments; EAX actual `_atexit` status; RET |
| Static shutdown | `CE0CB0..CE0CB9` | 10 | No arguments; tailcall `B3E5B0` with ECX=`108DB70` |

All 32 original static bytes remain unchanged in both runtime postimages.
The private reference retains their literal canonical pool and callback
addresses. The complete original `4B46B0` allocator-list dispatcher also runs
unchanged on the native side, using actual shared head `E188B4` and profile
`D61944`. Eleven fresh guarded Ghidra reads, 153 bytes total, match the installed
PE, including the zero-initialized pool/head and initializer-table entry
`CE3518 -> CD7B80`. All eight existing native seed checks match.

The fixture reserves original addresses in its own suspended child before
the loader runs, copies only the verified spans, and uses four explicit entry
bridges:

| Private original entry | Real dependency |
| --- | --- |
| `B3F090` | Final production cube pool initializer with the same actual shared domain |
| `B3E5B0` | Final production cube pool destructor with the same actual shared domain |
| `BF6FF5` | Real `std::atexit` |
| `B3E690` | Final production cube trim on the actual dispatched cube owner |

Each bridge changes five bytes at the dependency entry. There are no other
reference-image changes. Inner original lifetime/allocation/trim bodies are
not executed by this fixture; their full reconstructed implementations are
shared dependencies, with separate native evidence in their own packets.

Both sides link the same frozen primary `bsp_core.linked.lib`, SHA-256
`c8ac226e4373d04226066e0a7205893292ff442719a50c4a609e2c117d8d1d12`.
No production source is recompiled under an observation alias and no production
symbol is replaced. The linker map identifies 16 production providers across
static lifetime, pool lifetime, allocation, trim, allocator-list and allocation
services, plus the two CRT registration providers. Small production helpers
can be inlined; map omission of an inlined helper is not a separate proof of its
original ABI. Twelve captured production source/header files match current
primary files and their recorded hashes.

## Binding, allocation and real exit

The actual canonical pool is writable `38h` storage at `108DB70`. The domain
borrows the actual shared `E188B4` head. An independently initialized second
cube pool already occupies the list before canonical setup. Binding preserves
all canonical pool bytes, that existing head, and the neighbor's links.

The code linked from the frozen library was independently decoded: the call
to the genuine `D61944/B3E690` trim binder precedes stores to both borrowed
canonical pointer variables. At initialization, the real Win32 call receives
the actual canonical section at `pool+0C`, after profile and shared-list
publication.

The fixture makes the first canonical 128-byte malloc attempt return null.
The actual production allocation service invokes the actual CRT new-handler.
That handler traverses the same shared `AllocatorListDomain`, successfully
dispatching both real cube trim bindings before allowing the real allocation
retry. This proves the binding is usable at the allocation point following
native publication. The handler does not replace the pool, allocation service,
trim implementation, or shared head.

After construction succeeds, the observed `_crt_atexit` import receives the
exact original `CE0CB0` pointer or exact production shutdown function pointer.
The observer immediately forwards that pointer to the actual CRT. Successful
registration returns zero, which is the initializer's observed result.

The same scenario then performs:

1. 33 calls to the real canonical allocation adapter, creating two `6C4` slabs.
2. 32 calls to the real canonical return adapter, leaving one slab fully free
   and one containing a live slot with slab index1.
3. Actual global-list trim, removing the empty slab, moving the partial slab
   to index0, and rewriting all 32 slot index DWORDs at `slot+30`.
4. Prepending a real list element, then entering the actual critical section
   twice and retaining depth2 until process exit.

The fixture returns from `main`. Real CRT validators registered before and
after the production callback bracket the actual shutdown through LIFO exit
processing. There is no callback collector and no manual shutdown invocation.
The registered callback frees the remaining slab and pointer table, decrements
depth before each real leave (depth1/0 while actual recursion is still2/1),
deletes the section, restores base profile `D7A0C0`, and repairs the current
predecessor and neighbor links. Own stale links, table/count/capacity/earliest
remain retained as native code requires. The separate neighbor is fixture
cleaned only after validation and observer removal.

## Results and limits

Strict MSVC Win32 fixture compilation passed with `/W4 /WX /O2 /Oy- /MD
/EHsc /fp:strict`. Both real process exits produced identical 75,724-byte
traces: 18,931 DWORDs, 33 events, three real successful pool allocations, three
real frees, one canonical section initialization/deletion, one production exit
registration, and one actual new-handler retry. There are 67 canonical leaves:
65 from allocation/return and two during exit.

All 22 runtime reference postimages and all 12 observer-import postimages
match their declared edits and linker symbols. The six actual service imports
are malloc, free, `_crt_atexit`, and Initialize/Leave/DeleteCriticalSection.
Runtime module headers identify 32-bit CRT and Win32 providers; their disk
hashes resolve WOW64 paths before verification. Observers execute the actual
services except the single deliberately failed malloc attempt.

Snapshots preserve all canonical/neighbor pool words, predecessor links, and
all live or pre-free retained table/slab bytes. Normalization covers explicit
object/allocation pointer identities and initialized section debug, owner-thread
and semaphore identities. Allocated storage is prefilled with `A5` to expose
untouched bytes; this is fixture preparation, not a native zeroing claim.

The native initializer is explicitly called; this does not reconstruct the
entire original CRT initialization dispatcher or registration storage. CRT
registration exhaustion and construction failure before registration are
source-order findings, not scenarios tested here. Original binary replacement
compatibility, logical cube owner construction, game execution and rendering
remain unproved. Exact artifact pins, linked publication instructions and
reproduction commands are recorded in the paired report.
