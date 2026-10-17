# cc13-dyn2: TerrainConvexIntersect slot 0 and the Mit pool <36,128> free

Files: `src/cc13_dyn2.cpp`, `include/bsp/cc13_dyn2.hpp`. Nothing is wired. Status of every
function: reconstructed and build-tested; none is fixture-tested, ABI-compatible or
game-validated. Descriptive names are hypotheses, not recovered symbols. These are the two
items cc13-dyn1 left in the in-house Dyn block (docs/CC13_DYN1.md, "Skipped").

| Address | C++ | Original ABI |
| --- | --- | --- |
| 00C58EB0 | `terrain_convex_intersect_00c58eb0` | `__thiscall`, ECX owner unused; stack result, shape A, matrix A, shape B, matrix B; `RET 14h`; AL hit |
| 00407960 | `pool36_free_00407960` | EAX = slot; no stack arguments; `RET` |
| 00C36990 | `pool36_free_thunk_00c36990` | `JMP 00407960`, same register ABI |

## 00C58EB0 Dyn::TerrainConvexIntersect slot 0

Vtable `00D7A1C4`, RTTI type descriptor `00E170DC`, static object `00E1743C`.

### Reachability

The method is only reachable through that vtable, and nothing installs the object in a
dispatcher. `00E1743C` holds the vtable word statically in the PE, and its only references
are the two in `00406010` (`MOV [00E1743C],00D7A1C4 / MOV EAX,00E1743C / RET`).
`00406010` has no callers: the rel32 and absolute-dword census (`tools/callsite_census.py`)
finds only one unaligned hit at `00D0599D`, inside float data (`00 10 60 40` there is 3.5f).
As a positive control, the same census finds the scene constructor's read of the neighbouring
object `00E17438` at `00C38113`. The scene dispatch table in docs/DYN_SCENE_RUNTIME.md lists
no TerrainConvexIntersect pair; terrain pairs 29 and 34 use TerrainConvexMeshIntersect
(`00C53630`). So there is no per-tick caller.

The body could not complete in the image even if it were called. Past the empty-range test
it builds a stack `Dyn::TriangleShape` with the `Dyn::Shape` base constructor inlined. That
constructor copies from its descriptor (`+8` from `desc+10h`, `+24h..30h` from `desc+0..0Ch`,
as in dyn1's `construct_shape_base`). Here the descriptor is the constant 0, so the image
reads the absolute addresses `10h`, `0`, `4`, `8` and `0Ch` (`00C59EDA..00C59F15`, confirmed
from the disk bytes). The PE has no relocation directory. The first of these reads faults.
The source does the same reads, as `volatile` loads.

### Precision

If the method were dispatched, it would run like its sibling dispatchers in the collision
pass, on the Dyn task workers at control word `027Fh` (53-bit; docs/CC13_FPU_PRECISION.md,
"Dyn physics: 53-bit"). Each x87 chain is computed in `double` in the image's grouping and
narrowed to `float` at each `FSTP float`. The expressions come from `tools/x87_sym.py`, which
decodes reg-reg operand direction from the bytes, run with a local copy that names every
float store. The paths were: both prologue arms (`00C58EDB` taken and not taken), the common
range code, and the loop body with all four sample-mode branches taken. The only
non-popping reg-reg op in the body is `00C5A7DD FMUL ST1`, bytes `D8 C9` (ST0 = ST0*ST1).
Products of two floats, or of a float and a small integer, are exact in double, so only the
grouping of the sums matters. Every sum is `(p0 + p1) + p2`, with the translation term added
last. Constants: 0.5 double at `00D7A280`, 3.0 double at `00D7A2B0`, -1.0 double at
`00D7A250`, -1000.0f at `00D7A240` and 1.0f at `00D7A24C`. The SSE conversions are
`CVTSI2SS` (column to float) and `CVTTSS2SI` (truncating; NaN and out-of-range give
`80000000h`, as `_mm_cvtt_ss2si` does). The grid row reaches x87 by `FILD`/`FIMUL`, exactly.
No RNG: nothing here calls `00BD2F10`.

### Behaviour

1. Kind `[A+8] == 5` makes A the terrain (swap flag 0, `00C5933D`). Otherwise B is the
   terrain and the flag is 1 (`00C5980A`). The terrain and convex local transforms
   (`S+34h`) are composed with their matrices into world 3x4s (the frame's `ESP+D4h` and
   `ESP+98h`).
2. The method forms the convex rotation in terrain space (`R[i][j]` = convex row i · terrain
   row j) and the translation difference in terrain space. The convex box (`S+0Ch..20h`)
   becomes a center and half extents (`* 0.5`). That gives the box center in terrain space and
   its extents through nine `BSP_Math_AbsFloat` (`00401170`) calls in row order. Only the x
   and z spans are used; the y span goes to frame slots that nothing reads.
3. The spans are divided by the X/Z spacing (`T+21Ch`/`T+220h`) and truncated. The column
   start is clamped to [0, width-1] and the row start to [0, height]. Each end is
   `trunc+1` (sign test `JNS` on the wrapped sum) clamped to [0, size]. If the product of the
   two counts (32-bit `IMUL`) is zero, the method returns AL = 0 with the result untouched.
4. Otherwise it clears the result count and builds the stack triangle: base vtable
   `00D7A04C`, the null-descriptor fields, `+204h/208h/20Ch` = 0, TriangleShape vtable
   `00D7A1B0` (EH state 0), an identity local transform, and the terrain's body at `+4`.
5. Columns form the outer loop and rows the inner loop. Each cell reads four samples. Mode
   `[T+22Ch]` 0 means floats. Mode 1 means uint16 times `[T+224h]` plus `[T+228h]`, with
   `FFFFh` giving -1000.0f. Any other mode stores XMM0, which is 0.0 at all four sites
   (`XORPS` at `00C59F6B` and `00C5A7A1`). The cell makes two triangles,
   (x0,h00,z0)(x1,h10,z0)(x1,h11,z1) and (x0,h00,z0)(x1,h11,z1)(x0,h01,z1). The box of each
   is its centroid twice, computed with the image's own sums. Each triangle is passed to
   `00C53010` with ECX = terrain world, EDX = convex, EAX = convex world, and stack work,
   contact and triangle. Work `+1F0h`/`+1F4h` hold the GeneralConvexIntersect object's
   directions and lock (`0109EA50`/`0109ECC0`), rewritten before every call. On a hit, the
   nine contact floats are appended at `result+4+count*24h` and the count goes up by one. The
   method stops scanning as soon as the count is exactly 8, which is checked after every
   triangle.
6. If the swap flag is set, each contact's normal is multiplied by -1.0 and its two points
   are exchanged. AL = (count != 0).

### Simplifications and uncertainty

- The SEH frame (handler `00CC84D0`, state 0 after the triangle vtable store) is not
  reproduced. The normal path never runs a destructor on the triangle.
- The image's literal `0109EA50`/`0109ECC0` become `&general.support_directions_08` and
  `&general.critical_section_278` of the host's `DynGeneralConvexIntersectStorage`. The two
  triangle vtable words are parameters that default to the image values (dyn1's convention).
  `00C53010` calls the triangle's support slot `+0Ch` through the second one, so a host must
  pass a natively callable table.
- `00C53010` is called through the existing `intersect_native_dyn_convex_00c53010`
  (docs/NATIVE_DYN_GENERAL_CONVEX_R140.md). Its C++ form takes the borrowed
  `CameraAxesCrtAccess`, which this function forwards.
- The frame offsets of the triangle (`ESP+198h`, 234h bytes), contact (`ESP+3D0h`, 128h) and
  work record (`ESP+4F8h`, 1F8h) are reproduced as local buffers of those sizes.

## 00407960 Mit::cPoolAllocator<36,128> free, and its thunk 00C36990

The pool is the global at `0109EA0C`, built by constructor `00407590`: vtable `00D7A1DC`, the
critical section at `+0Ch` (`0109EA18`), lock depth `+24h`, the page array `+28h`, count
`+2Ch`, capacity `+30h` (20h) and first-free page `+34h` (-1). The function takes
`EnterCriticalSection` (`[00CE2218]`) and raises the depth. It reads the page index at
`slot+24h` and computes the slot number `(slot - page) / 28h` (signed: `IMUL 66666667h`,
`SAR 4`, plus the sign bit). It pushes that number as a uint16 on the page's free stack at
`+1400h` and increments the uint16 depth at `+1500h`. If the page index is below the hint
(`JNC`, unsigned), the hint is lowered. Then the depth drops and `LeaveCriticalSection`
(`[00CE2210]`) runs. The image addresses the pool absolutely; the C++ takes the host's
instance. `00C36990` is a five-byte `JMP 00407960`. Its seven callers are the unwind funclets
`00CC838D`, `00CC8395`, `00CC839D`, `00CC85BC`, `00CC85C7`, `00CC85D2` and `00CC85DD`, each
of which loads EAX from its frame and jumps. Callers of `00407960` itself: `00406190`,
`00C48100` (two sites, one of them a tail jump), `00C51480` (seven sites) and `00C5D900`.

## Blocked callers and adoption

- 00C58EB0 has no callers and no installed dispatcher cell, so it unblocks nothing and needs
  no cc9 adoption note. Wiring it would not change any run, because the image never runs it
  to completion.
- 00407960/00C36990 are the free half of the convex-hull triangle pool. With them recorded,
  the pool dependency left for each of the four callers of 00407960 listed above is the
  allocate `004077F0`. Neither function is per-tick on its own; they run when hulls are
  built. No RNG.
