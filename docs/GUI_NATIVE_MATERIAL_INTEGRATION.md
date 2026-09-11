# Canonical native materials, pools and GUI color

GUI color now reaches the same actual material retained by the model/mesh/section
chain. Material construction, cloning, final-zero deletion and both slab pools
are integrated over their real storage and existing owner/allocator domains.
The source and focused probes are pinned to commit `b527f87` in the report.

## Integrated behavior

`NativeMaterialStorage` models the actual110h prefix; `NativeMaterialReference`
borrows its single atomic+04. The114h pool slot retains its live slab index at
+110. Construction, clone and destruction preserve native uninitialized bytes,
actual resource counts, current deletion profile and release-before-clear order.
See [material owner evidence](NATIVE_MATERIAL_OWNER.md).

The concrete material and parameter pools share the existing allocator list and
use their actual38h globals/critical sections. Their64x114h and128x88h slabs,
free stacks, table growth, trim compaction and moved slot IDs are reconstructed.
The parameter return fragment begins after its real NativeString destruction;
it reloads the same slot's+84 under the lock and does not release the name twice.
See [pool evidence](NATIVE_MATERIAL_POOLS.md).

AA6870 resolves raw section+20 through the model's same `NativeRenderActualOwners`,
requires its canonical live material companion, and writes the original forward
DWORD copy sequence to material+38. It preserves flag+10C and reference counts.
`GuiMaterialBindingServices` now requires that owner domain in its last field,
replacing the semantic color-material callback. The canonical model guard still
rejects group-backed roots whose+180 is an integer capacity. See
[raw color evidence](GUI_RAW_MATERIAL_COLOR.md).

## Validation

- MSVC Win32 Release and both existing CTests passed after all three source
  packets were combined. Eight native seed spans matched.
- Four original material ordinary/clone/flags0 destruction comparisons matched
  all114h bytes. A real generic source owner reached zero while material+0C
  remained visible to its terminal callback, then the field cleared.
- The pool fixture passed33-slab growth for both pools, shared-list trimming,
  moved slot IDs, unchanged payload/address, LIFO return, real lock behavior,
  and actual material final-zero/name/parameter/pool retirement.
- Four original AA6870 comparisons passed over the actual model/mesh/section/
  material chain, including self-source, lighting+4 and forward-overlap inputs.
  Counts remained1/1/1/1, flag+10C remained81, and teardown returned the complete
  chain to zero without a protective retain.

Worker artifacts are preserved with hashes in `local/gui-native-material-workers`.
The report also pins separate copies of the tested library, executable and CTest
log, so later builds cannot silently change the recorded validation evidence.

## Saved analysis

Twenty selected standalone functions already existed. Their previous names and
comments were captured, annotations applied under the write lock, and names and
comment preservation verified before refreshing exports. B193FA remains an
embedded fragment of B192F0; no independent function was invented for it.

Eleven false free-call continuations across nine pool functions were repaired.
All nine then reported zero remaining call gaps. Unreachable alignment after
unconditional jumps was left alone. The separate AA9730 stored-body limitation
from the preceding batch is unchanged.

## Remaining requirements

Color publication no longer requires a second material copy. Clip/owner
registration still uses the semantic `MaterialCloneState` parameter table and
requires actual widget/page/ancestor lifetime. Native parameter allocation and
registration, factory effect acquisition, pool static routing, and coordinated
widget identity/deletion remain separate work. These fixtures do not execute
original pool instructions or exercise allocation-failure unwind. Nonnull
effect/texture ownership, a complete renderer draw, binary ABI replacement and
gameplay remain unvalidated.
