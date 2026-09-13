# Obstacle constructor and validity production

Packet `orch6_obstacle_owner_l` adds a **partial constructor field projection**
for the existing `ShipAiObstacleNode` and `ShipAiNeighbourNodeMotion` records.
It does not allocate a native node, register an observer, populate geometry,
or supply the missing per-frame neighbour binding. Base checkout: `4c97c137`.

```cpp
void ship_ai_obstacle_owner_initialize_projection_009e52e0(
    ShipAiObstacleNode&, ShipAiNeighbourNodeMotion&,
    const void* observed_owner, float lifetime) noexcept;
```

The projection assigns owner, lifetime, no-arc=false, pass-side=0, no-pose=true,
and cached maximum/minimum Y=-1000. It preserves every other semantic field,
including both headings, all footprint geometry, and `owner_gone_5e`.
The latter represents a byte in the observed unit, not a field written by the
node constructor. Neither existing type nor its defaults is changed.

## Coverage and ABI

| Routine | Native body | Coverage in this packet |
|---|---|---|
| 009E52E0 | 009E52E0..009E53A5, 55 instructions | All normal-path writes recovered; only represented fields projected |
| 00694A60 | Existing concrete observer registration | Reused contract; no new implementation or annotation |
| 009EAE20 | 009EAE20..009EAFB4 | Existing near refresh; validity writers inspected |
| 009EAFC0 | 009EAFC0..009EB651, including separately defined arc 009EB4D1..009EB610 | Existing avoid refresh; validity writers inspected |
| 009F0EA0 | 009F0EA0..009F115D | Native ordering inspected; existing source covers ageing/compaction only |

009E52E0 takes ECX=destination, then three stack dwords: observed owner,
controller pointer, lifetime float. EAX returns destination. RET 0Ch is at
009E53A3. ESI retains destination; EDI loads owner from ESP+20; EBX is zeroed
at 009E52F7 and preserved across registration. The constructor has one call,
009E532D -> 00694A60, and zero flow gaps. The sole incoming call is 009F0E53
inside 009F0D20..009F0E83. There, ESI is the controller from 009F0D37 and EBP
is candidate owner from 009F0D58. Lifetime is pushed first, controller second,
owner third, and ECX receives the successful 90h allocation from 009F0E2F.

The input lifetime is settings+194 plus the original double at 00CEC160,
spilled to float at 009F0DE1. Existing nodes for the same owner receive a
lifetime extension when the new value is ordered greater; the constructor
only runs for an appended node after successful allocation. The projection
accepts that produced lifetime rather than inventing one.

## Every normal-path constructor store

Offsets below are relative to the native 90h allocation. A zero is only written
where shown; this is not a zero-filled allocation.

| Store address | Native field | Value / ordering | In semantic projection? |
|---|---|---|---|
| 009E52FC | +04 dword | 0, observer edge array data | No |
| 009E52FF | +08 dword | 0, observer edge count | No |
| 009E5302 | +0C dword | 0, observer edge capacity | No |
| 009E530A | +14 dword | 0 before testing owner | Final value only |
| 009E530D | +10 byte | 1 | No |
| 009E5311 | +00 dword | native vtable identity 00CF5C94 | No |
| 009E5321 | +18 dword | 0 | No |
| 009E532A | +14 dword | nonnull owner, before registration | Yes |
| 009E5338 | +1C dword | controller argument, after registration | No |
| 009E5359 | +78 float word | input lifetime if owner nonnull, otherwise BF800000 (-1) | Yes |
| 009E5361 | +69 byte | 0 | Yes |
| 009E5364 | +88 dword | 0 | Yes |
| 009E536A | +75 byte | 0 | No |
| 009E536D | +8C dword | 0 | No |
| 009E5373 | +74 byte | 0 | No |
| 009E5377 | +68 byte | 1 | Yes |
| 009E537B | +7C float word | +0 | No |
| 009E5380 | +84 float word | C47A0000 (-1000), from 00D7A240 | Yes |
| 009E5388 | +80 float word | same -1000 word | Yes |
| 009E5390 | +70 float word | +0 | No |

These stores cover 61 unique bytes. The remaining 83 bytes are untouched by
the normal constructor body: +11..13, +20..67, +6A..6F, and +76..77. In
particular, neither box, either heading, nor corner axes are initialized.
The observer call may separately mutate edge arrays; its behavior is not part
of this write-mask claim. Existing `NativeObserverOwnerStorage` supplies the
actual 10h prefix and `register_observer_pair_00694a60` implements registration
with `NativeObserverLifetime`. A semantic node is not an alias to that prefix.

009E532D uses ECX=actual observed endpoint and EDX=actual node callback owner.
The existing callee captures the shared lock, finds an edge, then creates one
or increments its reference count. The projection omits that call and all
native observer storage. Native constructor exception handling, callback
dispatch, unregister/destruction, and allocation ownership are not supplied.

The prior Ghidra plate has three address/layout errors: the registration site
is 009E532D, not 009E5300; the vtable store is 009E5311, not 009E5310; the
controller argument is +1C, not +18. +18 is explicitly zeroed. Root owns the
annotation correction; this worker made no Ghidra changes.

## Validity byte sequence

The names `no_pose_68` and `no_arc_69` describe reader gates; they do not prove
that a mathematically valid footprint exists merely because a byte is zero.

1. Constructor writes +69=0, then +68=1. Default semantic geometry must not
   be substituted for the uninitialized native geometry.
2. Native 009F0EA0 first subtracts dt from each lifetime and spills the result.
   Ordered positive lifetimes survive. Zero, negative, or unordered results
   enter removal. For a surviving null/deleted owner it sets only +68=1 at
   009F110A; +69 and geometry remain unchanged, and neither box refresh runs.
3. For a live owner, 009F104D invokes 009EAE20. It writes heading, shared axes,
   near center and near extents. **It never writes +68 or +69.** Its own
   null/deleted-owner early return also leaves those fields unchanged.
4. The party filter is evaluated after near refresh at 009F1052..009F109B,
   then passed as the last stack dword's low byte to 009EAFC0 at 009F10FF.
5. If avoid refresh itself observes a null/deleted owner, it writes +68=1 at
   009EAFDB then +69=1 at 009EAFDE and returns without geometry changes.
6. Otherwise the model virtual +20 is queried. A nonnull first result causes
   a second query and bounds copy; +80 receives maximum Y and +84 minimum Y.
   With no model, cached bounds are retained, including constructor -1000.
   This is not a request to synthesize bounds from a different representation.
7. +69 receives `low_byte == 0` at 009EB040. Vertical comparisons reject when
   observed minimum exceeds self maximum, or self minimum exceeds observed
   maximum. They are JA branches, so equality and unordered comparisons do
   not reject at these gates.
8. A vertical rejection sets +68=1 at 009EB619, copies the near center into
   the avoid center, writes both avoid extents to 1, restores cached min/max
   Y to -1000, and leaves axes/headings unchanged.
9. After vertical acceptance and the settings lookup, the routine seeds both
   avoid extents to 1 and clears +68 at 009EB076. The later near-copy arm
   overwrites center, corner axes, extents and projected heading from near
   fields. The straight/arc arms write their own geometry.
10. The shrink rejection can set +68 back to 1 at 009EB2C5 before copying the
    near box. +69 retains the filter result across these geometry arms.

The point predicates now on main consume these bytes directly: near rejects
+68; avoid rejects +68 or +69. Both use the shared axes +28..34 even when
avoid corner axes +4C..58 differ. Neither predicate produces geometry.

There is a concrete **external NaN limitation** in the existing avoid refresh:
009EB2B5 compares 0 with shrink, and JC at 009EB2B7 proceeds for positive OR
unordered shrink. Existing C++ uses `if (!(shrink > 0.0f))`, which sets no-pose
for NaN instead. Finite shrink behavior agrees at this branch. This packet
records and reports the discrepancy without changing unowned refresh code;
the full refresh's floating-point behavior is not newly fixture-validated.

## Concrete next runtime binding contract

At base `4c97c137`, `GameShipAiHost` calls only the existing semantic
`ship_ai_neighbour_list_refresh_009f0ea0`, which ages/compacts and sets no-pose
for gone owners. That helper does not invoke near or avoid refresh. The host
records the add-list boundary; this packet does not claim a live neighbour
population or frame result. Root is keeping point-predicate binding pending.

A future binding needs these actual inputs and this order:

- Retain node/motion records across frames. Initialize a fresh admitted node
  with the produced owner and lifetime; source owner-deleted state from the
  actual observed unit. Retain real observer/owner lifetime separately if
  claiming native registration and destruction semantics.
- Prepare self inputs from the 009F0EA0 prologue before walking neighbours:
  world XZ position after the +C8 pose-cache gate; velocity virtual +34; the
  original magnitude/epsilon and class+500 * settings+1B8 speed-floor sequence;
  full hull length * the actual widened 0.55f double; and controller+1BC/+1C0
  cached maximum/minimum Y. The produced beam *0.75 stack slot is unread by
  avoid refresh. The current semantic ageing helper omits this prologue.
- For each live survivor, run existing near refresh with real observed
  heading virtual +50, body-axis speed, settings+1A8, full length/beam from
  +9C8/+9CC, and cached/updated world pose. Do not clear either flag here.
- Produce the party-filter byte using actual controller+3F0, observed category
  +54, self class+241, and stored settings+4. Then call existing avoid refresh
  with the nine native argument slots and real observed model/bounds,
  velocity, reference speed, length and command yaw services. Its settings
  fields are +1A0,+1A4,+1AC,+1B0,+1B4,+1BC,+1C0,+1C4,+1D0.
- Preserve the resulting flags and geometry through compaction into the
  sector scan. No default model box, constant flag clearing, or per-call node
  reconstruction can replace these producers.

Source-excerpt inspection establishes these dependencies; it is not runtime
proof. No changes to GameHosts, neighbour types, or refresh algorithms are
included in this packet.

## Verification and reproduction

Win32 Release and both existing CTests passed for the projection. All five
fixture cases passed, with 110/110 projected bytes equal to native output.
The ignored `local/obstacle_owner_probe.cpp` checks five constructor cases:
null owner, nonnull finite lifetime, negative zero, a NaN payload, and negative
lifetime. It executes all 198 original constructor bytes (55 instructions),
with the single observer call patched to a fixture spy. The spy checks the
ECX/EDX identities and captures that owner assignment precedes registration,
while controller and validity writes follow it. It performs no observer work.

All 61 native-written bytes and 83 untouched bytes are checked. Each of the
22 bytes projected into semantic fields is compared with the original output;
all remaining semantic bytes and headings are checked unchanged. The fixture
also checks native EAX destination, restored FS:[0], and x87 TOP balance.
Saved result records canonicalize owner/controller pointers only after checking
their actual identities. No native exception dispatch, observer allocation,
callback, destructor, whole-refresh, or gameplay execution is claimed.

Preparation verifies the original function and two constant words against
both the installed PE and live Ghidra. Two absolute constant operands are
relocated into the fixture image; the registration CALL targets the spy. The
original vtable and SEH handler words remain identities and are never invoked.

```powershell
./scripts/build.ps1
python local/prepare_obstacle_owner_probe.py
cmd /c local\build_obstacle_owner_probe.cmd
./local/obstacle_owner_probe.exe
python tools/verify_report_calls.py reports/ship_ai_obstacle_owner.json
```

The preparation script uses Python pefile/capstone and the existing configured
PE/Ghidra connection. The probe uses VS 2026 Community vcvars32, /O2 /fp:strict,
the current bsp_core.lib, and /MANIFEST:EMBED. Reported artifact hashes preserve
the exact sources, original bytes, relocation manifest, records and logs.

## Correction from the neighbour-frame producer review

The earlier next-binding assessment's “self class+241” label is incorrect.
At 009F1067, 0080E160 returns the director pointer from unit+738. The subsequent
filter therefore reads director+241, together with the persistent request and
stored settings+4. The original getter body is MOV EAX,[ECX+738]; RET.
The constructor projection and its fixture are unaffected. This correction
supersedes the earlier class-getter wording retained above and in the report.
