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

## The one function that gates both blockers

**`0082FE30` `BSP_ShipClass_BindModelData_Provisional`** - body `0082FE30`-`00831801`, roughly
9.9 KB, `void __fastcall(ship class descriptor)`. It is **slot 8 (`vtable+20h`) of every ship-kind
vtable** (`00D1ACE4`, `00D1AD18`, `00D1AD58`, `00D1AD98`, `00D1ADD8`), so every ship class runs it.
Only `0082FEA9`-`0082FEE8` has ever been read - about 64 bytes of 9.9 KB.

Its string immediates are model **node names**: `shipcenter`, `deckline`, `bottomline`, `wave`,
`wave_stern`, `explosion`, `idle`, `path`, `farviz`, `debarkation`, `kemeny`, `orrhullam`. A
function that looks up a dozen named node groups in a model is where a class learns its model's
structure.

That makes it the common ancestor of both stuck numbers:

1. **Part damage** needs a decoded element list attached to the unit for the narrowphase to walk.
2. **The barrel-count divergence** needs the model's `fire` node group, which is the same kind of
   named-node lookup this function performs.

Recovering `0082FE30` is therefore worth more than either blocker taken alone. It is large, so it
suits the listing-sweep method for oversized bodies - anchor on the EH-state `ESP` stores and
back-propagate - rather than a decompile.

## Packet order, smallest first

1. **`ship_class_bind_model_data`** - recover `0082FE30`. What model does a ship class open, how
   does it reach the node groups, and what does it keep? Read the listing; the body is too large to
   decompile. This is the prerequisite for everything below and pays for itself twice.
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
