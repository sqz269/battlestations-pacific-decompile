# What stands between the current build and a non-zero `part` count

Two gameplay numbers have been stuck at a faithful-looking zero for several milestones:
`part=0 fires=0 floods=0` in the damage summary, and a barrel count that diverges from the authored
weapon rows. This sizes the remaining gap with measurements rather than estimates, so the work can
be dispatched as packets instead of rediscovered.

## The measured fact: the host never opens a model

`IJN01`, 3000 mission ticks, 78 units and 606 guns:

```
grep -ic mmod local/cc7_ijn01_3k.log   ->   0
```

**Not one `.MMOD` is touched for the entire mission.** This is the honest size of the gap. It is not
that models load and their parts are ignored; the model load path does not exist in `bsp_game.exe`
at all. Every conclusion below follows from that.

## What is already in hand

- **The payload decoder works on real authored data.** `docs/GEOM_MESH_RESOURCE.md` section 5
  decodes chunks out of nine real `.MMOD` files from this installation - `Farragut_1934`,
  `North_Carolina`, `Yamato1945`, `yamato`, `Colorado`, `Essex` - into named elements (`body`,
  `underwater`, `magazine`, `engineroom`, `fueltank`, `steering`, `fizika`), with
  `root remaining == 0` after every run, which is the reader's own statement that it consumed the
  payload exactly. The parser is `src/geom_mesh_resource.cpp`; it is registered nowhere in the game
  build.
- **The narrowphase exists** (`src/hit_narrowphase.cpp`, `src/narrowphase_unit_part_shape.cpp`) but
  runs on three hard-coded shapes that write kind `0Ah` (`none`) and index `-1`, so `hit+30h` and
  `hit+34h` can never name a part.
- **The triangle-to-element mapping is solved**: each element's ordinal list indexes `mesh+28h`, so
  a ray-triangle test that names a triangle yields the owning element, and therefore the
  `(kind, index)` pair, by lookup.

## The function that was supposed to gate both blockers, and does not

**`0082FE30` `BSP_ShipClass_BindModelData_Provisional`** - body `0082FE30`-`00831801`, roughly
9.9 KB, `void __fastcall(ship class descriptor)`. It is **slot 8 (`vtable+20h`) of every ship-kind
vtable** (`00D1ACE4`, `00D1AD18`, `00D1AD58`, `00D1AD98`, `00D1ADD8`), so every ship class runs it.
Only `0082FEA9`-`0082FEE8` has ever been read - about 64 bytes of 9.9 KB.

Its string immediates are model **node names**: `shipcenter`, `deckline`, `bottomline`, `wave`,
`wave_stern`, `explosion`, `idle`, `path`, `farviz`, `debarkation`, `kemeny`, `orrhullam`. A
function that looks up a dozen named node groups in a model is where a class learns its model's
structure.

~~That makes it the common ancestor of both stuck numbers... Recovering `0082FE30` is therefore
worth more than either blocker taken alone.~~

**CORRECTED. `0082FE30` gates neither blocker, and the reasoning above was wrong in a specific and
instructive way.** `docs/SHIP_CLASS_BIND_MODEL_DATA.md` swept the body and found:

* **It opens no model.** The first call after the prologue already dereferences `[class+50h]`, and
  across the whole 9.9 KB body that field is read thirteen times and **never written**. The model
  is already open when this runs; the function is a consumer, not the route in.
* **It never reaches the `Resource` container.** It touches exactly two model-side containers -
  `model+64h` nodes and `model+54h` pointers - and neither carries the `{kind +4h, index +8h}`
  elements `00727310` produces. It is downstream of the GeomMesh parser, not a way to reach it.
* **It does not bind `"fire"`.** That string does not appear in the body at all;
  `007325A0` binds it on the **gun** class's own `[class+50h]`, at indices 0 and 1.

So the inference "a function that looks up a dozen named node groups is where a class learns its
model's structure" was plausible and false. It looks up node groups because a model is already
there. The twelve string immediates were read as evidence of the model *arriving*, when they are
evidence of it being *used* - and the distinction is exactly what a byte census of `[class+50h]`
settles and a list of string literals cannot.

**The real gate is the producer of `class+50h`, and it is still not found.**

~~byte scans for `MOV [reg+50h], reg` and for every `CALL [reg+20h]` form turned up no candidate~~ -
**two of those negatives were vacuous, and this document published them as evidence.**
`docs/MODEL_HANDLE_PRODUCER.md` counted the encodings image-wide:

| form | occurrences in `.text` |
| --- | --- |
| `MOV [reg+50h], reg` **disp32** | **0** |
| `MOV [reg+50h], reg` disp8 | 244 |
| `CALL dword ptr [reg+20h]` | **0** |
| `MOV r,[reg+20h]` then `CALL r` | 287 |

`50h` is 80, which fits a signed disp8, so MSVC never emits the disp32 form for it; and this
compiler always emits the load-then-call pair for a virtual. **A scan for either zero row returns
nothing regardless of what the code does**, so neither could ever have supported a conclusion.
Confirmed here directly: `scan-bytes '89 ?? 50'` returns hits across the image, `'89 ?? 50 00 00 00'`
returns none.

**The rule that follows applies to every offset search on this image: count both encodings and check
the total is non-zero before reading anything into an empty result.** An empty scan is only evidence
once the pattern is known to occur somewhere.

What *is* settled: `class+50h` is null-initialised at construction (`0087C6A3` in
`BSP_DamageableClass_ConstructBase`), and nothing writes it in any form or window scanned - `MOV`
register and immediate in **both** encodings, `LEA` (the out-parameter form, 119 image-wide and none
in the class region), and `MOVSS`. The earlier exclusion of `00960230` now stands on byte evidence
rather than inspection: its `00962981` hit is the middle of a field-by-field copy of a ~`5Ch`-byte
record inside a Lua iteration loop.

Explicitly **not** excluded, and one of these must hold it since the shipped game loads models: a
`memcpy`/`REP MOVSD` covering the field, a SIB-indexed write (every scan skips those by design), a
write on an object later aliased to the class, or a window outside the two searched.

Two corrections to this document's own text while we are here: `0082FE30` occupies slot 8 on
**eight** class vtables, not the five listed above; and the invoker of that slot is also unfound -
none of the six slot-`+20h` dispatches in the class region is on a descriptor. The cheapest next
step is the **twenty callers of `BSP_VehicleClass_GetOrCreate`**, listed in
`docs/MODEL_HANDLE_PRODUCER.md`, of which exactly one has been checked.

What the packet does give is the consumption contract: sixteen descriptor fields mapped from twelve
named node groups, all of them **point data** rather than mesh data. Worth keeping for whoever
builds the model path, and worth noting that a missing `debarkation` node synthesises a twelve-point
ring from the class box half-extents rather than leaving the field empty, and that `wave` is
dereferenced at `0083089D` with no null check unlike every other optional group here.

## Packet order, smallest first

1. ~~`ship_class_bind_model_data`~~ **DONE, and it answered "none of the above"** -
   `docs/SHIP_CLASS_BIND_MODEL_DATA.md`. Replaced by **`model_handle_producer`**: find the writer of
   `class+50h` and the invoker of ship-vtable slot 8. That is the actual prerequisite for everything
   below, and it is currently unknown - the sweep ruled out `009633C0`, `00960230` and `00964020`
   and found no candidate by byte scan. Start from `docs/UNIT_PARTS.md`, which already carries this
   as an open item.
2. **`model_resource_load_path`** - the host side of reaching an `.MMOD`'s `Resource` container and
   dispatching it. `docs/GAME_RESOURCE_PARSER_REGISTRATION.md` has the native registration row.
   `docs/GEOM_MESH_RESOURCE.md` notes the registry needs a fourth variant alternative and that this
   **belongs to the registry's owner**, so it needs coordination, not a unilateral edit.
3. **`narrowphase_part_elements`** - the real blocker for `part`, and a narrowphase packet rather
   than a parser one: fill `hit+30h` and `hit+34h` from the element the ray actually hit
   (`00723F62` and `00723F6C` are the native copies) instead of the hard-coded shapes.
4. **`part_controller_slot`** - `00937C90` sizes the per-part health vector to exactly 20 entries;
   confirm the controller slot exists before expecting `fires` and `floods` to move.

## What this does not claim

None of this is reconstructed yet beyond the payload decoder. `part=0 fires=0 floods=0` remains a
**faithful** zero - the reconstruction reports zero because the executable it models genuinely has
no part data loaded, not because the damage path is wrong. It stops being faithful the moment a
model is loaded and the count stays at zero. **Gameplay validation of part damage is unsatisfied and
stays unsatisfied until then.**
