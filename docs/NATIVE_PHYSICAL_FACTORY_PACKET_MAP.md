# Physical factory and provider construction packet map

This is a read-only discovery of BED990, BF4DF0, BF34D0 and BF4D30, based on
the current `bsp` project, `/battlestationspacific.exe`, and the installed PE.
No source implementation, Ghidra definition, annotation, runtime test or library
port is supplied. Names describe observed behavior and remain hypotheses.
`reports/native_physical_factory_packet_map.json` preserves byte spans, ABI,
call-site rows, dependencies, raw tails and validation. The earlier
`NATIVE_VFS_MANAGER_PACKET_MAP.md` was a starting point, not a readiness claim.

## Owned bodies and incoming uses

All ends are inclusive. Coverage describes analysis, not reconstructed source.

| Entry and end | Bytes | Original ABI | Coverage |
|---|---:|---|---|
| BED990..BEDA5D | 206 | no consumed inputs; EAX factory; RET | complete analysis |
| BF4DF0..BF4EAA | 187 | ECX factory ignored; stack system/virtual raw string headers; EAX provider/null; RET8 | complete analysis |
| BF34D0..BF360B | 316 | ECX initialized raw38h pool; EAX raw3Ch slot; RET | complete analysis, including post-free gap |
| BF4D30..BF4DC7 | 152 | ECX raw provider; stack system header/full DWORD whose low byte is copied; EAX same provider; RET8 | complete analysis |

The four bodies contain **21 CALL instructions: 16 direct and 5 imports**, with
no tail jumps to other functions. BED990's sole direct incoming call is BEDA90
inside BEDA60; that caller next registers EAX through BE0660 at BEDA98. BF34D0
and BF4D30 each have two incoming calls, both read in full inside BF4DF0.
Ghidra reports no BF4DF0 incoming xrefs: the installed D68CFC factory table's
+4 word at D68D00 is BF4DF0. The full BDB040 listing establishes BDB078 as a
current factory+4 virtual call with unchanged system/virtual header pointers,
RET8, and first-nonnull-result selection. This is a qualified table resolution;
it does not recover every possible indirect caller.

## Factory publication and secondary lifetime interface

BED990 first captures current 109DBE8 and returns that captured nonnull value.
On the slow path it gets the canonical manager, captures manager+10, optionally
enters that actual critical section and increments section+18, then rechecks
109DBE8. It allocates eight bytes and writes secondary+4=D68CF4, primary+0=D68CFC,
then secondary+4=D68CF8 before publication. A null allocation publishes null.

The registration target is a critical ordering detail: BEDA1D reloads the factory,
BEDA26 adjusts nonnull to factory+4, and BEDA2D pushes that captured pointer
**before** the second 415350 call at BEDA2E. BEDA35 registers the captured
secondary pointer with the newly obtained manager; it does not reload the
factory afterward. Normal release uses the originally captured section. BEDA49
reloads current publication after LeaveCriticalSection for the slow return.

FH3 data E02018/E02010 declares state0 -> CC74C0 -> 411EE0 over the captured
guard at EBP-14. The state is armed before the publication recheck. Allocation
or registration exceptions invoke guard cleanup; published factory storage is
not rolled back on registration failure. The handler CC74C8..CC74D1 is an
undefined raw MOV-EAX/JMP-to-runtime span, not part of BED990.

D68CF8's virtual0 is undefined thunk BED910..BED917: `SUB ECX,4; JMP BED950`.
BED950..BED989 clears 109DBE8, writes secondary CE3818 and primary CFE9F4,
conditionally frees the complete original address on flags bit0, and returns
that address with RET4. Its hidden BED981..BED983 is `ADD ESP,4`. A shutdown
dispatcher must accept the registered secondary pointer and adjust it before
deletion. Raw base/standalone bodies BED8C0..BED8E8, BED8F0..BED906 and
BED920..BED941 also exist; they are not extra callees of BED990's inline writes.

Existing raw `get_native_singleton_manager_00415350`,
`register_native_singleton_object_00bd0c30`, and
`destroy_native_singleton_guard_00411ee0` are concrete reusable dependencies.
Use the application's current publication and manager domain. Do not introduce
a competing typed owner or assume that registering factory+0 is equivalent.

## Static provider pool and allocation ordering

BF34D0 is not a simple allocation thunk. It consumes the actual initialized
38h owner at 109DBF0: allocator-list identity/links at +0/+4/+8, real critical
section at +C, depth+24, block pointer array+28, count+2C, capacity+30, and first
available block index+34. BF3250 is the producer: it links into shared E188B4,
sets D68E68, initializes the section and zero headers, sets available=-1, and
reserves an initial 32-pointer/80h array. The canonical `AllocatorListDomain`
already represents this shared list; `native_physical_pool_acquire.hpp` instead
describes renderer buffer pools, whose 30h slots/644h blocks are incompatible.

The undefined static entry CD9010..CD9025 calls BF3250 with ECX=109DBF0, then
registers CE10F0 with the existing CRT atexit service (POP ECX cleanup). CE10F0
is a ten-byte MOV-ECX/JMP-to-BF33A0 tail thunk. This pool is initialized at
static startup, not lazily by BED990. Explicit source startup/teardown must
preserve that relationship; the pool is not another singleton registration.

Each 1F4h block contains eight 3Ch slots, reverse WORD free indices at+1E0,
WORD free count+1F0, and untouched final padding. BF2D50 writes count8, indices
7..0, and each slot's +38 block-index DWORD; all other slot bytes remain dirty.
BF34D0 locks unconditionally, increments depth and, if available=-1, publishes
current count as available before allocating/constructing a block. Vector growth
publishes DWORD-wrapped `capacity*2+2` before allocation, copies against current
count, frees old array, then publishes the new pointer. BF3566..BF3568 is the
missing `ADD ESP,4`; the decompiler's return after free is false.

After append, acquisition decrements a block's WORD free count and returns the
indexed 3Ch slot. Exhausting the block scans forward for a nonempty free stack,
or leaves available=-1. Both exits decrement the captured depth and unlock.
There is no EH frame or exceptional unlock in BF34D0; adding unconditional
rollback or unlock would change the native allocation-failure path.

BF2FC0 returns a raw slot using its preserved +38 block index and signed
division of its block-relative offset by 3Ch; it pushes the WORD slot index,
increments free count, and lowers available when needed under the same lock.
BF3200 is the ECX-slot -> stack-slot/global-pool wrapper used by both BF4DF0
constructor-failure funclets. BF4DD0 later calls full provider destructor BF4C70
and returns the slot through BF2FC0 only when deleting flags bit0 is set.

The pool's published D68E68 virtual0 is BF3430, which frees wholly unused
blocks. Its hidden BF3456..BF348C continuation swaps the final block pointer
into the vacated cell, decrements count, rewrites **all eight moved slot+38
indices**, and reprocesses that position before recomputing available. Its
other gaps are unreachable alignment bytes. BF33A0 frees all blocks and the
pointer array, drains positive lock depth, deletes the section, and unlinks the
same E188B4 element; its hidden BF33BD..BF33C7 loop continuation is essential.
BF3250 EH chains pointer-array free BF2E30, section destructor 402F70 and
allocator-list unlink 403970. The report separates these tails from CALL rows.

## Provider construction and unresolved cleanup

BF4DF0 rejects only zero system length or a final byte other than backslash.
It does not normalize the root or query disk existence. It compares the virtual
header against CFF208 `persistent_data` using complete existing 425850
case-insensitive comparison, then allocates through global-pool BF34D0 and calls
BF4D30 with low-byte flag1 or flag0. It preserves null allocation branches.
Both constructor states in E02A98/E02A88 independently unwind through
CC7CD0/CC7CD8 -> BF3200, returning the raw allocation to the same pool.

BF4D30 first calls external shared BB5590 (ECX this, stack system header, RET4).
That body writes CEB130, refs+4=1, D641A0, zeroes/copies actual root+8/+C, then
device+10=-1. It copies the system name, not the virtual prefix. Its self-header
case is significant: destination is zeroed before the equality branch.
BB5590 and matching BB5380 cleanup remain owned by the FileStore packet.

The physical constructor then writes D69168; zero pending header+14/+18/+1C;
zero cache header+20/+24; copy incoming low byte to+28; and construct index
header+2C with existing BDABA0. The returned raw20h node is published at+30,
nil byte+1D becomes1, its parent/left/right become self, and count+34 becomes0.
Provider+2C, padding+29..2B, and pooled slot metadata+38 are untouched. The
4254B0 diagnostic call is a verified RET with caller ADD ESP,4.

FH3 E02A64/E02A4C has state0 -> BB5380, state1 -> BF4B80(this+14), state2 ->
41DD20(this+20), chained 2->1->0->-1. Normal code writes state0 after base and
state2 before BDABA0; state1 is reached during unwinding. BF4B80's saved body
stops after free at BF4B91, but raw BF4B92..BF4B96 is ADD ESP,4/POP ESI/RET.
Its full contract calls BF3ED0(queue,0), then frees the current backing pointer.
Do not label it a no-op because this constructor initially zeroed the queue.

BB5380, BF4B80, named BF3ED0, BF4C70 and the index cleanup BE0C30 are not
complete actual-storage implementations in this checkout. BF3ED0 also needs
BF3DA0 reserve and BF3880 name destruction; their complete behavior was not
audited here. Ordinary provider deletion therefore remains a separate lifecycle
packet, including pending-record ownership and tree erase. The older typed
`PhysicalDirectory` and `create_physical_directory_00bf4df0_fragment` cover an
empty-index/shared-ownership domain and cannot satisfy these raw dependencies.

## Smallest useful implementation packets

Ready means the inspected contracts support beginning that bounded source packet;
it does not mean application startup or mixed-owner shutdown is already wired.

1. **Ready: factory singleton and secondary deletion.** BED990, BED910 and
   BED950, exact secondary pointer lifetime dispatch, existing raw singleton
   services/guard and real CRT allocation/free. Keep the raw-handler and source
   EH identity limitations explicit. No provider pool or FileStore changes.
2. **Ready: pool acquisition and return over supplied initialized storage.**
   BF2D50, BF34D0, BF2FC0, BF3200. Preserve raw fields, captured lock, wrapping
   capacity, no exceptional unlock, and metadata+38. This yields complete
   primitive bodies but requires the following lifecycle packet before startup.
3. **Ready after a focused lifecycle ABI review: same pool startup, trim and
   teardown.** BF3250, BF3430, BF33A0, BF2E30, CD9010 and CE10F0; consume the
   existing allocator-list domain and section cleanup. Bind actual BF3430 before
   publishing the element. Coordinate the E188B4 domain and CRT shutdown order;
   do not fabricate another registry. All operational bodies and hidden spans
   are identified here; full constructor-unwind provider ABI proof remains.
4. **Blocked composition: BF4D30 and BF4DF0.** Reuse completed packets above,
   external BB5590/BB5380, existing BDABA0 and 425850, and complete pending
   cleanup BF4B80/BF3ED0 dependencies. Add BF4C70/BF4DD0 and index teardown before
   claiming usable owned providers. BDB040/BE0660 manager insertion and BEDA60
   startup composition remain separately owned; none becomes complete here.

No permanent test or build is needed for this doc/report-only discovery.
All byte-match claims refer to the captured installed PE and saved Ghidra bytes.
The captured 32 code spans total 1,908 bytes. Report-call validation checked
18 numeric rows with zero failures, including the qualified indirect consumer;
the five operating-system import rows have independent import-directory evidence.
The report-call checker covers ordinary saved-body CALL rows; raw undefined
CALL sites and unconditional JMP tails are recorded separately, without passing
them off as checked calls. No source, ABI, fixture or game validation is claimed.

## AQ parent integration, 2026-09-12

Correction from docs/NATIVE_PHYSICAL_FACTORY_PRIMITIVES.md: seven factory publication, adjustment/deletion and provider-pool primitive bodies now have source reconstructions and bounded native comparisons. The full pool initialization/link/trim lifecycle and complete physical-provider construction are still dependency boundaries. This closes only the corresponding primitive entries of this discovery map.

## AR parent integration, 2026-09-12

Correction from docs/NATIVE_PHYSICAL_PROVIDER_POOL_LIFECYCLE.md and docs/NATIVE_PHYSICAL_PENDING_RECORDS.md: actual physical-provider pool initialization, trim, teardown, table free and static lifetime are reconstructed, as are pending-record copy/name cleanup and queue reserve/resize/destruction. These close the corresponding dependencies of this discovery map. Complete physical-provider construction and deletion remain open; the normal deletion path still depends on full range erase and balancing, so readiness is not inferred from named helpers alone.
