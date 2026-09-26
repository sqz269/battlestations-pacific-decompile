# Blast part-hit entries per GeomMesh element, and the 20 hull-segment healths

Addresses: 0070F720 00723F80 00723B70 0085BF90 0085DF60 006D2E30 008777D0 008778E4 0087790F
00877A37 009635D0 004407A0 0092D1F0 00937C90 0092CED0 00821FF0 0080E440 00934150

Packet `cc9_blast_element_parts`, Ghidra read-only. Every descriptive name is a hypothesis, not a
recovered symbol. `docs/HIT_HULL_SEGMENT.md` (1b, the sphere chain), `docs/NARROWPHASE_UNIT_PART_SHAPE.md`
(the element box arrays), `docs/UNIT_HIT_PATH.md` (the part pass), `docs/PART_DAMAGE_REACHABILITY.md`
(the 20 slots), `docs/UNIT_PARTS.md` (message 99h) and `docs/EXPLOSION_RADIAL_DAMAGE.md` (the burst)
are cited, not restated.

## 1. What a burst on a ship records in the image

The burst gathers collision nodes (`0098C630`); a ship's unit-part node answers through its sphere
slot `0070F720`, which runs `00723F80` over the node's GeomMesh. For **every** element in order,
`00723B70`:

1. rejects when the squared distance from the centre (in node space) to the element's root box
   (`0085BF90(centre, element+20h, element+24h)`) is above `radius^2` (`00723B98 FCOMIP / JBE`);
2. walks the element's triangles through `0085DF60` (point-triangle squared distance; the
   decompilation shows Eberly's region split) and keeps the smallest value **strictly** below
   `radius^2` (`00723CFB`);
3. on a hit writes `sqrt` of it (`00723D34 CALL 00BF7030`) as the distance.

Each hit appends `{element+4h, element+8h, 0, distance}` through `006D2E30`. A ship whose elements
all miss produces no entry.

`008777D0`'s part pass (`008778E4..00877A37`) then skips entries with `+4h == -1`, takes
`[unit+354h]->vtable[24h]()` as armour for `+0h == 4` (`0087790F`; the ship class slot is
`009635D0`, `UnderwaterArmour` at `+6B4h`) and `unit+368h` otherwise, times the gameplay modifier
(1.0 here). It keeps the **largest** `004705C0` result, starting from `-FLT_MAX` (`00D7A244`), and
adds that one value (`00877A37`). The host added every entry, which only agreed while there was one.

`00826F10`'s R11b calls `0092D1F0` for every entry whose kind is `0Dh` (fizika), on a ship of mass
100 or more.

## 2. The 20 segments

`00937C90` sizes `controller+310h` to 20 floats. Each starts at class HP (`+48h`) divided by the
number of `fizika_NN` model nodes found, and gets a gate byte at `controller+34Ch+i` that is -1
for an index with no node. `0092D1F0` refuses a gated index. Otherwise it subtracts the damage
while the slot is above -10000.0. At 0.0 or below it parks -10000.0 and routes message 99h
(`0092CED0`), which `00821FF0` hands to `0080E440 -> 00934150`. From its callees, that is scene-node
unlinking, point effects, the dynamics list and `DYN_physics`: debris and the hull body, not guns
or engines.

Labelled: the node census reads the MMOD hierarchy names `fizika_00..fizika_19`
(case-insensitive). The image's lookup goes through `[[controller+1Ch]+4A4h]`, which is not read.

## 3. Switches

| switch | binds |
| --- | --- |
| `kBlastElementEntriesBound` | the element entries of a burst on a meshed ship, the max rule, the -1 skip and the kind-4 armour |
| `kHullSegmentHealthBound` | the 20 slots, `0092D1F0`, the destroyed list (`GameGunneryHost::destroyed_hull_segments`) |

## 4. Predictions (written before any run)

Same-tree builds `be_off`, `be_elem` (entries only) and `be_all` (both), with
`BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, on USN02 9200/9000 and USN04 4700/4500.

1. **Entries.** A contact burst sits 5 cm off the struck triangle, so its nearest element is at
   about 0.05 m against the box's 0, and its damage falls by about 0.1%. A near miss is measured to
   the triangles instead of the enclosing box, so it is farther and weaker. Some near misses find
   no element and do nothing (`blast_no_entry > 0` on USN02). `UnderwaterArmour` (at least `Armour` on
   the Lexington and Fletcher rows read) only wins when no other element is as close.
   * USN02: total damage and element-blast damage fall a few percent. Some ship deaths come
     later, and one or two may not happen within 450 s. The 44.60 s failure does not move.
   * USN04: no ship took gunnery damage inside 4500 frames on current main, so identical.
2. **Segments.** No unit's health changes, so the `be_all` death table matches `be_elem` line for
   line. Each slot holds HP / fizika nodes (for example 2500 / 4 = 625 on a destroyer). Direct
   fizika hits of 100 to 400 and torpedo bursts of up to 1200 per fizika entry should destroy 5 to
   30 segments on the USN02 9000 run. `segment_gate` counts fizika indices with no node.

## 5. Results

Pending.
