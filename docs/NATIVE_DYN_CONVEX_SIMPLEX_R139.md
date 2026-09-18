# General-convex simplex reduction

Addresses: 00C3CC30; consumed math references 00401C20, 00401CD0;
read-only parent references 00C51EF0, 00C53010, 00C535E0, 00C48BE0, 00C51C20.

R139 reconstructs the complete normal body `00C3CC30..00C3F174` (9,541
bytes, 2,297 instructions) in `src/native_dyn_convex_simplex.cpp`. The
descriptive name is a hypothesis, not a recovered symbol. This is the
simplex-reduction dependency of the general-convex collision search, not a
complete source implementation of that search or dispatcher.

## Storage and producer

The original takes its work pointer in ESI, reserves 64h stack bytes and uses
plain RET. The new explicit fastcall interface receives ECX and saves/restores
ESI. `00C535E0` creates a 1F8h scratch object and supplies its direction table
and critical-section pointers at +1F0/+1F4. `00C53010` supplies the shapes and
composed transforms. `00C51EF0` generates each pair of support witnesses,
writes their difference into the simplex array, increments the count and
calls this reducer.

| Offset | Native content used by the reducer |
| --- | --- |
| 80h | Search direction, three doubles |
| 98h | Four difference points, three doubles each, stride 18h |
| F8h | Four corresponding support witnesses for shape A |
| 158h | Four corresponding support witnesses for shape B |
| 1B8h | Point count |

The reducer keeps the three arrays in corresponding order when it copies
points. It leaves inactive/stale slots intact except for the native explicit
stores, including self-copies. Counts 1..4 enter the point, segment, triangle
and tetrahedron paths. Other values take the original unsigned range check
and return without changing storage.

The paths retain the original ordered vertex/edge/face tests, direction
calculations, count updates, witness copies and tie behavior. The four-point
path can retain four points; its caller interprets that as an overlap result.
No degeneracy repair or denominator clamp is added. Masked underflow,
division and invalid-operation behavior follow the recovered arithmetic.

The verified table at C3F178 contains C3CC4C, C3CC7C, C3CDF6 and C3D5D6.
The source encodes these four destinations as direct branches, avoiding
original executable addresses. This adds integer comparisons; it does not
claim preservation of uncontracted EFLAGS or native private-stack aliases.
Subsequent x87 operations, spills and storage accesses retain their order.

## Consumed vector helpers

Four calls use existing `00401C20` (47 bytes): output EAX, input vectors ECX
and EDX, sequential stores for the reversed cross product, plain RET. Two
calls use existing `00401CD0` (29 bytes): vector EAX, stack double factor,
RET8. Private copies of their recovered instruction schedules preserve the
native register and x87 contracts. These previously reconstructed game math
leaves are references, not new reconstruction or naming claims.

All six call sites were reviewed and mechanically verified. Flow inspection
found no listing gaps; no Ghidra control-flow repair was needed.

## Validation

- Strict MSVC Win32 build with warnings as errors and all three existing
  CTests pass. No new permanent test suite was added.
- Live Ghidra/PE comparison covers **16,961 bytes**: 9,541 new reducer bytes,
  76 consumed vector-helper bytes, 7,224 read-only parent bytes, the 16-byte
  switch table and 104 constant bytes.
- A COFF instruction audit checks **2,327 original instructions**. Every
  instruction outside relocated CALL/JMP/Jcc operands has identical encoded
  bytes. Call symbols, all 41 resulting branch destinations, the ECX/ESI
  adapter, 15 return restores and four-way switch expansion are checked
  separately. The source reducer is 9,576 compiled bytes.
- **86,016 direct native/source pairs** cover 1,024 deterministic records,
  counts 0..5 and FFFFFFFF, and all 12 x87 precision/rounding controls.
  Inputs include signed zero, repeated/collinear/coplanar points, large and
  tiny finite coordinates, and ordinary random points with paired witnesses.
- **7,020 copied-parent pairs** exercise the reducer inside the original
  C53010/C51EF0 call chain and its result/direction helpers. The shapes use
  the existing real convex support methods, eight-vertex cube adjacency,
  translated and rotated bodies, and the concrete general-convex direction
  constructor. Both sides share those existing services and recovered CRT
  sqrt; only the reducer is exchanged. All four incoming simplex counts
  occur, with 122,808 reduction calls across both sides.
- Comparisons match **113,860,584 bytes exactly**, with no pointer or float
  normalization: complete work records, parent results, support-seed writes
  and x87 CW/SW/tag/MXCSR. Floating-point instruction pointers and private
  stack bytes are outside the comparison. Integer-only exit markers in the
  copied reducer show that all **15 native return sites** were reached.

The direct four-point corpus reaches outcomes of one, two, three and four
points. Full exit coverage does not establish coverage of every predicate
combination or arbitrary floating-point input.

## Remaining work

The new reducer has no external allocator, lock or global state. Valid work
storage and initialized active points/witnesses are required. Unmasked FP
traps, malformed pointers, private-stack aliasing and original exception ABI
remain unproved. The parent fixture uses borrowed shape support tables but
does not invoke their lifetime slots; it is not a shape-lifetime test.

The general-convex search C51EF0, fallback direction C51C20, result projection
C48BE0, composed intersection C53010 and dispatcher entry C535E0 remain
read-only native references in this packet. Reconstruct and connect those
bodies to this reducer before supplying the general-convex source table to
the application's physics context. Full geometry dispatch, ordinary
application admission and gameplay validation remain open.

See `reports/native_dyn_convex_simplex_r139.json` for byte/call evidence,
instruction-audit results, annotation receipts and sealed artifact provenance.
