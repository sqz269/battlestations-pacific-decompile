# Ordered avoidance polygon partition

Addresses: `004F6F20`, `004F65F0`, `004F6560`, `004F4EC0`, `004F5500`,
`004F5790`, `004F4CD0`, `004F4C60`, `004F2F40`, `004F3EF0`, `004F6710`,
`004F6E90`. Descriptive names are reconstruction hypotheses.

`avoid_zone_polygon_partition_004f6f20` reconstructs the ordered index-vector
algorithm consumed by `00423C50` at `00423D08`. It accepts the unchanged
`AvoidZoneDraftPoint` vector and actual angular tolerance (the avoidance
caller supplies bits `3C8EFA35`). The result retains every piece's index order,
the ordered cut pairs, and the final active count. It does not insert a fan,
reverse winding, remove duplicate coordinates, or invoke another triangulator.

## Coverage and original interfaces

| Routine | Original ABI | Coverage |
|---|---|---|
| `004F6F20..004F7173` | ECX destination 68h, stack point-vector pointer and float, EAX destination, RET8 | Complete valid-container partition algorithm; new C++ storage/exception interface |
| `004F65F0..004F66E0` | ECX partitioner, plain RET | Complete cyclic next/previous initialization; native resize allocation is replaced by C++ vector ownership |
| `004F6560..004F65EC` | ECX partitioner, AL success, plain RET | Complete seed selection and three-index piece initialization |
| `004F4EC0..004F51FA` | ECX partitioner, AL found, plain RET | Complete greatest-angle admissible seed scan |
| `004F5500..004F5783` | ECX partitioner, EAX boolean, plain RET | Complete previous-end extension predicate |
| `004F5790..004F5A09` | ECX partitioner, EAX boolean, plain RET | Complete next-end extension predicate |
| `004F4CD0..004F4EB2` | ECX partitioner; five uint32 stack indices; EAX boolean, RET14h | Complete active-chain triangle exclusion |
| `004F4C60..004F4CCB` | Six float stack words (three points); ST0 angle, RET18h | Complete normalized signed angle, existing `0042CF10` reused |
| `004F2F40..004F2FAD` | ECX two-float vector, in-place stores, plain RET | Complete normalization; reconstructed helper adds actual CRT access in EDX |
| `004F3EF0..004F400C` | ECX remaining-weight output, EDX first weight output; four two-float points and second-weight pointer on stack, RET24h | Complete x87 barycentric kernel with original register/output ABI |
| `004F6710..004F6792` | ECX eight-byte element vector, stack source pointer, RET4 | Complete valid-container append semantics; native allocator/iterator/growth/SEH execution excluded |
| `004F6E90..004F6F12` | ECX 16-byte vector-element vector, stack source vector pointer, RET4 | Complete valid-container deep-copy append semantics; native allocator/iterator/growth/SEH execution excluded |

The native constructor's layout is established by its stores, not inferred
from the consumer: active count `+00h`, current seed `+04h`, borrowed source
vector `+08h`, next vector `+0Ch`, previous vector `+1Ch`, current piece vector
`+2Ch`, last/next endpoint `+3Ch`, first/previous endpoint `+40h`, output piece
vector `+44h`, cut-pair vector `+54h`, angular tolerance `+64h`. Each native
vector is 16 bytes: allocator word, begin, end, capacity. The point element is
eight bytes and the output piece element is a deep-copied 16-byte vector.

## Recovered algorithm

`004F65F0` establishes `next[i]=i+1` and `previous[i]=i-1`, then closes the
cycle. Even before the constructor's count-below-three return, it writes
`previous[0]` and `next[count-1]`. Thus an empty input is native-invalid. The
new interface rejects it explicitly; one or two points return no pieces.

For each piece, `004F4EC0` visits one cycle starting at the current seed. At
each vertex it evaluates the signed angle from `(current-previous)` to
`(next-previous)`. The initial score is the live `00D7A244` word `FF7FFFFF`
(negative maximum finite float), not zero. A strict greater-than comparison
and an empty triangle select the seed, so the earliest equal score wins.
Negative scores are eligible. If no score qualifies, construction returns
the prior pieces and leaves active count at least three; no fallback occurs.

`004F6560` starts the piece as `[previous,seed,next]`. `004F5500` is always
tried before `004F5790`. It checks the two endpoint angles against negated
tolerance, then checks the still-outside chain against the added triangle.
Success moves the previous endpoint backward and inserts its index at the
front. Only failure tries the next endpoint, appending its index on success.
When both fail, `004F7077` records `{first,last}` in the cut-pair vector.

The constructor splices `next[first]=last` and `previous[last]=first`, copies
the current piece by value into the result, updates active count by
`2-piece.size()`, clears only the temporary piece, and resumes from `last`.
The number of attempted extra vertices is the original active count minus
three. There is no area sort, spatial sorting, or second merge pass.

## Geometric and assembly details

`004F2F40` rounds vector differences, squared length, actual sqrt result and
each normalized component to binary32 at the native stores. A squared length
at or below double `1e-10` (`00CE3820`) uses double `1e-5` (`00CE3C70`) as
the divisor, rounded to binary32. The x87 `JBE` also selects this branch when
unordered. The clamped asin is the existing `camera_asin_clamped_0042cf10`;
its owning CRT binding remains required. The square-root call receives the
same process-owned `CameraAxesCrtAccess` as other reconstructed geometry.

`004F4C60` and the inlined seed score preserve the same normalized cross
product and binary32 spill schedule. The growth tests use x87 `JA` to reject
only `-tolerance > angle`; unordered angles are not rejected by that branch.
In contrast, the seed's `JBE` rejects unordered or non-increasing scores.

`004F4CD0` has five integer index arguments. Its decompiler aliases the first
argument's stack slot with a barycentric output and incorrectly prints float
index comparisons. Assembly at `004F4CD4/CD8` establishes integer traversal;
the callee receives ECX and EDX output pointers at `004F4E35/E39`, and the
actual cleanup is `RET14h`. It examines the active `[begin,end)` chain. Two
`COMISS` checks reject negative/unordered barycentric coordinates, followed
by an inclusive x87 sum-at-most-one test. Thus points on edges exclude a
triangle too. There is no determinant epsilon or degenerate-triangle fallback.

`004F3EF0` retains its complete x87 stack schedule. Several dot-product
operands survive binary32 stores, and numerators divide without an extra
spill. Capstone byte decoding independently checked ambiguous abbreviated
Ghidra x87 operands, including `D8CA` at `004F3F5D` and `D8CD` at
`004F3F65/FD9`: these multiply ST0 by another register. This kernel does not
replace the native arithmetic with a higher-level geometry library.

The native call table in the report includes STL bounds checks and the
constructor's memmove branch. All twelve owned entries already exist in the
correct Ghidra program. No missing-function definition or false-free-flow
repair was needed in these bodies. Ghidra writes are deferred to integration.

## Validation and limits

One ignored Win32 native differential fixture runs ten installed original
code spans after matching each span against live Ghidra bytes. It relocates
eight absolute operands, preserving relative calls and branches. Explicit
hooks supply valid-container resize/insertion/append/deep-copy operations and
the existing reconstructed CRT sqrt/clamped-asin boundaries; no unresolved
geometric predicate is replaced by a fixture result. Native output indices,
cut pairs and remaining counts are compared exactly.

The fixture passed 939 cases: 8,840 pieces and 7,904 cuts, including both windings,
concave polygons, collinear/duplicate coordinates, one/two-point returns,
and an all-NaN seed failure. Finite cases run under all four x87 rounding
modes. Exact probe, log, source and native-span hashes are in the report;
the private probe and installed bytes remain ignored under `local/`.

This establishes ordered algorithm agreement within those explicit storage
and CRT boundaries. It is not original allocator/debug-iterator/SEH parity,
whole-CRT exceptional-input validation, drop-in 68h object compatibility,
authored-scene runtime integration, Dyn hull creation or gameplay validation.
No native STL destructors are provided by the new semantic result. The
original game installation and Ghidra state were not modified by the worker.

The final `./scripts/build.ps1` MSVC Win32 Release build passed both existing
CTests after native seed verification. The fixture was relinked against that
worktree's final `bsp_core.lib` and exited zero with the same 939-case result.
The live call-report check passed 91/91 rows. No tracked tests were added.
