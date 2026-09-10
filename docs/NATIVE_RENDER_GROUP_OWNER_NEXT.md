# Native render-group storage and model ownership frontier

The 4Ch group allocation has a concrete constructor and ordered destruction
contract. Its remaining ownership dependency is the actual model and binding
terminal path. `OwnedInstanceGroup` currently represents that behavior with
host containers and shared owners; it cannot be reinterpreted as this storage.

The companion report records fresh Ghidra/installed-PE span comparisons, exact
extents and source hashes. Every live query verifies `C:/Users/sqz269/bsp.gpr`
and `/battlestationspacific.exe`. This packet changes no C++ or Ghidra state.

## Actual allocation and construction

`00B1DFF0` allocates **4Ch bytes** before `00B1D6F0`. Only the first 3Ch bytes
are initialized by that constructor. Do not zero or assign meanings to the
remaining 10h bytes merely because the allocation is larger.

| Offset | Established field |
| --- | --- |
| 00h | Intrusive binding pointer |
| 04h, 08h | Owned pooled-string length/data |
| 0Ch, 10h | Two per-category counts |
| 14h, 18h | Two borrowed output-entry pointers |
| 1Ch, 20h | Two model node pointers |
| 24h, 30h | Two 0Ch borrowed entry-pointer array headers |
| 3Ch..4Bh | Unwritten allocation tail; meaning unresolved |

`00B1D6F0` receives the actual group in ECX, returns the same address in EAX,
and `RET`s without stack arguments. It zeros binding and string fields,
constructs the two arrays through the actual `00B1C4F0`, then writes zero in
the order 20h, 1Ch, 10h, 0Ch, 18h, 14h. `00B1C4F0` is an unrecognized 13-byte
function: ECX is the actual array; it clears all three words and returns that
address in EAX. Its bytes end at `00B1C4FD`.

The source-array specialization is independently ready: `00B1C500` grows only,
clamping requested capacity to at least one; `00B1C770` clears newly exposed
cells and changes the count without releasing entries; `00B1D1D0` resizes to
zero and frees data. The data/capacity fields remain stale after destruction.
These have the same storage shape as the separate command group-pointer
arrays, but they are different original functions and must retain their own
addresses and evidence.

`00B1CA50` takes ECX = destination cell, EDX = source cell, returning the
destination in EAX with `RET`. It captures both pointers before publication.
On identity change it publishes the incoming pointer, increments incoming+04,
then decrements the captured old+04 and calls its **current** virtual zero
entry if the new count is zero. Same identity skips all stores and counts.
The terminal companion must describe that actual count; a second host count
would change the operation.

## Destruction and unwind

`00B1D760` receives ECX = actual group and `RET`s without stack arguments.
It performs these operations in order:

1. Capture binding+00h, decrement its actual +04, dispatch its current
   virtual zero only at zero, then clear group+00h after the callback.
2. Reload model+1Ch, call `00B6DFA0` if nonnull, and clear it after return.
3. Reload model+20h and do the same. The first model callback can change
   what the second load observes.
4. Destroy source arrays in reverse order, 30h then 24h, using `00B1D1D0`.
5. Reload name data+08h, capture current length+04h plus one, then return its
   actual storage through the shared sized pool. Name fields are not cleared.

`00B6DFA0` unlinks the actual parent/root relation and invokes current
virtual+18h. The call is a real logical release, not an unconditional scalar
free. Retained model references may leave the model alive afterward.

Constructor FuncInfo `00DF4E90` maps state 0 to pooled-name destruction
`00CBCA10`; the vector constructor handles already-constructed arrays itself.
Destructor FuncInfo `00DF4EC4` maps state 1 to reverse-array destruction
`00CBCA3B`, then state 0 to name destruction `00CBCA30`. Thus an exception
from a binding/model release cleans storage, but does not release remaining
binding/model owners. State -1 precedes normal final name release. These are
static native EH facts; host cleanup is not binary-compatible EH validation.

`00B1D8E0` calls this destructor, ordinary-frees the allocation only when
flags bit 0 is set, then returns the original address and `RET 4`. The saved
pseudocode's `extraout_EAX` following `_free` is incorrect: the instruction
at `00B1D8F8` restores EAX from the saved original pointer.

## Ready model-pool packet

The actual model pool is independent of model construction and scene behavior.
It is dispatched as `native_model_pool`, with only a dedicated header, source,
document and report. The primary owns CMake, ledgers and Ghidra annotation.

| Property | Original evidence |
| --- | --- |
| Canonical storage | 01090054h, actual 38h pool |
| Profile/current virtual zero | 00D62DD0 / 00B74C60 |
| Slot | 188h bytes; model bytes 000h..183h, pool ID at 184h |
| Slab | 3144h bytes, 32 slots |
| Free index stack/count | WORD[32] at 3100h, WORD count at 3140h |
| Initialize slab | 00B74450 |
| Construct / acquire / return | 00B74B80 / 00B74D00 / 00B74750 |
| Trim / destroy / table unwind | 00B74C60 / 00B74690 / 00B74530 |
| Canonical acquire / return | 00B74EB0 / 00B748C0 |
| Static initialize / shutdown | 00CD7F00 / 00CE0E50 |

Use the existing shared allocator-list domain, native CRT allocation service,
and actual Win32 critical section. Allocation publishes the chosen slab ID
before allocating a slab; capacity growth publishes `2*capacity+2` before
allocating the replacement table. The native acquisition path has no local
exception cleanup or automatic unlock.

Trim's missing post-free continuation `00B74C86..00B74CC0` moves the last
slab into the freed entry, decrements count, rewrites **all 32** moved slot
IDs, and retries the same position. Constructor, acquire and destructor also
contain omitted continuations after incorrectly classified no-return frees.
`00CD7F00` is currently an unrecognized 22-byte function; it constructs the
same static pool and registers `00CE0E50` with atexit. The inventory label
calling `00B74EB0` a static destructor is incorrect: it acquires a raw slot.

## Actual generated-model owner still required

`00B75030` constructs a 184h object in that 188h slot. After the existing
actual node constructor, it installs profile `00D62DE8`, initializes +174h,
+178h, +17Ch, +180h, then writes its pose fields at +08h..+2Ch in the
observed SSE order. The +184h slab ID survives unchanged.

The current `construct_native_node_00b6f5a0` host guard requires 1F0h even
though its complete body writes only the 174h prefix. That was the original
node-pool slot size; it cannot be imposed on the smaller original model
caller. Lower the reusable prefix guard to `sizeof(NativeNodeStorage)` and
keep alignment validation. The original node pool itself remains 1F0h.

Model profile +00h/+04h resolves `00BD30E0 -> 00B75290(1)`. `00B75290`
calls `00B750C0`, then returns storage through canonical pool+`00B74750`.
`00B750C0` releases/clears current +174h, then reloads/releases/clears +180h,
then runs the complete actual node destructor `00B6F440`. Logical release
remains current +18h = `00B6F310`; scene removal is +54h = `00B6EE10`.

The model's current type predicate and other model-specific virtual entries
need their own evidence before a complete native owner/reference is bound.
Do not substitute camera/light type dispatch, the existing typed geometry
`shared_ptr`, or an empty-group fixture for a populated model lifetime.
Native command composition must wait for those actual owner contracts.

This discovery is exported and byte-checked. It is not a new C++
reconstruction, build result, native differential fixture, or game validation.
