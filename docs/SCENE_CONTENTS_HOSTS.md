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

## 9. Handoff: cc9-scene-entities retires after this segment (2026-09-27)

Worker cc9-scene-entities, successor of cc9-gunnery2. Every switch below was measured by
same-tree pairs with `tools/pair_diff.py`.

**Landed ON:**

| switch | file | doc |
| --- | --- | --- |
| `kScenePathLandscapeCreatorsBound` | `include/bsp/game_hosts_scene_contents.hpp` | this doc, section 5 |
| `kSceneLandscapeTerrainBound` | same | section 6 (heightmaps, the four ground queries) |
| `kLandscapeSpatialAttachBound` | `src/game_hosts_gunnery.cpp` | section 8 |
| `kDeathRouteDestroyBound` | `include/bsp/game_hosts_gunnery.hpp` | `docs/CONSTRUCT_WORLD.md` section 22 |

**Committed OFF:** `kShipDirectorEnablesBound` (`include/bsp/game_hosts_gunnery.hpp`,
`docs/SENTITY_INIT_PASSES.md` section 7).
- It moves USN02 hard: 314 -> 0 ship torpedo launches, and the mission no longer fails at 39.65 s.
- The flip waits for one read: whether any image path launches a ship's torpedoes past the
  category-7 mask. The candidates are the command step 00836920's torpedo arm, the ship AI's
  attack orders and 009F1BC0.

**Open items, in the order I would take them:**
1. **The torpedo-mask read above.** Then flip `kShipDirectorEnablesBound`, or bind the bypass
   path.
2. **The units host's ground-height hunks** (section 7, cc9-plane-release):
   - the torpedo aim's `ground_height_00903860` should call the scene query, which answers -1000
     at sea;
   - the approach probe's `segment_blocked_00903bc0` should drop the water-surface stand-in.
   - Put a blocked-probe counter in first.
3. **The movie camera** (`docs/HUD_PICK_SEGMENT_QUERY.md` 7.1): `MovCamNew_AddPosition`'s
   keyframe mover. It is what the pick ray casts from on an airfield mission. Until it exists,
   USN01's pick ray is (0,0,0) -> (0,0,0).
4. **The quadtree segment walk** 00ADA240 -> 00AEA2B0 / 00AE9D80 / 00AECC40. Today it is a
   labelled half-cell march in `landscape_entry_segment_hit`. The open question is whether it
   reports a segment that starts below the surface.
5. **The rotation question.** The ground query samples a rotated island unrotated (section 6), and
   the renderer's use of the frame is unread.
6. **Reference d** (`docs/GAME_EXECUTABLE.md`, the protocol of 2026-09-26 c), when the lead
   confirms the in-flight landings.

**Tools kept in this worktree's `local\`** (not committed):
- `rot_probe.py` with `tdt.py`: a Python `.tdt` reader and the zone-point rotation probe;
- the `cc9-scene-entities_*.py` edit scripts;
- the `*_runs.ps1` pair runners.


## 10. Slot 3Ch reconstructed: the quadtree segment walk (packet `cc9_terrain_segment_quadtree`, `kTerrainSegmentQuadtreeBound`)

Worker cc9-init2, 2026-09-27, on main f424d4880. Ghidra was read only. The lead repaired
00AE9D80's boundary (00AE9D80..00AEA292, the seven-dword table 00AEA294..00AEA2AF outside it).

### 10.1 The walk, from the image

- **00ADA240, slot 3Ch** `(from, to, out)`, `RET 0Ch`.
  - Both world points go to local space through the node's inverse world matrix (00B6E0D0, then
    004142E0 at 00ADA262 and 00ADA283).
  - **Vertical case.** When |dx| < 0.001 and |dz| < 0.001 in local space, it calls 00AECC40 with
    ECX = the terrain, EDX = the world `from`, and the world `to` and `out` on the stack
    (00ADA2DC..00ADA2EE). 00AECC40 transforms both points again itself.
  - **Otherwise.** It subtracts the origin `+80h/+84h` from x and z, calls 00AEA2B0, then adds the
    origin back to the hit and transforms it through the node's world matrix `+F0h`.
- **The tree, built at the end of 00ADDA60.**
  - **Depth.** d = ceil(log2(max(tiles wide, tiles deep))), taking the max unsigned. 00AEA900
    stores the box (0, -1000, 0)..(2^d·300, 1e10 [+14h], 2^d·300) and `+38h` = 2^d.
  - **Nodes.** 00AEA820 pushes the root, and 00AEA5F0 appends four children per node, recursing
    while the half size is above 1. Each node is 14h bytes: min y, max y, first child (-1 for a
    leaf), tile x, tile z.
  - **Child order.** Children are appended as (x,z), (x,z+h), (x+h,z), (x+h,z+h). Their tile
    fields are `(k>>1)+x` and `(k&1)+z`.
  - **Height range, 00AE9C80.** A node's range is the min and max of its tiles' `+14h/+18h`,
    seeded [1000 (00CE3804), -1000]. A tile outside the grid, or one the file lacks, contributes
    [-1000, 1000].
  - **Tile range.** A tile's `+14h/+18h` come from the block's vt+0Ch 00AED020, the min and max of
    all 33×33 samples with holes as -1000.0. 00ADF8B0 stores them (00ADF90A, 00ADF915).
- **00AEA2B0, the ray** (a Revelles-style parametric walk in x and z).
  - The direction is `(q-p)/|q-p|` (00419440).
  - A negative x or z component is negated, the origin is mirrored about the box, and the flags
    get 4 (x) or 2 (z).
  - Each of dx and dz is then raised to at least 0.001 (00D7A23C).
  - It computes the entry and exit parameters against the box and returns false when they do not
    overlap.
  - Otherwise it calls 00AE9D80(0, tx0, tz0, tx1, tz1, &ray). The record holds the origin (+0h),
    the direction (+18h), the hit (+24h), the length (+30h) and the flags (+34h).
- **00AE9D80, one node** (`__thiscall`, `RET 18h`).
  - **Returns 0** when tx1 or tz1 is at most 0.
  - **Returns -1** when t_in = max(tx0, tz0, 0) is not below t_out = min(tx1, tz1, length); the
    ray has ended.
  - **Pruning.** It un-mirrors the ray and returns 0 when the ray's y range over [t_in, t_out]
    misses the node's [min, max].
  - **Leaf** (first child -1): 00AE9BD0 with the tile and the un-mirrored start and end points.
  - **Otherwise, the four children in ray order:**
    - the first child from tx0 < tz0 and the midpoints (0.5 double, 00D7A280);
    - children at `first + ((flags ^ k) >> 1)`, through the jump table's arms 0, 2, 4 and 6
      (00AEA0B0);
    - the next child from 00AE9A00, `(f1 <= f2) ? a : b`; 8 ends the walk.
- **00AE9BD0, the leaf.** The tile must be inside the grid (00ADAC30 `+38h`, 00ADAC40 `+3Ch`) and
  present. The sub-segment's squared length must be at least 1e-6 (00D7A2B8, 004193E0). Then it
  calls 00ADF1B0.
- **00ADF1B0, the tile.**
  - A 2-D DDA over the tile's 32×32 cells of 9.375, from floor(local/9.375) toward the end cell.
  - It steps z when `tz < tx` or the two are unordered (FCOMI, `JB` at 00ADF55D), else x. It stops
    when the stepped axis reaches its end cell.
  - For each cell with i, j < 32 it fetches four samples through the block's slot 8h 00ADC5F0
    (no node y). It calls 00ADEB80 with
    `(ECX = V(i,j), EDX = V(i+1,j), V(i+1,j+1), V(i,j+1), p, q, out)`, where
    `V(i,j) = (tile_x·300 + i·9.375, h(i,j), tile_z·300 + j·9.375)`. The ESP was traced by hand
    through 00ADF3E2..00ADF54A.
- **00ADEB80, the cell** (fastcall a, b; then c, d, p, q, out). It is a scalar-triple
  line-against-quad test in the form of Ericson's `IntersectLineQuad`, split on the b–d diagonal:
  - m = pb × pq (004F9B30 at 00ADEC6A) and v = m · pd;
  - **v < 0:** triangle a b d. It needs u = m·pa ≥ 0 and w = pq·(pd×pa) ≥ 0 (00ADEB40), and gives
    r = (u·d − v·a + w·b)/(u − v + w);
  - **otherwise:** triangle b c d. It needs u = −m·pc ≥ 0 and w = pq·(pc×pd) ≥ 0, and gives
    r = (u·d + v·c + w·b)/(u + v + w).
  - **It tests the LINE through p and q.** Nothing clips r to [p, q], so a hit just past either
    end of the segment, inside the first or last cell the DDA visits, counts.

### 10.2 The island rotation

- **The image's point queries ignore rotation.** Ground height 00903860, ground normal 009038F0
  and the landscape query 009039D0 subtract only the node's translation (section 6). The host
  matches this already, and nothing changes here.
- **The image's segment test uses the full frame.** 00ADA240 transforms through the inverse world
  matrix, rotation included.
  - The host's `landscape_entry_segment_hit`, which the pick, seat, line-of-fire and projectile
    traces use, already did.
  - The host's 00903BC0 sweep did not: it marched in world space against the unrotated height.
    Under the switch it asks each Landscape's slot 3Ch, as 00903C20..00903C42 does
    (vt+3Ch on the node's `+3D0h`).
- **So in the image a rotated island's segment test and its ground height disagree.** The segment
  sees the island where it is drawn. The height query sees the island un-rotated about its node.
- **Rotated Landscape rows in this installation's `.scn` files:** 18 rows in 9 scenes. The rest of
  the 763 rows in 223 scenes are identity.

| scene | Landscapes (yaw) |
| --- | --- |
| usn_1_marshall | 04 (-180, tilt 3e-4), 05 (91.5, tilt 0.02) |
| usn_13_truk | 05 (-180), 07 (-150), 11 (91.5) |
| yamato (chg) | 05 (-180), 06 (91.5), 07 (-150) |
| shogo_four | 02 (-150), 03..06 (-180) |
| bsm_01_stationed_at_pearl | 02 (-180) |
| empires_fall | 05 (134) |
| bulls_run | 01 (-150) |
| us_osumi | 01 (-56) |
| ijn_2_force | "Bruh" (134) |

  usn_2_java (USN02) and usn_19_coralus (USN04) author no Landscape at all.

### 10.3 The binding (`kTerrainSegmentQuadtreeBound`, committed OFF)

- **In `src/game_hosts_scene_contents.cpp`:**
  - the tree is built on the first query (`build_quadtree_00aea820`);
  - `ray_walk_00aea2b0`, `node_walk_00ae9d80`, `leaf_00ae9bd0`, `tile_walk_00adf1b0`,
    `line_quad_00adeb80` and `scalar_triple_00adeb40` implement the chain above;
  - `landscape_entry_segment_hit` calls the chain for a non-vertical segment.
  - `world_segment_blocked_00903bc0` asks every Landscape's slot 3Ch after its two endpoint tests.
- **LABELLED STAND-IN:** the vertical case 00AECC40 keeps the half-cell march. Its sub-walk
  00AECA60 and the terrain's vt+48h are unread. It is counted as `vertical`.
- **SUBSTITUTION, labelled:** the host's `fraction` is the hit's projection on from→to. 00ADA240
  answers a point only, and with the line test that projection can fall outside [0, 1].
- **Rounding.** Float stores are kept as floats and x87 register chains are evaluated in double.
  The result is not bit-verified.
- **Census.** The gunnery `landscape attach` line gains
  `slot3c bound walks=N/hits vertical=N/hits leaves cells`. The load self-check adds, under the
  switch, a slanted trace per object: `scene terrain slot 3Ch self-check ... slant_hits
  on_local_surface_25cm worst`. Its hits must lie on that Landscape's own local surface.

### 10.4 Predictions (written before the pairs; both variables set, lockstep 0.05, idle player)

- **USN01 3200/3000** (usn_1_marshall, four Landscapes, two of them rotated). Reference e reads
  `pick=5357/22 line_of_fire=141/0 projectile=17981/0 impacts_land=0`, and 00903BC0 is never
  called (`segment calls=0`).
  - **Self-check:** every Landscape's slanted traces hit. `on_local_surface_25cm` equals
    `slant_hits`, because a planar cell and the bilinear surface differ by centimetres over a 9.4 m
    cell.
  - **Pick land hits 22 -> 18..26.** The surface changes from bilinear to two triangles per cell,
    and the line test may add a hit just past a segment's end. The call counts stay 5357 / 141 /
    17981.
  - **Line-of-fire and projectile land hits stay 0**, and `impacts_land` stays 0.
  - `vertical` is small next to `walks`; the pick rays come from the camera at an angle.
  - The ground-height census rows do not move, since the point queries are unchanged.
  - No plane dies by terrain, because 00903BC0's consumers are unbound.
  - **Gameplay identical** (`pair_diff` exit 1): the pick result feeds no gameplay row, and the
    line-of-fire and projectile traces hit nothing either way.
- **USN04 4700/4500 and USN02 9200/9000.** No Landscape: the census reads `walks=0/0 vertical=0/0`.
  Only the new `bound` field differs, so `pair_diff` exits 1 with gameplay identical.

### 10.5 Pairs and verdict

- **The runs.** The OFF binary is `local\bin\qt_off` (a build of d35cf3545). The ON binary is
  `pair_export` of d35cf3545 with the switch flipped (SHA-256 5DCD50B222BC). Both variables were
  set, lockstep 0.05, idle player. Logs: `local\QT_{OFF,ON}_{USN01,USN04,USN02}.log` in worktree
  cc9-init2.

| row | USN01 OFF | USN01 ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| slot 3Ch self-check, Landscape 03 | - | 51 hits, 51 on the local surface, worst 0.071 m | all on the surface | held |
| self-check, Landscapes 04..06 | - | 0 hits | - | no authored object on them to trace |
| pick land hits | 5357/22 | 5357/22 | 18..26 | held (unchanged) |
| line-of-fire, projectile land hits | 141/0, 17981/0 | 141/0, 17981/0 | 0 | held |
| slot 3Ch walks / vertical | 0 / 0 | 5564/22, 3/0 | vertical small | held |
| leaves / cells tested | 0 / 0 | 33 / 1026 | - | - |
| ground-height census, `impacts_land` | unchanged | unchanged | unchanged | held |
| deaths, death rows, plane death modes, unit table | 7, 7, 7, 28 | identical | identical | held |
| `pair_diff` | | exit 1 | exit 1 | held |
| USN04 / USN02 `pair_diff` | | exit 1 / exit 1, only the `bound` fields | identity | held |

- **The pick rays.** 5564 slot 3Ch calls from 5357 pick traces over the four entries, and 22 land
  hits, the same count the march found. The cell test ran on 1026 cells in 33 leaves, so the
  y-pruning discards almost every node.
- **The rotated islands are not measured by these runs.** USN01's Landscapes 04 and 05 own no
  authored object for the self-check, and no pick ray that hit land is attributed per Landscape.
  00903BC0 has no bound consumer yet (`segment calls=0`), so its per-Landscape path is also
  unexercised.

**Verdict: ON.** Every prediction held, and no gameplay row moved on any of the three missions.

### 10.6 Open

- **00AECC40's sub-walk 00AECA60** (callees 00AEBD20, 00AEB770, 00AEADE0, 00AEB890, 00AEBA00,
  00AEBB90, 00ADAC30/40) and the terrain's vt+48h. The vertical case is still the march: 3 calls
  on USN01.
- **Bit-exact x87 rounding** of the walk: not verified.
- **A measuring mission for the rotation.** usn_13_truk, yamato or shogo_four have rotated
  Landscapes. A pick or line-of-fire hit on one of them would compare the host with the image
  there.

## 11. Handoff: cc9-init2 stops here (2026-09-27)

Worker cc9-init2 stops at about 65 % of its context, after `cc9_terrain_segment_quadtree` (ON at
b0f94edec).

### What this worker landed (all ON)

| switch | doc | what |
| --- | --- | --- |
| `kLoadTimeSquadronHooksBound` | SENTITY_INIT_ATTACH_ORDER.md 19 | scene squadrons call the pass A and pass C hooks at load |
| `kSceneHomeBaseQualifiedNameBound` | CONSTRUCT_WORLD.md 32 | 009251F0's case-insensitive last-segment HomeBase match |
| `kLoadWingSquadronIdBound` | SENTITY_INIT_ATTACH_ORDER.md 20 | load wing planes get `SquadronID` |
| `kTerrainSegmentQuadtreeBound` | this doc, 10 | slot 3Ch's quadtree walk, and 00903BC0 through every Landscape's slot 3Ch |

It also closed two items by reading and one by bisect:
- the SpawnNew bag `HomeBase` (CONSTRUCT_WORLD.md 33);
- the 00AE9D80 boundary extent, which the lead repaired at f424d4880;
- the USN02 552 -> 579 reference flag, the torpedo stock alone (GAME_EXECUTABLE.md reference e).

### Open items, in order

1. **The vertical case 00AECC40's sub-walk 00AECA60** (packet `cc9_terrain_vertical_subwalk`,
   read, not bound; the half-cell march still answers it, labelled in 10.3). USN01 takes this case
   3 times, with 0 hits. What was read:
   - **00AECC40.**
     - Returns 0 when both world points are above the terrain's `+14h` (1e10).
     - Scales both local points to tile units (origin off, times 1/`+18h`).
     - When they are float-equal, it answers vt+48h(x, z) and a hit only for a height strictly
       between the two y values. That hit is written in tile units, not transformed back.
     - Otherwise it calls 00AECA60 and transforms the result back.
   - **00AECA60** (ECX = terrain; EDX and the stack hold the two points in tile units, then out).
     - **Clip.** 00AEBD20 clips the pair to [0, `+38h`] × [0, `+3Ch`]: 00AEBA00 (lower) and
       00AEBB90 (upper) per axis 0 and 2. A clip moves the endpoint outside the bound along the
       segment, is skipped when |delta| < 1e-8 [00CF7FE8], and returns 0 when both endpoints are
       outside.
     - **Edge exits.** It returns 0 when both x equal the tile count exactly, or both z do (double
       compares).
     - **Walker set-up.** 00AEB770 fills a context {terrain, tiles wide, tiles deep, node world y
       `+124h`}. 00AEADE0 copies 54h bytes into the walker. The clipped pair goes to walker
       +54h..+68h. 00AEB890 sets the 2-D segment (+6Ch/+70h to +74h/+78h), y0/y1 (+7Ch/+80h) and
       the slope (y1 − y0)/sqrt(…) (+84h).
   - **00AEC7C0, the walk** (00AEC7C0..00AECA4B; its decompile leaves ECX-held locals
     uninitialised, so it needs the listing).
     - Returns 0 for a degenerate 2-D segment.
     - Calls 00AEB430. When y0 < -1000 (00CE6658), it answers (x0, -1000, z0) as a hit.
     - Otherwise it takes one of two arms:
       - when a span ≤ 0.001 [00CF3F30] or 00AEB680 is false: 00AEC660 on the whole segment;
       - otherwise: 00AEAB30, then a loop of 00AEB6D0 (next cell), 00AEC3F0 (the cell test) and
         00AEC700 (the last piece).
     - The hit comes from walker +48h..+50h.
   - **Bodies still to read** (all defined in Ghidra, the ends checked for the first six):
     00AEB430 (00AEB430..00AEB616), 00AEB680, 00AEC660, 00AEAB30, 00AEB6D0, 00AEC3F0
     (00AEC3F0..00AEC62E), 00AEC700, and the terrain's vt+48h.
   - **Predictions for the binding, carried over:** on USN01 the 3 vertical calls keep 0 hits, the
     pick land hits stay 22, and gameplay is identical. USN04 and USN02 stay identical, with no
     Landscape.
2. **A measuring mission for the island rotation.** usn_13_truk, yamato or shogo_four author
   rotated Landscapes (10.2). A pick, line-of-fire or, once bound, pilot trace against one of
   them would measure the image's segment/height disagreement in the host.
3. **00903BC0's consumers** (section 7's contracts) are still unbound, so its per-Landscape
   path runs 0 times on the reference missions.
4. **From SENTITY_INIT_ATTACH_ORDER.md 19.1:** the host issues authored commands before the load
   walk, and the image issues them after it, at 0046AAB0. This is labelled, and no measured row
   depends on it yet.

5. **Bit-exact x87 rounding** of the slot 3Ch walk (10.3) is not verified. x87 register chains
   are evaluated in double, and float stores are kept.
6. **cc9-units3's queue** (RELEASE_ISSUE_STAGE.md "Handoff: cc9-units3's queue") is closed apart
   from the items above:
   - the load-time hooks, the name match and the load wing ids are ON;
   - the SpawnNew bag is closed by a read;
   - the USN02 552 -> 579 flag is closed by bisect;
   - the quadtree walk is ON.

### State left by this worker

- Worktree cc9-init2 is clean after this commit. No lease is held after the report.
- Scripts are under `local\` with the prefix `cc9-init2-`:
  - `pe.py` reads dwords from the image on disk;
  - `landscapes.py` is the rotated-Landscape census;
  - `bis.ps1` is the first-parent bisect runner.

## 12. The consumers of 00903BC0 (packet `cc9_terrain_segment_consumers`, `kAvoidTerrainWaterProbeBound`)

Worker cc9-terrain2, 2026-09-27, on main 2b026d35b. Ghidra was read only.

### 12.1 The three image callers

`bsp.py callers 00903BC0` lists three, and the xrefs agree.

| caller | site | what it does with the answer | host today |
| --- | --- | --- | --- |
| 009D3420 torpedo approach | 009D39D3 | a blocked sector ray inside 400 m unmarks the sector (009D39E0) | **bound** (`kGroundHeightHunksBound`, ON): every probe runs the endpoint tests and then every Landscape's slot 3Ch |
| 0099F1C0 AvoidTerrain, branch A | 009A15C1, 009A172E | a live plane on the water (unit+900h == 6) probes +-30 degrees ahead; blocked, it turns and brakes (12.2) | **unbound**: the labelled early return |
| 007C3CB0 | 007C3E00 | a timer at unit+964h; when it runs out for a plane below 8 m ([00CE3918]) or on the water, it takes the translation of world+1ED4h's vt+114h matrix and tests the VERTICAL segment (x, 20 [00CE3930], z) to (x, 0, z). Clear: it re-arms the timer from 00BD2F10 and spawns the class's point effect +538h+218h at (x, 0, z). It reads as a water spray under a low plane, suppressed over land | **no host caller**: its only caller is 0060AC1A in 00609BD0, the player-plane GUI update; cosmetic, and an idle player flies no plane. Its segment would take slot 3Ch's vertical case (item 3 of section 11) |

**The handoff's premise was wrong.** It read `segment calls=0` from the `summary scene terrain`
line, but that line prints at load, before play (line 1098 of `QT_ON_USN01.log`). During play the
torpedo approach calls 00903BC0 1152 times on USN01 (`summary mission ground queries`). The endpoint
tests clear over the sea, so each probe asks all four Landscapes. That is 4608 of the 5564 slot 3Ch
walks in the gunnery `landscape attach` line; the other 956 are picks. None hit. The new mission-end
line `summary mission world segment 00903BC0` counts this directly (`sweep_entries`).

### 12.2 Branch A, from the image (009A1420..009A17A1)

Branch A sits inside 0099F1C0 (body 0099F1C0..009A17CB). 0099F1E7 jumps to it when
(unit+72Ch)->vt+38h is false, and 009A142D leaves unless unit+900h == 6. The caller 009A17D0
already admits only mode 7 or mode 6, and only a live AI plane (+5Dh clear).

- **Length.** L = 00419010(0, T+2F4h TakeOffMaxLength 250, m, T+2F8h TakeOffMinLength 140, speed).
  - m = 007C4810 on pilot+2F8h (the class): T+28Ch * cls+184h, the minimum control speed.
  - speed = unit->vt+38h.
  - 007C4810 ends in a plain `RET`, so the two floats pushed for it at 009A1454..009A1461 stay on
    the stack as 00419010's fourth and fifth arguments.
- **First probe.** p = (unit+FCh, 0.1 [00D7A2F0], unit+104h).
  - pilot+3F0h picks the side and flips each tick: clear gives -30 degrees [00CEC728] and sets it;
    set gives +30 degrees [00CEC724] and clears it.
  - q = p + L * 007BA2E0(00438AA0(vt+50h heading, side)), with q.y = 0.1. 007BA2E0 is
    (cos a, 0, sin a) for a = pi/2 - h, plus 2 pi when negative.
  - Clear: return (009A15C8). No band and no throttle write.
- **Blocked.** pilot+3F0h is flipped back (009A15DE). Then a fan of up to 22 probes, each of
  length L at y 0.1 (the y offset is L * 0.0 [00D7A258]):
  - offsets idx/12 * pi from the heading, idx = 3, -3, 4, -4, ..., 12, -12, 13 (009A160D..009A173D);
  - the first clear probe with idx > 0 inserts (-1.1, 0.9) into pilot+4h through 0099B790; a clear
    negative idx, or a fan with no clear probe, inserts (-0.9, 1.1). 0099B790 widens -1.1 to -5
    and 1.1 to 5.
  - pilot+25Ch = 00419010(1, 1, 8 [00CE3918], -1 [00D7A260], 007D99C0 forward speed): full
    reverse throttle at 8 m/s and above.

### 12.3 The binding (`kAvoidTerrainWaterProbeBound`, committed OFF)

- `terrain_avoidance_water_009a1420` in `src/game_hosts_units.cpp` implements 12.2 and asks
  `world_segment_blocked_00903bc0`.
- **SUBSTITUTIONS, labelled:** unit->vt+38h is the live velocity length, as in the free-flight path;
  vt+50h is the host's `plane_heading_c6c`; the position is the host's world row, without the
  00414DB0 refresh.
- **Census.** A new mission-end line in both builds:
  `summary mission world segment 00903BC0 calls endpoint_blocks sweep_entries sweep_blocks | water
  probe bound probes blocked fan_pos fan_neg fan_exhausted`.

### 12.4 Predictions (written before the pairs; both variables set, lockstep 0.05, idle player)

Branch A needs a live AI plane in mode 6. On every reference log of section 10.5 the
`terrain avoidance` line reads `water_ticks=0`. The three USN01 water contacts are Mavis flying
boats that were already dead (`dead=1`), which the caller 009A17F8 rejects.

- **USN01 3200/3000.**
  - `water probe probes=0` on both sides, so no plane pulls up, turns or crashes through it.
  - `calls` equals the torpedo approach's `segment probes 009D39D3` on the line above, with
    `endpoint_blocks=0`, `sweep_entries` = 4 x calls and `sweep_blocks=0`, on both sides.
  - Deaths 7, hit records 150, shots 561 and pick land hits 22 do not move.
  - `pair_diff` exit 1: only the `bound` field differs.
- **USN04 4700/4500 and USN02 9200/9000** have no Landscape: `sweep_entries=0`, `probes=0`,
  `pair_diff` exit 1.

### 12.5 Pairs and verdict

- **The runs.** OFF is `local\bin\wp_off`, a build of 3f15c9c8b. ON is `pair_export` of 3f15c9c8b
  with the switch flipped (SHA-256 E263CD8C502F). Both variables were set, lockstep 0.05, idle
  player. Logs: `local\WP_{OFF,ON}_{USN01,USN04,USN02}.log` in worktree cc9-terrain2.

| row | USN01 OFF | USN01 ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| water probe probes / blocked | 0 / 0 | 0 / 0 | 0 | held |
| 00903BC0 calls, endpoint blocks | 1152, 0 | 1152, 0 | calls = torpedo probes (1152), 0 | held |
| sweep entries / blocks | 4608 / 0 | 4608 / 0 | 4 x calls / 0 | held |
| deaths, hit records, shots | 7, 150, 561 | 7, 150, 561 | unchanged | held |
| pick land hits | 5357/22 | 5357/22 | unchanged | held |
| death rows, plane death modes, unit table | 7, 7, 28 | identical | identical | held |
| `pair_diff` | | exit 1, only `bound` 0 -> 1 | exit 1 | held |
| USN04: calls / entries / probes | 7128 / 0 / 0 | 7128 / 0 / 0 | entries 0, probes 0 | held |
| USN04 deaths, hits, shots; `pair_diff` | 44, 789, 6321 | identical; exit 1 | exit 1 | held |
| USN02: calls / entries / probes | 0 / 0 / 0 | 0 / 0 / 0 | entries 0, probes 0 | held |
| USN02 deaths, hits, shots; `pair_diff` | 21, 652, 1095 | identical; exit 1 | exit 1 | held |

**Verdict: ON.** Every prediction held. No plane pulls up, turns or crashes through 00903BC0 on the
reference missions: the only bound consumer that runs is the torpedo approach, and none of its
probes is blocked. Branch A is bound but unexercised; a mission with a live flying boat on the
water near land would be its first measurement.

### 12.6 Open

- **00609BD0's Ghidra body is truncated** at 0060ABCC. The code runs on to the `RET 4` at
  0060C5A9 (INT3 from 0060C5AC), and it holds 007C3CB0's only call (0060AC1A). The lead redefines it.
- **007C3CB0** stays unbound: its caller, the player-plane GUI update, has no host counterpart.

## 13. The rotated islands measured (packet `cc9_terrain_rotation_measure`, `kLandscapeScaledTransposeInverseBound`)

Worker cc9-terrain2, 2026-09-27, on main 50e601f97. Ghidra was read only.

**Correction to 12.1 and 12.6.** 00609BD0 does end where Ghidra first said: `RET 0Ch` at 0060ABCA,
then INT3 at 0060ABCD..0060ABCF. The call to 007C3CB0 at 0060AC1A belongs to a separate function
at 0060ABD0..0060C5AB, inclusive:
- it opens with an SEH prologue (handler 00C77557, `sub esp, 1B8h`) and ends in `RET 4` at 0060C5A9;
- it has no direct caller; its one reference is the vtable slot 00CF43CC;
- it calls 00609BD0 at 0060C35F.

So 007C3CB0's caller is that virtual method, not the player-plane GUI update, and its class is
unidentified. The earlier range repair of 00609BD0 should be undone.

### 13.1 The smoke run, USN13 (usn_13_truk) 3200/3000

The run used both variables, lockstep 0.05 and an idle player; the log is `local\ROT2_USN13.log`
in worktree cc9-terrain2.

- **The four checks.**
  - The milestone line reads `menu_select=USN13 frames=3200 mission_frames=3000`.
  - The module directory is `local\bin\rot\`.
  - The final `native renderer final COM release: device=0 api=0` is printed.
  - There is no crash record.
- **Totals.** 34 deaths, 204 hit records, 2551 shots; 12 Landscapes and 2151 blocks loaded.
- **The ground queries in play.**
  - 00903BC0 made 13968 calls, with 167616 Landscape entries asked and 0 blocks.
  - The gunnery line has pick 4809/0, seat 4647/0, line-of-fire 547/0 and projectile 74493/0,
    with 181483 slot 3Ch walks and 0 hits.
  - No trace in play touches land on USN13.
- **Identity.** A second run of the same binary (`local\ROT_USN13.log`, before the `own_*` fields
  were added) matches on gameplay, so the mission is stable.

### 13.2 The census: segment test against ground height on each island

The load self-check gains one line per Landscape: `scene terrain rotation census`.
- **The grid.** 48 x 48 points across the drawn footprint: local (origin + [0, tiles·300]),
  taken to the world through the Landscape's frame.
- **The two answers at each point.**
  - The segment test: this Landscape's slot 3Ch (full inverse frame), on a trace from y 3000 to
    -500 with a 0.5 m run in x, so it takes the quadtree.
  - The ground height: 00903860, the maximum over all Landscapes, translation only.
  - `own_*` compares against this Landscape's own 00ADA900, translation only, which removes the
    neighbouring islands.
- **Land** means a surface above y 0.

| Landscape | frame | segment land | own height land | both | segment only | height only | mean / worst dy on both (m) |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 01, 02, 03, 04, 06, 08, 09, 10, Shipyard | identity | 26..163 | same +-2 | all but <= 2 | <= 2 | <= 2 | 0.8..3.0 / 3.7..37.4 |
| 05 | yaw 180 (tilt 3e-4) | 92 | 88 | 18 | 74 | 70 | 45.1 / 136.6 |
| 07 | sheared (13.3) | 90 | 231 | 3 | 87 | 228 | 36.7 / 96.3 |
| 11 | yaw 91.5 (tilt 0.02) | 87 | 89 | 4 | 83 | 85 | 68.6 / 118.7 |

- **The rotated islands.** The image's two answers overlap on 18, 3 and 4 points out of about 90.
  - The segment test finds each island where it is drawn.
  - The height query finds it un-rotated about its node, so at most points one answer says land
    and the other says sea.
  - Where both say land, the heights differ by 37 to 69 m on average.
  - The host reproduces this faithfully; it is the image's behaviour (10.2), not a host gap.
- **The identity islands** agree to within 2 points, which is the control. A few steep cells
  differ by up to 37 m because the trace runs 0.5 m in x.
- **Neighbours.** The world query's extra `height_only` points on 04, 08 and Shipyard (122, 158,
  316) are neighbouring islands inside the footprint grid. The `own_*` columns remove them.

### 13.3 A bindable gap: the image's inverse is a scaled transpose

- **The inverse.** 00ADA240 takes its points to local space through 00B6E0D0. 00B6E0D0's inverse is
  00B63B30 (called at 00B6E0F0), `BSP_Matrix_InvertOrthogonalScaledAffine`: each row divided by
  its squared length, transposed, with no shear fallback. The host's `frame_inverse_point` is a
  general 3x3 inverse. The two agree only when the rows are mutually orthogonal.
- **The frames that are not orthonormal.** 5 of the 763 authored Landscape `localframe` rows in
  this installation's scenes:
  - usn_13_truk 07, yamato 07, shogo_four 02 and bulls_run 01: the "-150" rows of 10.2. Each is
    `-0.866 0 0.5 / 0 1 0 / -0.5 0 0`: the third row has lost its z (-0.866), so its length is
    0.5, it is 0.433 off orthogonal and the determinant is 0.25.
  - us_osumi 01: unit rows, 0.829 off orthogonal.
- **What the image does with them.** For usn_13_truk 07 the scaled transpose maps a drawn local
  point (lx, lz) to (lx + 0.433 lz, 1.732 lx + lz). So the image's segment test sees a sheared
  island that is not where the island is drawn. The host's general inverse sees the drawn one.
- **The binding.** `kLandscapeScaledTransposeInverseBound` (committed OFF) makes
  `landscape_entry_segment_hit` use 00B63B30 and 004142E0 through the host's existing
  `invert_camera_affine_00b63b30` and `transform_point_004142e0`. The forward transform of the
  hit (+F0h) is unchanged.
- **ASSUMPTION, labelled:** the node's world matrix is the authored `localframe` as the host
  composes it. Whether the loader re-orthonormalises it is unread.

### 13.4 Predictions (written before the pairs)

- **USN13 3200/3000.**
  - Landscape 07's census row moves: `segment_land` falls from 90 into 0..60, and `both` stays
    below 20.
  - The other eleven rows are identical. For 05 and 11 the scaled transpose equals the inverse to
    about 1e-7.
  - 00903BC0's 13968 calls stay at 0 blocks, and picks, line-of-fire and projectile stay at 0
    land hits.
  - Gameplay identical; `pair_diff` exit 1 (the census line).
- **USN01 3200/3000.** No frame is sheared, so everything is identical, including picks 5357/22
  and walks 5564/22. `pair_diff` exit 0.
- **USN04 4700/4500 and USN02 9200/9000** have no Landscape: `pair_diff` exit 0.

### 13.5 Pairs and verdict

- **The runs.** OFF is `local\bin\inv_off`, a build of 49f44fcff. ON is `pair_export` of 49f44fcff
  with the switch flipped (SHA-256 06F975244F58). Both variables were set, lockstep 0.05, idle
  player. Logs: `local\INV_{OFF,ON}_{USN13,USN01,USN04,USN02}.log` in worktree cc9-terrain2.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN13 Landscape 07 segment land / both | 90 / 3 | 54 / 8 | 0..60 / < 20 | held |
| USN13 Landscape 07 mean / worst dy on both | 36.7 / 96.3 m | 68.5 / 161.9 m | - | - |
| USN13 Landscape 11 counts | 87 / 89 / 4 | identical | identical | held |
| USN13 Landscape 11 mean / worst dy | 68.588 / 118.746 | 68.601 / 118.732 | identical | **failed** (cm level) |
| USN13 other ten census rows | | identical | identical | held |
| USN13 00903BC0 calls / blocks; land hits in play | 13968 / 0; all 0 | identical | identical | held |
| USN13 deaths, hits, shots | 34, 204, 2551 | identical | identical | held |
| USN13 `pair_diff` | | exit 1 (the two census lines) | exit 1 | held |
| USN01 Landscape 05 (91.5) mean / worst dy | | moved at the cm level, counts identical | identical | **failed** (cm level) |
| USN01 picks 5357/22, walks, deaths 7, hits 150 | | identical | identical | held |
| USN01 `pair_diff` | | exit 1 (that one census line) | exit 0 | **failed** |
| USN04, USN02 `pair_diff` | | exit 0 | exit 0 | held |

- **The failed prediction.** I expected the scaled transpose to equal the inverse to about 1e-7
  for the 91.5-degree frames. Those frames are authored to four decimals (`0.9995`, `0.0195`...),
  so their rows are off unit length and off orthogonal by about 1e-4. The two inverses therefore
  differ by about 1e-4 of the distance from the node. That moves the hit heights of the census's
  3000 m traces by centimetres, and no count.
- **Landscape 07** now answers the image's sheared island: 54 land points on the drawn footprint
  instead of 90, and 8 of them agree with the height query instead of 3.

**Verdict: ON.** Gameplay, death rows and unit tables are identical on all four missions. The
only moved lines are the switch's own census rows. Nothing in play touches land on these missions,
so no gameplay row measures the change yet.

### 13.6 Open

- Whether the scene loader re-orthonormalises `localframe` before it becomes the node's world
  matrix (the 13.3 assumption).
- The vertical case 00AECC40 transforms again through the same inverse. The host's stand-in
  already takes the local points from the switched inverse.

## 14. The vertical case read and bound (packet `cc9_terrain_vertical_subwalk`, `kTerrainVerticalSubwalkBound`)

Worker cc9-terrain2, 2026-09-27, on main 15e669ebd. Ghidra was read only. Every body below is
a defined Ghidra function whose listing ends where Ghidra says (RET, then INT3), so there is no
`no_ghidra_function` body.

### 14.1 00AECC40 (fastcall ECX terrain, EDX world from; world to, out; `RET 8`)

- **World y test.** It returns 0 when both world y values are above terrain `+14h` (1e10). That
  cannot happen.
- **Tile units.** Both points go to local space through the node inverse (00B6E0D0, then
  004142E0), the same transform 00ADA240 applies. Then x and z are taken to tile units:
  f32(f32(1/`+18h`) * f32(local - origin)), with `+18h` = 300. y stays local.
- **Equal points** (float-equal x and z in tile units): it calls vt+48h, which is 00ADB480, with
  the TILE coordinates. 00ADB480 expects cell units (00ADA900 passes (x - node - origin) / 9.375).
  So the height comes from the cell at index = tile coordinate, near the grid origin.
  - When lo < h < hi (the two local y values), it writes (x tile, h, z tile) to `out` in tile
    units, with no transform back (00AECE22..00AECE3F), and returns 1.
  - Otherwise it falls through to the walk. This behaviour is the image's own and is kept.
- **The walk.** Otherwise it calls 00AECA60(from, to, out) in tile units. On return it maps x and z
  back with `x*300 + origin`, refreshes the node and transforms through node `+F0h`
  (00AECE66..00AECEB8).

### 14.2 00AECA60 (ECX terrain, EDX from; to, out; `RET 8`)

- **Clip.** 00AEBD20 clips the segment to [0, `+38h`] x [0, `+3Ch`]; a clipped-out segment
  returns 0.
  - 00AEBA00 is the lower bound and 00AEBB90 the upper; each is __thiscall(segment; value, axis,
    an unused dword), `RET 0Ch`, all SSE float.
  - A segment wholly outside a bound returns 0, and one wholly inside returns 1.
  - Otherwise it moves the outside endpoint along the segment onto the bound, unless |delta| < 1e-8
    [00CF7FE8].
- **Edge exits.** It returns 0 when both x equal the tile count, or both z do (double compares).
- **Walker set-up.** 00AEB770 fills {terrain, `+38h`, `+3Ch`, node `+124h` world y}. 00AEADE0
  copies 54h bytes of which only those 10h are initialised. The pair then goes to walker
  +54h..+68h, and 00AEB890 sets the 2-D segment (+6Ch..+78h), y0 and y1 (+7Ch, +80h) and the slope
  f32(double(y1 - y0) / f32 2-D length) at +84h.
- **Then** 00AEC7C0.

### 14.3 The grid iterator (shared by both levels)

- **00AEB430** (fastcall ECX iterator, `RET`) sets up the per-axis crossing parameters:
  - the 2-D delta, the length (sqrt of the float sum of float squares) and the unit direction
    through 00419260;
  - per axis with |dir| > 0.001: tDelta = f32(sqrt(f32((dz/dx)² + 1))) and the first crossing
    (1 - fmod(x0, 1)) · tDelta, or fmod(x0, 1) · tDelta when dir < 0;
  - otherwise both are 2 · length.
- **00AEB680** is false when the length ends before the first crossing.
- **00AEAB30** snaps a start that sits within 0.001 of a line. It sets trunc(p + 0.5) with 00BF7420
  truncation, bumps that axis's count and flag (+74h), and advances its crossing.
- **00AEB6D0** picks the step:
  - a corner, 00AEB0B0, when |tMaxX - tMaxZ| < 0.001;
  - x, 00AEB1F0, when tMaxX < tMaxZ;
  - otherwise z, 00AEB310.
- **Each step** records:
  - the new point, snapped on the crossed line (trunc(p + 0.5));
  - the cell left behind (+58h, +5Ch): the smaller of the old and new line index, and floor of
    the other coordinate;
  - the crossed edge's two corners (+60h..+6Ch), with k and k+1, or k twice when the coordinate is
    integral;
  - the fraction along it (+70h, fmod; a corner step leaves it as it was);
  - t at the crossing (+34h);
  - the next crossing, tDelta · count + first.

### 14.4 The two walks

- **00AEC7C0, tiles** (thiscall walker; out; `RET 4`).
  - Returns 0 for equal 2-D ends.
  - When y0 < -1000 it answers (x0, -1000, z0).
  - A length of 0.001 or less, or no crossing, gives 00AEC660: the whole segment in the tile of its
    minimum corner.
  - Otherwise 00AEAB30, then per crossing: the tile left behind, y = slope · t + y0 and the new
    point, then 00AEC3F0.
  - The last piece is 00AEC700, which returns 0 for a tile outside the grid.
- **00AEC3F0, one tile** (fastcall walker; `RET`).
  - It takes the tile record at `+40h[+38h·j + i]`, unchecked in the image. A null record is a miss.
  - It is a miss unless tile max `+18h` + node y > min(y at the two ends).
  - It rescales the piece to cells: (p - tile) · 32, with [00D5D658] = 32 in `.rdata`.
  - It clips to [0, 32]², **ignoring the result** (00AEC544).
  - It builds the cell walker: {tile record, node y}, copied by 00AEAE70; the piece; 00AEB7E0
    slope. Then it runs 00AEC120 and maps the hit back to tile units (x/32 + i, z/32 + j).
- **00AEC120, cells** (thiscall walker; out; `RET 4`). It has the same structure. Two differences:
  - It answers the start itself when the bilinear ground there (00AEBDE0) is above y0.
  - Each crossing ends on a cell edge. 00AEC090 takes the ground there from the edge's two samples
    lerped by +70h (00AEAFA0), or from the corner sample.
  - Its whole and last pieces use 00AEBFF0 and 00AEC050 with the bilinear ground.
- **The crossing, 00AEB940 then 00AEAAA0** (ECX = &t; h0, y0, h1, y1; `RET 10h`).
  - False when both ends are above ground.
  - t = 0 when the start is below.
  - Else t = (y0 - h0) / (f32(h1 - h0) - f32(y1 - y0)), with no zero guard.
  - The hit is 005803E0's lerp of the GROUND points (x0, h0, z0) to (x1, h1, z1) by t.
- **00AEBDE0**: the bilinear ground at a cell point. Four samples go through the tile record's
  +1Ch sampler vt+8h (00ADC5F0) plus the node y, with the +1 index held at 32.

### 14.5 The binding (`kTerrainVerticalSubwalkBound`, committed OFF)

- `vsub_vertical_00aecc40` and the bodies above are in `src/game_hosts_scene_contents.cpp`, and
  `landscape_entry_segment_hit` calls them for the vertical case.
  - They reuse the host's `quad_tile`, `tile_range_00aed020`, `block_sample_00adc5f0`,
    `cell_height_00adb3a0` and `main_menu_map_lerp_005803e0`.
  - They reuse the local points already taken through the switched inverse (13.3).
- **LABELLED:**
  - a tile index outside the grid reads as missing, where the image reads the table unchecked;
  - 00419260's zero-length answer is taken as 0;
  - x87 chains are evaluated in double and not bit-verified;
  - `fraction` is the world-space projection of the answer.
- **Census.** The gunnery `landscape attach` line gains
  `vsub bound equal walks tiles cells`.

### 14.6 Predictions (written before the pairs)

- **Exactly vertical traces now almost never hit.** The load self-check's straight-down probe
  through each authored object is float-equal in x and z, so it takes the equal-point test at the
  wrong cell. When that fails, the walk sees a zero-length 2-D segment and returns 0.
- **USN01 3200/3000.**
  - Load summary `vertical=51/51` becomes `vertical=51/0..5`, and Landscape 03's
    `segment_probe_hits` falls from 51 to 0..5. `equal` = 51 at load.
  - The slanted slot 3Ch self-check (51 hits) and the rotation census do not move: neither is
    vertical.
  - In play: `vertical=3/0` stays 3 calls and 0 hits; picks stay 5357/22.
  - Gameplay identical; `pair_diff` exit 1.
- **USN13 3200/3000.** In play `vertical=4/0` stays 4/0. The load probe hits (204 objects over 12
  Landscapes) fall the same way; the rotation census does not move; gameplay identical.
- **USN04 4700/4500 and USN02 9200/9000.** No Landscape: exit 1, only the `vsub bound` field.

### 14.7 Pairs and verdict

- **The runs.** OFF is `local\bin\vs_off`, a build of bc4d68859. ON is `pair_export` of bc4d68859
  with the switch flipped (SHA-256 7300491D9226). Both variables were set, lockstep 0.05, idle
  player. Logs: `local\VS_{OFF,ON}_{USN13,USN01,USN04,USN02}.log` in worktree cc9-terrain2.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN01 load `vertical` hits | 51/51 | 51/0 (equal 51, walks 51) | 51/0..5 | held |
| USN01 Landscape 03 `segment_probe_hits` | 51 | 0 | 0..5 | held |
| USN01 in play `vertical` | 3/0 | 3/0 (equal 3, walks 3) | 3/0 | held |
| USN01 picks, slanted self-check, rotation census | 5357/22, 51, - | identical | identical | held |
| USN01 gameplay, death rows, unit table; `pair_diff` | 7 deaths, 150 hits | identical; exit 1 | exit 1 | held |
| USN13 load `vertical` hits; probe hits on 03/06/08/Shipyard | 204/204; 50/116/3/35 | 204/0; all 0 | fall to 0..few | held |
| USN13 in play `vertical` | 4/0 | 4/0 | 4/0 | held |
| USN13 gameplay (34 deaths, 204 hits), native table; `pair_diff` | | identical; exit 1 | exit 1 | held |
| USN04, USN02 `pair_diff` | | exit 1, only `vsub bound` | exit 1 | held |

- **The walk measured.** Every vertical trace in these runs has float-equal ends in tile units, so
  none reached a tile (`tiles=0`). A near-vertical load check was added under the switch: 0.0009 m
  of run in x, inside the 0.001 vertical test but no longer equal in tile units. An ON build of the
  flip commit ran USN01 and USN13 (`local\VS_ON2_*.log`):

  | Landscape | near-vertical hits | within 25 cm of the ground | worst |
  | --- | --- | --- | --- |
  | USN01 03 | 51 of 51 | 51 | 0.000 m |
  | USN13 03 / 06 / 08 | 50 / 116 / 3 | all | 0.001 m |
  | USN13 Shipyard | 35 of 35 | 32 | 37.6 m (the same objects are off the ground by authoring, `worst=36.6`) |

  Each of those walks tests one tile (`tiles` = 51 and 204). A vertical-case segment is shorter
  than 0.001 / 300 tile units, so the tile and cell stepping loops (00AEB6D0 and its three steps)
  are reachable only in principle. They are reconstructed but unexercised.
- **Noise seen.** The two ON runs of USN13 (the flip-commit build against the pair's ON export)
  differ in `ShipAiClearance::category_enabled_009ec770` (6999 -> 7026) and
  `avoidance_enabled_0080e160` (1108 -> 1109) calls. They came with the known noise rows
  `static_zone_blocks`/`clearance` and `sector_scan` (absent -> present), and gameplay is identical.
  The only difference between the two builds is a load-time diagnostic, so these two rows are
  recorded as candidates for the noise list, not attributed.

**Verdict: ON.** Every prediction held, gameplay is identical on all four missions, and the walk
lands on the ground where it runs.

**What it means for callers.** An exactly vertical slot 3Ch trace misses in the image unless the
tile-unit vt+48h sample happens to fall between its y values. 007C3CB0's spray test (12.1) is such
a trace, so in the image its land suppression would almost never fire.

## 15. Handoff: cc9-terrain2 stops here (2026-09-27)

Worker cc9-terrain2 stops after four packets. Items 1 to 4 of section 11 are closed.

### What this worker landed (all ON)

| switch | doc | what |
| --- | --- | --- |
| `kAvoidTerrainWaterProbeBound` | this doc, 12 | 0099F1C0's branch A (009A1420..009A17A1): a live plane on the water probes +-30 degrees through 00903BC0, then a 15-degree fan, a band and the throttle |
| `kLandscapeScaledTransposeInverseBound` | this doc, 13 | slot 3Ch's inverse is 00B63B30's scaled transpose; it differs on the 5 sheared or skewed authored Landscape frames |
| `kTerrainVerticalSubwalkBound` | this doc, 14 | the vertical case 00AECC40 and its sub-walk 00AECA60 (tile and cell grid walks), in place of the half-cell march |
| `kAuthoredCommandsAfterLoadWalkBound` | SENTITY_INIT_ATTACH_ORDER.md 23 | authored commands are issued after the load walk (0046ED0F, then 0046ED1E) |

Diagnostics added, printed in both builds unless noted:
- `summary mission world segment 00903BC0`: every call in play, and branch A's probes.
- `scene terrain rotation census`: segment test against ground height over each island.
- `scene terrain vertical self-check`: ON only, a near-vertical trace per authored object.
- The `vsub` fields on the gunnery `landscape attach` line.

### Open items, in order

1. **Two noise-list candidates for `tools/pair_diff.py`.**
   - The rows are `ShipAiClearance::category_enabled_009ec770` (6999 / 7026) and
     `avoidance_enabled_0080e160` (1108 / 1109).
   - They differ between two USN13 3200/3000 runs of ONE binary: `local\bin\vs_on2`, SHA-256
     B544BC1BA071, logs `local\VS_ON2_USN13.log` and `local\VS_ON3_USN13.log`.
   - Both times they moved together with the listed `static_zone_*` and `sector_scan` rows
     (absent / present). Gameplay was identical.
2. **Does the scene loader re-orthonormalise `localframe`?** Section 13.3 assumes the node's world
   matrix is the authored frame as the host composes it.
   - If the loader normalises the frame, the 5 non-orthonormal frames (usn_13_truk 07, yamato 07,
     shogo_four 02, bulls_run 01, us_osumi 01) are not sheared in the image, and the
     scaled-transpose binding changes nothing on them.
   - Where to read: the path from the `localframe` record to node +F0h.
3. **007C3CB0's caller.** It is the virtual 0060ABD0..0060C5AB, vtable slot 00CF43CC (neighbours
   00605CC0, 00606230, 006067E0). Its class is unidentified. The statement "player-plane GUI path"
   in 12.1 is provisional. 007C3CB0 stays unbound, and its vertical segment would miss in the image
   (14.7).
4. **Branch A has never run.** No reference mission has a live plane in mode 6. A mission with a
   flying boat taxiing near land would give its first measurement.
5. **Unexercised and unverified.**
   - The tile and cell stepping loops (00AEB6D0 and its steps 00AEB0B0, 00AEB1F0, 00AEB310) cannot
     be reached from the vertical case.
   - The x87 rounding of the walks (10.3, 14.5) is not bit-verified.
   - 00419260's zero-length answer is taken as 0.
6. **Carried from section 11:** item 5, the bit-exact rounding of the slot 3Ch walk, and the rest
   of its list after item 4.

### State left by this worker

- Worktree cc9-terrain2 is clean after this commit. No lease is held after the report.
- Scripts are under `local\` with the prefix `cc9-terrain2-`:
  - `pe.py` reads floats and doubles from the image on disk;
  - `pairs.ps1` runs USN13/USN01/USN04/USN02 pairs, and `pairs2.ps1` runs JM08/USN01/USN04/USN02;
  - the `edit*.py` files are the applied edits.

## 16. The entity reader normalises every `localframe` (read, 2026-09-27)

Worker cc9-terrain2, on main 26fa4ca42. Ghidra was read only. This closes item 2 of section 15.

- **Where.** `BSP_SceneFile_ReadEntityBlock` (0046CF40..0046D927) reads the 16 floats into its
  frame (0046D150..0046D166, 008D9B40 per float). Before anything uses them it rebuilds the basis:
  - s = sqrt(row0 · row0), stored float (0046D168..0046D1A5); s = 0 when the square is not above
    1e-10 [00CE3820];
  - `BSP_Matrix_OrthonormalizeBasisRows` 0085DC80 on the frame (0046D1BA). Row 2 is the authority
    and is renormalised; row 1 is made orthogonal to it; row 0 = row1 x row2 (the host's
    `orthonormalize_basis_rows_0085dc80`, `src/plane_pose_commit.cpp`);
  - rows 0..2 (elements 0-2, 4-6, 8-10) are multiplied by s (0046D1BF..0046D222). The translation
    row is untouched.
- **Then** the normalised frame is what 0046D592 passes to the class creator. PlaceInWorld 00928860
  hands it to 009258F0, which copies it verbatim into the node's +74h (004134F0). The terrain node's
  frame comes from that (00ADE820).
- **This applies to every entity the scene file reads,** not only Landscapes.

### What it does to the five frames of 13.3

| frame | authored | in the image |
| --- | --- | --- |
| usn_13_truk 07, yamato 07, shogo_four 02, bulls_run 01 | `-0.866 0 0.5 / 0 1 0 / -0.5 0 0` | rows (0, 0, 1) / (0, 1, 0) / (-1, 0, 0), s = 1: an exact yaw of -90 degrees, no shear |
| us_osumi 01 | `0.559 0 0.829 / 0 1 0 / 0 0 1` | identity, s = 1: the intended -56 degrees is dropped because row 2 is (0, 0, 1) |

- **So the image has no sheared island.** The segment test's scaled transpose (13.3) and the
  general inverse agree on these normalised frames.
- **The host's gap is upstream.** `src/scene_file.cpp` composes the authored frame as read, with
  no 0085DC80 step, so the host places and traces these five islands sheared or wrongly rotated.
  `kLandscapeScaledTransposeInverseBound` answers the image's inverse of a frame the image never
  builds. It stays correct once the frames are normalised, and harmless before.
- **The census `yaw=-90.0` for usn_13_truk 07 (13.2)** came from row 2, which is the row the image
  keeps.
- **Every other authored frame** is orthonormal to about 1e-4 (four-decimal authoring), so
  normalisation moves it by about 1e-4. That is the same scale as the 13.5 failed prediction.

### Proposed packet (not bound here)

- **Name:** `cc9_scene_frame_normalise`. At the point where the host takes an entity's
  `localframe` (`parse_entity` in `src/scene_file.cpp`, before the 00413920 composition),
  apply s = |row 0|, then `orthonormalize_basis_rows_0085dc80`, then scale rows 0..2 by s.
  Put it behind one switch.
- **Expected effect:**
  - usn_13_truk, yamato, shogo_four, bulls_run and us_osumi change island placement;
  - every ship and plane's start frame moves by about 1e-4;
  - so gameplay rows on USN01, USN02 and USN04 may move slightly and must be judged per entity.
  It needs its own pairs on those three and on USN13.

## 17. 007C3CB0's caller is the plane effects screen (read, 2026-09-27)

Worker cc9-terrain2, on main 26fa4ca42. Ghidra was read only. This closes item 3 of section 15
and the provisional wording of 12.1 and 13.

- **The object has two vtables.**
  - Its destructor 00606040 and its constructor 00606470 both store 00CF43AC at +0 and 00CF4394 at
    +8.
  - The slot 00CF43CC is +20h (slot 8) of the primary vtable 00CF43AC. Its neighbours are slot 7
    00605CC0, slot 9 00606230 and slot 10 006067E0.
  - Neither vtable has RTTI: the dword before each is not a complete-object locator.
- **The class.**
  - 00606470 opens with `BSP_FrontEndScreen_Construct`, and its only caller is
    `BSP_InGameInterface_Init` 0068CC70 (0068CF87, 110h bytes, stored at interface +6Ch).
  - That is screen slot 3Fh in docs/IN_MISSION_INTERFACE_MANAGER.md.
  - docs/HUD_SCREEN_PAGES.md names it the **plane effects screen**. Its register 00607BE0 builds
    `Planewindsmoke`, `warning heartbeat only` and `Turbo_Effect`, and its page `GUI_plane_effects`
    is loaded by slot 3Eh.
- **The virtual 0060ABD0..0060C5AB (+20h)** reads the plane at screen +B8h (0060ABF0).
  - With none, it calls 006485A0 and leaves (0060ABFD..0060AC0A).
  - With one, it calls 007C3CB0 on it (ECX = +B8h, the frame time pushed; 0060AC1A), and further
    on calls the player-plane GUI update 00609BD0 (0060C35F).
- **So 007C3CB0 is a cosmetic effect of the plane the in-mission interface shows,** the player's
  plane. 12.1's statement holds, with the caller corrected to the screen's +20h virtual, which
  reaches 00609BD0 itself.
- **The host carries slot 3Fh only as a register-table row** (`src/hud_screens.cpp`, 00607BE0),
  with no +20h update, so 007C3CB0 stays unbound. Its test is a vertical
  segment, which misses in the image (14.7).

## 18. Every localframe normalised at parse (packet `cc9_scene_frame_normalise`, `kSceneFrameNormaliseBound`)

Worker cc9-terrain2, 2026-09-27, on main 743373aae. Ghidra was read only. This is the packet
section 16 proposed.

### 18.1 The binding (committed OFF)

- `normalise_localframe_0046d168` in `src/scene_file.cpp` runs right after `parse_entity` reads the
  16 floats, so every consumer of `SceneEntity::frame` sees the image's frame: the composition in
  `visit_entity` and the scene contents host's `entity.frame` reads. It does three things:
  - s = f32(sqrt(f32(row0 · row0))), or 0 unless the stored sum is above 1e-10;
  - the host's `orthonormalize_pose_matrix_0085dc80`;
  - rows 0..2 each times s, stored float.
- **Consequence.** The unit creators seed a unit's motion pose rows straight from the composed frame
  (`src/game_hosts_units.cpp`, "The frame 0046cf40 composed"). So the normalised rows become the
  start pose.

### 18.2 What moves, per mission (a census of this installation's `.scn` files; script `local\cc9-terrain2-framecensus.py`)

| mission | entities with a frame | changed | largest element change | what |
| --- | --- | --- | --- | --- |
| USN01 (usn_1_marshall) | 147 | 70 | 7.2e-5 | four-decimal rounding: 32 LandForts, 14 Paths, 10 DestroyerGen, 8 LandingPoints, Landscapes 04/05, the airfield, a command building, the carrier and one squadron |
| USN02 (usn_2_java) | 34 | 18 | 0.866 | 18 DestroyerGen carry the sheared `-0.866 0 0.5 / 0 1 0 / -0.5 0 0` frame: row 2 becomes (-1, 0, 0) and row 0 becomes (0, 0, 1); heading unchanged (-90) |
| USN04 (usn_19_coralus) | 58 | 54 | 0.866 (NavPoint `IJNRetreat` only) | ships and carriers at up to 5.1e-5; the NavPoint has the sheared frame |
| USN13 (usn_13_truk) | 524 | 307 | 0.866 | the Maru transports and Landscape 07 have the sheared frame; the rest is rounding |

The translation row never changes, so no start position moves.

### 18.3 Predictions (written before the pairs; streams and the death table on, lockstep 0.05, idle player)

- **USN01 3200/3000.** Rows move by at most 7.2e-5, and no start position moves.
  - The start headings (the unit table) agree to the printed precision.
  - A deterministic run can still carry a 1e-5 difference into gunnery.
  - Predicted: deaths 7 -> 7 and hit records 150 within +-5. `pair_diff` 1 or 3; if 3, at most one
    death row differs.
  - Rotation census (13.2): Landscape 05's tiny tilt is normalised, so its dy moves at the
    centimetre level; the counts are unchanged.
- **USN02 9200/9000.** The 18 sheared destroyers start with a unit forward row, where the host gave
  them a forward row of length 0.5 and a skewed right row.
  - Every motion rule that reads the pose rows (speed along the forward row, turning, hull
    segments) changes for them from frame 1.
  - Predicted: `pair_diff` 3. Those destroyers' tracks, hits and deaths move, in either direction;
    deaths 19 and hit records 566 move.
- **USN04 4700/4500.** Ships and carriers change by up to 5e-5; the NavPoint's rows do not feed a
  unit pose.
  - Predicted as USN01: deaths 44 and hit records 789 within noise (+-10), `pair_diff` 1 or 3 with
    at most a death row or two.
- **USN13 3200/3000.** The sheared Marus change as USN02's destroyers do, so `pair_diff` 3.
  - Landscape 07's rotation census row changes: the drawn footprint becomes the -90 degree island,
    and the segment test and the ground height still disagree (rotation).
  - Deaths 34 and hit records 204 move.

### 18.4 Pairs and verdict

- **The runs.** OFF is `local\bin\fn_off`, a build of 6ad4e6ff0. ON is `pair_export` of 6ad4e6ff0
  with the switch flipped (SHA-256 20B1473EA717). Streams and the death table were on, lockstep
  0.05, idle player. Logs: `local\FN_{OFF,ON}_{USN01,USN13,USN04,USN02}.log` in worktree
  cc9-terrain2.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN01 gameplay, death rows, unit table | 7 deaths, 150 hits, 561 shots | identical | within noise | held |
| USN01 start positions | | 5 avoid-zone path points move 0.7..0.8 m | no start position moves | **failed**: a child's world position composes its parent's normalised rows |
| USN01 Landscape 05 census dy | | cm level | cm level | held |
| USN01 `pair_diff` | | exit 1 | 1 or 3 | held |
| USN04 deaths, hits, shots | 44, 789, 6321 | identical | within +-10 | held |
| USN04 per-entity | | Northampton-class05 dealt 584 -> 583; one more gun fire call (38342 -> 38343) | at most a death row or two | held |
| USN04 `pair_diff` | | exit 3 (that unit row) | 1 or 3 | held |
| USN02 hit records, hull hits, shots | 566, 217, 850 | 573, 228, 863 | move | held |
| USN02 deaths | 19 | 19 (Encounter survives, John2 dies; 16 rows change in time or killer) | move | count **failed**; the rows moved |
| USN02 mission end | failed at 39.65 s | the same | - | - |
| USN02 `pair_diff` | | exit 3 | 3 | held |
| USN13 deaths, hit records, shots | 34, 204, 2551 | identical | move | **failed**: the counts held; hull hits 103 -> 102, two death rows change in detail, five units' `dealt` moves |
| USN13 Landscape 07 census | segment 54, height 231, both 8 | segment 90, height 90, both 3 | footprint matches, still disagrees | held |
| USN13 `pair_diff` | | exit 3 | 3 | held |

- **What moved gameplay.** USN02's 18 sheared destroyers start with a unit forward row and a right
  row at 90 degrees to it, as in the image. Their fight changes from about 40 s on.
- **USN04 and USN13** carry mostly rounding-level changes. USN13's sheared Marus sit far from the
  fighting (their `nearest` moves by metres).

**Verdict: ON.** The moves are the image's frames reaching the host's start poses, and each one
traces to a normalised frame. USN02's reference rows (19 deaths, 566 hit records) are superseded
by 19 and 573 on this base.

## 19. USN13's 670 plane water contacts (packet `cc9_usn13_water_contacts`, read, nothing bound)

Worker cc9-terrain2, 2026-09-27, on main 93e12f5a4. Logs: `local\FN_{OFF,ON}_USN13.log`,
`local\ROT2_USN13.log`, `local\WC_TRAJ_USN13.log` in worktree cc9-terrain2.

### 19.1 What the 670 counts

- **The row is a record count, not an event count.** It counts `Plane::water_contact_007cb7f0`,
  UNIMPLEMENTED. The host records it once per tick while a live plane is below the water line with
  007BC5B0's gate false (`src/game_hosts_units.cpp`, the "plane water contact ignored" path). It also
  records it at each state 7 -> 6 contact.
- **The events on USN13 3200/3000:**
  - 21 live torpedo bombers of the `bruh #1.7`..`#1.15` squadrons go below the surface, fall to
    -30 m and die by the depth kill (`007CE3A7`) about 0.85 s later;
  - 15 of them then register a dead-plane contact (state 7 -> 6).
  - About 30 ticks under water per plane gives the 670.
- **All 34 USN13 deaths are `bruh` planes.** 21 are these undamaged sea crashes (`first_damage=-1`,
  no killer) and 13 are shot down.
- **The same 670 appears OFF and ON** of the frame normalisation (section 18) and in the older
  `ROT2` run.

### 19.2 What it is not

- **Not terrain.** The water line is the flat sea (`water=0.00`). The crashes happen during the
  torpedo approach on Enterprise, 5.6 km from the nearest ship. No island is involved.
- **Not branch A.** `water probe probes=0`: no live plane is ever in mode 6.
- **Not the frame normalisation.** The count is identical OFF and ON.
- **Not a height-source defect.** The contact test compares the plane's y with the ocean height
  0.0, as the image's 007CB7F0 does.

### 19.3 What it is: live bombers stall into the sea

- **The surface probes** (below 5 m, once a second) show the crashed planes in a deep, sinking
  stall. Two examples:
  - `bruh #1.12` at 127.75 s: pitch `+C64h` 1.18 rad, live pitch input -1 (full nose down), throttle
    1.0, 25.9 m/s, sinking at 17.6 m/s;
  - `bruh #1.12|.-2` at 90.9 s: 50.7 m/s, sinking at 36 m/s, throttle 0.071.
  - The commanded altitude is 719..1450 m and the commanded pitch 0.46..0.48, the climb cap.
- **They are the planes that fought their wingmen.** Averaged per plane:

  | | crashed (21) | other bombers (39) |
  | --- | --- | --- |
  | vehicle-avoidance plane ticks | 560 | 88 |
  | vehicle-avoidance bands | 5674 | 1347 |
  | vehicle-avoidance throttle writes | 1473 | 337 |
  | terrain-avoidance ticks | 47 | 12 |

- **Why the wingmen are close.**
  - The squadrons spawn stacked: members 0-1 and 2-3 coincide and the pairs are 2.78 m apart. That
    is the image's own spawn (docs/SQUADRON_SPAWN_SEATS.md section 6).
  - In the torpedo approach's `follow` state, members 0-2 stay 12..41 m apart and 1-3 15..68 m
    through tick 1200 (`plane formation geometry`), while 0-1 open to 570 m.
  - So 007DF4F0 keeps firing for those pairs. Its throttle write `interp(3, 1, 15, -1, s)` cuts
    power when they close.
- **The open question is the one docs/CLIMBOUT_SPEED_GATE.md section 4 left.** Does the image's
  free-flight law (007DA710 and the pose advance) drop the nose in a deep stall? The host plane
  keeps a nose-up pitch of 1.0..1.19 rad under full nose-down input while sinking. The rate law
  itself cannot move the body rate away from its target (docs/PLANE_CONTROL_RATE_LAW.md). So the
  nose-up growth comes from elsewhere in the host's pose advance or from `+C64h`'s sign convention,
  and either needs its own read.

### 19.4 Verdict

- **Not the image's behaviour proven, and not a terrain gap.** Nothing is bound here.
- **Two reads for the plane owners, in order:**
  1. The follow state's station keeping for the torpedo approach: why members 0-2 and 1-3 hold
     12..68 m apart, and whether 009C1FD0's follow law gives them distinct stations.
  2. The high-angle-of-attack flight: why `+C64h` rises to 1.19 rad with the pitch input at -1
     (007DA710's pitch target and the pose advance).
- **The reference flag in `reports/cc9_reference_rebaseline_6.json` stays.** That file is cc9-ships2's
  lease, and the contacts are not shown to be the image's own. Its reason should read "21 live
  torpedo bombers stall into the sea (host flight / formation question, docs/SCENE_CONTENTS_HOSTS.md
  19)" rather than a water-contact defect.

## 20. The deep stall traced (packet `cc9_plane_deep_stall_law`, read, nothing bound)

Worker cc9-terrain2, 2026-09-27, on main 9cd1ff0ae. Ghidra was read only.
- **The trace.** A temporary per-tick trace of one plane, not committed, is kept as the patch
  `local\cc9-terrain2-pitchtrace.patch`. It is gated on `BSP_PITCH_TRACE=<unit name>` and prints
  `pitchtrace` and `pitchdemand` lines.
- **The runs:** `local\DG_USN13.log` and `local\DG2_USN13.log`, worktree cc9-terrain2, USN13
  3200/3000, plane `bruh #1.12`.

### 20.1 The plane's law is not what fails

- **In the image, a plane's rotation comes only from the rate law 007DA710.** The body rate at
  ctl+48h moves toward `PitchSpd · f1 · latched pitch` and cannot pass it. f1 is 007D9A70's
  authority, `t²` where t = max(an airborne term up to 0.25, a speed ramp that is 0 below 1.1 x
  StallSpd).
- **So a plane below that speed keeps an authority of 0.0625** and can change its pitch rate only
  slowly. No stall term, lift table or nose-drop exists in the rotation. The pose advance 007C6500
  only integrates the rate (docs/PLANE_ADVANCE_POSE.md), plus a roll-levelling lerp.
- **The host reproduces this** (`control_authority`, `plane_control_axis_step_007da710`). With
  authority at 0.0625, `bruh #1.12` needed about 40 s to fall 1370 m in a tail slide:
  - nose 1.2..1.56 rad, forward speed -10 to -17 m/s;
  - the rate law was moving the pitch rate by about 0.01 rad/s either way.

### 20.2 What does fail: a nose-up command against a nose-down demand

- **The onset, at 1237 m.** Authority was 1.0 and speed 50 m/s. The latched pitch input was +1 and
  the body pitch rate was +0.1745 (PitchSpd, nose up) for about 6 s, as the nose went 0.57 -> 1.18
  rad and the speed fell to 29 m/s. Then authority collapsed.
- **Over the same ticks the attitude demand 0099E490 asked nose-down.**
  - target plan+2BCh 0.10..0.13, bank about 0, `demand` -4.0 -> -8.4 (and -36..-66 once
    authority fell);
  - `plan_pitch_0099e68d` clamps that to a desired value of -1, sign kept.
- **So something writes the pitch slot or command after the demand, with the opposite sign.**
  - The pitch slot's desired value has ten writers in `src/game_hosts_units.cpp`.
  - The one the torpedo approach's `follow` state reaches is the follow hold arm 009BEE56
    (`plane_follow_hold_command_009bee56`, around line 4880). It writes `desired = c.pitch_29c`,
    sets slot 3 active and plan+2D0h = 0, from the follow gains `PF_Pitch*` and `PF_VertPos*`.
  - This is the same code as read 1 (station keeping in 009C1FD0). The clumped wingmen of section
    19 and a leader commanded nose-up would both come from it.

### 20.3 Next step (the packet continues as read 1)

1. **Extend the trace.** Log `c.pitch_29c`, the station offset and the vertical position error in
   `plane_follow_hold_command_009bee56` for `bruh #1.12` and its wingmen, and the slot's final
   `desired` and `cmd[kPilotCmdPitch]` each tick.
2. **Compare against 009BEE56's listing,** starting with the sign of the vertical term (`PF_VertDir`)
   and what the leader of a squadron follows. An AI leader in `follow` should have no station
   (`c.locked` is never set for it).
3. **Bind the difference behind one switch, with predictions:** USN13 21 sea crashes -> few, 34
   deaths -> fewer, the water-contact record 670 -> small, 204 hit records moving. USN04's air
   battle (44 deaths) and USN01's Mavis and scout rows would also be judged by the pairs.

Nothing was bound in this packet.

## 21. Handoff: cc9-terrain2 stops here (2026-09-27)

Worker cc9-terrain2 stops at about 75 % of its context. Section 15 was its first handoff; this one
supersedes it.

### Landed after section 15 (all ON)

| switch | doc | what |
| --- | --- | --- |
| `kDisablePhysicsBound`, `kAddMatrixInterpolatorBound`, `kExplodeToPartsBound` | LUA_BINDING_MISSION.md, "BSM01's state natives" | bound but unexercised: no reference run reaches a call |
| `kSceneFrameNormaliseBound` | this doc, 18 | every localframe normalised at parse, as 0046D168..0046D222 does |

Reads: 16 (the entity reader normalises frames), 17 (007C3CB0's caller is the plane effects screen),
19 (USN13's water contacts), 20 (the deep stall).

### Open items, in order

1. **The follow hold arm's pitch (sections 19 and 20).**
   - A USN13 torpedo bomber in `follow` is commanded pitch +1 while 0099E490 asks for -1; the
     likely writer is `plane_follow_hold_command_009bee56` (`desired = c.pitch_29c`).
   - Wingmen 0-2 and 1-3 stay 12..68 m apart in the same state.
   - Next: extend `local\cc9-terrain2-pitchtrace.patch` to the hold arm's terms, compare with
     009BEE56's listing (the vertical sign, and whether an AI leader should run it at all), and
     bind.
   - Predicted payoff: USN13's 21 sea crashes and most of its 670 water-contact records.
2. **The first measurement of the unexercised bindings.** Branch A (12), the three BSM01 natives,
   and the tile/cell steppers of 14 need a scenario run: a live flying boat on the water, a damaged
   Pearl Harbor battleship, a trace that crosses a grid line in the vertical case.
3. **Noise-list candidates** (15 item 1) are with cc9-tooling.
4. **Bit-exact x87 rounding** of the slot 3Ch walks (10.3, 14.5) is not verified.
5. **USN02's reference** is now 19 deaths / 573 hit records on the normalised frames (18.4); the
   next reference rebaseline should take it.

### State left by this worker

- Worktree cc9-terrain2 is clean after this commit; no lease is held after the report.
- Scripts are under `local\` with the prefix `cc9-terrain2-`:
  - `pe.py` reads image constants;
  - `vtscan.py` walks a vtable back to its start and finds its constructors;
  - `framecensus.py` is the .scn frame normalisation census;
  - `pairs*.ps1` are the pair runners;
  - `pitchtrace.patch` is the plane pitch trace (apply, build, set `BSP_PITCH_TRACE`).

## 22. The USN13 stall is a spawn spacing defect (packet `cc9_plane_follow_pitch_flip`, `kSpawnNewMemberOffsetsBound`)

Worker cc9-plane2, 2026-09-27, on main 1c2e84d27. Ghidra was read only. Addresses: 0099BF30,
0099B940, 007DF4F0, 009C1FD0, 0094A140, 00949300, 00941D30, 00948CC0, 009481A0.

### 22.1 The hold arm is not the writer

- **The trace.** Section 20's patch, extended to the repair pass 0099BF30, the hold arm 009BEE56,
  the slot evaluation 0099BC00 and the vehicle-avoidance bands of 007DF4F0. It is kept uncommitted as
  `local\cc9-plane2-pitchtrace.patch` in worktree cc9-plane2. Runs: `local\T1_USN13.log`,
  `local\T2_USN13.log`, USN13 3200/3000, plane `bruh #1.12`.
- **`bruh #1.12` is its squadron's leader.** 007F23A0 gives it no station, so 009C1FD0 does nothing
  but store pilot+26Ch = 2. The image does the same when 009BFD70 answers false. The hold arm never
  ran for it (0 records), and plan+2D0h stayed 2 on all 1065 thinks.
- **The +1 comes from the repair pass.** On 572 of 1065 thinks, 0099B940 found the pitch command
  inside a band covering all of [-1, 1]. 0099BF30 then substitutes +1.1 for |bank| <= pi/2
  (`00CE3830`, `00CE6448`), which 0099BF30's own clamp makes +1. That fallback is already bound
  and its constants match (docs/PILOT_COMMAND_BAND_REPAIR.md).
- **The band is `[-5, 5]` from the first think at spawn, for 367 thinks in a row.** It is 0099B790's
  clamp of a vehicle-avoidance band (007DFDC4) whose half-width w reaches 1.4..2.0. Another plane
  inside the combined radius rr = 25 m gives w = 1.25 x 1.6.
- **The planes inside that radius are other squadrons' planes.** The avoidance partners of
  `bruh #1.12` were the leaders and wingmen of `bruh #1.7`..`#1.15` (2700 records). Its own wingmen
  gave 15. The leader climbs at pitch command +1 from 1200 m, reaches 1419 m with the nose at
  1.25 rad and falls into the tail slide section 20.1 describes.

### 22.2 Why the squadrons are 5 m apart

- **USN13's wave is one `SpawnNew` request with 15 group members** (`usn_13_truk.lua`
  `luaSpawnAttackWave`, this installation's file dated 2024-08-13). It passes `angleRange` +/-10
  degrees, `lookAt` Enterprise, no `distRange`, and `excludeRadiusOverride` with all five radii 500.
- **The host placed each member by its own fan-out contract** (`spawn_member_frame_0094a140`,
  labelled CONTRACT in the source): the angle spread by member index at the 200 m low distance. That
  is about 5 m between consecutive squadrons.
- **The image places the members from offsets built by the record constructor 00948CC0**
  (00948E56-009492C0). 00949300 composes each offset with the one group frame (BSP_Matrix_Multiply4x4)
  and 009483D0 reads the vector back at 00948440. The loop, per row r of three members A, B, C:

  ```
  L = max(fH, (A.A4 + B.A4) * 0.5, (A.A4 + C.A4) * 0.5) * 2.5 + 5     ([00CE3DE0] 2.5, [00D7A370] 5.0)
  G = 5 + max(fH, (prev.A0 + this.A0) * 0.5 for each seat) * 1.5       ([00CE3D78] 1.5)
  z_r = z_(r-1) - G, z_0 = 0
  full row: A (0,0,z), B (-L,0,z), C (+L,0,z); two in the row: (-L/2,0,z), (+L/2,0,z)
  ```

- **fH is record+ACh = `formationHorizontal`.** 009481A0 reads the five keys in the order
  ownHorizontal, enemyHorizontal, ownVertical, enemyVertical, formationHorizontal (strings
  `00D19968`, `00D19958`, `00D1994C`, `00D1993C`, `00D19928`) into block+0..+10h. It stores the
  squares of the first two at +14h/+18h, and the constructor copies the seven dwords to record+9Ch.
  00941D30 confirms the layout: it tests own entities against +8h and +14h and enemies against
  +Ch and +18h.
- **So on USN13 the image's squadrons are 1255 m apart across and 755 m between rows.** The host has
  them 5 m apart. On USN04 (`usn_19_coralus.lua`, fH 100, two members) the image's pair is 255 m
  apart; the host has 69 m.

### 22.3 The binding

`kSpawnNewMemberOffsetsBound` (include/bsp/lua_spawn_new.hpp), committed OFF at a5ebad82c and flipped ON by 22.5. With it on, a request
with a positive `formationHorizontal` takes one group frame and 00948CC0's offsets.
- **The group frame is the contract's mid-angle candidate.** That is 0094A140's first candidate: the
  mid angle, at the low distance record+70h.
- **SUBSTITUTION, labelled:** the frame's axes are the host's yaw toward `lookAt`. 0094A140's
  rotation pair about the 008F8680 reference frame was not decoded.
- **SUBSTITUTION, labelled:** the class extents +A0h/+A4h are not carried in the request and are
  taken as 0. That is exact while fH covers every half sum, which aircraft under fH 100 and 500 do.
  A request with fH <= 0 keeps the contract.
- **Not changed:** the placement test 00941D30 and 0094A140's retry still do not run
  (docs/LUA_SPAWN_NEW_HOST.md section 8).

### 22.4 Predictions (written before the pairs)

| row | OFF | predicted ON |
| --- | --- | --- |
| USN13 3200/3000 undamaged `bruh` sea crashes (first_damage -1) | 21 | 0..3 |
| USN13 deaths | 34 | 15..40, with the shot-down share rising |
| USN13 `Plane::water_contact_007cb7f0` records | 670 | under 150 |
| USN13 hit records 204, shots 2551 | - | both move |
| USN04 4700/4500 | 44 deaths | gameplay moves (pair_diff 3); deaths within 36..52 |
| USN01 3200/3000 | - | identical: no `SpawnNew` request |
| USN02 9200/9000 | - | identical: no `SpawnNew` request |

### 22.5 The pairs (flipped ON)

OFF is this tree's build at a5ebad82c. ON is `pair_export --flip kSpawnNewMemberOffsetsBound=true` from the
same commit. Environment `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, lockstep 0.05, idle player.
Logs `local\SP_{OFF,ON}_<mission>.log` in worktree cc9-plane2.

| row | OFF | ON | predicted | verdict |
| --- | --- | --- | --- | --- |
| USN13 undamaged `bruh` sea crashes | 21 | 0 | 0..3 | holds |
| USN13 deaths | 34 (13 shot down) | 20 (20 shot down) | 15..40, shot-down share rising | holds |
| USN13 water-contact records | 670 | 5 | under 150 | holds |
| USN13 hit records / shots | 204 / 2551 | 460 / 4033 | both move | holds |
| USN04 gameplay | - | pair_diff 3 | moves | holds |
| USN04 deaths | 44 | 44 | 36..52 | holds |
| USN04 torpedo / dive releases | 3 / 4 of 16 / 19 | 6 / 8 | not predicted | - |
| USN01 3200/3000 | - | pair_diff 0 | identical | holds |
| USN02 9200/9000 | - | pair_diff 0 | identical | holds |

- **The spawn geometry is the read one.** USN13's members 1 and 2 are 1255 m apart, and members 1
  and 4 are 755 m apart. The rows trail away from Enterprise. USN04's pair is 255 m apart.
- **USN13's 20 deaths are all shot down.** More bombers survive the climb-out, so the carriers'
  fighters and the ships' guns engage more of them: hit records 204 -> 460.
- **USN04's death set swaps one row.** `A6M Zero #6.2|.-2` dies OFF only and `A6M Zero #6.2` ON
  only; the other 43 rows change time and killer. The releases double because the pairs no longer
  start inside each other's avoidance radius.
- **No failed prediction.**
- **What stays open:** the placement test 00941D30 and 0094A140's retry still do not run. The group
  frame's axes and the class extents are the two labelled substitutions of 22.3. Section 19.4's
  read 1 (the follow-state station keeping, pairs 0-2 and 1-3 at 12..68 m) should be re-measured
  on this binding, because the other squadrons' avoidance bands are gone.

## 23. 0094A140's frame, 00941D30 and the retry (packet `cc9_spawn_new_placement`, `kSpawnNewPlacementBound`)

Worker cc9-plane2, 2026-09-27, on main 2d4862ea0. Ghidra was read only. This packet retires section
22.3's three labels. Addresses: 0094A140, 00949300, 00941D30, 00948CC0, 00949750, 008F8680,
0085DC80, 00B646E0, 00413920.

### 23.1 The group frame

- **The reference frame R is record+10h through 008F8680.** 008F8680 answers an entity's world
  matrix (+14h set) or the matrix at object+18h. A position-table `refPos` stores the identity basis
  at that point (00949BC7..00949C5F, then 008F84D0).
- **`lookAt` rewrites R's row 2** as lookAt - refPos, vertical part included (00949E82..00949EE0).
  0085DC80 then orthonormalises: row 1 is made orthogonal to row 2 and row 0 = normalize(row1 x
  row2). 0094A140 normalises the three rows again (0094A17A, 0094A1F9, 0094A278).
- **A candidate is RotY(a) T(0, 0, d) RotY(-a) R**, in row-vector order. This is read from the
  three 00413920 calls at 0094A6A0..0094A6AE, whose left operand is in ECX. 00B646E0's rows are
  (cos, 0, -sin), (0, 1, 0), (sin, 0, cos). So the candidate keeps R's basis, and its origin is
  refPos + d (-sin a R.row0 + cos a R.row2).
- **The member frame is T(offset) candidate** (00949380..009493E4, offset as the left operand).
- **Consequence on USN13.** R pitches 8.7 degrees down toward Enterprise, so the first candidate
  sits 200 m along that slope, at 1169.8 m. The rows of 22.2 trail up the slope at
  1169.8 / 1283.9 / 1398.0 / 1512.2 / 1626.3 m instead of flat at 1200.
- **Consequence on USN04.** The bomber groups pass no `lookAt`, so R is the identity basis and the
  frame is section 22's.
- **SUBSTITUTION, labelled:** an entity `refPos` would bring that entity's own basis; the request
  keeps only its position. No reference call site passes one.
- **SUBSTITUTION, labelled:** the created entity is built from a heading (SpawnNewFrame), so the
  frame's pitch does not reach it.

### 23.2 The search and the test

- **0094A140's search.** d runs from distLow (record+70h, at least 10) while d <= distHigh, in
  steps of 250 (double [00CF8850]). The arc runs from 0 while arc <= d x halfwidth, also in steps
  of 250. Each arc tries a = mid + arc/d, then a = mid - arc/d when arc > 0. The search returns
  at the first candidate that 00949300 accepts.
- **The aircraft distance loop runs only in multiplayer.** 0094A6C3.. pulls the distance in by
  globalConfig+2E4h while a same-party aircraft is within globalConfig+2E0h, but only when
  game+1FE4h != 0. That word is 0 in single player (docs/CONSTRUCT_WORLD.md), so the loop is not
  reproduced.
- **00941D30's registers.** ECX is record+78h, the party. EDX is the member's translation row. The
  stack carries the member's aircraft flag (vtable+18h(0Fh), 00949420), the class, the frame and
  the exclude block. It returns with RET 10h.
- **What 00941D30 refuses.** It refuses outside the map (0071C4F0). An aircraft skips the terrain
  and depth probes. Then, over [[00E188A8]+19CCh]+58h:
  - a same-party entity with |dy| <= ownVertical and a squared 3-D distance below ownHorizontal
    squared;
  - an other-party entity with |dy| <= enemyVertical and a squared 3-D distance below
    enemyHorizontal squared.
- **The squares** are 009481A0's block+14h/+18h.
- **00948CC0's row maxima now take the class extents** (+A0h `Length`, +A4h `Width`).

### 23.3 What the host needs (applied in 23.6)

The unit list and the class rows are reachable only from the Lua host, `src/game_hosts_lua.cpp`,
which cc9-hud3 held at the time of writing. The prepared hunk
(`local\cc9-plane2-luahost.patch`, worktree cc9-plane2) does three things:
- registers a SpawnPlacementWorld over the live units (row party, unit_position_00fc);
- fills each member's `Length`/`Width` through read_vehicle_class_row;
- stops the member loop on `frame.refused`, so the record is requeued as 0094C5AD does.

**SUBSTITUTION, labelled in the hunk:** 0071C4F0's bounds live in the zone runtime, which the Lua
host does not reach, so no member is refused as outside the map. Without the hunk, the solver has
no world and takes the first candidate.

### 23.4 Predictions (written before the pairs)

| row | OFF | predicted ON, axes only (no hunk) | predicted ON, with the hunk |
| --- | --- | --- | --- |
| USN13 member altitudes | 1200 flat | 1169.8 .. 1626.3 by row, member 1 at (-4855.5, 1169.8, -5398.7) | same, first candidate accepted |
| USN13 deaths / hits | 20 / 460 | move, deaths within 12..30 | as the axes-only run |
| USN13 sea crashes | 0 | 0..2 | 0..2 |
| USN04 4700/4500 | 44 deaths, releases 6 / 8 | identical | identical: first candidates accepted, extents under fH 100 |
| USN01, USN02 | - | identical | identical |

### 23.5 The axes-only pairs (flipped ON)

OFF is this tree's build at 91544ca32. ON is `pair_export --flip kSpawnNewPlacementBound=true` from
the same commit. No placement world is registered (the hunk of 23.3 is not applied), so the test and
the retry are inert and the first candidate is taken. Environment as in 22.5. Logs are
`local\PL_{OFF,ON}_<mission>.log` in worktree cc9-plane2.

| row | OFF | ON | predicted | verdict |
| --- | --- | --- | --- | --- |
| USN13 member positions | flat 1200 | member 1 (-4855.5, 1169.8, -5398.7), rows 1169.8..1626.3 | as computed | holds, to 0.1 m |
| USN13 deaths | 20 | 27 | 12..30 | holds |
| USN13 hit records / shots | 460 / 4033 | 527 / 4180 | move | holds |
| USN13 sea crashes (first_damage -1) | 0 | 0 | 0..2 | holds |
| USN13 water-contact records | 5 | 8 | not predicted | - |
| USN13 torpedo-task releases | 0 of 60 | 2 of 60 | not predicted | - |
| USN04 4700/4500 | - | pair_diff 0 | identical | holds |
| USN01, USN02 | - | pair_diff 0 | identical | holds |

- **No failed prediction.**
- **The switch is ON with the test inert.** Applying the 23.3 hunk activates the test and the
  retry. Its own pair is predicted to be identical on all four missions, with the first candidate
  accepted.

### 23.6 The full binding (hunk applied, stays ON)

The 23.3 hunk is applied to `src/game_hosts_lua.cpp` at be664aa2f, with one log line per solved
request that names the accepted candidate, the entity count and the nearest entity. OFF is
`pair_export --flip kSpawnNewPlacementBound=false` from be664aa2f; ON is this tree's build. Logs
are `local\FP_{OFF,ON}_<mission>.log` in worktree cc9-plane2.

| row | OFF | ON | predicted (23.4, with the hunk) | verdict |
| --- | --- | --- | --- | --- |
| USN13 accepted candidate | - | first (angle 0, d 200), 313 entities, nearest 2316 m | first candidate | holds |
| USN13 deaths / hits / shots | 20 / 460 / 4033 | 27 / 527 / 4180 | as the axes-only run | holds |
| USN13 sea crashes | 0 | 0 | 0..2 | holds |
| USN04 eight requests | - | all first candidates, 21..73 entities, nearest 2051..8032 m | first candidates | holds |
| USN04 gameplay | - | pair_diff 1 (log lines only) | identical | holds |
| USN01, USN02 | - | pair_diff 0 | identical | holds |

- **The full binding is gameplay-identical to the axes-only run** of 23.5 on all four missions. The
  OFF side also matches 23.5's OFF (pair_diff 0 on USN13).
- **The test is not vacuous.** It walked 313 live units on USN13 and 21..73 on USN04, but every
  unit was over 2 km from every member, beyond the 500 m and 150 m radii. So no reference mission
  exercises the refusal and retry path; a mission that spawns a wave on top of existing units
  would be the first measurement.
- **No failed prediction.**

## 24. The follow-state station keeping on the new spacing (packet `cc9_follow_station_keeping`, read, nothing bound)

Worker cc9-plane2, 2026-09-27, on c056d9112 (sections 22 and 23 ON). Ghidra was read only. This
is section 19.4's read 1, re-measured.
- **The trace.** `local\cc9-plane2-pitchtrace2.patch` in worktree cc9-plane2, uncommitted. It is
  the section 22 trace plus a `geotrace` line: position, station, heading, leader heading and the
  yaw slot.
- **The runs.** `local\T3_USN13.log` and `local\T5_USN13.log` trace `bruh #1.12|.-4`;
  `local\T4_USN13.log` traces the leader `bruh #1.12`. All are USN13 3200/3000.

### 24.1 The clustering is gone

- **`bruh #1.12` holds its wing on the new spacing** (`plane formation geometry`, `FP_ON_USN13`):
  0-1 145.8..148.4 m and 0-2 147.8..149.5 m from tick 400 to 2800. That is the 141 m seat of
  (-100, 0, -100) plus a few metres.
- **Section 19's 0-2 = 12..41 m and 1-3 = 15..68 m do not recur.** They were the other squadrons'
  avoidance bands of section 22, not a station-keeping defect.
- **Across the wave at tick 800**, 0-1 and 0-2 are 65..300 m in every squadron.

### 24.2 What remains: member 3 runs ahead of its station

- **It never latches.** `bruh #1.12|.-4` is on the fly-to arm on all 1268 thinks (fw_arm 2). It
  starts 281 m from its station, closes to 154 m, then opens steadily to 441 m.
- **It is ahead, not behind.** The bearing to the station is 0.79..1.02 rad while the heading is
  -2.33..-2.47 rad, so the station is almost directly astern. The member flies the leader's
  heading to within 0.03 rad.
- **It is faster than the leader.** It holds 64..69 m/s at throttle 1.0, while the leader holds
  about 63 m/s at throttle 0.81..0.98.
- **The mechanism is 009BEE30's alignment ramp**, already read in docs/PLANE_FOLLOW_LAW.md:
  - with the station astern, the cosine is -1;
  - that is below `MaxFollowSpdTargetDir` = DEG(30) = 0.5236, used as a cosine (section 5.4
    there);
  - so the command is the distance ramp's catch-up end, turbo x leader speed, since len > 100 m =
    `GoodPositionDist`.
- **Same run-ahead as before.** docs/PLANE_FOLLOW_LAW.md section 16.1 measured this "wing runs
  away ahead of a slow leader" on USN04. Here the leader's torpedo-approach speed of 63 m/s is
  only a little below what the member reaches, so the drift is slow (about 160 m over two
  minutes).
- **Not a new defect.** The units asymmetry of 5.4 is the image's own arithmetic, recorded there
  and not corrected.

### 24.3 Open, for the plane owners

1. **The torpedo approach leader's commanded speed** in the follow state. Its throttle stays below
   1 at about 63 m/s. Whether the image's approach flies faster, as docs/PLANE_FOLLOW_LAW.md 16.2
   found for the moveto slot, would decide whether seat 3 ever reaches its station.
2. **The seat-3 station itself.** It is 281 m from the stacked spawn point. Only seat 1's local
   offset is logged (`seat1 local=(-100, 0, -100)`); seat 3's offset from 007ED260 / 007F23A0 was
   not printed.

Nothing was bound in this packet.

## 25. The torpedo approach leader's speed (packet `cc9_torpedo_approach_leader_speed`, read, log line added)

Worker cc9-plane2, 2026-09-27, on main 39fc36f96. Ghidra was read only. Run `local\LS_USN13.log`
(worktree cc9-plane2), USN13 3200/3000; it is gameplay-identical to 23.6's ON run (pair_diff 1,
log lines only).

### 25.1 The leader is not in the follow state

- **Section 24's leader `bruh #1.12` flies the torpedo task's moveto** (state 0x544) for 1101
  thinks, then the attack run (0x6B4) for 167. Only the wingmen are in follow (0x580).
- **Its speed is 009C1850's**, bound as `kMovetoSpeedBlendBound` (docs/PLANE_FOLLOW_LAW.md 16.2):
  `009BECD0(squadron+3A0h, 007C47F0(), sep)` = L + (M - L) x max(k, W), where:
  - M is TravelSpeed x NewTravelSpeedMul and L is LevelFlight x StallSpd;
  - k = interp(WingmenWaitDist1 -> 0, WingmenWaitDist2 -> 0.5, sep);
  - W is 007EF2C0's minimum over the members' 009BE3E0 values.

### 25.2 The measured command, term by term

- **The command is constant.** The new `plane formation seats` line logs the leader's plan+2B4h at
  63.19 m/s from tick 400 to 3200, with a measured speed of 63.06..63.44 m/s. The throttle of
  0.81..0.98 in section 24 is the speed controller holding that command. At tick 3600 the attack
  run commands 88.89.
- **k = 0.5.** sep stays above WingmenWaitDist2 through the approach, so k sits at its cap.
- **W = 0, from seat 3.**
  - Seats 1 and 2 latch (+85h) and answer 1.0: 009BE3E0 returns 1 at once when +85h is set.
  - Seat 3 answers 0. The image's 009BE3E0 takes unit - state+30h, and state+30h is 007F23A0's
    station: 009BFDC4 LEA EBP,[ESI+30h] is its output argument. For seat 3, that vector points
    along the leader's heading, so the angle is below DontWaitForHdgDiff and the first term is 0.
    It is also beyond NearbyDist, so the second term is 0.
- **So the leader flies the midpoint L + (M - L)/2.** Every term is read and matches the host.

### 25.3 Why seat 3 is ahead: the stacked spawn

- **Every seat's station is now logged.** The new line gives, for `bruh #1.12`:
  - seat 1, index 1, local (-100, 0, -100);
  - seat 2, index 2, local (100, 0, -100);
  - seat 3, index 3, local (-200, 0, -200).
- **At tick 0 each member spawns on its leader** (docs/SQUADRON_SPAWN_SEATS.md 6), so it starts
  ahead of its station by the station's own offset: 141.4 / 139.5 / 280.9 m.
- **Seats 1 and 2 latch.** They reach their stations (6..18 m by tick 400) and hold there.
- **Seat 3 never falls back.** It is 155.8 m from its station at tick 400, then 178.7 m, and
  411.9 m by tick 3200.
- **The loop, all from bound image code:**
  - with the station astern, its fly-to speed is the catch-up end of 009BEE30's ramp
    (docs/PLANE_FOLLOW_LAW.md 5.4);
  - being ahead, it answers W = 0 to the leader, which holds the leader at the midpoint speed;
  - seat 3 stays faster than the leader and the gap grows.

### 25.4 Verdict

- **Nothing to bind.** 009C1850, 009BECD0, 009BE3E0 (including its +85h early return and the
  unit - station vector), 007EF2C0 and 009BEE30 are all read and match. The 63 m/s is the image's
  command for a squadron whose seat 3 starts ahead of its station.
- **Not verified.** Whether the image's own spawn leaves seat 3 ahead by the full 281 m. That
  depends on 007F4580's member seats, which docs/SQUADRON_SPAWN_SEATS.md reads as stacked.
- **Added:** the `plane formation seats` log line (src/game_hosts_units.cpp, beside
  `plane formation geometry`). It prints every seat's index, local offset and distance, plus the
  leader's plan+2B4h and speed. It is gameplay-neutral.

### 25.5 Section 21 item 2, where a cheap scenario exists

None does, so nothing was run:
- **Branch A (the flying-boat water probe, 12)** needs a live Mavis in mode 6 on the water. No
  idle reference run gives one: USN01 logs `water probe probes=0`.
- **The three BSM01 natives** (`DisablePhysics`, `AddMatrixInterpolator`, `ExplodeToParts`) need
  a script that calls them. No reference run reaches a call.
- **The tile and cell steppers of 14** need a trace that crosses a grid line in the vertical case.
  Section 21 item 2 records that no reference run was found to do so.

Each needs a built scenario, which the lead ruled out for this pass.

## 26. The stacked spawn verified (packet `cc9_squadron_spawn_seats_check`, read, nothing bound)

Worker cc9-plane2, 2026-09-27, on main 3c6d89753. Ghidra was read only. This closes section 25.4's
unverified step.

### 26.1 007F4580 places every member at one point

- **The placement arms carry no seat.** 007F4800..007F48D2 (disk bytes) create the member through
  `[this]->vtable[28h]` (007F4811), then place it through `vtable[98h]` in one of two arms:
  - with a parent (007F481D..007F48BA), an identity 4x4 built on the stack at ESP+64h, local to
    the parent;
  - without one (007F48BE..007F48D2), the squadron's own matrix at `squadron+74h`.
- **Neither arm reads a member index or an offset.**
- **The tail moves nothing.** docs/SQUADRON_SPAWN_SEATS.md section 1's claim about 007F4B43.. is
  confirmed for its two callees:
  - `0077FAD0` is the squadron entity's own init slot. 007F4BA0 BSP_PlaneSquadron_SEntityInitSlotA4
    calls it on the squadron (ESI = this), and it joins a unit group.
  - The `+170h` virtual at 007F4BD8..007F4BE6 is called on the sub-object at +170h of the pointer
    held in squadron+3D0h. Its body was not read, so it is the one unverified step.
  - 0077FAD0 writes no member pose.
- **So seat 3 really starts 280.9 m ahead of its (-200, 0, -200) station,** as the section 25 log
  showed. The stacked reading holds and nothing is bound.

### 26.2 Two of docs/SQUADRON_SPAWN_SEATS.md 6's questions, closed by evidence

1. **"Does 007DF4F0 separate two aircraft at zero offset?"** No.
   - Its aircraft arm keeps only a closing contact: `dot(lp, lv) < 0` at 007DFAB6.
   - Two coincident planes have lp = 0, so the dot is 0 and the pair is skipped.
   - Stacked members separate only through their follow states.
3. **"Why do the carrier flights not follow in their first 20 s?"** It no longer reproduces. On the
   current base (`local\FP_ON_*.log`, worktree cc9-plane2), none of the 46 squadrons of USN01,
   USN02, USN04 and USN13 keeps identical pairwise spacing between report ticks 400 and 1600
   (script `local\cc9-plane2-static.py`).

Question 2 was answered by docs/PILOT_MOVETO_TASK.md parts 1-4 (section 6a there).

## 27. The release-order gate is not what limits USN04's releases (packet `cc9_release_order_gate`, read, markers made concrete)

Worker cc9-plane2, 2026-09-27, on main 7ca25aa95. Ghidra was read only.

### 27.1 007EE7F0 is already modelled

- **The UNIMPLEMENTED row was a marker, not a missing body.**
  `PilotControl::pre_issue_hook 007ee7f0` is the host's hook for 007EEF3B CALL 007EE7F0. That
  body stores ctl+3ECh = 0 and the armed fraction ctl+374h (docs/TORPEDO_RELEASE_ORDERS.md (5)).
- **The fraction is computed before 007EEF40 reads it.** `read_issue_inputs` computes it through
  `bsp::flight_armed_fraction_007ee7f0` every time. The hook only logged.
- **The threshold is the one live substitution beside it.** ctl+390h stands in at 1.0 x 0.95 for
  `*(unit+538h)+A0h` x 0.95 (0079CD36). It can close the gate only when the armed fraction reaches
  0.95. With the caller's own round deducted, a four- or five-plane flight tops out at 0.75 or 0.8.
  So it cannot refuse any reference squadron.

### 27.2 Where USN04's releases go

- **Most torpedo bombers never reach the gate.** On `local\FP_ON_USN04.log` (worktree cc9-plane2)
  the per-unit `issue gate 007EEF40` report shows ctl+390h = 0 for every Kate but one. Those
  aircraft never ran 007C0D90's issue path. The one that did had the gate open (0.95 > 0.0).
- **They die first.** All 16 Kates die in the run (16 death rows), 6 of them after releasing. Of
  the 19 Vals, 12 die and 8 release.
- **So the low release counts are an attrition question, not a gate question.** Nothing is bound
  here. Where the Kates die and to whom is in the per-entity death table; that belongs to the
  gunnery owners.

### 27.3 Two markers made concrete (gameplay-neutral)

- `PilotControl::pre_issue_hook` (007EE7F0) now calls `done`: its effect is the modelled armed
  fraction.
- `BotApproach::command_altitude` (009FBA50) now calls `done`: the site runs
  `bsp::cruise_altitude_command_009fba50`, and its one labelled gap (the squadron+394h leg) is
  unchanged.
- **Both logged UNIMPLEMENTED whatever the site computed.** The hook called `log.unimplemented`
  directly, and the 009FBA50 site used the string form of `record`, which does the same. Eight more `record(name, "<address>")` sites remain in src/game_hosts_units.cpp; each
  needs its own check before it is made concrete.

| row | before | after | predicted | verdict |
| --- | --- | --- | --- | --- |
| USN04 4700/4500 gameplay | FP_ON (44 deaths) | MK_USN04 | identical | holds (pair_diff 1) |
| USN13 3200/3000 gameplay | LS (27 deaths) | MK_USN13 | identical | holds (pair_diff 1) |
| host methods concrete / unimplemented, USN04 | 1095 / 558 | 1097 / 556 | two rows move | holds |
