# The producer of the buoyancy element list at class+52Ch

Addresses: 0082D040, 0082FE30, 0082A920, 0082C960, 0082C4B0, 0082A280, 00759120, 00718000,
0071F9F0, 004215D0, 00963380, 00963600, 008936A0, 00933DA0, 009329C0, 00937C90

`docs/SHIP_HYDRO_FORCES.md` recorded that "the buoyancy element list's producer was not found,
so its field roles are a hypothesis reconciled between its two readers and labelled as such",
and `docs/SHIP_AI_CLASS_FIELD_0524.md` went further: "No site in the image appends to it, so it
stays empty in a shipped build." Both are wrong, and the same thing hid the producer from both.

**The producer is `0082D040`.** It never writes the displacement `52Ch`. `009329C0` does not
either: at `00932C3C` it materialises `class+528h` once with `ADD EDI,528h` and then uses the
one-byte displacements `[EDI+4]` and `[EDI+8]`. `class+528h` is the vector object's own base —
the `_Myproxy`/`_Myfirstiter` word that `_SECURE_SCL` puts in front of `_Myfirst`, which is why
`docs/SHIP_AI_CLASS_FIELD_0524.md` found "nothing writes `+528h`" and a `52Ch` scan found no
appender. The producer does the same and goes one step further: it hands `class+528h` to a
member function. `0082D4E9`/`0082D4ED` compute `EBX = descriptor + 528h`, and `0082D602` and
`0082D673` pass that in `ECX` to `0082C960`, which is
`std::vector<Element>::push_back(const Element&)` — `[ESI+4]`/`[ESI+8]`/`[ESI+0Ch]` are
`_Myfirst`/`_Mylast`/`_Myend`, the reciprocal `38E38E39h` at `0082C977` divides by `24h`, and
`0082C9CA` advances `_Mylast` by `24h`.

Both earlier searches scanned displacement forms of `52Ch`, `530h` and `534h`. No such store
exists outside `00963380`'s zeroing and `00963600`'s teardown, and none ever will: the only
writer is a vector method whose `this` is the vector, not the class.

## The path

`0082FE30` (body `0082FE30-00831801`, `void __fastcall(descriptor)`) is the ship class
descriptor's model-binding virtual. It sits at slot 8 of every ship-kind vtable: the base vtable
starts at `00D1ACC4` (`009633C0` writes it) and `0082FE30` is referenced from `00D1ACE4`,
`00D1AD18`, `00D1AD58`, `00D1AD98`, `00D1ADDC`, `00D1AE18`, `00D1AE58` and `00D1AE98`, each
`20h` past its own vtable head — `00D1ACC4 + 20h = 00D1ACE4`, and the Destroyer vtable
`00D1ACF8 + 20h = 00D1AD18`. The ninth kind overrides the slot with `00759120` (`00D1AEDC`),
which calls `0082FE30` at `0075913D` first and then adds its own `runwaycenter` and
`liftexitpoint` nodes.

`0082FEA9..0082FEE8` is the buoyancy step:

```
0082fea9  MOV ECX,[EDI+0x50]      ; the class's loaded model
0082feba  CALL 00718000           ; find the node named "deckline"    (00D09A0C, length 8)
0082feca  CALL 00718000           ; find the node named "bottomline"  (00D09A00, length 10)
0082fecf  FLD [EDI+0x71c]         ; Hull.WaterLineRatio               -> param_4
0082fedc  PUSH ESI                ; deckline record  + 44h            -> param_3
0082fee0  PUSH EAX                ; bottomline record + 44h           -> param_2
0082fee3  CALL 0082d040
```

`00718000` (`00718000-007180D5`, `void* __thiscall(model, const std::string* name, int kind)`)
walks the model's record vector at `model+68h..+6Ch`, compares each record's `std::string` name
at `record+0Ch` with `004B3FC0`, requires an exact length match and `record+24h == kind`, and
returns the last match. Record `+44h` is a `std::vector<Vec3>` of points; `0071F9F0` (a vector
assign) copies it out at `0082D1FE` and `0082D376`.

So the element list is **generated from two named polylines in the ship's model**, not authored
per ship and not derived from the hull's cross-section.

## What 0082D040 does

`void __thiscall(descriptor, const vector<Vec3>* bottom_points, const vector<Vec3>* deck_points,
float water_line_ratio)`, `RET 0Ch` at `0082D6FB`.

1. `0082D067..0082D192` walks the parts at `descriptor+50h` (`+10h` array, `+14h` count),
   calls a per-part predicate (`00721C90` then vtable slot `+0Ch`) and a per-part AABB
   (`00723170`), and accumulates a min and a max corner. **Nothing reads either corner
   afterwards.** The block is dead and is not projected.
2. `0082D1FE` copies the bottom-line points into a scratch vector, `0082D21F..0082D365`
   selection-sorts them into a second vector by ascending z (take the first strictly smallest
   z, `004215D0` push_back it, shift the tail down over it), and `0082D376`/`0082D380..0082D4C5`
   does the identical thing for the deck line. The sort is not cosmetic: the shipped DeRuyter
   stores its bottom line in descending z and the sampler below requires ascending.
3. `0082D4CA..0082D690` walks `Hull.Segments` stations and pushes **two** elements per station.

The station's z, `0082D500..0082D54E`:

```
segments < 2 -> 0
otherwise    -> i * Length / (segments - 1) - Length * 0.5
```

`Length` is `descriptor+A0h`, halved by the double `0.5` at `00D7A280` (`0082D1B2`). Both
profile lines are then sampled at that z by `0082A920`, and the record is built from the two
samples.

## 0082A920, the profile sampler

Body `0082A920-0082AA7C`, `RET 14h`, result on the x87 stack; five arguments, two of which are
the `_SECURE_SCL` iterator's owning container and carry no value. The contract is a clamped
piecewise-linear sample of y over z along a z-ascending polyline:

| case | result | site |
| --- | --- | --- |
| `z < first->z` | `first->y` | the tail return `*(float*)(param_2+4)` |
| z inside a span `p..q` | `(1-t)*p.y + t*q.y`, `t = (z-p.z)/(q.z-p.z)` | the loop body |
| z past the last point | `last[-1].y` | the `*(float*)(param_4-8)` return |

## The record, settled by its producer

Nine floats, `24h` bytes. The producer's own order (`0082D5E4..0082D651`) and the correct roles:

| offset | value the producer stores | role | site |
| --- | --- | --- | --- |
| `+00h` | `(Mass * 10.0 * 0.5) / draught / Hull.Segments` | buoyancy coefficient | `0082D637..0082D651` |
| `+04h` | `deck * (1 - ratio) + bottom * ratio` | the **waterline** | `0082D611..0082D622` |
| `+08h` | the deck-line sample at this station | the **deck line** | `0082D585`, stored `0082D58A` |
| `+0Ch` | the bottom-line sample at this station | the **keel** | `0082D5D8`, stored `0082D5DD` |
| `+10h` | `deck - bottom` | section height | `0082D604..0082D608` |
| `+14h` | `waterline - bottom` | the **draught** | `0082D629..0082D630` |
| `+18h` | `+Width*0.5` then `-Width*0.5` | lateral offset | `0082D507`, `0082D65D..0082D675` |
| `+1Ch` | the same deck-line sample as `+08h` | point y | `0082D58A` |
| `+20h` | the station z | point z | `0082D51C..0082D539` |

`Mass` is `descriptor+B0h`, `Width` is `descriptor+A4h`, `ratio` is `Hull.WaterLineRatio` at
`descriptor+71Ch` and `Hull.Segments` is `descriptor+720h`; the last two are the Lua keys
`docs/SHIP_CLASS_FIELDS.md` already binds to `00832D9A` and `00832DE8`.

`+14h` is the draught on the image's own authority, not by inference: `008936A0`, the
`luaMW_GetDraught` binding (the literal `luaMW_GetDraught failed:` is in its body), loops the
list and returns `max(element[+14h])`.

### Why the hull floats

`coefficient * draught` is `Mass * 10 * 0.5 / Hull.Segments` for every record, and there are
`2 * Hull.Segments` records, so the sum over the list is exactly `10 * Mass` whatever the model
geometry is. `00937C90`'s displacement sum (`00937DBB..00937F7A`) multiplies each term by
`Gravitacio / 10` and a shape factor, and then `00937F74` stores
`controller+84h = sum - Gravitacio * Mass`. With the shape factor at 1 the reserve buoyancy is
therefore **exactly zero**. The shape factor is `frac^Kitevo` with
`frac = |(bottom - deck) / (waterline - bottom)| = 1 / (1 - ratio)`, so `Hull.WaterLineRatio` is
the only thing that moves it off 1.

## The installed values for VehicleClass[20], the DeRuyter

From `scripts/datatables/autoload/vehicleclasses.lua` (block at line 13618) and
`models/ships/us/Deruyter.mmod`:

| key | value |
| --- | --- |
| `Length` | 171 |
| `Width` | 16 |
| `Mass` | 7688 |
| `Hull.Segments` | 5 |
| `Hull.WaterLineRatio` | 0.55 |
| `Mesh` | `models/ships/us/Deruyter.mmod` |

The model carries both nodes. Each is stored as a length-prefixed `Identifier` string followed
by a `Points` chunk holding a byte count and that many `float x, y, z`. The deck line has four
points and the bottom line six, stored in descending z:

| deckline (x, y, z) | bottomline (x, y, z) |
| --- | --- |
| -0.0953, 6.3400, 52.4409 | -0.0487, -4.9100, 63.5168 |
| -0.0953, 6.1480, 11.9249 | -0.0487, -5.0100, 55.5529 |
| -0.0953, 6.0555, 9.2761 | -0.0487, -4.9620, 29.7489 |
| -0.0953, 6.1280, -90.4472 | -0.0487, -4.9620, -45.8951 |
| | -0.0487, -4.9980, -71.1791 |
| | -0.0487, -4.8860, -81.6272 |

The ten elements that follow (the lateral offset is `±8.0` on every row):

| i | z | deck | bottom | waterline | height | draught | coefficient |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0 | -85.500 | 6.1244 | -4.8860 | 0.0687 | 11.0104 | 4.9547 | 1551.664 |
| 1 | -42.750 | 6.0933 | -4.9620 | 0.0129 | 11.0553 | 4.9749 | 1545.359 |
| 2 | 0.000 | 6.0623 | -4.9620 | -0.0011 | 11.0243 | 4.9609 | 1549.715 |
| 3 | 42.750 | 6.2941 | -4.9862 | 0.0899 | 11.2803 | 5.0761 | 1514.544 |
| 4 | 85.500 | 6.3400 | -4.9100 | 0.1525 | 11.2500 | 5.0625 | 1518.617 |

Two independent confirmations fall out of those numbers. The waterline column lands within
0.15 m of zero at every station, which is what an authored ratio of 0.55 means when the model's
origin is on the waterline — and it only does so with the deck/bottom assignment above; swapping
them puts the waterline at +1.12 m. And `sum(coefficient * draught)` over the ten rows is
76880.000, exactly `10 * Mass`.

Station 0 at `z = -85.5` lies before the bottom line's first point at `z = -81.63`, so it takes
the sampler's clamp, not an extrapolation. That is the one case in the shipped data where the
clamp branch decides the answer.

## Coverage

| routine | address | coverage |
| --- | --- | --- |
| `ship_buoyancy_build_list_0082d4ca` | `0082D4CA..0082D690` | complete |
| `ship_buoyancy_make_element_0082d5e4` | `0082D5E4..0082D67C` | complete |
| `ship_buoyancy_station_z_0082d500` | `0082D500..0082D54E` | complete |
| `ship_buoyancy_sort_profile_by_z_0082d21f` | `0082D21F..0082D365` | complete (the duplicate at `0082D380..0082D4C5` is the same body) |
| `ship_buoyancy_sample_profile_0082a920` | `0082A920..0082AA7C` | complete |
| `ship_buoyancy_build_from_model_0082fea9` | `0082FEA9..0082FEE8` | partial: `0082FE30`'s body is `0082FE30-00831801` and only this window is projected. Not read: `0082FE51` `0095F500`, `0082FE58` `0082D700`, and everything from `0082FEE8` on |
| `0082D040` as a whole | `0082D040-0082D6FD` | partial: `0082D067..0082D192` (the AABB over `descriptor+50h`'s parts, computed and never read) is understood and not projected; `0082D1A1..0082D21E`, `0082D36A..0082D37B` and `0082D696..0082D6FD` are `std::vector` lifetime and are not projected |
| `0082C960` push_back | `0082C960-0082CA00` | analysed library contract, not ported. The grow path `0082C4B0` and the element construct `0082A280` are not read |
| `00718000` node lookup | `00718000-007180D5` | read in full for the host contract, not ported (the model is Codex's) |

## Corrections

| claim | where | what the producer says |
| --- | --- | --- |
| "No site in the image appends to it, so it stays empty in a shipped build" | `docs/SHIP_AI_CLASS_FIELD_0524.md`, the `+52Ch` row | `0082C960` appends, twice per station, with `this = class+528h`. The list holds `2 * Hull.Segments` records |
| "`+528h`: nothing found, unsettled" | `docs/SHIP_AI_CLASS_FIELD_0524.md` | `+528h` is the vector object's own base word, the `_SECURE_SCL` `_Myproxy`/`_Myfirstiter` that precedes `_Myfirst`. `00932C3C` and `0082D4ED` both address the vector from there. It is not a separate field |
| "Its producer was not read" | `docs/SHIP_HYDRO_FORCES.md`, the `class+52Ch` section | `0082D040`, reached from the class's model-binding virtual `0082FE30` at slot 8 of the ship vtables |
| `+04h` = `level_top` | `docs/SHIP_HYDRO_FORCES.md` record table and `ShipBuoyancyElement::level_top` | the **waterline**, `lerp(deck, bottom, Hull.WaterLineRatio)`. It is not a top of anything |
| `+08h` = `level_draft` | same | the **deck line** height. The draught is `+14h`, a different field |
| `+0Ch` = `level_base` | same | correct: the keel (bottom line) height |
| "`+10h`, `+14h`: neither reader touches them" | same | the producer caches `deck - bottom` at `+10h` and `waterline - bottom` at `+14h`, which are exactly the two differences both readers recompute. `+14h` is read by a third reader, `008936A0` `luaMW_GetDraught`, as the draught |
| "the field roles are a hypothesis reconciled between its two readers" | same | there are five readers, not two: `009329C0`, `00937C90`, `008936A0` (`+14h`), `00933DA0` (`+1Ch`, at `00933E62`, for the spray effect) and `00963600` (teardown) |

The member names in `include/bsp/ship_hydro_forces.hpp` are left alone so nothing downstream
breaks; `include/bsp/ship_buoyancy_elements.hpp` carries accessors with the corrected names.

## Follow-up packets

| packet | addresses | question |
| --- | --- | --- |
| `ship_model_profile_nodes` | `00718000`, `0071F9F0`, model `+68h..+6Ch`, record `+0Ch`/`+24h`/`+44h` | the model's named point-set records are the model loader's (Codex's) output. What is `record+24h`, the kind the lookup matches against, and which other node names the ship path asks for. `0082FE30` resolves many more after this step |
| `ship_class_model_binding` | `0082FE30` body `0082FE30-00831801`, `0082D700`, `0095F500`, `00759120` | the other 14 KB of the model-binding virtual. `0082D700` copies the model's node list into `class+6BCh..+6C8h`; `00759120` adds `runwaycenter` and `liftexitpoint` |
| `ship_class_part_bounds` | `0082D067..0082D192`, `00721C90`, `00723170`, `descriptor+50h` `+10h`/`+14h` | the AABB the producer computes and discards. Either a leftover or a block whose result another build of the routine used |
| `vector_element_growth_0082c4b0` | `0082C4B0`, `0082A280`, `004215D0` | the `24h`-element vector's grow and construct helpers, for a complete container contract |

## no_ghidra_function

none. Every address in this doc lies inside a Ghidra function body: `0082D040-0082D6FD`,
`0082FE30-00831801`, `0082A920-0082AA7C`, `0082C960-0082CA00`, `00718000-007180D5`,
`0071F9F0-0071FB40`, `00759120-00759299`, and the reader bodies the corrections cite.
