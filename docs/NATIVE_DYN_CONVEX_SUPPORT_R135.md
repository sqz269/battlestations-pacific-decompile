# Native convex support queries and callable table R135

Addresses: 00C358A0, 00C386E0, 00C385B0. Existing method bindings: 00C57C40,
004062C0. Read-only CRT references: 00BF7420, 00BF7456.

The remaining ConvexMeshShape virtual methods now have complete normal source
bodies. `NativeDynConvexShapeRuntime` supplies all four callable methods using
the original convex pool and mutable CRT conversion cell. Its stable `table()`
can be passed to `DynBodyCreationContext`; the owner and borrowed state must
outlive every shape that uses it. There are no placeholder methods in this table.

## Native contracts

| Body | Bytes | Native inputs | Behavior |
| --- | ---: | --- | --- |
| C358A0 | 342 | EAX mesh, stack output/direction; RET8 | Select seed and climb adjacency to a support vertex |
| C386E0 | 279 | ECX shape, three stack pointers; RET0C | Float direction/output support query through a12-float transform |
| C385B0 | 299 | ECX shape, three stack pointers; RET0C | Double interface with binary32 direction and result spills |

The three new bodies total **920 bytes**. C358A0 uses the mesh layout already
produced by C389C0:16-byte vertices, ushort adjacency offsets/counts/neighbors,
and27 seed entries. It selects `13 + tz - 3*(3*tx + ty)` using the original CRT
truncations and exact native positive/negative factor bits. The seed is sign
extended; neighbor IDs are unsigned. It accepts a neighbor only when its
binary32 dot product exceeds the current score plus the native double epsilon,
then restarts with that vertex's adjacency list. Equal candidates retain the
current vertex. No generic search, clamping or replacement seed is introduced.

The shape methods transform direction into mesh space, query borrowed mesh210,
then transform the selected point back with translation. The double method first
narrows its inputs to float and widens final float results. Assembly preserves
x87 operand direction, arithmetic order, spills, comparisons and memory stores.
For example, C358CD's opcodeDCC9 multiplies ST1 by ST0; Ghidra's compact `FMUL ST1`
text does not show the full operand direction.

Source adapters supply the actual CRT mode address to the existing BF7420/BF7456
implementation. The local kernel retains a context DWORD beside its native locals;
the shape kernels carry context in volatile XMM1 until the mesh call. These are
explicit source interfaces, not native private-stack or full-register replacements.

## Complete source table

The four-entry table maps bounds refresh to existingC57C40, scalar deletion to
R134's4062C0, and support queries to the two new methods. The standard-layout,
noncopyable owner contains the table first, followed by borrowed pool/mode pointers.
The scalar method captures its pool before stamping the shape's base profile.
The table supplies Win32 callable slots; it does not supply original-image RTTI or
exception metadata. Other shape classes and allocator-class tables remain separate.

## Validation

- Strict MSVC Win32 build and all three existing CTests pass.
- 1,909 live Ghidra bytes match the original PE:920 new support bytes,145 existing
  CRT bytes,775 existing bounds bytes,29 existing scalar bytes and40 data bytes.
  No listing repair was needed; complete body bounds and direct calls were checked.
- **4,464 native/source pairs match1,196,352 bytes**, including exact outputs,
  ordinary graph writes, x87 control/status/tag state and MXCSR. The fixture executes
  copied original queries and both original CRT paths, over an actually produced
  eight-vertex hull. It crosses both conversion modes with three x87 precisions and
  four rounding settings, seeded status flags, several transforms and output aliases
  with direction, matrix or mesh vertex storage.
- All four production table slots were called through `__thiscall`, including
  changing the borrowed CRT mode after owner construction and reusing a deleted
  shape slot from the original pool. Bounds smoke checks the existing source result
  and body/shape writes; this is not new independent native bounds proof.
- A36-case physics comparison uses the production table identity in source world
  teardown and matches790 observations /20,777,328 normalized bytes. R134's failure,
  replay, missing-context and foreign-table checks remain passing.

An initial fixture mismatch came from compiler padding in its result structure.
Those bytes are now explicit initialized DWORDs. Final comparisons include exact
floating-point results and state; no tolerance or status masking was added.

## Limits and follow-up

The mesh, adjacency and signed seed must be valid. The original hull producer leaves
center seed13 unspecified, so zero/short transformed directions that select it are
not given invented behavior. Nonfinite or malformed inputs, private stack aliases,
unmasked traps, concurrency, native FH3/SEH and whole-program ABI remain unproved.
Constructors are shared services in the fixture. R134's task, controlled normal CRT
iterator and post-comparison cleanup limits still apply to the physics envelope.

This supplies a complete callable table for ConvexMeshShape. Other shape classes,
SAP update/dispatch methods, allocator teardown, raw-game admission and gameplay
remain open. See `reports/native_dyn_convex_support_r135.json` for exact evidence,
annotations, fixture provenance and artifact archives.
