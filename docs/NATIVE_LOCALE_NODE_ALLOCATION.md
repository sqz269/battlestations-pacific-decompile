# Qualified raw locale node-slot allocation

`allocate_native_locale_node_00a9f0b0(owner, frame)` is a dormant ordinary
MSVC Win32 C++ binding of the already reconstructed game-owned allocator.
Original A9F0B0 receives the actual pool in ECX, returns the slot in EAX and
uses plain RET. This API is not that register/stack ABI, FH3 or fault contract.
No startup caller, new Native body credit or vendor body is supplied.

The frame's pure constructor fixes one `GameNativeLocalePoolProcess` identity.
The function consumes its one attempt before borrowing that SAME owner's
healthy `pool_storage_00e1768c()`. It never initializes a pool implicitly.
The published owner's E188B4 list/profile/heap are retained unchanged. Native
effects start only after the actual pool and section+C are saved in the frame.
The frame is noncopyable/nonmoving, has no native-cleanup destructor, and
exposes only copied diagnostics and a private-state-gated normal result.
Rebinding, reuse and caught same-frame reentry poison it before diagnostics
can throw; an outer call cannot overwrite failure with a normal receipt.
Saved `normal_returned` is historical, not permission after later rejection.

## Native schedule retained

Enter the actual section, then DWORD-RMW depth+24. If CURRENT firstfree+34 is
FFFFFFFF, store CURRENT count+2C there BEFORE allocating 7804h bytes. Capture
the actual allocation immediately. If nonnull, call the existing host-free
`cc13_sweepC::string_map_node_pool_init_block_00a9e970` with CURRENT firstfree
read AFTER allocation. Normalize its void Source result to SAME supplied
block only after normal return. It writes WORD freecount+7800=400 first,
then 1024 reverse WORD indices at +7000 and DWORD slab indices at node+18,
stride1C. Node payload/key/wide/next bytes and padding are not initialized.

Read capacity+30 then CURRENT count+2C. On equality, publish wrapping
`2*capacity+2` BEFORE allocating the wrapping capacity*4 table. Capture the
returned table immediately; its destination cursor is retained while each
copy reloads CURRENT source table and CURRENT loop count. Test each computed
destination address, not a made-up overall allocation success condition.
Capture CURRENT old table, free it if nonnull, then publish captured new table.
Append at CURRENT count/table computed address if that address is nonzero;
increment CURRENT count regardless. There is no progressive repair/rollback.

Reload CURRENT firstfree/table/block. Actual WORD-RMW freecount-- is followed
by a real WORD reload and captured free-index read; form block+index*1C. If
exhausted, read CURRENT firstfree+1 and compare CURRENT count BEFORE writing
FFFFFFFF. Capture the scan table cursor ONCE, advance it by4, reread count on
each continuation, and choose the next nonempty block without wrapping.
This corrects the old SweepC2 spelling's table reread inside that scan while
leaving that public body/API untouched. Raw-width assembly accesses preserve
wrapping address/RMW behavior without C++ signed-overflow/container casts.
Normal completion decrements depth, calls real Leave, records its normal
return, checks sticky private state, then issues the private normal receipt.

## Providers and failure custody

Enter/Leave are actual Win32 imports. Both real allocations use
`singleton_lifetime_allocate({object, bytes, bytes})`; table release uses
`singleton_lifetime_free`, matching the canonical owner's current heap.
No broad `SweepC2Host`, unrelated virtual stubs or injectable callbacks exist.
The current malloc/_callnewh/throw/free domain is qualified Source behavior,
not Original CRT/new-handler/import ABI equivalence. The native NULL slab
path continues into table/current-block accesses: it is not graceful NULL
success. This helper preserves that conditional flow; its real allocator
normally returns nonnull or throws. Invalid/null backing and hardware faults,
SEH/resumption and FH3 cleanup equivalence remain outside qualified success.
The reused E970 currently spells its WORD/DWORD stores through `at<T>&` into
raw allocated bytes; this dependency retains its existing MSVC storage/lifetime
qualification. This packet neither repairs that old helper nor claims a new
standard C++ object-lifetime proof for it. The new allocator uses raw accesses.
Root's separately owned E970 byte-write correction is a coupled prerequisite
for raw-access closure; its future result is not inherited by this freeze.

Every escaping C++ failure only latches metadata and rethrows. There is NO
unlock, free, count/capacity restoration, rollback or retry. Failure may leave
the section held, depth advanced, firstfree/capacity published and a real
unpublished slab/table retained only by the frame. Keep that whole frame,
pool/list/section/host/import/heap domain and all actual backing alive, and
do not infer safe reuse/trim/process-exit cleanup from startup health. The
canonical atexit callback is not a recovery service. Future caller integration
must close this obligation before it can become active; no caller is added.

Caller supplies disjoint stable frame/native/provider backing, external
serialization of pool/list/trim and its common-domain lifetime window, and
excludes unsupported reentrant teardown, arbitrary alias mutation/import
replacement and destruction of the frame during callbacks. A returned slot
proves storage production only. Payload construction/borrowers/737CE0 cleanup,
736090 return/map insertion, manager loading and application startup remain
separate. Source-ready/runtime/native-ABI/game/production claims remain false.

Evidence: saved A9F0B0 assembly F0B0..F1EB and E970..E9B0, existing SweepC2
allocator and host-free SweepC initializer, canonical pool c1e76db25, and
`reports/native_locale_node_allocation.json`. The only validation is the
recorded strict object compile; no link, test, probe, executable or game run.
