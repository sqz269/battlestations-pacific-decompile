# Native physical-buffer pool acquisition

Four complete original entries now have raw-storage implementations in
`native_physical_pool_acquire.cpp`: B4AC60[316], B48DF0[64] and the two
B4B350/B4B360[10] pool-selection thunks. These are 400 original bytes. They
complete acquisition for the same actual pools used by the existing B49500
slot-return implementation. Pool construction and whole lifetime remain separate.

| Original entry | Original ABI | Source interface |
| --- | --- | --- |
| B4AC60 | ECX initialized pool, EAX raw slot, RET | `acquire_native_physical_buffer_slot_00b4ac60` |
| B48DF0 | ECX slab, stack slab index, EAX slab, RET4 | `initialize_native_physical_buffer_slab_00b48df0`, with unused EDX |
| B4B350 | No arguments; select108FDA8 and tail-call B4AC60 | `acquire_native_index_buffer_slot_00b4b350`, explicit owner context |
| B4B360 | No arguments; select108FDE0 and tail-call B4AC60 | `acquire_native_vertex_buffer_slot_00b4b360`, explicit owner context |

The two ten-byte entries are allocation thunks, not static destructors. Their
new interfaces read the corresponding actual pool pointer from the existing
`NativePhysicalBufferOwnerContext`. They do not read its other fields, create
another pool, initialize it, or infer an allocation policy from a caller's
nominal 2Ch physical wrapper size. The slot is **30h/48 bytes**, of which the
last DWORD at+2C records its slab index.

The pool's accessed 38h prefix contains a real critical section at+C, tracked
depth at+24, slab-pointer array at+28, count+2C, capacity+30 and earliest
available slab index+34. Each 644h slab contains32 raw30h slots,32 free-slot
indices at+600 and a16-bit free count at+640. Slab initialization writes count32,
then interleaves each free index31-i and each slot's slab-index word. Everything
else remains untouched. Its entire64-byte compiled body is byte-identical to
the original, including the three-byte alignment instruction.

Acquisition captures pool+C, enters the real critical section and increments
its depth. CursorFFFFFFFF causes publication of current count into the cursor
before allocating644h. A nonnull result is initialized using the **current**
cursor. If count equals capacity, it computes wrapped2*capacity+2, publishes
that capacity before allocating its low-DWORD byte count, and copies current
slab-pointer entries with unsigned comparisons. A zero computed destination
skips its store. Current old array is freed only if nonnull; publication of the
new array follows the returning free. Appending the captured slab uses current
array/count, then increments current count.

The routine then selects current cursor/current array, decrements the slab's
16-bit free count, and indexes its current free-index array to obtain the slot.
If the slab is exhausted, it reads the next-index/count condition, publishes
cursorFFFFFFFF, and scans only later slab pointers for a nonzero free count.
The pointer-array scan base is captured once; count is reread on its loop.
The selected cursor is published before decrementing the captured lock's depth
and leaving. The result is the raw slot, with no buffer constructor or zeroing.

All integer/address arithmetic has the original DWORD or16-bit wrap. The raw
pool must be initialized and internally readable for the reached operations.
The source adds no bounds checks, overflow policy, null-output repair or
exception cleanup. Allocation uses the existing concrete
`singleton_lifetime_allocate` CRT malloc/current-new-handler retry boundary;
BF55BE is a complete tail jump to BF681B. BF6989 is the complete tail jump to
BF65AC and the existing current CRT free boundary. An allocation exception
leaves the lock held and already published fields intact. This is preserved,
not replaced with lock or allocation rollback. Native CRT exception/heap ABI
and hardware SEH remain outside the source interface.

The original B4ACF1 returning-free call had a saved flow override that hid
the ADD ESP,4 atB4ACF6. The complete316-byte span includes that continuation
and the new-array publication atB4ACF9. Only that call-site flow is repaired;
the existing full body extent and global library annotations are preserved.

The strict main Win32 build, both existing CTests and eight native seeds pass.
Ten guarded Ghidra spans,428 bytes, matched the installed PE. A single ignored
fixture executes all four complete original entries and the current main-library
implementations against the same actual pool storage and concrete CRT/Win32
services. It compares1,182 trace words across both pools, including capacity
growth2to6, old-array free continuation, exhausted-slab scanning and exact
returned-slot reuse through the existing complete B49500 provider. A separate
part of that same fixture compares all1,604 slab-storage bytes after initialization.

Windows had already reserved the original B40000 region in this probe process.
The fixture therefore relocates the original block and applies exactly five
address changes: three critical-section IAT operands and the two fixed pool
addresses. Internal relative calls remain unchanged. The complete400 owned
code bytes are checked after execution against those five declared fixups.
Only the two five-byte CRT allocator/free entry preimages are replaced with
explicit bridges to current complete library services. Original BF55BE/BF6989
tail thunks execute; original allocator internals do not.

The fixture compiles no source providers. Four exact objects from the frozen
actual main archive account for196 mapped COFF sections,557 relocations and80
linked functions. The complete14,883-byte executable section matches its runtime
postimage. The64-byte slab constructor is exactly original; the compiled pool
algorithm is393 bytes and the two new context thunks are8 bytes each. Their
source ABI and code are not binary replacements for the originals. Import cells
are captured and matched to the probe's import table; DLL entry-byte postimages
are not captured or claimed by this fixture.

Allocation failure, allocator callback mutations, malformed pools, native SEH,
full pool initialization/teardown and gameplay are not exercised. The remaining
source order is grounded in full original assembly and compiled code. Names
are descriptive hypotheses. No permanent tests or installed-game files change.
The immutable proof is stored under ignored `local/physical_pool_acquire/`.
