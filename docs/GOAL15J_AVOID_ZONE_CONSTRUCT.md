# Complete raw avoid-zone constructor

`construct_native_avoid_zone_0041ccd0` reconstructs the whole 994-byte body
`0041CCD0..0041D0B1` (279 listed instructions). It composes the genuine raw point,
bounds, clip, winding and ancestor entries with the current allocation service.
The older typed interface in `avoid_zone_owner.cpp` remains separate and unchanged.
`BSP_AvoidZone_ConstructFromScenePath` remains a descriptive hypothesis, not a
recovered C++ symbol.

The worker started from main `4ddbdcb55` and merged reviewed complete clip commit
`80309742e` through dependency merge `05dddcb26`. No CMake, shared registry or
Ghidra mutation is part of this packet. Primary accepted and registered clip in
`020cca8eb` (receipt `bc9054aa7`), with three CTests, complete emitted mapping and
its archive-only real-heap fixture passing. Constructor registration and its own
final registered archive-only probe remain primary integration gates.

## Physical contract

Original: ECX is actual writable 24h zone storage, followed by the path and layer
DWORDs on the stack; EAX returns the receiver, and `RET 8` consumes both words.
The corrected Original prototype remains unchanged. Source exposes:

```cpp
void* __fastcall construct_native_avoid_zone_0041ccd0(
    void* actual_zone, void* const volatile* actual_world_pointer_cell,
    void* actual_path, std::uint32_t layer_word);
```

Source EDX borrows the address of an actual live world-pointer cell. It is an
explicit Source binding, not an Original argument. The address is captured at
entry, but its current pointer value is loaded only at the late Native world-load
point. No world snapshot or default object is supplied. The caller owns the cell,
world, raw object and reached vtable backing for the whole call. There is no
`noexcept` promise on the public entry.

The 18-byte naked public thunk passes the address of its actual path/layer stack
cells into a current C++ EH frame and performs `RET 8`. The private complete
algorithm kernel uses persistent raw scratch storage; it does not manufacture a
second path/layer pair. All eight accesses to the incoming cells are explicitly
lowered to that original address. In particular the incoming layer value is stored
to zone+C before its actual word is reused for signed pointcount, and the actual
path word is reloaded inside the point loop and again before association.

The zone fields are pointer0/count4/capacity8/layerC/association10/bounds14..20.
The constructor first zeros pointer/count/capacity, then reads the actual layer
and path words, publishes layer at +C, and clears association at +10. A path whose point list is null or has signed count zero skips point
construction and leaves the bounds unwritten. Subsequent clip behavior is retained;
no synthetic safe-empty branch is added. Backing and alias relationships remain
the caller's responsibility.

## Complete operation schedule

Point zero seeds the X/Z bounds and last accepted position. Each accepted point
allocates and fills one raw 24h corner, including the recovered zero fields and
clearance `-1`. The later-point filter retains the original x87 products, sums,
float spills, compare order and branch flags. A squared distance of exactly 25 is
rejected; unordered comparisons also take the rejection edge. Accepted points
update bounds through the actual raw bounds entry.

Both inline pointer-array append/growth schedules remain complete: equality with
current capacity gates growth, unsigned `2*capacity+2` must exceed the old value,
new capacity is published before allocation, and unsigned `MUL 4` plus overflow
flag saturates the requested byte count to `FFFFFFFF`. Copies reload the current
owner pointer/count, and the old current array is freed before replacement is
published. The pending 24h record has no extra guard.

After all point/allocation work, the Native load at `0041CFEE` is rebound to a load
through the captured actual world-cell address followed by a load of its current
value. The six physical field reads and intervening scratch/push instructions
remain in this order:

| World offset | Meaning passed to clip |
| --- | --- |
| 711C | minimum X |
| 7120 | minimum Y |
| 7128 | maximum X |
| 712C | maximum Y |
| 7124 | maximum Z |
| 7130 | minimum Z |

The genuine full clip entry is followed by a current count reload. Nonzero count
invokes genuine winding with flag word 1. Clip itself has no derive edge. The
constructor then makes up to three distinct ancestor(level1) calls. It rereads
path+14 for each call; the second result supplies the actual vtable slot+5C,
invoked with code44 and AL predicate, and the third result is stored to zone+10.
The literal indirect `CALL EAX` remains present. No ancestor cache or callback
predicate replaces this path.

## Exception and lifetime contract

The complete Native metadata was read independently: handler `00C5E198` selects
FuncInfo `00D842D8` (`19930522`, maxState1, zero try blocks); unwind map `00D842D0`
contains `[-1,00C5E190]`. Its action reads the current saved receiver from
`[EBP-68]` and tail-transfers at `00C5E193` to genuine `00412DC0`.

State0 is armed at `0041CD11`, before point work, and stays active through clip,
winding and association. The sole constructor unwind action frees the current
array and zeroes pointer0 through that real child. It does not free corner records,
reset count/capacity, roll back association or guard a pending allocation. Normal
return performs no ownership cleanup.

Source has one current C++ guard referencing the working set. On unwind it tests
the mapped phase and reloads the current saved-receiver word before calling
`free_and_clear_native_avoid_pointer_00412dc0`. The emitted current compiler frame
has maxState2 and one guard unwind action. Its extra state, cookie, frame addresses,
and destructor's current C++ termination policy are explicit Source EH boundaries;
they are not Native FH3/SEH/throw/catch identity. Native scratch addresses are
rebased, while the actual incoming argument cells retain their identity.

## Concrete providers and remaining boundaries

| Original provider | Source provider |
| --- | --- |
| 007AF800 | `transform_native_scene_path_point_007af800` |
| 00415010 | `include_native_avoid_bounds_point_00415010` |
| 0041A540 | full `clip_native_avoid_zone_world_bounds_0041a540` from 80309742e |
| 0041A200 | `derive_native_avoid_zone_0041a200` |
| 00923810 | `get_native_entity_ancestor_00923810` |
| BF681B / BF55BE | fixed bridge to actual `singleton_lifetime_allocate` |
| BF6989 | actual `singleton_lifetime_free` |
| 00412DC0 unwind | `free_and_clear_native_avoid_pointer_00412dc0` |

All 14 normal direct transfers and the indirect vtable transfer are represented.
The paired current heap retains its real allocation/new-handler/failure behavior;
this packet does not reconstruct private Native CRT dispatch, allocation failure
payload/RTTI or global state. The point child retains its current SDK invalid
parameter boundary. Winding uses its accepted hidden-x87 current UCRT services,
not an ordinary-double bridge or a claim that the small Native CRT entries are
reconstructed. No typed access callbacks or substitute child stubs are used.

A genuine process world cell exists in `GameNativeSettingsProcess`, but production
constructor wiring and the six raw world-bounds producers remain unproved. This
API accepts that cell type; it does not claim production adoption, game validation,
drop-in Native ABI compatibility or Native frame/private CRT identity.

## Verification

Target-verifying `bsp.py` captured the whole assembly, exact bytes, EH metadata and
constant data from existing project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 base00400000. The 994 captured bytes equal the
installed PE. Native data `CE3880=0000000000003940` (double25) and
`D7A260=000080BF` (float-1) are bound to actual read-only Source data.

Strict MSVC Win32 `/MD /O2 /W4 /WX /EHsc /fp:strict /permissive-` compilation passed.
The ignored checker verifies the emitted 1066-byte private kernel, all 279 mapped
Native rows, all 24 branch destinations, all 81 FP operations, every one of 14
provider CALL relocations and three read-only data operands, the literal vtable
CALL, current receiver reload, current EH guard and exact public 18-byte thunk.
This is complete Source instruction/operand composition proof, not whole-body
Native binary/frame equivalence.
The report call-site checker also passed all 15 direct call/unwind-tail rows.

One ignored combined real-provider fixture passed. It repairs an actual dirty
path/pose through the real current SDK invalid-parameter service, changes the
actual public path argument cell, and publishes an initially null world cell
during point work. It checks layer-word reuse, point-loop path reload, late world
publication, rejection at exact distance5, accepted count4, bounds, full clip and
winding. A second phase in the same fixture reaches actual saturated `FFFFFFFF`
allocation failure through the real heap/new-handler service and verifies pointer0
is cleared while published count/capacity remain. The captured first record is
freed explicitly by the fixture. The Native-unguarded pending record is intentionally
not retroactively cleaned up; process exit reclaims it. This is not a leak-free
rollback claim. The nonnull three-ancestor/vtable association path has emitted
coverage but was not dynamically exercised by this fixture.

The fixture used the strict constructor object, complete clip803 object and the
integrator's actual registered core archive. Final registered archive-only linkage
is a primary gate. `scripts/build.ps1` also passed both existing CTests; the new
module was unregistered in the worker build, so that run does not cover it.

Reproduction helpers under ignored `local/goal15j_avoid_constructor/`:

* `verify_emitted_constructor.py [registered-object]` checks the final constructor
  object and writes `emitted_receipt.json`.
* `probe_registered.cmd <registered-bsp_core.lib>` links only the archive for the
  primary final probe. Its optional second/third arguments were used for the
  worker's pre-registration constructor/clip objects.
* `native.asm.txt`, `native.bin`, `original_eh.txt`, `kernel_manifest.json`,
  `object.dump.txt`, `probe_result.txt` and `standard_build.log` retain local evidence.

The tracked report records final Source/object hashes and all direct transfer rows.
