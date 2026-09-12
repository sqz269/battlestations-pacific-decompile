# Scene contents hosts: effects preload, avoid zones, clouds, weather

Addresses: 004d0ee0 004c17d0 004248a0 004239e0 00424680 00424730 00423960 00423af0
0041df40 00412e20 004ba870 0046df00 0041ef90 004219a0 004cb160 004caf50 00bd2f10
00b6a020 00b69d40 00b66bd0 00b669a0 00b67980 00b67800 00b67720 00b65fb0 00b662b0
00b66270 008f41a0 008f2260 008f3370 008f5a00 008d9cf0

The four steps `004D4DF0 BSP_Game_LoadSceneContents` delegates to and that
`docs/MISSION_SCENE_CONTENTS.md` records only as unimplemented host methods.
Reconstruction: `include/bsp/scene_contents_hosts.hpp`,
`src/scene_contents_hosts.cpp`. The outer routine, the scene-record offsets and
`select_scene_cloud_kind` stay in `include/bsp/mission_scene_contents.hpp`; this
packet adds only what is inside the four steps.

Coverage per step: **complete** for `004D0EE0`, for the `.nav` grammar of
`004248A0`/`004239E0` and for `004BA870`; **complete for the table walk and the
selection rule** of the weather pass, with the property-bag and Lua-accessor
callees taken as contracts. Read-only analysis: no Ghidra mutation, no run-time
evidence (checklist rule 6 is unmet by construction, as it was for the parent
packet; `bsp_game.exe` reaches `load_scene_contents()` only as a stub).

## 1. `004D0EE0 BSP_SceneRecord_PreloadEffects`

`__fastcall(SceneRecord*)`, `ECX = [game+5FCh]` at `004D4FDB`. Body
`004D0EE0..004D0F87`, `RET` (no stack arguments). Coverage: complete.

The record fields, all consumer-side reads (checklist rule 4: the header pass of
`0046DF00` that writes them was not opened, so the `.scn` keys are unknown):

| Offset | Type | Site | Meaning |
| --- | --- | --- | --- |
| `+0C6Ch` | `NativeString[]`, 8-byte stride | `004D0F15`, `004D0F1B` | effect name array |
| `+0C70h` | `int` | `004D0F0C`, `004D0F6D` | its count, re-read each iteration |
| `+0D50h` | vector | `004D0EFB` | the effect-handle vector the step fills |

"Preload" means exactly this: the name list is resolved to refcounted handles the
record holds, and nothing is instantiated. The temporary that `00871BA0` writes
through its hidden out pointer is released inside the loop
(`InterlockedDecrement` on `+4h`, then vtable slot 0 at zero), so the vector's
push is what keeps the reference.

| # | Site | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `004D0F05` | `004CB160` | `clear_effect_handles` | `__thiscall(record+D50h, 0)` | none |
| 2 | `004D0F24` | `00871BA0` | `acquire_effect_by_name` | `__fastcall(out = [ESP+14h], EDX = [record+C6Ch] + i*8)`, push 1; returns the handle in `EAX` | `i < [record+C70h]` |
| 3 | `004D0F34` | `004CAF50` | `push_effect_handle` | `__thiscall(record+D50h, EAX)` | same |
| 4 | `004D0F4F` | `[00CE2220]` then vtable `+0h` | `release_temporary` | `InterlockedDecrement(temp+4)`, destructor at zero | `temp != 0` |

`00871BA0 BSP_EffectHandle_AcquireByName` is already reconstructed as
`acquire_gameplay_effect_by_name_00871ba0` in `src/gameplay_effect_acquisition.cpp`;
this packet cites it and does not re-derive it. The `PlaneRumble` acquire at
`004D502B` is **not** part of this routine: it is the caller's own acquire,
published into `00E18A78`, and `docs/MISSION_SCENE_CONTENTS.md` rows 11 and 12
already own it.

## 2. The `.nav` file: `004C17D0`, `004248A0`, `004239E0`

### The registry

`004C17D0 BSP_AvoidZoneRegistry_GetSingleton`, `int(void)`, double-checked
singleton on `00E17620` under the lifetime manager's critical section:
`operator new(14h)` at `004C182A`, `00424730` at `004C1843`, registered at
`004C1864`.

| Offset | Site | Meaning |
| --- | --- | --- |
| `+00h` | `0042475D` | vtable `00CE38E4` |
| `+04h` | `00424752`, `00424763` | `std::list` allocator base; `0041BFD0` buys the head node |
| `+08h` | `00424768`, `0042493C` | `_Myhead` |
| `+0Ch` | `0042476B` | `_Mysize` |

The constructor does not leave the list empty: `0042476E..004247B7` allocates one
`28h` element and constructs it with `00424680(layer, [00CE3990])`, the
ten-degree default. `004248A0` appends to that list without clearing it, so a
loaded registry holds the default layer plus the file's layers.

### The element class is `TerrainGridLayer`

**Correction to `docs/MISSION_SCENE_CONTENTS.md`.** That doc names the `28h`
element `AvoidZone` from the literal at `00CE38AC` sitting near the vtable. The
literal the class actually writes is `TerrainGridLayer` at `00CE38D0`, referenced
by the writer `0041EF90`, pushed at `0041EFD9` and copied at `0041EFDF`, and every shipped `.nav` carries that
string in each element. `AvoidZone` (`00CE38AC`) and `AvoidZoneG` (`00CE38A0`)
are referenced only from `0041D380` and `00424D00`, neither of which is this
class's serializer. The container's own name string is `TerrainGrid`.

| Offset | Type | Reader | Writer | Meaning |
| --- | --- | --- | --- | --- |
| `+00h` | vtable | `00424918` | - | `00CE38CC`, one slot (`0041F640`) |
| `+04h` | `float` | `00423A24` | `0041EF90` +58h | half extent, `12000.0` (`00CE3968`) |
| `+08h` | `int` | `00423A32` | `0041EF90` +54h | grid dimension `n` |
| `+0Ch` | `float` | `00423A40` | `0041EF90` +58h | cell size; `00423993` computes it as `24000.0 / n` (`00CE3960`) |
| `+10h` | `float` | `00423A4E` | `0041EF90` +58h | per-file scalar, default `2.0` (`00CE3958`) |
| `+14h` | `float` | `00423A5C` | `0041EF90` +58h | slope limit, `tan(angle)`; `00424680` computes it with `00412E20` |
| `+18h` | `vector<uint8_t>` | `00423A6D` | `0041EF90` +28h | `n*n` cells; `_Myfirst/_Mylast/_Myend` at `+1Ch/+20h/+24h` |

`00412E20` is `tan` in radians: `FLD; FSINCOS; FDIVP`. The constructor default
`00CE3990` is `0.17453286` (ten degrees), and `tan` of it is the `0.176327` the
first layer of every shipped file carries.

`00423AF0` (called from `00880CD0`) resets every layer in place to the
constructor defaults with `n = 10` and cell size `2400.0` (`00CE396C`), which is
the same `24000.0 / n` rule.

### The `.nav` grammar

```
file  := str name ; i32 count ; layer[count]
layer := str name ; f32 half_extent ; i32 n ; f32 cell ; f32 scalar ;
         f32 slope ; u8 grid[n*n]
str   := u32 length ; char[length]          (no terminator)
```

Everything is little-endian. The stream vtable slots the two readers use are
`+60h` (length-prefixed string into a scratch native string, released in the
epilogue), `+38h` (int), `+44h` (float) and `+24h` (raw byte block); `0041EF90`
writes with the matching `+64h`, `+54h`, `+58h` and `+28h`.

Loose `.nav` files **are** present in the installation, which corrects the
parent packet's "not present loose, only reachable through the VFS packages".
`local/nav_survey.py` parses all **253** of them under
`universe/scenes/**` with this grammar and every one ends exactly at the file
length, no leftover bytes. Every file is the same shape:

| Field | Value in all 253 files |
| --- | --- |
| root name | `TerrainGrid` |
| layer count | 3 |
| layer name | `TerrainGridLayer` |
| `+04h` half extent | `12000.0` |
| `+08h` dimension | `240` |
| `+0Ch` cell size | `100.0` (= `24000 / 240`) |
| `+14h` slope limit | `0.176327`, `0.363970`, `2.747478` = `tan(10deg)`, `tan(20deg)`, `tan(70deg)` |

`+10h` is the only authored field that varies: it is constant across the three
layers of a file and ranges from `0.005` to `17.19` across files. The all-zero
grids (25 open-water missions) carry `0.005`; the land-heavy ones carry the large
values. No reader of `+10h` was found in this packet, so its meaning is an open
question rather than a claim. Grid bytes span `0..255`, so a cell is a value, not
a flag, and the nonzero count falls as the layer's slope limit rises
(`54474 / 46974 / 45980` for Pearl Harbor).

### Who queries the zones

`0041DF40`, `__thiscall(registry, float value, char value_is_angle)`, `RET 8`.
With the flag set the value goes through `00412E20` first. The walk over
`registry+8h` keeps the layer with the **largest** `+14h` that is still strictly
below the requested slope, and `0041DF88` primes the result with the first
layer's data before the walk, so an unmatched query returns that first layer
rather than null.

| Site | Caller | Contract |
| --- | --- | --- |
| `007F1DA7` | `007F1D90` | `__thiscall(obj, float, char)`; result stored at `obj+350h` (the registry comes from `004C17D0` at `007F1D93`) |
| `007F1DC5` | `007F1D90` | `query(1.5, true)` (`00CE380C`), result at `obj+34Ch`; `tan(1.5 rad)` is `14.1`, which selects the `tan(70deg)` layer |
| `007F5239`, `007F5255` | no Ghidra function | read as raw listing, contract unread |
| `00880CF5` | `00880CD0` | not a query: `00423AF0` resets every layer alongside the spatial-index detach (the singleton is fetched at `00880CEE`) |

`007F1D90`'s sole caller is `007F4580`. The consumer class and what it does with
the two cached layers are `contract: unread` for this packet.

| # | Site | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `004D5251` | `004C17D0` | `ensure_registry` | no arguments, returns `[00E17620]` | `AL != 0` from the VFS open at `004D5232` |
| 2 | `004248C1` | `0041DED0` | `begin_load` | `__thiscall(registry)` | none |
| 3 | `004248D9` | stream vtable `+60h` | `read_string` | `(out, 0)`; the root name, dropped | none |
| 4 | `004248E7` | stream vtable `+38h` | `read_int` | `(0)` -> count | none |
| 5 | `00424906` | `00BF681B` | `operator new(28h)` | CRT | per element |
| 6 | `0042492F` | `004239E0` | element deserialize | `__thiscall(layer, void** streamHolder)`, `RET 4` | `ESI != 0` |
| 7 | `0042494F` | `0041C1F0` | `append_layer` | `__thiscall(registry+4h, node, next, &value)` | per element |
| 8 | `0042495A` | `00420140` | list size increment | `__thiscall(registry+4h, 1)` | per element |
| 9 | `00424975` | `0041E000` | `end_load` | `__thiscall(registry)` | none |
| 10 | `00423A0F` | stream vtable `+60h` | `read_string` | `(out, 0)`; `TerrainGridLayer`, dropped | none |
| 11 | `00423A22`/`3E`/`4C`/`5A` | stream vtable `+44h` | `read_float` | `(0)` -> `ST0`, stored to `+04h`/`+0Ch`/`+10h`/`+14h` | none |
| 12 | `00423A30` | stream vtable `+38h` | `read_int` | `(0)` -> `+08h` | none |
| 13 | `00423A6D` | `004219A0` | grid resize | `__thiscall(layer+18h, n*n, 0)` | none |
| 14 | `00423A9D` | stream vtable `+24h` | `read_bytes` | `__thiscall(stream, [layer+1Ch], n*n, 0)` | `_Myfirst != 0 && size != 0` |

## 3. `004BA870 BSP_SceneRecord_ScatterClouds`

`__fastcall(SceneRecord*)`, `ECX = [game+5FCh]` at `004D56BF`. Body
`004BA870..004BAC12`, `RET`. Coverage: complete. `ESI = record + C88h` for the
whole body, so the listing's `ESI+x` are the offsets below minus `C88h`.

| Offset | Type | Site | Meaning |
| --- | --- | --- | --- |
| `+0C84h` | `int` | `004BA879` | gate; `< 1` returns at once |
| `+0C88h`/`+0C8Ch`/`+0C90h` | `float` | `004BA97F`, `004BA99C`, `004BA9BD` | box minimum x/y/z |
| `+0C94h`/`+0C98h`/`+0C9Ch` | `float` | `004BA970`, `004BA990`, `004BA9AE` | box maximum x/y/z |
| `+0CA0h` | `int` | `004BA8A0`, `004BABCA` | cloud count |
| `+0CA4h`/`+0CA8h`/`+0CACh` | `float` | `004BA8A3`, `004BA8C0`, `004BA8C8` | the three kind weights |

### The roll

`004BA8A3..004BA8CE` computes the total as
`((w0 + 0.0) + w1) + w2`, each partial rounded back to float by an
`FSTP float`/`FLD float` pair. **The constant the parent packet left undecoded,
`00D7A258`, is the double `0.0`** of that first add (`00D7A250..00D7A25F` is
`double -1.0` then `double 0.0`); it contributes nothing, and the earlier note
that a constant is "added to the cloud weight total" is only an x87 accumulator
idiom.

`004BA929` draws `roll = random(0.0, total)` and `004BA938..004BA95A` subtracts
the weights in order, stopping at the first index whose running remainder is
`<= 0`, bounded by `index < 3`. That walk is
`select_scene_cloud_kind` in `include/bsp/mission_scene_contents.hpp`, which this
packet reuses unchanged. The chosen kind's minimum separation is
`[00E081DC + index*18h]` (`004BA9DB`), and its class-name pointer is
`00E081C8 + index*18h` (`004BAB0B`).

### The placement rule

Per point, `004BA970..004BAA9A`:

1. `x = random(min.x, max.x)`, `y = random(min.y, max.y)`, `z = random(min.z, max.z)`.
2. Against every already-placed point: `d2 = dx^2 + dy^2 + dz^2`; the distance is
   `sqrt(d2)` when `d2` exceeds the double at `00CE3820` (about `9.99e-11`) and
   `0.0` otherwise (`004BAA31..004BAA59`, so coincident points reject rather than
   divide by zero). The candidate is rejected as soon as
   `placed_separation + candidate_separation > distance` (`004BAA6E`, `JA`).
3. On a rejection the attempt counter increments and the draw repeats while
   `attempts < 1000` (`004BAA91`, `3E8h`). After 1000 attempts the last candidate
   is kept as it stands.
4. The accepted point is stored into the `count*12` array and its separation into
   the `count*4` array (`004BAAAE..004BAAD7`).

`004BAADC..004BAAF7` then draws one more value in `[-pi, +pi]`
(`00CE684C` to `00D7A264`) and **pops it unused** at `004BAB02`. The projection
keeps that draw because it advances the random stream.

`004BA8D7` and `004BA8F6` allocate both arrays with `operator new`, and only the
`count*4` separations array is freed (`004BABE0`, `00BF6989`). The `count*12`
point array leaks `12 * record+CA0h` bytes per scatter. The projection uses
vectors and does not reproduce the leak.

| # | Site | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `004BA8D7` | `00BF55BE` | `operator new` | `(count*12)` | `[record+C84h] >= 1` |
| 2 | `004BA8F6` | `00BF55BE` | `operator new` | `(count*4)` | same |
| 3 | `004BA929` | `00BD2F10` | `random_range` | `ECX = 1`, `(0.0, total)` -> `ST0` | per point |
| 4 | `004BA984`/`9A2`/`9C0` | `00BD2F10` | `random_range` | `ECX = 1`, `(min, max)` per axis | per attempt |
| 5 | `004BAA41` | `00BF7030` | `sqrt` | CRT, `ST0` | `d2 > [00CE3820]` |
| 6 | `004BAAF7` | `00BD2F10` | `random_range` | `ECX = 1`, `([00CE684C], [00D7A264])`; result discarded | per point |
| 7 | `004BAB12` | `0046D930` | `create_cloud_entity` | `ECX = [00E18680]`, `(kind name, "Cloud" 00CE7524, 0)` -> entity | per point |
| 8 | `004BABB4` | vtable `+88h` | `place_entity` | `__thiscall(entity, &matrix)`; row-major, `1.0f` (`00D7A24C`) diagonal, point in row 3 | per point |
| 9 | `004BABC0` | vtable `+D8h` | `activate_entity` | `__thiscall(entity)`, no arguments | per point |
| 10 | `004BABE0` | `00BF6989` | `operator delete` | the separations array only | always on the taken branch |

`0046D930` is reconstructed: `docs/SCENE_ENTITY_CREATE.md`. This packet treats it,
`00BD2F10` and the two entity virtuals as contracts.

## 4. The weather-descriptor pass of `0046DF00`

`0046E0A4..0046E6FE` (`0046DF00 + 1A4h .. + 7FEh`), inside
`BSP_SceneFile_Read`. It runs on all three passes, before the `header` block.
Arguments referenced: `[EBP+8]` = `scenePath` (arg 1), `[EBP+18h]` =
`overrideName` (arg 5). Coverage: complete for the walk and the selection; the
property-bag and Lua accessors are contracts.

### The table

`SCRIPTS\datatables\Weather.lua` (`00CE59DC`), read into a Lua state the pass
opens and closes itself:

| # | Site | Callee | Name | this / args / ret | Gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `0046E0B6` | `00B66BD0` | state owner construct | `__thiscall(owner at ESP+358h)` | none |
| 2 | `0046E0CC` | `00B6A020` | `open_lua_state` | `__thiscall(owner, 4)` | none |
| 3 | `0046E119` | `00B69D40` | `run_script` | `__thiscall(owner, &path, 0)`; the file plus every VFS override, in search order | none |
| 4 | `0046E154` | `00B67980` | globals | `__thiscall(owner, &out)` | none |
| 5 | `0046E170` | `00B67800` | `Weathers` | `__thiscall(globals, &out, 00CE59D0)` | none |
| 6 | `0046E1F4`, `0046E6A1` | `00B67720` | entry by index | `__thiscall(table, &out, i)`, `i` from 1 | loop |
| 7 | `0046E1A9`, `0046E6B0` | `00B65FB0` | entry nil test | `__thiscall(entry)` -> `AL`; nil ends the loop | loop |
| 8 | `0046E221` | `00B662B0` | `sceneFile` | `__thiscall(field)` -> `const char*`, key `00CE59C4` | per entry |
| 9 | `0046E296` | `00BF7FBF` | `_stricmp` | `(scenePath, entry.sceneFile)`; both-empty also matches | `both non-empty` |
| 10 | `0046E2E4` | `00B67800` | `SubScenes` | key `00CE59B8` | on a match |
| 11 | `0046E30D`, `0046E62F` | `00B67720` | sub-scene by index | `k` from 1, nil ends the inner loop | on a match |
| 12 | `0046E373`, `0046E382` | `00B67800`, `00B662B0` | `ID` | key `00CE59B4` | per sub-scene |
| 13 | `0046E3EA`, `0046E3F9` | `00B67800`, `00B662B0` | `Descriptor` | key `00CE59A8` | per sub-scene |
| 14 | `0046E4B2` | `00BF7FBF` | empty test | `(Descriptor, "")`; equal means skip the file entirely | `Descriptor len != 0` |
| 15 | `0046E52B` | `008D9CF0` | descriptor tokenizer | `__thiscall(tok, &path, "{}(),;:=" 00CE599C)` | `Descriptor non-empty` |
| 16 | `0046E757` | `00BF7FBF` | `_stricmp` | `(overrideName, ID)` -> the selection | `both non-empty` |
| 17 | `0046E58A` | `008F5A00` | parse, discarded | `__thiscall(local bag, &tok, 1, 0)`, then `008F5410` at `0046E59E` | not selected |
| 18 | `0046E77E` | `008F5A00` | parse into the reader's bag | `__thiscall(bag at ESP+128h, &tok, 1, 0)` | selected |
| 19 | `0046E7CB`, `0046E859`, `0046E8EA`, `0046E97F` | `008F2260` | bag lookup by full key | `__thiscall(bag, key)` -> property or null | per shadow key |
| 20 | `0046E7FB` | `008F3370` | string write | `__thiscall(property, value)` | key found and Lua value not nil |
| 21 | `0046E892`, `0046E923`, `0046E9BC` | - | float write | `property+0Ch = 00B66270(field)` | key found and Lua value not nil |
| 22 | `0046E5B2` | `008D9C30` | tokenizer destroy | `__thiscall(tok)` | `Descriptor non-empty` |
| 23 | `0046E6F9` | `00B669A0` | `close_lua_state` | `__thiscall(owner)` | none |

The Lua shape, confirmed against the shipped
`scripts/datatables/weather.lua`:

```lua
Weathers = {
  { ["sceneFile"] = "<path>", ["UniqueID"] = <int>, ["SubScenes"] = {
      { ["ID"] = "clear02", ["Text"] = "FE.multi_weather_clear02",
        ["Descriptor"] = "", ["g_StaticShadowTexture"] = "",
        ["ga_StaticShadowShotOffsetX"] = 0, ["ga_StaticShadowShotOffsetZ"] = 0,
        ["ga_StaticShadowShotSize"] = 0 }, } },
}
```

`UniqueID` and `Text` are never read by this pass.

### Which descriptor is selected

1. Both loops run to the end of their table; neither breaks on a match, so the
   **last** matching entry and the last matching sub-scene win.
2. An entry matches when `_stricmp(entry.sceneFile, scenePath) == 0`, with the
   image's own both-empty rule (`0046E271..0046E28D`). This settles the parent
   doc's "weakest link": the comparison is against the **`scenePath` argument
   verbatim**, case-insensitively, not a derived name.
3. A sub-scene whose `Descriptor` is the empty string is skipped outright: no
   tokenizer is built (`0046E4DD`).
4. Otherwise the sub-scene is selected when `_stricmp(overrideName, ID) == 0`,
   and when either side is null or empty the empty-`ID` row is the selected one
   (`0046E536..0046E766`).
5. The selected row's `Descriptor` `.ptr` file is parsed into the reader's own
   property bag, the one `008F41A0` constructs at `0046E097` and `008F5410`
   destroys at `0046ED41`. Every other non-empty `Descriptor` is still opened and
   parsed, into a bag that is destroyed immediately (`0046E563..0046E5A3`), so
   the file is touched and its values dropped.
6. The four shadow keys are then read out of the **Lua row**, not out of the
   `.ptr`, and written over the matching `g_Terrain.*` properties of that same
   bag. The key/variable table is already in `docs/SCENE_FILE_READER.md`.

**Correction to `docs/SCENE_FILE_READER.md` step 5.** It reads "When
`overrideName` is null or empty, the matched descriptor's shadow keys are pushed
into four console variables; otherwise the descriptor is applied wholesale and
the shadow keys are skipped." The branch is not on `overrideName` alone: it is
the `ID`-versus-`overrideName` comparison above, and the two outcomes are
"parsed into the reader's bag plus shadow keys" versus "parsed into a throwaway
bag". A null `overrideName` therefore selects the row whose `ID` is empty, which
for the shipped table is no row at all, since every shipped `ID` is a name.

### Where the descriptor lands

Nowhere on the scene record. The bag is a stack local of `0046DF00`, and it is
passed by address into the header handler at `0046E73B`:
`00469BF0(this, tokenizer, sceneRecord, instantiate, &bag)` (`0046E72E..0046E73B`).
So the weather descriptor is the **base property set the `header` block of the
`.scn` is read over**, and it dies with the reader call. `00469BF0` writes only
`record+905h` and `record+1098h`, which `docs/SCENE_FILE_READER.md` already
records.

## Ledger names recorded

| Address | Name |
| --- | --- |
| `004239E0` | `BSP_TerrainGridLayer_Deserialize` |
| `00424680` | `BSP_TerrainGridLayer_Construct` |
| `00423960` | `BSP_TerrainGridLayer_SetDimension` |
| `00423AF0` | `BSP_AvoidZoneRegistry_ResetLayers` |
| `0041DF40` | `BSP_AvoidZoneRegistry_SelectLayerBySlope` |
| `0041EF90` | `BSP_TerrainGridLayer_Serialize` |
| `004CB160` | `BSP_EffectHandleVector_Resize` |
| `004CAF50` | `BSP_EffectHandleVector_PushBack` |

`00412E20` already carried the correct name `BSP_Math_TangentX87Float` and was left
alone.

## Open questions

- `TerrainGridLayer+10h`: authored per file, constant across a file's three
  layers, `0.005` to `17.19`, default `2.0`. No reader was found. It correlates
  with how much land the mission has, which is a hypothesis, not evidence.
- The grid byte's meaning: it is a value in `0..255`, not a flag, and nothing in
  this packet reads a cell. The world-to-cell mapping (`+04h` half extent and
  `+0Ch` cell size) is implied by the fields, not observed at a sampler.
- `007F5239` and `007F5255` call `004C17D0` from a region with no Ghidra
  function; they are listed as call sites and their contract is unread. So is
  `007F4580`, the only caller of `007F1D90`.
- Whether anything reads the reader's property bag after `00469BF0` returns. The
  bag is destroyed at `0046ED41`, so the four `g_Terrain.*` writes are
  reader-scoped unless `00469BF0` copies them out.
- `004D0EE0`'s effect names, the cloud block and the remap textures are all
  consumer-side reads; the `.scn` keys behind `record+C24h..CB0h` remain unknown
  (inherited from `docs/MISSION_SCENE_CONTENTS.md`).
