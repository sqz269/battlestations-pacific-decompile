# Native instance category sort

`sort_instance_entries_00b1dce0` reconstructs the complete native category-one
sort, including pointer permutations when more than32 entries have equivalent
material/depth values. It uses the established `00b51ab0` predicate: signed
material order ascending, then float depth descending. It does not use a host
library sort or impose distinct-key restrictions.

The C++ interface accepts a vector of borrowed `InstanceRenderEntry*` values.
Entries, their sections and sort fields remain stable during the call. Duplicate
entry pointers are valid. The interface rejects null entries/sections, nonfinite
depths and counts whose native four-byte pointer distance exceeds `INT32_MAX`,
before any mutation. The native function itself does not perform those checks.

## Complete helper graph and ABI

All addresses below were checked through the verified `bsp` project and
`/battlestationspacific.exe` program. Full function bytes compare equal to the
installed executable. Names in source and annotation proposals are descriptive
hypotheses; no original library symbol or vendor version is claimed.

| Address | Role | Native arguments | Return cleanup | Bytes |
| --- | --- | --- | --- | ---: |
| `00b1dce0` | Introspective sort | ECX first, EDX last; stack ideal, comparator | `RET8` | 235 |
| `00b1d420` | Insertion sort | ECX first, EDX last; stack comparator | `RET4` | 150 |
| `00b1c210` | Pointer-range rotation | ECX first, EDX middle; stack last, two unused words | `RET0C` | 145 |
| `00b1d280` | Equal-pivot partition | ECX output pair, EDX first; stack last, comparator | `RET8` | 382 |
| `00b1cf80` | Median pivot selection | ECX first, EDX middle; stack final element, comparator | `RET8` | 156 |
| `00b1c8c0` | Order three samples | ECX first, EDX middle; stack final element, comparator | `RET8` | 77 |
| `00b1d020` | Build max heap | ECX first, EDX last; stack comparator, two unused words | `RET0C` | 62 |
| `00b1d6a0` | Drain heap | ECX first, EDX last; stack comparator | `RET4` | 70 |
| `00b1c910` | Adjust heap hole | ECX first, EDX hole; stack length, value, comparator | `RET0C` | 110 |
| `00b1c1b0` | Push value up heap | ECX first, EDX hole; stack top, value, comparator | `RET0C` | 92 |

`first`/`last` are pointer-slot bounds except where the table explicitly says
final element. The decompiler hides register inputs to the comparator and some
stack words; the implementation follows the complete assembly. The category-one
caller at `00b1e9eb..00b1ea0d` supplies entry count as the initial ideal budget.
The public vector interface uses that same budget and fixes the comparator to
the actual category-one predicate. It is not a binary-compatible replacement for
the native arbitrary-comparator function.

## Permutation-sensitive behavior

`00b1dce0` selects insertion sort when count is at most32. `00b1d420` first
compares each value with the first element, then its predecessor; a backwards
scan uses the first element as a sentinel. `00b1c210` rotates the insertion
range with GCD cycles beginning at `first+gcd` and descending. Strict comparisons
retain equivalent-item input order in this small-list path.

For larger ranges, `00b1d280` chooses the midpoint at `first + count/2` and asks
`00b1cf80` to order pivot samples. The latter compares final-element distance
against40 at `00b1cf8f`, so nine-sample selection begins at42 entries. At33..41
entries it orders first/middle/final. At42 or more, step=`count/8` and it orders
three triples near first, middle and final, then orders their middle samples.
`00b1c8c0` always makes three strict comparisons in middle/first, final/middle,
middle/first order.

Partitioning first expands the midpoint's existing equal band to either side.
It then scans outward regions, exchanges mispartitioned values, and moves newly
found equivalent pointers into the equal band. When one scan reaches its bound,
the native routine slides the equal band with a specific two-swap sequence.
Those swaps at `00b1d381..00b1d3aa` and `00b1d3ba..00b1d3d3` are preserved in
source. Large-list tie order is consequently not generally stable.

After partition, ideal becomes `ideal/2 + (ideal/2)/2`. The smaller unequal side
recurses; equal side lengths recurse right. The larger side continues in the
current invocation. If ideal reaches zero while more than32 entries remain,
`00b1d020` builds a heap and `00b1d6a0` drains it.

Heap adjustment compares the right child against the left child at
`00b1c930..00b1c93f`. A strict right-less-left result selects left; equivalent
children select RIGHT. After descending, the saved value moves upwards only
while the parent is strictly less. Both details affect equivalent-entry pointer
permutations and are retained exactly.

## Native differential validation

The existing `ghidra_export.py verify-seeds` prerequisite passed. A temporary
Win32 fixture copied only the ten complete PE/live-verified functions into an
isolated code allocation at their original relative offsets. Gaps contained
`INT3`; the completed allocation was changed from read/write to execute/read.
The native routines received a fastcall adapter to the existing projected
`00b51ab0` predicate. The fixture did not call the game's object graph or modify
the running game or saved Ghidra program.

One bounded corpus covered sizes0,1,2,31,32,33,41,42,65 and257, with six patterns:
all equal, repeated random material/depth classes, reversed classes containing
`INT32_MIN` and `INT32_MAX`, distinct sorted depths, signed-zero ties, and
repeated pointer identities. Sixty runs used the caller's normal count budget;
thirty additional runs used zero ideal to exercise the native heap fallback.

All90 runs matched the exact pointer permutation at all5,652 compared positions.
The fixture also confirmed nonfinite host rejection before mutation. Result:

```
Native instance sort differential: cases=90 forced_heap=30 pointer_positions=5652 exact_permutation=1 finite_guard=1
```

The fixture compiles the actual new source with MSVC Win32
`/std:c++17 /W4 /WX /EHsc /fp:strict`. No permanent test or CMake changes were
added by this worker. Temporary fixture source and byte header remain under
ignored `local/`; hashes and validation boundaries are recorded in
`reports/instance_category_sort_audit.json`.

This establishes native algorithm/permutation agreement for the recorded corpus
using the established comparator projection. It does not establish original
object-layout ABI, native comparator-object traversal, nonfinite behavior or
in-game rendering. Primary integration owns the full build and replacement of
the earlier bounded upload-local sort.
