# Actual material and parameter pools

`NativeMaterialPool` implements the existing `NativeMaterialSlotPool` service,
and `NativeMaterialParameterPool` implements `NativeMaterialParameterSlots`.
Both borrow the actual 0x38-byte pool storage and the shared `AllocatorListDomain`.
There is no second free list, reference count, slab map or allocator registry.
The material owner's 0x110-byte object remains in the same 0x114-byte slot;
only the pool owns the live slab index at +0x110.

| Field or operation | Material | Parameter |
|---|---|---|
| Actual global | F8D3AC | F8D3E4 |
| Allocator table / virtual0 | D5E518 / B18160 | D5E51C / B18500 |
| Slots per slab | 64 | 128 |
| Object bytes / slot stride | 0x110 / 0x114 | 0x84 / 0x88 |
| Slot's slab index | +0x110 DWORD | +0x84 DWORD |
| Slab allocation | 0x4584 | 0x4504 |
| Free-index stack / count | +0x4500 / +0x4580 | +0x4400 / +0x4500 |
| Initialize pool | B17FA0 | B18340 |
| Initialize slab | B17440 | B17510 |
| Allocate slot | B18200 | B185A0 |
| Return slot | B17A80 | B193FA..B19463 fragment |
| Destroy pool | B180D0 | B18470 |
| Table unwind leaf | B17630 | B17670 |

The actual prefix is allocator element +00/+04/+08, 24-byte Windows critical
section +0C, signed recursion word +24, slab pointer table +28, unsigned slab
count +2C, table capacity +30 and first available slab +34. The first-available
sentinel is FFFFFFFF. Win32 static assertions verify every offset and size.

## Construction, allocation and return

The host companion first binds its concrete allocator virtual0 in the existing
domain. Native initialization prepends the SAME element to the shared E188B4
list, publishes its derived table, initializes the real critical section and
recursion word, then initializes the table fields and reserves 32 pointers.
The constructor publishes capacity before allocation. The table is not filled
with null entries; only entries below the live slab count are valid.

Both allocators enter pool+0C and increment the SAME +24. When +34 is FFFFFFFF,
they publish the current slab count there, allocate and initialize a slab, grow
the pointer table to `old_capacity * 2 + 2` when full, append the slab and increment
count. The slab initializers write a descending uint16 free-index stack and the
same DWORD slab index in every slot. They preserve object payload bytes and the
two bytes following the uint16 free count.

Allocation decrements the uint16 count and reads that stack position, giving
ascending physical slots for a new slab. When exhausted, it scans later slab
entries for nonzero free count. Returning a slot pushes its physical slot index
and lowers +34 if needed. Both returns read the current slot slab ID only after
the real lock is acquired. Signed division toward zero reproduces the native
magic-multiply sequences (76B981DB/shift7 for 0x114, 78787879/shift6 for 0x88).
The return increments the live count word after storing the free-stack entry.

The parameter return is exactly the embedded B193FA..B19463 fragment. Its original
input is ESI=parameter slot and EBP=1; the pool comes from F8D3E4. It loads +84 at
B1940B after entering the section. B192F0 already released the real NativeString
name before entering this fragment. This service neither frees that name again
nor clears the parent material's parameter table/count. B192F0 remains owned by
the material packet; no parent function/name record was changed here.

All backing requests reuse `singleton_lifetime_allocate/free`, the existing
BF681B/BF55BE new-handler allocation owner. There is no optional allocation
callback or substitute allocator. The native allocation paths have no unwind
region: allocation failure leaves the lock recursion and early field publications
as observed. The implementation adds no rollback or automatic unlock there.

## Trimming and destruction

Allocator virtual0 trims without taking the critical section. On each empty
slab it frees the allocation, copies the last table pointer into that entry,
decrements count, then rewrites the slab index in EVERY moved slot. It retries
the same entry and finally rescans from zero to publish the first available
slab. Slot addresses and payloads remain unchanged, and pointer-table capacity
does not shrink. Callers must preserve the native exclusion/lifecycle contract;
the service does not add a new lock around shared allocator-list traversal.

Pool destruction publishes the derived allocator table, frees each live slab
and then the pointer table, drains positive +24 recursion with matching
`LeaveCriticalSection` calls, deletes the critical section, publishes the base
D7A0C0 table and unlinks the actual allocator element. It leaves stale native
table/count/link fields untouched where the original does. It does not call
material or parameter destructors: every payload owner/name must already be gone.

Construction has native unwind states for base-list unlink, critical-section
destruction, and table free. The C++ implementation preserves those actions for
exceptional construction with `__try/__finally`. The maps at DF4738/DF4774 and
leaves CBC4D0/CBC4D8/CBC4E3 and CBC500/CBC508/CBC513 establish this ordering.
B17630/B17670 free only the table pointer, preserving its fields.

## Integration boundary

Pass one initialized `NativeMaterialPool` and one initialized
`NativeMaterialParameterPool` to `NativeMaterialDestructionAccess`, along with
the existing canonical actual-owner lookup and NativeString storage. The same
material pool is used by `allocate_native_material_slot_00b18780`. Parameter
allocation is available through `allocate_slot_00b185a0`; its caller must still
initialize the real parameter payload. The pools never construct a
`MaterialCloneState`, shader, draw queue or widget retention token.

Initialization/destruction are explicit. Destroying a host companion does not
implicitly destroy the native global or unlink it. Actual storage, companion,
and shared allocator-list domain must outlive their callbacks. Static startup
thunks CD78D0/CD78F0 and atexit adapters CE0BF0/CE0C00 remain external routing
contracts; this packet does not create competing process-global pool instances.

## Evidence and validation

Every live query used `bsp.py ghidra`, whose client verifies project
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe` before querying.
Ghidra was read-only. Names are repository hypotheses, and previous saved names/
comments are retained in `reports/native_material_pools.json`.

The saved listing omits important free continuations. Disk decoding, checked
against the complete saved bytes, establishes:

- B18059..B1805B, B183F9..B183FB and B18296..B18298: `ADD ESP,4` before continuing.
- B180ED..B180F7 and B1848D..B18497: increment slab index, restore stack, compare
  live count and loop; B18105..B18107/B184A5..B184A7 restore the stack after table free.
- B18186..B181BF and B18527..B18560: all slab compaction/index-rewrite work.
- B1763C/B1767C: `POP ECX` before the actual return.

Other small gaps after unconditional jumps are unreachable alignment, not missing
behavior. All 13 standalone functions exist; B193FA is a fragment of B192F0.
The constructor EH handler entry labels CBC4EE/CBC51E have no Ghidra function,
so their raw bytes and unwind metadata were inspected without creating one.
Seventeen complete code, fragment, unwind and table spans match the installed
PE; exact bounds, ABIs, hashes, and listing hazards are in the report.

The MSVC Win32 Release build passed with `/W4 /WX`, both existing CTests passed,
and all eight native seed spans matched. One ignored fixture passed for both
pools' 33-slab growth, shared-list trimming, all moved slot indices, preserved
payload/address, LIFO return, actual-lock parameter return and explicit teardown.
It also exercised the SAME material +04 through retain, final-zero companion
dispatch, real NativeString destruction and both actual pool returns. The
fixture did not execute original pool code or test allocation-failure unwind.
The initial inherited ClipBox argument mismatch was resolved by consuming the
integrator's existing dependency commit; no unrelated source repair was added.

These are new C++ interfaces, not binary replacements or game/render validation.
No permanent tests are added.
