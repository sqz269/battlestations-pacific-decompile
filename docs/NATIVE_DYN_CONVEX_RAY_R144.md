# Complete convex/ray intersection and dispatcher table

## Scope

R144 reconstructs the complete `00C44780..00C47A8A` normal body: **13,067
bytes, 4,160 original instructions**. `NativeDynConvexRayRuntime` supplies
its one-slot callable source table for the actual `A0h` owner established
by [R143](DYN_CONVEX_RAY_OWNER_R143.md). Callers can bind that table through
`DynDispatchVtables::convex_ray_00d7a1f4`; the existing scene binding returns
the full owner. The explicit method and the table entry are both exercised.

The original method's RTTI-derived name is retained. Ghidra annotations and
the reconstruction ledger now distinguish its complete source body from
R143's earlier storage-only fragment. Field names remain descriptive hypotheses.

## Native contract and arithmetic

The original entry receives owner in ECX and result, shape, segment start and
segment end as four stack pointers. It returns hit in **AL only**, with
`RET 10h`. The shape must have its actual body pointer, local transform and
callable double-support method at vtable offset `0Ch`. Result storage holds
six floats: point followed by normal. Early misses retain the original output
behavior; no extra clear or normalization is added.

The method composes the shape/body transforms, clears the simplex count,
sets the current point from the start, and initializes the search direction.
It repeatedly obtains a support point, advances along the ray when separated,
and reduces a simplex with one through four points. It preserves the original
ordered comparisons, duplicate-support handling, point copies, stale slots,
degenerate arithmetic and normal updates. Constants retain their exact bits:

| Native address | Value | Observed use |
| --- | --- | --- |
| `00D7A250` | `-1.0` | initial advancement state |
| `00D7A2A8` | `1e-12` | squared duplicate-support distance comparison |
| `00D7A2B8` | `1e-6` | advancement/progress comparison |
| `00D7A2E0` | `-1e-5` | approach-direction comparison |
| `00D7A2E8` | `1e-9` | squared search-direction comparison |

The private stack remains aligned to 64 bytes. All original local offsets,
x87 operations, operand order and spills remain in the recovered instruction
schedule. Private consumed math leaves preserve their native register contracts:

| Entry | Bytes | Contract |
| --- | ---: | --- |
| `00401C20` | 47 | reversed cross product; EAX output, ECX/EDX inputs |
| `00401CB0` | 25 | reversed subtraction; EAX output, ECX/EDX inputs |
| `00401CD0` | 29 | in-place scale; EAX vector, stack double, `RET 8` |

These are existing recovered math operations, not new library claims. The
method's CRT square root call uses the existing recovered `00BF7030` service.
One extra final private argument supplies that service at `EBP+18h`; private
returns pop `14h`. The public table keeps the original four-stack-argument
interface. The runtime and its borrowed CRT pointees must remain alive at
stable addresses. Calls sharing mutable owner scratch must be serialized.

The original five-way table at `00C47A8C` maps selectors zero through four to
`C44CC2`, `C44CD9`, `C44D0F`, `C44EC2` and `C4577D`. The source uses explicit
branches to those verified labels, with a balanced `PUSHFD`/`POPFD` on every
path to preserve incoming flags and all registers. The preceding out-of-range
branch is retained. No original process addresses remain as executable targets.

## Verification

The collector verifies the target project/program before reading from Ghidra:
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. **13,400 live/PE bytes
in 12 spans** match: the root, three math leaves, five constants, switch table,
vtable and owner image. The root has 4,160 listed instructions and no gaps.
All 40 direct CALL rows are checked against their native instruction starts.

The COFF audit checks **4,200 original instructions**, including all three
math leaves, **112 branch destinations**, five constant bit patterns, the
flags-preserving switch and the five-instruction CRT adapter. It compares
every other encoded instruction byte. Allowed differences are explicit call,
branch and constant relocations, the private CRT argument, return cleanup and
the audited switch expansion. The explicit interface and callable table are
checked by execution, rather than by treating compiler-generated wrapper bytes
as original machine code.

One focused fixture compares the complete copied original chain against source:
512 deterministic ray/transform cases, zero and poisoned scratch, explicit and
table entry paths, and all 12 masked x87 precision/rounding combinations.
It uses the existing body and convex shape constructors, a valid cube adjacency
graph, and the production double-support method on both sides. Original math
calls stay within the copied original image; only CRT square root is shared.
Both sides reset FP state before constructing inputs and again before the call.

**24,576 pairs match 23,150,592 bytes**, with 11,440 hits and final simplex
counts zero through four. Compared data include AL result, full owner scratch,
output and guards, body/shape records, support seeds, and FP control/status,
tag and MXCSR. Only the differing owner vtable word is normalized; FP instruction
pointers are excluded. Neighboring aggregate objects and padding remain intact,
and rebinding preserves scratch. Uncalled identities guard the other dispatcher
table bindings; they are not replacement collision implementations.

The three hit/early-miss returns at `C47A47`, `C47A5E` and `C47A75` are exercised.
The out-of-range selector return at `C47A88` is statically audited, not claimed
as valid-input runtime coverage. Final count coverage does not prove execution
of every switch selector or predicate combination.

Strict MSVC Win32 build and all three existing CTests pass. The integrated
library repeats the fixture; separate immutable archives preserve the tested
and integrated artifacts. See [the report](../reports/native_dyn_convex_ray_r144.json).

## Remaining boundaries

The shared convex support and CRT implementations retain their earlier evidence;
this packet does not independently re-prove them. The fixture uses valid cube
geometry and masked FP controls; arbitrary hulls, malformed inputs, unmasked
traps, native exception metadata, concurrency and private-stack aliases remain
outside its proof. The table has no native RTTI/EH metadata.

Terrain/convex is still an unfinished dispatcher dependency. World task and
ray-query consumers, ordinary application admission and gameplay remain open.
This packet is not a runnable-game or drop-in binary compatibility claim.
