# Native render pointer-slot sort

`src/native_render_pointer_slot_sort.cpp` reconstructs the complete pointer-slot
sorting family rooted at `00B1DCE0`, plus the raw unsigned-key predicate at
`00B51B00`. It borrows and mutates the caller's actual four-byte pointer cells.
There is no vector copy, semantic entry overlay, allocation, reference-count
change, or queue acquisition. Entry pointers passed to the comparator are the
values currently loaded from those cells.

The original complete 11 functions total 1,506 bytes. A fresh read of saved
`C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, matched every byte
against the installed PE before this packet's native execution. Project/program
checks ran through the repository Ghidra query interface. All current names,
prototypes and comments were captured; this worker made no Ghidra or shared
ledger changes. The address-named C++ helpers and existing descriptive Ghidra
names are hypotheses, not recovered source symbols.

## Native interfaces

| Address | Reconstructed operation | Original inputs and stack cleanup |
| --- | --- | --- |
| `00B1DCE0` | Introspective sort | ECX first slot, EDX one-past-last; stack signed ideal and comparator; RET8 |
| `00B1D420` | Insertion sort | ECX first, EDX last; stack comparator; RET4 |
| `00B1C210` | GCD-cycle rotation | ECX first, EDX middle; stack last and two unread words; RET0Ch |
| `00B1D280` | Equal-band partition | ECX output pair, EDX first; stack last and comparator; RET8, EAX output pair |
| `00B1CF80` | Choose median pivot | ECX first, EDX middle; stack final slot and comparator; RET8 |
| `00B1C8C0` | Order three samples | ECX first, EDX middle; stack final slot and comparator; RET8 |
| `00B1D020` | Make heap | ECX first, EDX last; stack comparator and two unread words; RET0Ch |
| `00B1D6A0` | Sort heap | ECX first, EDX last; stack comparator; RET4 |
| `00B1C910` | Adjust heap | ECX first, EDX signed hole; stack signed length, saved entry and comparator; RET0Ch |
| `00B1C1B0` | Push heap | ECX first, EDX signed hole; stack signed top, saved entry and comparator; RET0Ch |
| `00B51B00` | Unsigned split-word key less | ECX actual left entry, EDX actual right entry; RET, EAX exactly 0 or 1 |

The current saved prototypes still display `undefined ...(void)`. Assembly,
including all returns, establishes the interfaces above. No body registers an
FS exception frame. Direct calls close within the ten sort functions; the only
external callable edge is the caller-provided comparator. No missing allocator,
owner, material, or singleton function is replaced by a fixture stub.

The public sorter has a new C++ interface:

```cpp
void sort_native_render_pointer_slots_00b1dce0(
    void* first, void* last, std::int32_t ideal,
    NativeRenderPointerSlotComparator comparator);
```

Only the comparator edge deliberately retains its original register-call
shape: `uint32_t (__fastcall*)(const void*, const void*)`. Sort calls test the
low byte of its result. A false AL with nonzero upper bits remains false;
noncanonical nonzero AL remains true. No `noexcept` policy is added to a
supplied comparator or to routines invoking it. The key leaf is `noexcept` and
returns the native canonical DWORD result. These declarations target MSVC
Win32; the public sorter is not a binary replacement for the original entry.

## Preserved behavior

Actual slot and field loads/stores use volatile DWORD accesses on the MSVC
Win32 native-address model. This keeps access width, order, callback-visible
reloads and self-aliasing swaps explicit without a semantic entry type.
Addresses add and subtract modulo 2^32. Signed distance is exactly the DWORD
subtraction followed by SAR2; signed heap/ideal values and masked signed byte
comparisons are retained. No host pointer subtraction or unsigned vector-size
preflight substitutes for native arithmetic.

- Signed count at most one returns without dereferencing slots or calling the
  comparator, including the original negative-distance and wrapping cases.
- Counts 2 through 32 use insertion and the descending GCD-cycle rotation.
  Insertion's sentinel search reloads the candidate entry on every comparison.
- Counts above 32 partition while signed ideal is positive. The new budget is
  `ideal/2 + (ideal/2)/2`; signed ideal at most zero selects the native heap path.
- The final-slot distance must exceed 40 for the nine-sample pivot, so the
  threshold is 42 entries. Order-three always makes its third comparison.
- Partition expands the equal band, scans both sides and performs the original
  two-swap band slides. The smaller side recurses; equal side lengths recurse
  right. Equal entries therefore retain the native permutation, which is not
  globally stable for larger sorts.
- Heap adjustment compares right child against left child, selecting right on
  equality; equal parent/saved-entry comparisons stop upward movement.
- Slot-pair calls load the right entry before the left entry. Swaps and heap
  copies reload current slots after a comparator returns, preserving actual
  callback mutation rather than using stale captured pointers.
- The key leaf reads left high DWORD at `+24h`, then right high DWORD. Only on
  a high-word tie does it read left low DWORD at `+20h`, then right low DWORD.
  Both comparisons are unsigned. There is no unconditional 64-bit load.

Native readable/writable storage, suitable actual object fields, and a valid
comparator remain caller preconditions. There are no added null-entry, finite
depth, bounds or comparator checks. The raw sorter itself never follows an
entry's section/material chain. It can accept the separate actual-field
material/depth comparator when that implementation is supplied by the caller.

## Verification and limits

One private focused fixture loads all 11 complete, freshly verified native
spans into a sparse relocated image, leaves gaps inaccessible or filled with
INT3, protects the code executable/read-only, and calls original `00B1DCE0`
with its actual ECX/EDX/RET8 interface. It supplies the same recording comparator
to original and reconstructed sorting. That comparator invokes the respective
native or reconstructed `00B51B00` and returns `0xA55A5A00` for false and
`0xA55A5A81` for true.

All 15 runs matched 6,423 comparator calls, 594,239 complete trace words and
933 final pointer positions. The corpus covers the insertion boundary,
33/41/42-entry partition and pivot boundaries, tied and duplicate pointers,
organ-pipe input, forced zero/negative-budget heaps, and positive-budget
exhaustion. Four runs exchange distinct equal-key pointers in the actual
caller array during the third comparator call, covering insertion, pivot,
equal-band and heap behavior. Every callback records both entry arguments and
the current full slot sequence. Entry bytes and cells outside the sorted range
remain unchanged.

The same fixture compares 49 split-word unsigned key pairs, including high-bit
values and ties. Four guarded leaf calls put both low fields in inaccessible
pages while leaving unequal high fields readable. Four no-access bound pairs
exercise empty, one-entry wrap, negative and sign-bit distances with a null
comparator. Original and reconstructed calls return without access.

`scripts/build.ps1` compiled the module and the existing repository targets
with MSVC Win32 `/W4 /WX /fp:strict`; both existing CTest checks passed. The
worker used an ignored local deferred source registration for its build and
left shared CMake ownership to the integrator. The native differential binary
also compiled with `/W4 /WX /fp:strict /O2`.

This is complete-body source reconstruction with bounded native differential
evidence and build checks. It does not establish a drop-in sorter ABI,
unrestricted invalid-memory behavior, concurrent memory behavior, or gameplay
and rendering validation. The full `00B51DF0` preparation root still requires
its real queue-acquisition contract; this packet supplies its raw sort leaf
without claiming that root is complete.

See `reports/native_render_pointer_slot_sort_audit.json` for complete original
bytes, native ABI records, call closure, preserved annotations, implementation
and private-fixture hashes, and the build/differential evidence. The existing
`src/instance_sort.cpp` typed/vector reconstruction is retained separately.
