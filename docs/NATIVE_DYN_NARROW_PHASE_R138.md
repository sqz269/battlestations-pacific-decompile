# Native pair intersection and contact manifold storage

Addresses: 00C44090, 00C3F4D0, 00C35260, 00C3F650, 00C3F760, 00403CC0;
read-only reference 00401170.

R138 completes six normal bodies (3,646 bytes) in
`include/bsp/native_dyn_narrow_phase.hpp` and `src/native_dyn_narrow_phase.cpp`.
Descriptive names are hypotheses, not recovered symbols. The existing semantic
projections in `dyn_collision_pass.cpp` remain separate. The new source uses
actual scene, body and manifold storage, with private MSVC Win32 assembly
kernels preserving the original x87/SSE instruction schedule and an explicit
borrowed allocator/service context.

## Recovered contracts

| Entry | Native inputs and cleanup | Complete behavior |
| --- | --- | --- |
| C44090, 718 B | ESI scene, EAX first, stack inclusive last; RET4 | Traverse pair proxies, bodies and shape chains; masks and 6x6 dispatcher selection; material values; manifold/contact insertion; listener filtering and spin-locked event vector growth. |
| C3F4D0, 374 B | EDI pool, stack first/second body; RET8 | Real critical section, directional manifold lookup, 1,000-record page growth, active-list insertion, both body reference appends. |
| C35260, 109 B | ESI body, stack manifold; RET4 | Grow body+74/+78/+7C reference vector by capacity*2+2, copy, free, publish, append. |
| C3F650, 261 B | EAX manifold, EDI nine-float candidate; RET | First contact whose local A **or** local B distance squared is strictly below D7A2D8; otherwise captured signed count. |
| C3F760, 2,166 B | ECX manifold, EAX candidate; RET | Normal-length gate, existing-contact update, append through four points, complete four-point reduction/replacement. |
| 403CC0, 18 B | ECX task; RET | Capture last+10, scene+8, first+C and invoke range processing. |

The candidate contains local A, local B and normal, three floats each. A
dispatcher receives ECX object and five stack arguments: result, shape A,
body A+8 matrix, shape B, body B+8 matrix; RET14. Only AL determines a hit.
The result count is followed by up to eight candidates in the native scratch
buffer. The source calls the actual dispatcher object supplied by the scene;
this packet supplies no collision geometry implementation.

Pool lookup scans the first body's captured reference vector and checks only
record+D0 against the second body. It is directional, not symmetric. New E0h
records initialize count+C8, body pointers+CC/+D0 and links+D8/+DC; other bytes
remain untouched. The second body is reloaded from the new record after the
first reference append. Page size is 36B00h, with 1,000 E0h slots. The real
critical section is at pool+1D8, and its pointer is captured before entry.

Contact records begin at manifold+8, stride 30h. Local A is record+C, local B
record+18, normal record+0, impulses record+24/+28 and depth record+2C.
Appending initializes both impulses to zero; matching and replacement preserve
them. Reduction retains the original coverage metrics, comparison order and
tie selection. The consumed 28-byte game abs helper retains its float spill;
it is an existing reference, not a newly claimed library implementation.

Material mixing preserves the original negative-zero constant D7A208
(`80000000`), COMISS branches, x87 multiplication and float spills, including
unordered behavior. Restitution is spilled before the pool call. Contact
matching uses the exact float bits `3B23D70B`; the normal gate uses the exact
double bits `3FA99999A0000000`. These are not rounded decimal replacements.

Event locking retains LOCK CMPXCHG and XCHG at scene+E4. Event records are
12 bytes (manifold, shape A, shape B), with vector fields D8/DC/E0. Allocation,
copy, free and publication keep their original order and reloads. Three false
returning-free continuations (9 bytes) were repaired under the Ghidra write
lock; final flow checks find no remaining CALL gaps in these bodies.

## Callable task owner

`NativeDynIntersectTaskRuntime` provides the complete one-slot IntersectTask2
table. It borrows the original construction allocator and a service object.
Each invocation keeps its context on its own stack; no global, TLS or shared
progress state is introduced. The owner must remain stable and outlive its
tasks. Services used from multiple tasks must support concurrent calls.

## Validation

- Strict MSVC Win32 build with warnings as errors; all three existing CTests pass.
- 3,706 live Ghidra/PE bytes agree: six new bodies, the existing 28-byte abs
  reference, 24 constant bytes and eight import-cell bytes. Both project and
  program were verified before collection.
- 205 copied-native/source pairs match **126,716,636 bytes**, with 37,182
  service events, 6,593 snapshots and 14,640 controlled dispatcher calls.
- Contact sequences exercise matching on either local point, rejection,
  append, impulse preservation, nonidentity body transforms, all four
  replacement indices, and masked NaN behavior. Small scenarios run across
  all 12 x87 precision/rounding settings; CW/SW/tag/MXCSR are compared.
- Pool stress creates all 2,016 unordered pairs of 64 bodies, crosses three
  pages and page/reference vector growth, then checks directional, repeated
  and self queries. Range cases cover masks, missing shapes/dispatcher,
  AL-false with nonzero upper return bits, zero/eight contacts, negative/NaN
  friction, listeners, event growth, empty and partial inclusive ranges.
- The actual production task slot is invoked through Win32 thiscall. The
  fixture shares existing body, shape, proxy and manifold-pool constructors;
  it does not independently prove those constructors again.

The fixture uses the same fixed arena addresses. It compares live, stale and
freed allocation bytes, service sites/order/sizes and final storage. Only the
task table word and each shape's verified table and mesh-pointer identity
are normalized. The 24 OS-owned critical-section bytes and floating-point
instruction pointers are excluded; real lock entry/exit and recursion depth
are observed separately. An initial mismatch at the shape table word was a
different stack-owner address; normalization is limited to these identified
pointer fields, with no mutation of the tested storage.

The dispatcher returns controlled candidates through its real calling
convention. This proves the consumer path, not geometry. Shapes use the
existing complete convex source table, but support and lifetime entries are
not invoked here. Diagnostic arena cleanup is fixture-owned, not a new
manifold/scene destructor comparison.

## Limits and next dependencies

Successful allocation and valid original storage/candidate domains are
required. Parallel scheduling, callback-driven storage mutation, unmasked FP
traps, allocation failure, native exception/RTTI metadata and private-stack
aliasing remain unproved. Public APIs are explicit source interfaces, not
drop-in replacements for original register entry points.

The concrete task table still needs admission through the application's
physics context. Actual dispatcher methods (including C49A30, C53630,
C518D0, C48330, C535E0, C44780, C50DD0 and C50740), other world task classes,
full simulation, ordinary application admission and gameplay remain open.
See `reports/native_dyn_narrow_phase_r138.json` for calls, annotation receipts,
hashes and sealed build/fixture provenance.
