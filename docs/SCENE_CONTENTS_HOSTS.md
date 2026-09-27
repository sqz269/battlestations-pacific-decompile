# Scene contents hosts: effects preload, avoid zones, clouds, weather

Addresses: 004d0ee0 004c17d0 004248a0 004239e0 00424680 00424730 00423960 00423af0
0041df40 00412e20 004ba870 0046df00 0041ef90 004219a0 004cb160 004caf50 00bd2f10
00b6a020 00b69d40 00b66bd0 00b669a0 00b67980 00b67800 00b67720 00b65fb0 00b662b0
00b66270 008f41a0 008f2260 008f3370 008f5a00 008d9cf0
004ea650 004f1460 0047b660 004f11c0 006af3d0 00928860 00487210 00487230 004f1400
004f1420 004f1390 00883bb0 00882ac0 00880000 00880cd0 0087f9a0 00903860 009038f0
009039d0 00903bc0 00add290 00adda60 00adb3a0 00ada900 00adb480 00adaba0
00adaa40 00ada890 00adac50 00adac70 00adc5f0 00adc6c0 00adfd70 00adfeb0 00adadc0 00ada240
00ade820 00ade500 00ada420

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

## 5. The Path and Landscape creators (packet `cc9_scene_path_landscape`, 2026-09-27)

Before this packet, both classes reached `SceneContents::class_creator` as a record
(`native_scene_creators=unresolved` in the avoid-zone summary). Read-only analysis of the image,
plus a host binding under `kScenePathLandscapeCreatorsBound` in
`include/bsp/game_hosts_scene_contents.hpp`.

### What the two creators build

The two creators share one template. Path is 004EA650..004EA753 and Landscape is
004F1460..004F1553. Both are `__fastcall` with EDX = the name and four stack arguments.
- **Arguments.** Argument 1 is the hierarchy parent and argument 2 the frame. Arguments 3 and 4
  are unread. `RET 10h` ends both.
- **The creator call.** It is `CALL [row+4]` at 0046D5A4. It pushes `[ESP+1B8h]`, the bag (EBX),
  the local frame at `ESP+50h` and the parent `[ESP+1B4h]`, with EDX = the name.

| step | Path | Landscape |
| --- | --- | --- |
| allocate + zero | 210h | 430h (004F147B, 004F1487) |
| network id | 006AF3D0 on `[game+21D8h]` consumes the name string and returns `XOR AX,AX` (006AF40F), so the id is 0 | same |
| constructor | 0047B660 at 004EA6BC | 004F11C0 at 004F14CE |
| place | `vtable+98h` = 00928860 `BSP_GameEntity_PlaceInWorld(arg1, [[00E188A8]+19CCh], frame)` at 004EA6FB; the Path copies the frame to a local first (004EA6D0 `REP MOVSD`, 10h dwords) | same call at 004F1502, frame passed as is |
| name | `entity+154h` length, `+158h` buffer (004EA6FF, 004EA724) | same (004F1506, 004F1527) |

**Layout.** Both are one 00928630 `BSP_GameEntity_Construct(0, 0)` base.
- **Path** (0047B660): vtable 00CE6290 at +0h, 00CE6274 at +10h, 00CE626C at +24h and
  00CE6268 at +170h. The path component at +1E4h is built by 007B28C0 and has vtable 00CE63E8.
  `+C4h` = 47h.
- **Landscape** (004F11C0):
  - vtable 00CEA090 at +0h (004F1272);
  - the collision node at +1E4h, vtable 00CEA05C, fields by 004E6480, with `+230h` = self;
  - `+344h`, the segment-trace sub-object (vtable 00CEA050);
  - `+360h` = &node;
  - `+348h..+35Ch`, two copies of the float3 at 00F87574;
  - `+B4h` = `+B8h` = 1 and `+C4h` = 44h (004F1338).

**Lists (PlaceInWorld, slot 130h).**
- PlaceInWorld stores the world at `entity+30h` and calls the class's slot 130h at 009288F1
  (`docs/UNIT_WORLD_REGISTRATION.md`).
- **Path** 00487210: 00928560 (`world+24h`, every entity), then 00484540 on `world+36Ch`.
- **Landscape** 004F1400: 00928560, then `world+348h`. Both appends are at the tail.
- **The list heads.** 004CB030 builds them at `world+18h + class*0Ch`, so `36Ch` is list 47h and
  `348h` is list 44h. The ledger's "kind 41h" wording for 00487210 is a bucket label, not this
  index.
- **The unregister slots.** Slot 134h is 00487230 for a Path and 004F1420 for a Landscape; each
  unlinks from the same list.

**The properties load in InitAll, not in the creator.** 00925F20 runs after the scene read at
0046EB4B. It calls slot 9Ch on every pending entity, then 0A0h, then 0A4h
(`include/bsp/lua_binding_mission_2.hpp`).
- **Path pass A** is 00928A00, the base `thisTable` attach, which the scene markers already
  model.
- **Path pass B** is 007B38D0: `Party` goes to `+54h` (007B38FC), and 007B34F0 loads `PathPoints`
  into the +1E4h component (`docs/SCENE_LANDSCAPE_CLASS.md`, `docs/GAME_SCENE_ZONE_PATHS.md`).
- **Landscape pass A** is 00883BB0 (**no Ghidra function**, 00883BB0..008840A0, `RET` at
  008840A0 followed by `INT3` padding). It runs:
  - it zeroes `+3CCh..+3DCh` and `+418h..+420h`, then calls 00928A00 and 00881A70;
  - `FilePath` goes to `+3C4h` (00883C5E) and `ModelPath` to `+424h` (00883C7B);
  - 00882AC0 (FilePath) runs at 00883DD3;
  - then the `ShallowWater` block (`Texture`, `Pos`, `Size`) becomes a decal at `+42Ch`,
    registered on `[[game+19F0h]+A8h]` vtable+1Ch (00883E15);
  - 00883F88..00883FA1 adds the segment-trace shape `+344h` to the collision node's shape array
    (`[+1E4h + [+2DCh]*4 + D0h]`, count at `node+F8h`), and 00883FAE sets `+230h` = self;
  - the terrain's box (terrain vt+0Ch at 00883FBE, less the pose translation `+CCh+30h..38h`)
    goes to 0098A920 `BSP_SpatialNode_SetLocalBounds` on `+1E4h` (00884066);
  - **00884078 calls 0098BA10 `(index 0042E630(), node +1E4h, parent 0, matrix +384h,
    static 1)`**: the island is a static root of the spatial index. This is a fourth attach site
    that `docs/SPATIAL_INDEX.md`'s table of three lacks, because the body has no Ghidra function;
  - `+168h` = 00740FE0 on `[00E1AEA0]` (00884083..0088408D).
- **Landscape pass B** is 009277E0, the base inherit-race thunk.

**00882AC0, the terrain load** (00882AC0..00883BA5) builds three names from FilePath:
- `terrain/<FilePath>_heightmap.tdt` and `terrain/<FilePath>_colormap.dds`;
- `models/terrain/<FilePath>.mmod`.

It then builds these objects:
- **+3D0h**, the terrain object, from 00ADD290 on `[game+19ECh]`. Its vtable+28h is height(x, z),
  +38h the normal and +40h the render geometry.
- **+41Ch**, the `.mmod` resource, from 007188A0 `BSP_Resource_LoadWithGameFactory`
  (00882FC8), and **+420h**, its instance (vtable+8h).
- **+3CCh**, a render node, built only when `[game+19F0h]` exists.
- **+418h**, from 007135C0 `BSP_UnitPartInstance_Construct` (008830F1). Its spatial-index
  attach at 00710B6D passes the static flag because the pose answers 44h
  (`docs/SPATIAL_INDEX.md`).

So an island enters the spatial index the segment queries walk twice: as the terrain node
(pass A, 00884078) and as the model's parts (00710B6D).

00882EE6..00882EFD test `[0109CEECh]` vtable+8h on a name and skip to 00883A1B when it answers
false. That branch, and the part loop after +418h, were not read.

**The two remaining Landscape slots.**
- **Slot DCh** (00880000..0088007A, **no Ghidra function**) is the per-frame tick:
  - `+3DCh += dt`;
  - 00BB0A90 on `+3D4h` and 00BAEC50 on `+3D8h`;
  - 00ADA0C0 on the terrain with `[game+19FCh]`;
  - `+3CCh` vtable+38h with `entity+74h`.
- **Slot 84h** (00880CD0) is the teardown:
  - 0098A500 detaches `+1E4h` from the spatial index;
  - 00423AF0 resets the avoid-zone registry's layers on 004C17D0's singleton;
  - it clears `+2DCh`, then tail-jumps to 009277F0.

**Timing.** In the load routine, construct_world is called at 004E01DE and load_scene_contents at
004E03E5 (scene read 0046EB0F, InitAll 0046EB4B). The avoid-zone rebuild follows at 004E07BE
(rel32 census, one caller each). So both classes exist and are initialised before the zones are
built and before any unit ticks. A Landscape's terrain exists before any ground-height query.

### Who reads them

| reader | what it reads | host today |
| --- | --- | --- |
| 00424D00 avoid-zone walk | list 47h, head `world+370h`; 0041D1E0 path interface 007AC9D0 = `+1E4h`, Party `+54h`; 0041CCD0 parent `vt+5Ch(44h)` | `GameAvoidZoneRuntime::rebuild` walks the records instead, in scene order; list 47h's head-first order is the same order |
| 00903860 `BSP_World_GroundHeightAt` | list 44h head `world+34Ch`, max of `[node+8h]+3D0h` vt+28h(x, z), seeded -1000.0 [00D7A240]; 30 rel32 call sites | `ground_height_00903860() { return 0.0f; }` in `src/game_hosts_units.cpp` (cc9-plane-release) |
| 009038F0 | same walk, then the winner's terrain vt+38h normal | none |
| 009039D0 | same walk; returns the winning Landscape | none |
| 00903BC0 segment test | two ground tests through 00903860, then an occluder sweep of list 44h via vt+3Ch; callers 007C3E00, 009A15C1, 009A172E (AvoidTerrain 0099F1C0), 009D39D3 (torpedo approach) | `World::segment_occluders_00903bc0` record in `src/game_hosts_units.cpp` |
| 009043A0 / 0098ADD0 segment query | the spatial index; an island is there through the terrain node (00884078, shape `+344h`, slot E4h 0087FEC0 `BSP_CollisionShape_TraceSegment_Extruded`) and the parts (00710B6D) | the query runs over the gunnery host's units only (`query_segment_units_impl`, `src/game_hosts_gunnery.cpp`) |
| 0084BC99 impact effect | a hit entity answering 44h selects mode 3, the terrain effect slot (`docs/PROJECTILE_IMPACT.md`) | only reachable once an island is in the query |
| the renderer | `+3CCh`, `+420h`, the decal `+42Ch` | none |

**The avoid-zone layer sample 0041BC20 is not a Landscape reader.** It samples the `.nav` layers
of 004C17D0. A Landscape only resets those layers at teardown.

### The binding

`kScenePathLandscapeCreatorsBound` (committed false for the pairs, **ON** after them) is declared in
`include/bsp/game_hosts_scene_contents.hpp` and used in `src/game_hosts_scene_contents.cpp`.
- **The object.** For each generated row of class 47h or 44h, `create_scene_world_object`
  builds a `SceneWorldObject`, the host stand-in for the native entity. It appends the object to
  `scene_world_class_lists()`, the stand-in for lists 47h and 44h.
- **Path fields.** Party and the point count come from the retained record, the projection of
  pass B.
- **Landscape fields.** FilePath, ModelPath and ShallowWater come from the merged bag, as pass A
  reads them. The three 00882AC0 names are resolved against this process's VFS as a census.
- **`created` stays false.** `create_units` makes a unit of every created record, and the marker
  pass keys on `generated && !created`. In the image both classes still get their `thisTable`
  slot through pass A.
- **Named records on the ON path:**
  - `Landscape::load_terrain` 00882AC0;
  - `Landscape::attach_terrain_vcall_9c` 00883BB0.
- **Labelled substitutions** (in the code and here):
  - **Timing.** The pass A and pass B fields are filled at creation, where the image fills them
    in InitAll after the whole scene is read. Nothing reads them in between.
  - **The frame.** The object keeps the record's composed world frame with no hierarchy parent,
    the convention of this file's unit creators. The native passes the authored localframe and
    the parent.
- **OFF** keeps the record path unchanged: `SceneContents::class_creator` stays UNIMPLEMENTED
  for 004EA650 and 004F1460.
- **The census line** `summary scene path/landscape creators` is printed in both states.

**Contracts for the other owners** (nothing here edits their files):
- **Units host** (cc9-plane-release). `ground_height_00903860` is:
  - the max over `scene_world_class_lists().list(0x44)` of the terrain height at (x, z);
  - -1000.0 when the list is empty or no terrain answers.

  Until a terrain object exists, the correct stand-in for an island mission is not 0.0. It is
  "no terrain", -1000.0, and the answer only moves once the `.tdt` height field is read.
  `segment_blocked_00903bc0` needs the same terrain and the occluder vt+3Ch.
- **Gunnery host.** The segment query gains islands when a Landscape's terrain node joins the
  index (00884078), which needs the terrain object's bounds and its segment-trace shape. The
  `.mmod` parts (00882AC0 -> +418h -> 00710B6D) are a second, separate source of island hits.
- **Avoid-zone runtime.** Once this lands ON, the summary's `native_scene_creators=unresolved`
  can read `bound`. The rebuild may walk list 47h instead of the records: the census proves the
  two orders equal.

### Names and bodies

Ledger names added (hypotheses): `004F1460 BSP_SceneDatabase_CreateLandscape`,
`004F11C0 BSP_Landscape_Construct`, `0047B660 BSP_Path_Construct`,
`004F1400 BSP_Landscape_RegisterInWorldLists`, `004F1420 BSP_Landscape_UnregisterFromWorldLists`,
`00487230 BSP_Path_UnregisterFromWorldLists`, `004F1390 BSP_Landscape_GetCollisionNode`,
`00883BB0 BSP_Landscape_InitAttachTerrain`, `00882AC0 BSP_Landscape_LoadTerrain`,
`00880000 BSP_Landscape_TickTerrain`, `00880CD0 BSP_Landscape_Teardown`,
`009038F0 BSP_World_GroundNormalAt` (renamed in section 6; it writes no height), `009039D0 BSP_World_LandscapeAt` and
`006AF3D0 BSP_EntityNetworkId_FromNameStub`.

Bodies with no Ghidra function (`ghidra proto --brief` answers `?`), all Landscape vtable slots:

| start | end (inclusive) | slot | evidence |
| --- | --- | --- | --- |
| 00883BB0 | 008840A0 | 9Ch | `RET` at 008840A0, `INT3` from 008840A1 |
| 00880000 | 0088007A | DCh | `RET 4` at 00880078, `INT3` 0088007B..0088007F |
| 004F1390 | 004F139D | B0h | `RET` at 004F139D, `INT3` 004F139E..004F139F |
| 0087F9A0 | 0087F9CE | 24h | `RET 0Ch` at 0087F9CC; calls `[+3D0h]` vt+40h, then 007407C0 on `+168h` |
| 0087F9F0 | 0087F9F0 | E0h | a lone `RET`, `INT3` to 0087F9FF |

### Predictions, written before the runs

One tree, one base, the switch false against true. Every run uses streams and the death table on,
with `--frames <F> --press-start-frame 30 --menu-select <M> --mission-frames <N>
--mission-frame-seconds 0.05`.

**USN01 3200/3000** (`usn_1_marshall.scn`: 4 Landscape, 55 Path, of which 21 are `AvoidZoneG`).
- **Census, OFF:** `bound=0 path seen=55 generated=55 rejected=0 created=0 landscape seen=4
  generated=4 rejected=0 created=0 list47=0 list44=0`.
- **Census, ON:** `path ... created=55`, `landscape ... created=4`, `list47=55 list44=4
  avoid_zone_paths=21 list_order_matches_records=1`.
- **Terrain files, ON:**
  - `models/terrain/islands/m07_a.mmod` resolves, and so do the three `dlc_l_03_s.mmod`
    (loose files exist in this installation): `models=4/4`.
  - `m07_a_heightmap.tdt` exists only under `models/terrain/islands/`, and the native name has no
    `models/` prefix. So whether `terrain/...` resolves depends on the search registration.
    Prediction, low confidence: `heightmaps=0/4 colormaps=0/4`.
- **Zones:** `groups=6 zones=21` on both sides.
- **Consumers:** no consumer moves.
  - Ground height stays 0.0 and the segment query has no islands.
  - The avoid-zone probe line and the gunnery segment rows are identical.
- **Outcome:** identity, a zero band, on deaths (7), hit records (150), shots (583) and torpedo
  drops (0). Every native row and death row is identical except:
  - the four implementation rows the switch adds (`create_path`, `create_landscape` and the two
    Landscape records);
  - the `class_creator [004f1460]` / `[004ea650]` record calls, which disappear.

  The two `scene class` rows' creator column and the census line also change.

**USN04 4700/4500** (`usn_19_coralus.scn`, no rows).
- **Census:** zero on both sides, `path seen=0 ... landscape seen=0`.
- **Outcome:** identity on every row.

### The pairs, measured

- **Builds.** One tree (`agent/cc9-scene-entities` at `0e10276ef`, which is main `76c08fbe2` plus
  this packet), built twice with only the switch flipped:
  - `local\pl_off`, SHA-256 prefix `7CA40E879BC2`;
  - `local\pl_on`, SHA-256 prefix `6ECF040F016B`.
- **Logs.** `local\pl_{off,on}_{usn01,usn04}.log`. Each shows the 1600x900 override and its own
  module directory in this tree, and each exited 0.

**USN01 3200/3000: identity on every measured row.**
- **Census.**
  - OFF: `bound=0 path seen=55 generated=55 rejected=0 created=0 landscape seen=4 generated=4
    rejected=0 created=0 list47=0 list44=0`.
  - ON: `bound=1 ... created=55 ... created=4 list47=55 list44=4 avoid_zone_paths=21
    list_order_matches_records=1 terrain heightmaps=4/4 colormaps=0/4 models=4/4`.
- **Zones.** Both sides: `groups=6 zones=21 source_points=2193 corners=2193 associated=21`.
- **Outcome.** Both sides: 7 deaths, 150 hit records, 583 shots and `damage=2690.0`. All 23
  death rows and every summary line are identical, except the census line.
- **Native table.** Six rows differ, all as predicted:
  - `create_path` 004EA650 (55) and `create_landscape` 004F1460 (4) appear;
  - `Landscape::load_terrain` 00882AC0 (4) and `Landscape::attach_terrain_vcall_9c` 00883BB0 (4)
    appear;
  - the `SceneContents::class_creator` record goes from 70 calls to 11.

  The native table keys that record by name and shows the first caller's address. OFF lists it
  as 004F1460 with 70 calls: 4 Landscape, 55 Path and 11 others. ON lists it as 004E9D40 with the
  11 others.
- **The four Landscapes:**
  - `Landscape 03`, FilePath `islands/m07_a`;
  - `Landscape 04..06`, FilePath `islands/DLC_L_03_S`.

  Each has a `ShallowWater` block and an empty `ModelPath`.

**Failed prediction: the heightmaps resolve, 4/4, not 0/4.** The native name
`terrain/islands/<F>_heightmap.tdt`, with no `models/` prefix, is found by this process's VFS
search, including the DLC island's, which has no loose file. So the `.tdt` height field is
reachable for a later terrain reader. The colormaps resolve 0/4 as predicted, and the models 4/4.

**USN04 4700/4500: identity on every measured row.**
- **Outcome.** Both sides: 41 deaths, 743 hit records, 5,603 shots and `damage=11494.8`. All 112
  death rows and every summary line are identical, except the census line.
- **Native table.** Three rows differ: `create_path` 004EA650 appears with 4 calls, and the
  `class_creator` record changes from 004EA650 (5) to 004E99B0 (1).

**Failed prediction: USN04 has four Path rows, not none.** `usn_19_coralus.scn` authors no
`AvoidZone` path and no Landscape, but it has four non-zone paths, `CarrierPath1..4`, with party 2
and 6, 6, 8 and 8 points. The census reads `path seen=4 created=4 list47=4 avoid_zone_paths=0`.

**Verdict: ON.** Both pairs are identity on every death, hit-record and shot row, and the census is
exact. The objects now exist for the consumers listed under the contracts above. No consumer reads
them yet, so no measured row moves.

## 6. The Landscape terrain and the four ground queries (packet `cc9_landscape_terrain`, 2026-09-27)

Section 5 left the terrain as a VFS census. This section reads the height field and the four world
queries over list 44h, and binds them in this host under `kSceneLandscapeTerrainBound`
(`include/bsp/game_hosts_scene_contents.hpp`). The binding is read-only analysis of the image plus
host code; no consumer file is edited.

### Where the height field comes from

- **Both arms of 00882AC0 reach the `.tdt` parser 00ADDA60.** 00882EE6..00882EFD test
  `[0109CEECh]` vtable+8h on a name.
  - The false arm calls 00ADDA60 directly at 00883A7D.
  - The true arm loads the `.mmod` (+41Ch, +420h) and calls 00ADE820 at 00883058, with the
    terrain in ECX, the model instance, the map extents and `&landscape+74h`.
  - 00ADE820 builds the terrain node +2Ch (`Terrain Visibility Group`) and hands it the
    Landscape's +74h frame through vt+38h. It then calls 00ADE500 on `terrain/<F>` plus a
    14-character suffix, the length of `_heightmap.tdt`.
  - 00ADE500 fills the box at +68h through 00ADA420, the AABB walk over the node tree that holds
    the model, and calls 00ADDA60.
- **The terrain object** is 00ADD290 (vtable 00D5D350) on `[game+19ECh]`. Its base 00AE9870 sets:
  - `+14h` = 1e10 [00CE4970];
  - `+18h` = 300.0 [00CE3AE8], the tile size;
  - `+1Ch` = 9.375 [00D0E658], the cell size (32 x 9.375 = 300).

**The `.tdt` format** (00ADDA60 with the recovered structured reader; checked against the bytes
of `models/terrain/islands/m07_a_heightmap.tdt`, 166,296 bytes):

| level | tag | payload | reader |
| --- | --- | --- | --- |
| root | `TRNV2` | two dwords: tiles wide (11), tiles deep (12) | 00ADDC17, 00ADDC26 -> 00ADADC0 (00ADDC45) grid `+38h x +3Ch`, pointers at `+40h` |
| child | `NODE` | two dwords: tile x, tile z; then chunks | `NODE` by `__stricmp` at 00ADDC91; x and z at 00ADDCA6 / 00ADDCB1; the tile from `[00F8C218]` vt+4h (00ADDCE2) stored at `+40h[+38h*z + x]` with no bound test (00ADDCE9..00ADDCEE); 00ADFEB0 at 00ADDCF8 |
| chunk | `U16` | float offset, float scale, 33x33 u16 samples, row-major by z | 00ADFEB0 -> 00ADFD70 (`00ADC420(21h, 21h)`, block vtable 00D5D314) -> 00ADC6C0 (+2Ch offset, +30h scale, +34h = 1/scale) |
| chunk | `F32`, `U8` | other sample forms | 00ADF960 / 00ADFD70 arms; not present in this installation's two island files as read |

m07_a has 75 of the 132 tiles.

**The origin.** For a `TRNV2` root, 00ADDB60..00ADDBFB compute:
- `+80h` = floor(`+68h` / 300 - 1.0) * 300;
- `+84h` the same from `+70h`;
- any other root gets 0 for both.

`+68h` and `+70h` are the box minimum 00ADA420 accumulates.
- **LABELLED SUBSTITUTION:** the host takes them from the island model's own `BoundingBox`.
- **The m07_a check.** Its box is (-599.68, -119.41, -711.07)..(2359.79, 90.60, 2248.41).
  - The origin is (-900, -1200), and 11 x 12 tiles of 300 m then span exactly the box's x and z.
  - The box's y range is the file's first float and the top sample.
- **The runtime check** is the self-check below: an authored object on the island sits on the
  sampled ground.

### The terrain slots

- **Sample**, block slot 8h 00ADC5F0 (**no Ghidra function**, 00ADC5F0..00ADC637): FFFFh answers
  -1000.0 [00D7A240]; otherwise the answer is float(s * inv_scale + offset). m07_a spans
  -119.41..90.60 m.
- **Cell**, slot 20h 00ADB3A0 (**no Ghidra function**, 00ADB3A0..00ADB475). It answers -1000.0 in
  three cases:
  - i or j is negative;
  - i or j is at or beyond slot 10h / 14h, which is tiles * 32 + 1 (00ADAC50..00ADAC64 and
    00ADAC70..00ADAC84, both **no Ghidra function**);
  - the tile is missing (00ADB40F).

  Otherwise slot 44h 00ADA890 (**no Ghidra function**, 00ADA890..00ADA8FF) splits the index:
  - tile = index >> 5 [00D5D65C = 5] and local = index - tile * 32;
  - an index one past the last tile takes that tile's column 32 [00D5D658 = 32].

  Neither global has a literal-address writer (`a3`/`89 ..` scans empty, `a1 58 d6 d5 00`
  found). The node y (+124h) is added to the block's answer, a hole's -1000 included.
- **Height**, slot 28h 00ADA900.
  - u = float((x - node x - `+80h`) * float(1/9.375)), and the same for v from z (x87 subtract
    chain, one store).
  - Slot 48h 00ADB480 then samples:
    - i = trunc(u) and j = trunc(v) (`CVTTSS2SI`);
    - a = h(i,j) + fu*(h(i+1,j) - h(i,j)) and b = h(i,j+1) + fu*(h(i+1,j+1) - h(i,j+1)), each
      stored to float;
    - fu = u - i and fv = v - j are floats;
    - the answer is a + fv*(b - a).
- **Normal**, slot 38h 00ADABA0.
  - Truncation only: the x index is a cast and the z index goes through `_ftol` 00BF7420.
  - Slot 30h 00ADAA40 takes P0 = (i, h(i,j), j), P1 = (i + 9.375, h(i+1,j), j) and
    P2 = (i, h(i,j+1), j + 9.375). The cell size is added to the grid index itself (00ADAA7E,
    00ADAADB).
  - It forms 004F9B30 cross(P2 - P0, P1 - P0) = (-9.375 dh10, 87.890625, -9.375 dh01).
  - 00419440 gives the length, and the result is scaled by float(1/length), or by 0 when the
    length is not positive.
- **Segment**, slot 3Ch 00ADA240.
  - Both endpoints go into node-local space through the full inverse world matrix, rotation
    included.
  - If |dx| and |dz| are under 0.001 [00D7A23C], it takes the vertical case 00AECC40.
  - Otherwise it subtracts the origin and walks the quadtree 00AEA2B0 -> 00AE9D80, built by
    00AEA900 / 00AEA820 at the end of 00ADDA60.
  - **Not reconstructed.**

The height and normal subtract only the node's translation, so they sample an island unrotated.
The segment test transforms fully. Two of USN01's four Landscapes are rotated (`Landscape 04` by
about 180 degrees, `Landscape 05` by about 90), which the self-check below measures.

### The four queries (list 44h at world+34Ch)

| query | body | walk | answer |
| --- | --- | --- | --- |
| ground height | 00903860, `RET 8` (point, out) | `*out` = -1000.0 (00903876); per node the terrain height, kept when strictly greater | AL = (`*out` != -1000.0 double [00CE6658]), 009038C2..009038E0; `*out` stays -1000.0 over no terrain |
| ground normal | 009038F0, `RET 8` (point, normal out) | the same walk, remembering the winner | best != -1000: winner's slot 38h into arg 2, AL = 1 (009039AF); else AL = 0, normal untouched. **It writes no height**: the section 5 name `BSP_World_GroundHeightAndNormalAt` is corrected to `BSP_World_GroundNormalAt` |
| landscape at | 009039D0, `RET 4` (point) | the same walk | the winning Landscape, or 0 (EBP zeroed at 009039E1) |
| segment | 00903BC0, `RET 8` (from, to) | 00903860 at `from`, blocked when it is above from.y (00903BE8 `JBE`); at `to`, blocked when above to.y (00903C18 `JA`); then every node's slot 3Ch | AL = 1 on the first block, else 0 |

**So the no-terrain answer is -1000.0.** It is written at 00903876 from the float at 00D7A240,
and 009038C2 tests it against the double at 00CE6658. `src/game_hosts_units.cpp`'s
`ground_height_00903860() { return 0.0f; }` is therefore not the no-terrain answer. Over open sea
the native answers -1000 with AL = 0; over an island it answers the sampled height.

### The binding

- **The load.** `kScenePathLandscapeCreatorsBound` is ON (section 5). With
  `kSceneLandscapeTerrainBound`, `create_scene_world_object` loads each Landscape's height field
  at the pass-A point:
  - the model box from `read_mmod_bounding_box`;
  - the `.tdt` through the recovered structured reader;
  - the node translation from the object's frame.
- **The queries.** `world_ground_height_00903860`, `world_ground_normal_009038f0`,
  `world_landscape_at_009039d0` and `world_segment_blocked_00903bc0` answer over
  `scene_world_class_lists()`. The first three and the segment's two endpoint tests are the
  native's.
- **LABELLED STAND-IN.** The per-terrain sweep of 00903BC0 marches the segment in half-cell steps
  against the height, instead of slot 3Ch's quadtree walk.
- **Rounding.** Every x87 store is kept as a float store. Where the native keeps an 80-bit
  intermediate, this uses a double: the sample `s*inv + offset`, the grid-coordinate subtraction
  chain and the length's sum.
- **The self-check.** At the end of the load, every generated object whose authored parent is a
  Landscape is queried at its world position with all three point queries. The line
  `scene terrain self-check` reports how many sit within 1 cm of the ground and whether 009039D0
  names their own Landscape. `summary scene terrain` carries the census, and the census is then
  reset.
- **OFF** loads nothing and the census is zero.

### Contracts for the consumers (not edited here)

Rel32 census of the four bodies: 00903860 has 30 sites, 009038F0 one, 009039D0 one and 00903BC0
four. The containing routine and the host file that names it:

| owner file | sites | routine |
| --- | --- | --- |
| `src/game_hosts_units.cpp` (cc9-plane-release) | 0099FA2C, 009A0624, 009A0E56 (height); 009A15C1, 009A172E (segment) | 0099F1C0 `BSP_PilotBot_AvoidTerrain`; the host's `segment_blocked_00903bc0` record and `avoid_surface_height` |
| `src/game_hosts_units.cpp`, `src/torpedo_aim_tick.cpp` | 009D16D1, 009D2265 (height) | torpedo aim tick; `ground_height_00903860() { return 0.0f; }` |
| `src/game_hosts_units.cpp`, `src/torpedo_approach_update.cpp` | 009D39D3 (segment) | 009D3420 torpedo approach |
| `src/airfield_taxi.cpp` | 006CF8FD (height) | 006CF730 `BSP_AirOpsSite_PoseQueuedPlane` |
| `src/fixed_step_callbacks.cpp` | 007AC175 (height), 007AC244 (normal) | 007AC000 |
| `src/unit_message_arms.cpp` | 00821165 (height) | 008206F0 |
| `src/mission_lua_host.cpp` | 0089494E; 00894DD1, 00894E3F, 00894EA9 (height) | 00894820, 00894C00 |
| `include/bsp/lua_spawn_new.hpp`, `include/bsp/lua_binding_spawn.hpp` | 00941E22, 00942965 (height) | 00941D30, 009426D0 |
| no host reference | 00430991, 00434F8D, 00487C64, 0049BB07, 00790555, 00795B62, 0079B57A, 007AB27C, 007AB435, 007ABCD9, 0092D086, 0092D0F6, 0092D15E, 009412E6 (height); 004B2131 (landscape at); 007C3E00 (segment) | 00430970, 00487A70, 0049B1F0, 00790540, 00795650, 0079A3B0, 007AB230, 007ABB30, 0092D000, 00941D30 region, 004B2030, 007C3CB0; 00434F8D, 007AB435 and 009412E6 lie in no Ghidra function |
| this host | 00903BD8, 00903C08 | 00903BC0 itself |

**The contract each consumer binds.**
- **Height.** Call `world_ground_height_00903860(point, out)` in place of its stand-in. It gets
  the island's height, or -1000.0 with `false` over sea.
- **Segment.** A segment test calls `world_segment_blocked_00903bc0`.
- **The gunnery segment query 0098ADD0** is a different body. It walks the spatial index, which the
  terrain node joins at 00884078 (section 5). Terrain there needs slot 3Ch's quadtree, not these
  queries.
- **The avoid-zone layer sample 0041BC20** reads the `.nav` layers, not this height field.

### Predictions, written before the runs

One tree, both switches of section 5 ON. The pair flips only `kSceneLandscapeTerrainBound`. Streams
and the death table are on, and the commands are as in section 5.

**USN01 3200/3000.**
- **OFF:** `summary scene terrain bound=0 landscapes=4 loaded=0 blocks=0 self_check objects=0`,
  and all counts 0.
- **ON, loaded=4:**
  - `Landscape 03`: root `TRNV2`, tiles 11x12, 75 blocks, box_min (-599.68, -711.07), origin
    (-900, -1200), node (3000, 0, -4000), height [-119.41, 90.60].
  - `Landscape 04..06` (`DLC_L_03_S`): box_min (-2900, -2950), so origin (-3300, -3300). Tiles
    and blocks are unknown before the run: that file is not loose.
- **Self-check, Landscape 03.** Most of its authored buildings, forts and parked aircraft are
  within 1 cm of the ground. The Python prototype of this sampler matched 30 of the 39 objects it
  listed to within 0.001 m. The misses are expected:
  - landing points at sea, within 0.6 m;
  - `Coastal Gun 01`, off by 66 m;
  - the airfield's `Multi Hangar 1`, a nested child.

  Prediction: `on_ground_1cm` at least 30, and `landscape_at_self` equal to its object count less
  the sea points.
- **Self-check, 04..06.** `Landscape 06` is unrotated: most of its objects on the ground.
  `Landscape 04` (about 180 degrees) and `Landscape 05` (about 90 degrees) are rotated, and height
  ignores rotation, so few of their objects should be within 1 cm. **Uncertain:** it would mean
  the native ground query does not match a rotated island's geometry, which the run will show.
- **Consumers:** zero calls after the load; no consumer is bound.
- **Outcome:** identity on every death, hit-record and shot row: 7 / 150 / 583, torpedo drops 0.
  Every native row is identical except `Landscape::load_height_field` 00ADDA60 (4, ON), and every
  summary line except `summary scene terrain`.

**USN04 4700/4500** (four Paths, no Landscape).
- **Census:** `landscapes=0 loaded=0` on both sides.
- **Outcome:** identity on every row, 41 / 743 / 5603.

### The pairs, measured

- **Builds.** One tree (`agent/cc9-scene-entities` at `3f5b154b8`, which is main `365b1b967` plus
  this packet), built twice with only `kSceneLandscapeTerrainBound` flipped:
  - `local\lt_off`, SHA-256 prefix `68F5491766B2`;
  - `local\lt_on`, SHA-256 prefix `EEECEEE00EDE`.
- **Logs.** `local\lt_{off,on}_{usn01,usn04}.log`. Each shows the 1600x900 override and its own
  module directory in this tree, and each exited 0.

**USN01 3200/3000: identity on every measured row.**
- **Outcome.** Both sides: 7 deaths, 150 hit records, 583 shots and `damage=2690.0`. All 23
  death rows and every summary line are identical, except `summary scene terrain`.
- **Native table.** The one differing row is `Landscape::load_height_field` 00ADDA60 (4 calls,
  ON only).
- **Loaded, ON:**
  - `Landscape 03`: `TRNV2`, 11x12 tiles, 75 blocks, box_min (-599.68, -711.07), origin
    (-900, -1200), node (3000, 0, -4000), height [-119.413, 90.548].
  - `Landscape 04..06`: `TRNV2`, 19x22 tiles, 234 blocks each, box_min (-2900, -2950), origin
    (-3300, -3300), height [-110.003, 209.148]. Their nodes are (-2000, 0, 3000),
    (-7000, 0, 4000) and (-6500, 0, -2000).
- **Self-check.** On `Landscape 03`, 41 of its 51 authored objects are within 1 cm of the
  sampled ground. The worst is 66.34 m, `Coastal Gun 01`, as the prototype found. 009039D0 names
  `Landscape 03` for all 51.
- **Census.** Height, normal and landscape-at are each 51 calls, 51 hits and 0 fallbacks. Segment
  calls are 0. No consumer calls any query after the load.

**Failed or vacuous predictions:**
- **Landscapes 04..06 author no child objects**, so the rotation question has no evidence on this
  mission (`objects=0` for all three). It stays open: does the native ground query sample a
  rotated island unrotated? 00ADA900 subtracts only the node translation, and the only way the
  answer differs from the island's geometry is a rotated Landscape with something standing on it.
- **`landscape_at_self` is 51, not "less the sea points".** The landing points lie inside the
  island's tile grid over shallow water, where the height field answers a sea-floor height rather
  than -1000.
- **The top of m07_a is 90.548, not 90.60.** The box's maximum is the model's; the height field's
  highest sample is 5 cm lower.

**USN04 4700/4500: identity on every row.**
- Both sides: 41 deaths, 743 hit records and 5,603 shots.
- The native table is identical: 0 rows differ.
- The census is `landscapes=0 loaded=0` on both sides.

**Verdict: ON.** Both pairs are identity on every death, hit-record and shot row, and the height
field reproduces the authored ground to 1 cm on the island that has objects. The queries now
answer for the consumers listed under the contracts above.

### Rotation: the ground query samples a rotated island unrotated (packet `cc9_landscape_rotation_read`)

**The listing.** No step between the world point and the grid applies the Landscape's rotation.
- 00903860 passes the caller's world x (`[EDI]`) and z (`[EDI+8h]`) straight to each terrain's
  slot 28h (0090387D..0090389E). 009038F0 and 009039D0 do the same (00903920..0090393F and
  009039F3..00903A10).
- Slot 28h 00ADA900 subtracts only the terrain node's world **translation**:
  - `+120h` x at 00ADA926 and `+128h` z at 00ADA950;
  - these are row 3 of the node's world matrix at `+F0h`, whose frame 00ADE820 took from the
    Landscape's `+74h`;
  - then `+80h` / `+84h`, and the scale by float(1/9.375).
- The slot-38h normal does the same. The rotation rows (`+F0h..+11Ch`) are never read.

So **the terrain grid sits axis-aligned at the entity's origin.** A Landscape's rotation reaches
two other places:
- the segment test, where slot 3Ch 00ADA240 goes through the node's full inverse world matrix;
- the avoid zones, where 0041CCD0 builds them with the parent Landscape's matrix.

It never reaches the height, the normal or the landscape pick.

**Ground truth in this installation.** `usn_1_marshall.scn` authors no object on Landscapes 04
and 05. Each authors the same five `AvoidZoneG` paths as `Landscape 06`: layers 1, 11, 3, 46 and
86, parents 93, 99 and 105. The zone points are in island-local space (the path frame composed
with each `Pos`), and the zones follow the seabed.

The probe `local/rot_probe.py` (in this worktree) samples each point two ways:
- **locally**: the height field at the island-local (x, z);
- **the image's way**: at R·local, which is what (world - translation) is.

It uses this installation's `terrain/islands/dlc_l_03_s_heightmap.tdt` (19x22 tiles, 234 blocks,
origin (-3300, -3300)):

| Landscape | rotation | layer 1 (144 points), local | layer 1, image's mapping | layer 11 (116 points), local | layer 11, image's mapping |
| --- | --- | --- | --- | --- | --- |
| 03 (m07_a, 84 / 21 points) | none | -56.1..-3.1 | identical | -106.2..-22.0 | identical |
| 04 | about 180 degrees | -76.0..-2.4 | -110.0..**+83.9** | -110.0..-41.2 | -110.0..**+159.0** |
| 05 | about 90 degrees, slight tilt | -76.0..-2.4 | 27 holes, -110.0..**+146.1** | -110.0..-41.2 | 16 holes, **-458.5**..+152.5 |
| 06 | none | -76.0..-2.4 | identical | -110.0..-41.2 | identical |

- **Local sampling** puts every zone layer on a consistent seabed contour on all four islands,
  and never on land.
- **The image's mapping** agrees exactly on the two unrotated islands. On the rotated two it
  scatters: zone points land up to 159 m up the island's slopes and in holes. On 05 one point
  crosses a hole-adjacent cell for -458.5.

The zones do rotate with the island (0041CCD0), so the image's own ground height disagrees with
its own avoid zones on Landscapes 04 and 05.

**Answer.** 00903860 does not apply the Landscape's rotation: the grid is axis-aligned at the
entity's origin. On USN01's two rotated islands the image's ground height is therefore not the
island's geometry. The host reproduces this as it stands: `height_00ada900` subtracts the
translation only.

**Not checked:** whether the renderer rotates the island model. The terrain node receives the
Landscape's full `+74h` frame through vt+38h at 00ADE820, which suggests it does. That would make
the rotated islands visibly disagree with their own ground height.

**Later ground truth.** If a consumer is bound (section 7), a unit that ends up standing on, or
a probe that crosses, Landscapes 04 or 05 is the evidence. The zone contours above are already
one.

## 7. The ground-query consumers (packet `cc9_ground_height_consumers_1`, 2026-09-27)

**No consumer of the four queries sits in a free file, so this packet binds nothing.** The rest of
this section is the reading of every consumer the executable reaches, and the exact hunks for its
owner.

### Where the 36 call sites land in this process

The section 6 census lists 30 sites of 00903860, one each of 009038F0 and 009039D0, and four of
00903BC0. Searched against `src/` and `include/bsp/` (every body address and every site address),
they fall into three groups.

**The executable reaches them, all through `src/game_hosts_units.cpp`**, leased by
cc9-plane-release for `cc9_ship_motion_tail`:
- 009D16D1 and 009D2265, the torpedo aim tick: `AimTickBinding::ground_height_00903860`, which
  returns 0.0.
- 009D39D3, the torpedo approach's short-range probe: the `segment_blocked_00903bc0` binding. It
  uses the water surface as the ground, and its sweep is a record.
- 009A15C1 and 009A172E, AvoidTerrain branch A: `terrain_avoidance_0099f1c0` returns before the
  probe. That is a labelled substitution ("no ground model").
- 0099FA2C, 009A0624 and 009A0E56, AvoidTerrain's height reads: never reached in the image
  either (below).

**Named in an unleased file, but not reached on the measured missions:**
- `IsLandscape` (00894820) and `IsAreaEmpty` (00894C00) are in `src/mission_lua_host.cpp`'s
  native table. They are called only from `scripts/global/commandhelpers.lua` (lines 4719..5029,
  9165 and 13459: move-order and spawn helpers). Neither native has a row in the section 6 logs
  of USN01 or USN04.
- The Lua files are cc9-world-init's in any case.

**No host counterpart:** the remaining 20 sites (section 6 table), among them 004B2131 (009039D0)
and 007AC244 (009038F0).

**None of the named free files has a site.**
- The ship AI calls none of the four; its 009E/009F bodies are not in the census.
- Gunnery's segment query is 0098ADD0, over the spatial index, a different body.
- The avoid-zone runtime reads `.nav` layers.

### What the image does with each reached result

| site | routine | use of the result | no-terrain answer | host today |
| --- | --- | --- | --- | --- |
| 009D16D1 | torpedo aim tick | altitude floor = max(sea 5.0 or land 30.0, alt pair, ground + 5.0) (009D16D6..009D16F8); AL not tested | -1000, so ground + 5 = -995 never wins | 0.0, so ground + 5 = 5.0, which equals the sea floor under a strict `>`: **no effect at sea** |
| 009D2265 | torpedo aim tick | arm the release timer when 1.0 [00D7A24C] > ground (009D2272); AL not tested | 1 > -1000: arms | 1 > 0: arms, **the same at sea** |
| 009D39D3 | 009D3420 torpedo approach | `00903BC0(target point, (px, 1.0, pz))` for a sector ray inside 400 m; blocked unmarks the sector (009D39E0) | endpoints clear against -1000 and no terrain to sweep: **never blocked at sea** | blocked when either endpoint is below the **water surface** (`ocean_water_height_0078cf20`), plus the record `World::segment_occluders_00903bc0` |
| 009A15C1, 009A172E | 0099F1C0 AvoidTerrain branch A (unit+900h == 6, on the water) | +-30 degree heading probe at y = 0.1; a clear probe returns (`JE 009A17A1`) and a blocked one steers | clear at sea | returns before probing |
| 0099FA2C, 009A0624, 009A0E56 | 0099F1C0 | reached only when pilot+268h > 0.0 (`JBE` at 0099FA11); AL = 0 substitutes 10.0 [00CE38B8] (0099FA35, 009A062D, 009A0E5F); the ground is then blended with the 0041BC20 layer sample by pilot+268h (00419010 at 0099FA6B) | **the image uses the boolean here**: no terrain means 10.0, not -1000 | never reached: pilot+268h is only ever stored as 0 (`docs/ATTACKER_EVASION.md`) |

The hot site is the torpedo approach probe: 1,152 record calls on USN01 and 7,164 on USN04 in the
section 6 logs (`World::segment_occluders_00903bc0`). Its target point is the target ship's
position (`approach_target_point`). So the host's water test can block a sector whenever the
ship's origin sits below the local wave height, which the image never does at sea.

### The hunks for `src/game_hosts_units.cpp` (cc9-plane-release; not applied here)

Both hunks need `#include "bsp/game_hosts_scene_contents.hpp"` for the queries.

1. **Torpedo aim, `AimTickBinding::ground_height_00903860`** (about line 13950):

   ```cpp
   float ground_height_00903860() override {
       // 009D16D1 / 009D2265: 00903860(world, unit+FCh, &out). Over no terrain
       // the image leaves out = -1000.0 (00903876) with AL clear, and neither
       // site tests AL (docs/SCENE_CONTENTS_HOSTS.md section 7).
       float out = 0.0f;
       bsp::game::world_ground_height_00903860(s_.motion.position, out);
       return out;
   }
   ```

2. **Torpedo approach, `segment_blocked_00903bc0`** (about line 8613). Replace the water-surface
   body with the world query:

   ```cpp
   bool segment_blocked_00903bc0(const float from[3], const float to[3]) override {
       // 00903BC0: the two ground tests against 00903860 (-1000.0 over no
       // terrain), then each Landscape's slot 3Ch (a labelled half-cell
       // stand-in in the scene host). docs/SCENE_CONTENTS_HOSTS.md 6 and 7.
       return bsp::game::world_segment_blocked_00903bc0(from, to);
   }
   ```

3. **AvoidTerrain branch A** (about line 5722) is a larger change. The probe geometry after
   009A1420 is not reconstructed in the host, so the early return stays. The contract for it is
   `world_segment_blocked_00903bc0` at y = 0.1: clear, then return; blocked, then steer.
4. **AvoidTerrain height reads**: no hunk. If pilot+268h ever becomes non-zero, they need
   `world_ground_height_00903860` with **AL tested**, and 10.0 substituted when it is false.

### Predictions for the owner's pair (hunks 1 and 2, switch only)

**USN04 4700/4500** has no Landscape, so the queries answer -1000 / false everywhere.
- **Hunk 1:** identity. At sea the floor and the arm test are unchanged, as the table shows.
- **Hunk 2 is NOT expected to be identity.** Every probe the water test blocked now clears. That
  leaves more sectors marked clear at 009D39E0 and changes the torpedo approach's sector choice.
  - Direction: blocked sectors go to 0; torpedo drops and hit records move in either direction.
  - Band: unknown. The host keeps no count of blocked probes, and 7,164 probes ran. A counter of
    blocked results in both builds should come first.

**USN01 3200/3000** has four islands.
- **Hunk 1** moves an aim tick only over land where the ground is above 0 m. That raises the
  floor to ground + 5 and closes the arm (1 > ground fails above 1 m).
- **Hunk 2** clears the sea-level blocks as on USN04. Probes that cross an island now block. The
  label is that the stand-in sweep marches the height rather than the quadtree.

  USN01's torpedo drops are 0 on this base (section 5 pairs), so the aim tick's release path is
  not exercised. The expected movement is in sector choice and in the approach geometry, not in
  the drop count.

## 8. The Landscape in the spatial index (packet `cc9_landscape_spatial_attach`, read, 2026-09-27)

This is the read-only part of the packet. The binding waits for `src/game_hosts_gunnery.cpp`,
leased to cc9-world-init for `cc9_loss_warning` when this was written.

### How an island enters the segment query

1. **The shape.** Landscape pass A 00883BB0 appends the segment-trace sub-object `+344h`
   (vtable 00CEA050) to the collision node `+1E4h`'s shape array (00883F88..00883FA1: slot
   `[+1E4h + [+2DCh]*4 + D0h]`, count `node+F8h`). 00883FAE sets `+230h` = the Landscape.
2. **The bounds.** 0098A920 `BSP_SpatialNode_SetLocalBounds` at 00884066 takes the terrain's
   vt+0Ch box less the pose translation.
3. **The attach.** 0098BA10 at 00884078 is called with index `0042E630()`, node `+1E4h`,
   parent 0, matrix `+384h` and static 1.
   - With no parent it takes steps 4..8 of `docs/SPATIAL_INDEX.md`'s attach.
   - An island's box spans far more than 2x2 cells, so step 8 puts the node in the **loose array**
     `index->loose[looseCount++]`. That array is what 0098ADD0 walks after the grid rectangle.
4. **The trace.** 0098AC20 calls each shape's vtable[0], here 0087FF80
   `BSP_CollisionShape_TraceSegment_Subobject`:
   - it loads `[shape+8Ch]`, which is Landscape `+3D0h`, the terrain (0087FF86), and returns 0
     when it is null;
   - it calls terrain vt+3Ch (from, to, &hit) at 0087FFA4: 00ADA240, the node-local quadtree
     walk of section 6 that is not reconstructed;
   - on AL != 0 it writes the hit point to record `+8h`, `+Ch`, `+10h`;
   - it sets the record's entity to `shape - 344h` = the **Landscape** (00470370 at 0087FFD9);
   - it stores record `+30h` = 0Ah (shape kind) and `+34h` = -1 (hull segment), and returns 1.
5. **What the hit means.** The Landscape is class 44h. `docs/PROJECTILE_IMPACT.md` maps a hit
   entity answering vt[5Ch](44h) to impact mode 3, the terrain effect slot (0084BC99).

So a land hit is a terrain height-field hit, not a collision-mesh hit. The `.mmod` parts at
`+418h` attach separately (00710B6D) and are not read here.

### The binding plan (for when the file is free)

`SegmentBinding` (`src/game_hosts_gunnery.cpp`, about line 5104) answers `loose_entity_count`
with the unit count and one shape per unit.

**The hunk, under one switch committed OFF:**
- `loose_entity_count` = units + `scene_world_class_lists().list(0x44)`, where a Landscape
  object has a loaded height field.
- **Entity handles.** Units keep 1..N; the Landscapes follow.
- **Per Landscape entry:**
  - `entity_owner` is itself;
  - `entity_is_kind` answers the native's class set (44h, 1, 0);
  - `entity_bounds` is the height field's world box (origin, tiles x 300 m, the sample range);
  - `shape_count` is 1.
- `shape_trace_segment` for a Landscape calls a new scene-host query, the section 6 segment test
  with a hit point. It returns the first point where the march goes below the height, the
  labelled stand-in for 00ADA240. It fills the record: point, entity, kind 0Ah, segment -1.
- `query_segment_units_impl` reports a land hit apart from a unit hit (`hit_unit` stays 0 and a
  land flag is set), so the three callers can be read against the image before they change.
- **The trace counters stay as they are.** `shell_mesh_hits` and `narrowphase_box_0085cdb0` are
  restored around the query, as today.

**What the consumers do with a land hit is read when the hunk lands, not here:**
- the pick 009043A0's hit kinds;
- the gun seat's aim at 00957DA0;
- the line-of-fire refusal.

### Predictions for that pair (switch only, streams on)

**USN04 4700/4500** (`usn_19_coralus.scn`).
- It authors no Landscape: its four class-47h rows are the non-zone `CarrierPath1..4`.
- So no island joins the index, and every ray answers as today.
- Identity on every row: 41 deaths, 743 hit records, 5,603 shots.

**USN01 3200/3000.**
- **Rays reach land.** Four islands join the index. The player controls `Airfield2` on
  `Landscape 03`, so the idle pick and seat rays start on or above that island.
- **The pick and seat queries** gain land hits.
- **Fort line-of-fire rays** leave from the island's surface. Any gunnery segment test that
  starts at a fort on land can be refused by the ground under or near it.
- **Direction:** land hits > 0; shots equal or down; hit records and deaths equal or down.
- **Band:** unknown until the per-consumer counters exist. On the current base these are 583
  shots, 150 hit records and 7 deaths.

### What each consumer does with a land hit (read, 2026-09-27)

0098ADD0 has 15 rel32 callers plus one tail-jump from 009043A0. 009043A0 has five callers. The
four the packet names:

**The pick screen, 009043A0 at 00526C4A (00526A40 `BSP_HudUnitPickScreen_PickUnit`).**
- A land hit leaves `record+0` = the Landscape, so 00526C62 does not take the no-hit exit.
- The hit-section block (00526C80) needs kind 6, so it is skipped.
- The seven `vtable[5Ch]` tests at 00526D1A..00526D84 ask for 6, 0Fh, 45h, 46h, 1Bh, 35h and
  then 1Eh. A Landscape answers only 44h, 1, 0 and its own class (004F1360), so every test fails.
- 00526D88 jumps to 00526DAF: the pick is zeroed, and execution falls into 00526DB7, **the
  lock-radius walk**, the same path a miss takes.
- **So a land hit gives no pick and does not block the radius walk.**
- **But the land is still an occluder.** 0098ADD0 keeps the nearest hit and shortens `to` to it,
  so a ship behind the island on the camera's line loses its ray pick and is only reachable by the
  radius walk.

**The player gun seat, 0098ADD0 at 00957DA0 (00957BD0), `docs/PLAYER_GUN_SEAT.md` 6.1.**
- The test at 00957DA5..00957DD2 is AL set and the hit's y above 0.0 [00D7A218]. **It does not
  look at the hit entity.** When the test passes, the land point becomes the aim point and the
  code jumps to 009580AE, past the range sphere and the sea cut.
- **So the seat aims at the island's surface** where the camera ray meets it above sea level. A
  land hit at y <= 0 falls through to the range-sphere point, like a miss.

**The gunnery line of fire, 0072CDD0 `BSP_LineOfFirePredicate_Blocked`, 0098ADD0 at 0072CE91.**
- The second half of the predicate (`docs/SHIP_PLATFORM_ATTACHMENT.md`: "or when the
  static-geometry query hits") is this call.
- The target point is clamped to y >= 5.0 [00CE3850] (0072CE62..0072CE76). 0042E630 gives the
  index.
- The call is `0098ADD0(from = EBX, to, exclude 0, record = ESI+4, kind filter **44h**)`: pushes
  0072CE81..0072CE8E.
- AL set jumps to 0072CEE5, `MOV AL,1` / `RET 10h`: **blocked**. Otherwise it goes on to the
  friendly-unit walk 0098B130 (0072CE9A).
- **So the only thing the static half can hit is a Landscape** (the kind filter is 44h). A gun
  whose line to its target crosses an island refuses the target, and 0072F6E0 caches the answer
  per (gun, target).
- The host models only the friendly-unit half (`src/game_hosts_gunnery.cpp` line 154: "the
  static-geometry half (spatial query, flags 44h) is not modelled").

**The projectile, 0098B370 `BSP_SpatialIndex_SweepSegments` at 0084C1D3 (0084BF00).**
- The entity sweep reaches the Landscape as a loose entity, so a shell's segment that crosses an
  island hits it: an entity hit, mode 1.
- **0084BC60** refines it to **mode 3** at 0084BC99..0084BCA1 (`vtable[5Ch](44h)`), then:
  - teleports the shell to the hit point (0084BCD4);
  - takes the effect switch with the terrain slot `d+38h` (0084B8C0 / 0084B6F0, the point effect
    on `[game+19ECh]`);
  - **kills the shell** (0084BE00);
  - queues the hit record with the Landscape as its subject (00926E80 at 0084BE20);
  - runs the class's explosion when `classDesc+6Ch` is set (0084BE32 -> 0084BAD0), so the blast
    radius can still reach a unit next to the impact.
- **The dispatcher 009239A0**, draining that record:
  - step 4 (00923A2C): the Landscape passes 44h, so `entity->vtable[24h]` = **0087F9A0** runs when
    `source+CCh >= 0`. It calls `[+3D0h]` vt+40h, the terrain's render geometry, and 007407C0 on
    `+168h` (section 5's slot 24h body), which reads as a terrain mark;
  - step 5 (00923A70): `vtable[ECh]` on a Landscape is 0042BAF0 `BSP_Entity_HitNotHandledStub`,
    and a top-level island has no `+3Ch` parent, so no hit handler accepts it.
- **So a shell that meets land stops there.** It makes the terrain effect and mark and deals
  only its blast, and it never reaches a target behind the island.

**What the binding must add, per consumer, when the gunnery file is free.**
- **The pick:** nothing beyond the entry. The query's nearest-hit shortening already makes the
  island an occluder, and the no-pick path is the miss path.
- **The seat:** nothing: it uses the point.
- **The line of fire:** the static half, a kind-44h query from the gun's point to the target
  raised to at least 5 m, blocked on any Landscape hit. This is the change that moves shots.
- **The projectile trace:** the Landscape entry in the host's shell sweep, with the mode-3
  outcome. The shell ends at the island, with the terrain effect and its blast, and no hit
  record reaches a unit.

**Predictions revised with this read (USN01; USN04 stays identity, no Landscape).**
- **Shots:** down or equal. Forts on Landscape 03 and ships firing across it lose targets behind
  the island to the refusal.
- **Hit records:** down or equal. Shells that would pass through land now end on it.
- **Deaths:** down or equal.
- **Band:** none until the counters of the hunk exist: static refusals, shells ended on land,
  and pick and seat land hits.

### The scene-host half, in place (packet `cc9_landscape_attach_scene_half`)

These entries are committed in `include/bsp/game_hosts_scene_contents.hpp` and have no caller
yet. The gunnery hunk is the loose entries plus the calls, under one switch.

| entry | contract |
| --- | --- |
| `std::size_t landscape_segment_entry_count()` | Landscapes of list 44h with a loaded height field, in list order (the order 00884078 appended them) |
| `int landscape_segment_entry_object(std::size_t entry)` | the entry's index into `scene_world_class_lists().objects()`, or -1 |
| `bool landscape_segment_entry_bounds(entry, float min[3], float max[3])` | the tile grid by the sample range, through the Landscape's world frame; the analogue of 0098A920's box |
| `bool landscape_entry_segment_hit(entry, from, to, LandscapeSegmentHit&)` | 0087FF80 -> 00ADA240's analogue, below |
| `bool landscape_segment_hit(from, to, float hit_point[3], int& landscape_index)` | the nearest hit over all entries, as 0098ADD0 keeps the nearest |
| `SceneLandHitCensus& scene_land_hit_census()`, `note_land_hit_query(LandHitConsumer, bool)`, `std::string format_land_hit_census()` | calls and land hits for `PickRay`, `GunSeat`, `LineOfFire` and `Projectile`, and the line-of-fire blocks |

**`landscape_entry_segment_hit`.**
- **The transform.** Both endpoints go into the Landscape's local frame through the full inverse
  of its world frame, as 00ADA240 does. So unlike the ground height, the segment test respects
  rotation.
- **LABELLED STAND-IN for the quadtree.** A half-cell march in local space against the height
  field. The first sample below the surface is refined by 24 bisections; a segment that starts
  below the surface hits at its start.
- **The hit.** The point goes back to world through the frame. The record gets
  `landscape_object`, `shape_kind` = 0Ah and `hull_segment` = -1 (0087FFDE, 0087FFE5), and
  `fraction` is the place along the segment.

**The census.** `summary scene terrain` gains `segment_entries=<n>` and the `land_hits` block
(`pick=calls/hits seat=... line_of_fire=... blocked=n projectile=...`). At load the block is zero
until a consumer calls. The gunnery hunk prints the same block in its end-of-mission summary.

### The gunnery hunk and its predictions (written before the runs)

`kLandscapeSpatialAttachBound` in `src/game_hosts_gunnery.cpp` is committed **false**. With it
true:
- **`SegmentBinding` entries.** It adds the Landscapes as loose entries after the units, with
  handles N+1.., the bounds of `landscape_segment_entry_bounds` and one shape.
- **The Landscape trace.** Its `shape_trace_segment` calls `landscape_entry_segment_hit` and fills
  the record: point, entity, kind 0Ah, segment -1. A unit hit clears `hit_landscape`, and a land
  hit clears `hit_unit`.
- **`query_segment_units_impl`** reports a land hit through a new `land_hit` flag and returns
  false.
  - **The pick** (`GameGunneryHost::query_segment_units`) sees no unit, which is 00526DAF.
  - **The seat** takes the land point as its hit, subject to the unchanged y > 0 test at 00957DAF.
- **The shell sweep.** A land hit ends the round: it runs `apply_impact_blast`, applies no hit and
  records `Landscape::on_hit_0087f9a0`. The new counter is `impacts_land`.
- **`line_of_fire_blocked_0072cdd0`** runs `landscape_segment_hit` from the raised muzzle to the
  clamped target first (0072CE91), and a land hit blocks. The existing per (gun, target) cache
  holds the answer.
- **The census.** `summary mission gunnery landscape attach bound=.. entries=.. pick=c/h
  seat=c/h line_of_fire=c/h blocked=n projectile=c/h impacts_land=n`.
- **Untouched:** the death route's `kLossWarningBound` call site, and the trace counters
  (`shell_mesh_hits` and `narrowphase_box_0085cdb0` are restored around the pick and the seat as
  before).

**The base for USN01 3200/3000** (section 6 ON log):

| consumer | volume |
| --- | --- |
| `UnitPickScreen::segment_query` | 6,160 calls |
| gun seat casts | 0 (the idle player never takes a seat) |
| AA line-of-fire queries | 144, 0 blocked |
| shell sweeps | 18,854 |

The shells break down as 583 created, 103 entity impacts, 451 expired and 0 water. The outcome is
7 deaths, 150 hit records and 583 shots.

**Predictions, USN01:**
- **Entries.** `entries=4`.
- **Pick.** 6,160 calls, and land hits between 1,000 and 6,160. The camera sits over the player's
  `Airfield2` on `Landscape 03`, so the 10,000-unit forward ray meets the island often.
- **Seat.** `0/0`.
- **Line of fire.** 144 queries, 0..40 blocked. AA on the island's forts fires upward at planes,
  and ships firing across `Landscape 03` can be cut.
- **Shells.** `impacts_land` between 0 and 150 of the 18,854 sweeps: rounds from the coastal forts
  and rounds crossing the island.

  **Uncertain:** `Coastal Gun 01` sits 66 m inside the hill (section 6 self-check). A segment
  that starts below the surface hits at its start in the stand-in, so its rounds and its line of
  fire end at once. Whether 00ADA240's quadtree walk reports a start below the surface was not
  read.
- **Outcome, with bands:**
  - deaths down or equal, 4..7;
  - hit records down or equal, 90..150;
  - shots 450..700, direction uncertain: refusals remove shots, but fewer kills keep targets alive
    longer;
  - torpedo drops 0 on both sides.

**Predictions, USN04 4700/4500.**
- `usn_19_coralus.scn` has no Landscape, so `entries=0`.
- The pick calls are counted with 0 land hits, and line of fire and shells show 0 land hits.
- Identity on every gameplay row: 41 deaths, 743 hit records, 5,603 shots. The census line is the
  only difference.

### The gunnery pairs, measured

- **Builds.** One tree (`agent/cc9-scene-entities` at `a7f606509`, which is main `fe93be072`
  plus this packet), built twice with only `kLandscapeSpatialAttachBound` flipped:
  - `local\sa_off`, SHA-256 prefix `DE033DD5025D`;
  - `local\sa_on`, SHA-256 prefix `4264BB75E479`.
- **Logs.** `local\sa2_{off,on}_{usn01,usn04}.log`. Each shows the 1600x900 override and its own
  module directory in this tree, and each exited 0.
- **The first pair, and the diagnostics.** The first pair (`local\sa_*`, before `a7f606509`) read
  0 land hits everywhere, so three diagnostics were added (`a7f606509`) and both pairs rerun:
  - the Landscape trace count past the broadphase;
  - the first three pick rays;
  - a vertical probe of the entry trace through every authored object at load.

**`tools/pair_diff.py`, USN01 3200/3000: exit 1, gameplay identical.**

```
GAMEPLAY: identical
  deaths                                 7                                        7
  hit records                            150                                      150
  hull hits                              85                                       85
  damage                                 2690.0                                   2690.0
  shots                                  583                                      583
  first hit                              53.75 s                                  53.75 s
  torpedo-task releases                  3 of 5                                   3 of 5
  dive-bomb-task releases                0 of 2                                   0 of 2
  torpedo drops                          0                                        0
  plane water contacts                   3                                        3
  controlled moved                       Airfield2 0.00                           Airfield2 0.00
  units                                  64                                       64
  mission end                            none (Mission.EndMission never true)     none (Mission.EndMission never true)
  host methods concrete/unimplemented    887 / 466                                887 / 466
DEATH ROWS: identical (7 rows)
PLANE DEATH MODES: identical (7 rows)
UNIT TABLE: identical (28 rows)
```

Native table: 1353 -> 1353 rows, 0 changed.

**The census, ON:**
`entries=4 pick=5999/0 seat=0/0 line_of_fire=144/0 blocked=0 projectile=18855/0 impacts_land=0`,
with `traces=6401`.

**Why every consumer reads 0.**
- **The machinery works.** The load probe hits all 51 authored objects on `Landscape 03`, and 50
  of the hits land within 5 cm of the ground height. The exception is `Coastal Gun 01`, 66 m inside
  the hill, which the probe meets at the hill's surface.
- **The broadphase passes.** 6,401 Landscape traces got past the box tests.
- **The pick ray is degenerate in this process.** All three logged rays run from (0,0,0) to
  (0,0,0): the HUD's pick has no camera (no camera at game+19FCh; `docs/HUD_PICK_SEGMENT_QUERY.md`),
  so its segment cannot meet land. This belongs to the HUD host's camera stand-in, not to this
  packet.
- **The seat never casts** on USN01 (idle player).
- **No other ray met land.** None of the 144 line-of-fire segments crosses an island below its
  surface, and no shell segment does either.

**`tools/pair_diff.py`, USN04 4700/4500: exit 1, gameplay identical.**

```
GAMEPLAY: identical
  deaths                                 43                                       43
  hit records                            788                                      788
  hull hits                              331                                      331
  damage                                 11917.1                                  11917.1
  shots                                  5075                                     5075
  first hit                              93.00 s                                  93.00 s
  torpedo-task releases                  4 of 16                                  4 of 16
  dive-bomb-task releases                1 of 19                                  1 of 19
  torpedo drops                          1                                        1
  plane water contacts                   16                                       16
  controlled moved                       Lexington-class01 3514.72                Lexington-class01 3514.72
  units                                  81                                       81
  mission end                            none (Mission.EndMission never true)     none (Mission.EndMission never true)
  host methods concrete/unimplemented    1042 / 549                               1042 / 549
DEATH ROWS: identical (43 rows)
PLANE DEATH MODES: identical (43 rows)
UNIT TABLE: identical (81 rows)
```

USN04 has no Landscape, so the entries are 0. The census counts pick 8999, seat 8997, line of fire
2242 and projectile 147931 calls, all with 0 land hits. The native table is identical.

**Failed predictions:**
- **Pick land hits.** I predicted 1,000..6,160 and measured 0: the pick ray is degenerate (above).
- **The USN04 base values** quoted before the runs (41 / 743 / 5,603) were an older main's. This
  main reads 43 / 788 / 5,075 on both sides. The identity prediction holds.

**Within the bands:** line of fire 0 blocked (0..40); shells 0 on land (0..150); USN01 deaths 7,
hit records 150 and shots 583 (bands 4..7, 90..150 and 450..700).

**Verdict: ON.** Identity on both missions. The entry is exercised: 6,401 traces, and the load
probe is exact. It takes effect on any mission where a shell, a line of fire or a camera ray
crosses an island.

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

### 7a. The two hunks bound (packet `cc9_ground_height_hunks`, `kGroundHeightHunksBound`, committed OFF)

2026-09-27, worker cc9-plane-release. Both hunks of section 7 are in `src/game_hosts_units.cpp`
under one switch, with the old bodies as the OFF arm. A census line,
`summary mission ground queries: torpedo approach segment probes 009D39D3=N blocked=M`, prints
in both builds. The counter landed first, in `c15c609b2`.

**Predictions** (written before the runs; pairs `local\gh_off` against `local\gh_on`, the
switch only, streams on, `tools/pair_diff.py`):

| row | USN04 4700/4500 | USN01 3200/3000 |
| --- | --- | --- |
| probes 009D39D3 | about 7,164 OFF, and within ± 20 % ON if the approach paths move | about 1,152, within ± 20 % |
| blocked, OFF | somewhere in 0..all. The water stand-in blocks whenever a ship's origin sits below the wave height, and the avoid-zone census has shown the stand-in answers 0.0, so it blocks when a hull origin is below 0 | same |
| blocked, ON | 0 at sea (ground −1000, no Landscape on the path) | the island crossings may block: 0..the probe count |
| torpedo aim tick | identical: ground + 5 = −995 never wins over the 5.0 sea floor, and 1 > −1000 arms as 1 > 0 did | identical at sea |
| torpedo sectors, releases, drops | identical if OFF blocks 0. Otherwise sector choice moves: torpedo releases ± 2, drops ± 1 | same, with island blocking added ON |
| deaths, hit records | identical if nothing moves; otherwise within ± 10 % and ± 15 % | same |

**7a measured** (`local\GH_OFF_USN01.log` / `GH_ON_USN01.log`, `local\GH_OFF_USN04.log` /
`GH_ON_USN04.log`, from `43ca22c1f`; `tools/pair_diff.py` says "DIFFERENT, gameplay identical"
for both):

| row | USN01 3000 OFF -> ON | USN04 4500 OFF -> ON | verdict |
| --- | --- | --- | --- |
| probes 009D39D3 | 1,152 -> 1,152 | 7,092 -> 7,092 | held |
| blocked | 0 -> 0 | 0 -> 0 | held for ON. OFF was the "0" end of the band: the water stand-in never blocked on these runs, so section 7's expectation that hunk 2 moves sector choice at sea does not arise here. The island crossings on USN01 do not block either: no Landscape stand-in lies on a probed segment |
| aim tick | `World::ground_height_00903860` concrete 1,567 / 3,369 | - | held (identity) |
| gameplay (pair_diff) | identical: 7 deaths, 150 hit records, 3 of 5 torpedo releases, 0 drops | identical: 43 deaths, 788 hit records, 4 of 16 releases, 1 drop | held |

**Verdict: `kGroundHeightHunksBound` ON.** The two torpedo sites ask the scene's world queries, as
the image does. On these missions the answer equals the old stand-ins.
