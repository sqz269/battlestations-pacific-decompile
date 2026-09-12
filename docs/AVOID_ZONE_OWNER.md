# Avoid-zone construction and owned corner storage

Addresses: 004167C0 0041A540 0041CCD0 007AF800

`avoid_zone_owner.hpp/.cpp` reconstruct the complete scene-path constructor
through real borrowed scene, allocator and CRT services. The constructor owns
its pointer allocation and separately allocated 24h corner records. It reuses
`ShipAiPathLateralRecord`, its first-eight-byte list view, the exact native
vector transform, the canonical pose refresh and the existing corner derivation.
No default scene, allocation stub, identity transform or generic polygon producer
is substituted. Descriptive names are hypotheses, not recovered symbols.

| Routine | Original ABI / inclusive native body | Coverage |
| --- | --- | --- |
| `0041CCD0` | ECX zone, stack scene path/layer, EAX zone, RET8; `0041CCD0-0041D0B1` | complete body control flow and allocation ownership through explicit services; native SEH ABI replaced by equivalent C++ cleanup |
| `0041A540` | ECX zone, stack minXYZ/maxXYZ, RET8; `0041A540-0041AC53` | complete clipping, record rebuilding and temporary allocation lifetime |
| `004167C0` | ECX zone, no stack arguments, RET; `004167C0-004167F7` | complete deletion loop and both return paths, including stored-flow gap |
| `007AF800` | ECX scene path, stack output/unsigned index, EAX output, RET8; `007AF800-007AF8D2` | complete transform through canonical pose/vector routines and explicit invalid-parameter service |

All four original bodies were read in pseudocode and full assembly. There are
no unread body branches projected away. Native allocation/CRT exception object
identity, SEH frame ABI and concrete entity-specific vtable dispatch are external.
These are new C++ interfaces, not drop-in binary replacements.

## Producer layout and source input

The Win32 `AvoidZoneNativeStorage` is exactly 24h bytes. It contains:

| Offset | Field | Producer evidence |
| --- | --- | --- |
| `00h` | corner pointer array, `corners.records` | zero `0041CCEF`, replacement `0041CE2F/0041CFA3` |
| `04h` | count, `corners.count` | zero `0041CCF5`, increment `0041CE40/0041CFAD` |
| `08h` | unsigned capacity | zero `0041CCF8`, store before allocation `0041CDE8/0041CF5C` |
| `0Ch` | layer bits, unsigned semantic interpretation not required | supplied stack word copied at `0041CD06` |
| `10h` | associated parent entity pointer | zero `0041CD09`, final parent lookup stored `0041D099` |
| `14h,18h,1Ch,20h` | minX,minZ,maxX,maxZ of retained source points | first point `0041CD43/48/73/78`, later `00415010` call `0041CFB9` |

Count storage reuses the existing signed-index list. Constructor/clip capacity
and count comparisons are unsigned; the source-point iteration limit is signed.
There is no duplicated semantic corner type. The record's nine floats retain
the producer layout established by `docs/SHIP_AI_LATERAL_RECORD.md`, including
`clearance_scale` at record+20h. Zone+20h is separately `max_z`.

Scene path+08h/+0Ch is a pointer range. Count is `SAR(end-begin,2)`, or zero
when begin is null. Each entry points at local XYZ. `007AF800` first accesses
scene path+14h's pose; if byteC8 is zero, canonical `00414DB0` refreshes it.
The bounds check can call `00BF6713` and return. The pointer range is re-read
after that service, so a repair is visible. The point is expanded to `(x,y,z,1)`,
transformed by entity+CC through full `00B62D10`, and divided by homogeneous w.
Reciprocal w is rounded once to binary32, then each product is rounded. No w
guard is added. The constructor retains transformed x/z and discards y.

The first point is always retained. Each subsequent point is compared against
the last retained point: x/z differences spill to float, squared sum retains
x87 intermediates until one float spill, and only ordered `sum > double(25)`
allocates a record. Equal distance and NaN do not retain a point. Bounds are
expanded only for retained points. The source limit is re-read after point0,
then captured for the remaining loop, matching `0041CE43-0041CE62`.

## Clip order and edge conditions

The constructor's world loads have a material noncontiguous mapping:

```
minimum XYZ = world[711Ch], world[7120h], world[7130h]
maximum XYZ = world[7128h], world[712Ch], world[7124h]
```

The z endpoints cross the adjacent XYZ groups. The stack-changing instructions
`0041D033-0041D04D` establish this mapping, not an assumed bounding-box layout.
The API takes these assembled values explicitly; it introduces no global owner.

Before allocating clip scratch, `0041A540` clears records and returns if any
ordered comparison holds: minX>=maxWorldX, minWorldX>=maxX, minZ>=maxWorldZ,
or minWorldZ>=maxZ. Touching bounds are rejected. Unordered comparisons do
not take this reject path. The bounds remain unchanged even on rejection.

Otherwise four half-planes are built in this order, with constants rounded
from double intermediates (`00D7A220` is double100):

1. `(1,0,100-minWorldX)`
2. `(-1,0,maxWorldX+100)`
3. `(0,1,100-minWorldZ)`
4. `(0,-1,maxWorldZ+100)`

Each plane distance is binary32 after extended `x*nx + z*nz + constant`.
Current distance strictly positive emits the current point, preceded by an
intersection only when previous distance is strictly negative. Otherwise an
intersection is emitted only when previous distance is strictly positive.
The current point at distance zero is not itself emitted; an exiting edge can
emit the same position as an intersection. NaN current distances use the latter
branch, including its previous-positive condition. There is no epsilon.

Entering ratio is `-previousDistance/(currentDistance-previousDistance)`;
leaving ratio is `previousDistance/(previousDistance-currentDistance)`.
Differences, ratio, scaled differences and final sums retain the native float
spill boundaries. Each pass starts with the last input point. Any pass yielding
fewer than three points stops and leaves the zone empty. Four successful passes
allocate fresh records and write clipped x/z before the constructor calls
`0041A200` with flag1. Bounds deliberately retain the pre-clip source bounds.

Empty scene input leaves the original four bound words untouched before calling
clip. If those bounds do not force rejection, native clip reads before its empty
temporary array at `0041A722`. The implementation adds no synthetic empty polygon
repair: callers need valid/readable native state. A one/two-point source can
enter clip and is dropped when a pass has fewer than three points.

## Allocation and identity lifetime

Records use `00BF681B(24h)`. Pointer arrays use `00BF55BE` and grow only at
count==capacity to unsigned `2*capacity+2` when that is larger than the old
capacity. Capacity is stored before allocation. Overflow of capacity*4 requests
FFFFFFFFh bytes, matching `MUL/SETO/NEG/OR`; growth overflow is not repaired.
All live pointer entries are copied and the old array is freed at `00BF6989`.
The native scalar and array allocators were read: malloc, new-handler retry,
then throw on failure. Required callbacks must supply that actual contract.

`004167C0` calls `00BF65AC` for each record and sets count zero, retaining
pointer allocation and capacity. Clip initially allocates two arrays of count*8
bytes, copies x/z to the first, and clears original records. Scratch growth uses
unchecked wrapping `(2*capacity+2)*8`; it does not use the pointer growth's
saturation check. Both scratch arrays are released in reverse declaration order
on return and unwind. Rebuilt records reuse the retained pointer allocation.

Constructor exception handler `00C5E198` references unwind table `00D842D0`:
`00C5E190` calls `00412DC0`, which frees and nulls the pointer array only.
The missed `00412DD2` store was checked as bytes/assembly. It does not destroy
allocated records, reset count or reset capacity. C++ catch preserves this
exceptional leak behavior. Clip's table `00D8423C` names cleanup thunks
`00C5E150/00C5E15B`, each calling `00412EF0` on its scratch array.
Native SEH metadata and CRT exception identity are not reproduced.

After successful nonempty clip/derivation, three possible `00923810(1)` calls
are kept separate. That callee starts at entity+3Ch and follows the chain for
additional signed generations. First lookup checks nonnull, second supplies
the actual parent to its vtable+5Ch with argument44h, and third is stored only
when AL is nonzero. The entity-specific vtable body is unresolved; the access
method retains its slot name and must dispatch the actual supplied parent.

`avoid_zone_release_owned_storage` is an explicit C++ cleanup convenience:
it clears records, releases the pointer array, and zeros pointer/capacity. It is
not attributed to an unread native destructor. Construct only into fresh storage.

## Evidence and verification boundaries

Whole listings establish register provenance: constructor ESI remains zone,
EBX starts as scene, becomes source limit/index, then reloads scene before
association; EBP keeps each pending corner; EDI alternates zero/new array.
Clip saves/restores zone EDI around growth copies, alternates source/destination
array descriptors in ECX/ESI, and uses EBP for saved count/pointer state. World
point ESI preserves scene, EDI starts as pose and changes to the unsigned index,
while EBX preserves the world-matrix address across validation.

All direct call sites and one unresolved vtable call are listed in the report.
Allocator/free pushes have ADD ESP,4; point/clip RET8 and include-point RET4
were read. No saved Ghidra state, names, comments, definitions or flow were
modified; the four affected exports were refreshed read-only. Existing names
are preserved in the ledger and extended with this packet's evidence.

Stored-flow gaps are explicit in the report. `004167DB-004167F1` includes the
deletion-loop continuation and its separate RET; the other clear return is at
`004167F7`. Constructor gaps at `0041CE2C/0041CFA0` and clip's seven gaps
after free are ADD ESP,4, not return paths. Padding gaps were read separately.
Unrepresented SEH dispatchers have last-instruction addresses/sizes recorded
under `no_ghidra_function`; they were not defined or annotated.

The Win32 build and existing CTest passed (1/1). The report-call verifier passed
39 direct rows; the entity-specific vtable row is explicitly unresolved. The
ignored manifested local clip probe produced an eight-point octagon from a
four-point diamond, grew capacity4 to10, retained the original bounds, rejected
touching bounds, and released12/12 record and6/6 array allocations. Paths and
results are recorded in `reports/avoid_zone_owner.json`.

No tracked test cases were added. The probe covers clip, record rebuilding,
clear and release with real allocations. Constructor/scene/association behavior
was build-checked and statically inspected here; the integrator owns its
separate whole-path fixture. This is not an original-whole-body differential
fixture. This packet does not wire `bsp_game.exe`, and makes no frame behavior
or gameplay claim.
