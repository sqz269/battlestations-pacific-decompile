# Mission scene contents: the Registration and Instantiate passes

Addresses: 004d4df0 004d4720 004d0ee0 004c17d0 004248a0 004bc890 004ba870 00780db0
00b0fd70 00b0fdc0 00b0fe10 00b0fe60 00be4380 00be44a0 004c8aa0 004697b0 00bb5ac0
0095c1c0 0095c550 0046df00 00871ba0 00bdf4c0 007f8d60 00468c90 00bdcb30 004cd7f0

`BSP_Game_LoadSceneContents` (`004D4DF0`), `__fastcall void(GGame*)`, sole caller
`004DFB70 BSP_Game_LoadMissionScene` at `004E03E5`. It is the `load_scene_contents()`
stub of `include/bsp/mission_scene_load.hpp` line 201. Reconstruction:
`include/bsp/mission_scene_contents.hpp`, `src/mission_scene_contents.cpp`.
Coverage: **complete** for `004D4DF0`; the named callees are covered to the depth
each row below states.

## The headline: the per-entity walk is not here

The packet asked what the two passes "then do per scene entity: the class lookup,
the registration call, the instantiate/factory call, the per-entity Lua string run".
They do none of that in `004D4DF0`. Both passes are the single already-reconstructed
routine `0046DF00 BSP_SceneFile_Read` called twice with different flags, and the
per-entity class lookup, the registration/instantiate creators and the tail blocks
are all inside it, documented in `docs/SCENE_FILE_READER.md` (its "The three passes"
section already names both of these call sites). `004D4DF0` contributes the *frame*
around them: what is primed before, what is resolved between, what is created after.

| site | arguments (push order reversed) | pass |
| --- | --- | --- |
| `004D54DE` | `(path, 0, 0, record, override, 1)`, ECX = `[00E18680]` | 2 Registration |
| `004D5530` | `(path, 0, 1, record, override, 0)`, ECX = `[00E18680]` | 3 Instantiate |

`path` is `[[game+5FCh]+910h]` or `00E18B1C` ("") when null (`004D54C0`, `004D5512`);
`override` is `[game+604h]` or the same empty string (`004D54A9`, `004D54FD`);
`record` is `[game+5FCh]`.

## What sits between the two passes, and why it matters

This is the part that is new. Pass 2 does not just mark entities: through
`0046CF40`/`0046DF00` it calls `0095C640`, which calls `0095C550`, which appends the
used vehicle class index to the global array `00F8A09C`/`00F8A0A0`/`00F8A0A4`
(`xrefs 00f8a0a0`: the only writer family is `0095C550`, the only readers are
`004C8AA0` and `004D4720`). `004D4DF0` brackets pass 2 with exactly those two:

1. `004D5499` `004C8AA0` clears the array and the intrusive list at `00F8A0B0`.
2. `004D54A4` `00468C90` destroys the scene database's range at `[00E18680]+8h`.
3. pass 2 fills the array.
4. `004D54E9` `004697B0` walks `[00E18680]+28h`, invokes each element's vtable
   slot 0 with argument 1 (scalar deleting destructor) and empties the vector.
5. `004D54F3` `004D4720` (ECX = `010904E4`) turns every collected index into a
   `"Class_" + <vehicle class name>` string (`0095C1C0`, which reads
   `BSP_VehicleClassRegistry_GetSingleton()+10h+index*4`) and links it into the
   list whose head pointer is `010904E8` = `010904E4 + 4`.
6. `004D54F8` `00BB5AC0` walks that same list and rewrites each entry's name
   through `BSP_VFS_ResolveExistingName`.
7. pass 3 instantiates.

So the Instantiate pass runs against a VFS whose class-asset names have already
been resolved from the registration pass's class census. That ordering is the
reason the two passes exist as separate calls in this routine.

## Body order

| # | site | callee | name | this / args / ret | gate |
| --- | --- | --- | --- | --- | --- |
| 1 | `004D4E1E` | - | inline store | `[00E188A8]+19D0h = 0`, `+19D1h = 0` | none |
| 2 | `004D4E4B` | `004254B0` | `TRIV_body_004254b0` | `__cdecl(fmt, path)`, `ADD ESP,8`; fmt `00CE7918` "Loading scene %s" | none |
| 3 | `004D4E69` | `006B8AD0` | `BSP_LuaMachine_RunString` | `(00CE4DE0, 0, 0, 2)`, no caller cleanup | `[[game+1A08h]+4] != 0` (`004D4E5E`) |
| 4 | `004D4E80` | `004CD7F0` | `BSP_Scene_DeriveShortName` | `__fastcall(out, &record+90Ch)`, push 1 | none |
| 5 | `004D4EEA` | `00BE0A30` | `BSP_FileBlock_Construct` | `(name, 1)`; name = `"2_"` (`00CE7914`) + short name | none |
| 6 | `004D4F78` | `00B0FD70` | `BSP_RenderResources_SetRemapTexture0` | ECX = `[00F8D39C]`, push `record+C24h` | none |
| 7 | `004D4F95` | `00B0FDC0` | `BSP_RenderResources_SetRemapTexture1` | ECX = `[00F8D39C]`, push `record+C2Ch` | none |
| 8 | `004D4FB3` | `00B0FE10` | `BSP_RenderResources_SetRemapTexture2` | ECX = `[00F8D39C]`, push `record+C34h` | none |
| 9 | `004D4FD0` | `00B0FE60` | `BSP_RenderResources_SetRemapTexture3` | ECX = `[00F8D39C]`, push `record+C3Ch` | none |
| 10 | `004D4FE1` | `004D0EE0` | `BSP_SceneRecord_PreloadEffects` | `__fastcall(record)`, ECX = `[game+5FCh]` | none |
| 11 | `004D502B` | `00871BA0` | `BSP_EffectHandle_AcquireByName` | `__fastcall(out, &name)`, push 1; name "PlaneRumble" `00CE7908` | none |
| 12 | `004D504E` | - | inline ref swap | publishes the handle into `00E18A78` | `[00E18A78] != new` |
| 13 | `004D5195` | `00469840` | `BSP_NativeString_Substring` | `(out, &path, 0, len-4)` | none |
| 14 | `004D51AE` | `004261A0` | `BSP_NativeString_Concat` | appends `".nav"` (`00CE7900`) | none |
| 15 | `004D522E` | `[0109CEEC]` vt `+8h` | VFS exists | `(&navPath)` -> `AL` | none |
| 16 | `004D523F` | `00BE4380` | `BSP_VFS_OpenStreamInto` | `__thiscall(out, &navPath, 2)`; wraps `[0109CEEC]` vt `+4h` | `AL != 0` (`004D5232`) |
| 17 | `004D5251` | `004C17D0` | `BSP_AvoidZoneRegistry_GetSingleton` | no arguments, returns `[00E17620]` | same |
| 18 | `004D5258` | `004248A0` | `BSP_AvoidZoneRegistry_LoadFromStream` | `__thiscall(registry, &stream)` | same |
| 19 | `004D5269` | `00BE44A0` | `BSP_VFS_ReleaseStream` | `__fastcall(&slot)` | same |
| 20 | `004D5336` | `00467CF0` | `BSP_NativeString_ReverseFindHeader` | `(this, &".", 7FFFFFFFh)` -> index of last `.` | none |
| 21 | `004D5347` | `00469840` | `BSP_NativeString_Substring` | `(out, &path, 0, index)` | none |
| 22 | `004D5418` | `0041DD40`/`00BF7680` | append | appends `".ema"` (`00CE7838`) | `len != 0` |
| 23 | `004D5458` | `00BDF4C0` | `BSP_VFS_ResolveExistingName` | ECX = `[0109CEEC]`, push `&emaPath` -> `AL` | none |
| 24 | `004D547B` | `007F8D60` | `STL_inst_007f8d60` | `__thiscall(game+650h, game+2198h)` -> found | `BL != 0 && AL != 0` |
| 25 | `004D548E` | `004BC890` | `BSP_Game_SetGameMode` | `__thiscall(game, 9, 0)` | previous row and `007F8D60 == 0` |
| 26 | `004D5499` | `004C8AA0` | `BSP_Scene_ClearPendingClassIds` | no arguments | none |
| 27 | `004D54A4` | `00468C90` | `STL_inst_00468c90` | ECX = `[00E18680]`, destroys `+8h` range | none |
| 28 | `004D54DE` | `0046DF00` | `BSP_SceneFile_Read` | pass 2, see table above | none |
| 29 | `004D54E9` | `004697B0` | `BSP_SceneDatabase_DestroyPendingObjects` | `__fastcall([00E18680])`, `+28h`/`+2Ch` | none |
| 30 | `004D54F3` | `004D4720` | `BSP_Scene_BuildClassPreloadAliases` | `__fastcall(010904E4)` | none |
| 31 | `004D54F8` | `00BB5AC0` | `BSP_Scene_ResolvePreloadAliases` | no arguments, list `[010904E8]` | none |
| 32 | `004D5530` | `0046DF00` | `BSP_SceneFile_Read` | pass 3, see table above | none |
| 33 | `004D557F` | `00780DB0` | `BSP_MultiScore_Construct` | `__thiscall(new(314h), 0)` | `BL == 0 && [00E188A8]+1FE4h == 1` |
| 34 | `004D5631` | vt `+98h` | placement | `__thiscall(obj, 0, [game+19CCh], &identity4x4)` | same |
| 35 | `004D56BA` | `00BDCB30` | `BSP_FileBlock_Destroy` | `__thiscall(&block)` | none |
| 36 | `004D56C5` | `004BA870` | `BSP_SceneRecord_ScatterClouds` | `__fastcall(record)`, ECX = `[game+5FCh]` | none |

Rows 13/14 and 20..22 are the two derived file names. Rows 6..9 and 11 are the
resource priming. Rows 26..32 are the two-pass block above. `004D5554`/`004D5563`
(`00BF55BE` `operator new`, `00BF79F0` `memset`) are CRT and are not rows.

## The one branch flag, computed once and used twice

`004D52B6..004D52EF`:

```
EAX = game+614h
if (game+61Ch != 0)        goto cmp
if (game+1FE4h != 0)       goto cmp
if (EAX == 9)              goto cmp
if (EAX == 8)              goto setz     ; ZF already 1
EAX = 8
cmp:  CMP EAX,8
setz: SETZ byte [ESP+3Fh]
```

That is `004BCA50` inlined (the same rule `docs/GAME_SIMULATION_GATE.md` line 194
records) followed by `== 8`, so the flag is
`effective_game_mode_004bca50(game+614h, game+61Ch, game+1FE4h) == kDefaultSinglePlayerGameMode`.
The reconstruction calls the existing `include/bsp/simulation_gate.hpp` helper
rather than restating the rule. The slot is read back as `BL` at `004D545D`
(`[ESP+37h]`; `ESP` is 8 lower at the `SETZ`, and `0041DD40` is `RET 8`).

It gates two things in opposite directions: with the flag set and the `.ema` file
resolvable, mode 9 is forced; with it clear and a network session live, the
`MultiScore` entity is created.

## Named callees

* **`004D0EE0` `BSP_SceneRecord_PreloadEffects`**, `__fastcall(record)`.
  `004D0EFB` takes `EBX = record+D50h`, the effect-handle vector; `004CB160`
  resizes it to 0 (releasing every held reference), then for
  `i < [record+C70h]` it calls `00871BA0` with `EDX = [record+C6Ch] + i*8` and
  pushes the handle with `004CAF50`. So `record+C6Ch` is a native-string array
  (8-byte stride) and `record+C70h` its count.
* **`004C17D0` `BSP_AvoidZoneRegistry_GetSingleton`**, no arguments. Double-checked
  singleton on `00E17620` under the lifetime manager's critical section;
  `operator new(14h)` then `00424730`, registered with
  `BSP_SingletonLifetime_Register`. The decompiler shows a bogus argument at the
  `004D4DF0` call site: the routine is `int(void)` and the `PUSH EDX` at
  `004D5248` belongs to the next call.
* **`004248A0` `BSP_AvoidZoneRegistry_LoadFromStream`**, `__thiscall(registry, &stream)`.
  Calls `stream` vtable `+60h`, reads the element count from vtable `+38h`, and
  for each one allocates `operator new(28h)` with vtable `00CE38CC`, deserialises
  it with `004239E0(stream)` and links it into the list at `registry+8h`.
  The class name is **AvoidZone**: `00CE38B0` is the literal the vtable at
  `00CE38CC` follows, with `AvoidZoneG` at `00CE38A0`. Depth: the element's 28h
  bytes were not decoded.
* **`004BC890` `BSP_Game_SetGameMode`**, `__thiscall(game, mode, forced)`.
  Writes `game+61Ch = forced`, `game+618h = mode`, `game+614h = mode`, then when
  `game+1FE4h != 0` and the record exists it pushes either `[record+98Ch]`
  (mode 7) or 0 into eight view blocks (`game+18CCh..18E8h` `+28h`,
  `game+1030h..17D8h`, `record+4h..7E4h`). `docs/MISSION_LOAD_PATH.md` line 134
  already cites the same routine as `004BC890(game, game+614h, 0)`.
* **`004D4720` `BSP_Scene_BuildClassPreloadAliases`**, `__fastcall(010904E4)`.
  Walks `00F8A0A0..00F8A0A4` in 4-byte steps; per index calls `0095C1C0`
  (`"Class_" + vehicle class name`, from `BSP_VehicleClassRegistry_GetSingleton`)
  and inserts through `BSP_NativeRenderResourceAliasNode_AllocateCopy` /
  `BSP_NativeAliasList_GrowCount` into the list at `[010904E4]+4`.
* **`00B0FD70`/`00B0FDC0`/`00B0FE10`/`00B0FE60`
  `BSP_RenderResources_SetRemapTexture0..3`**, `__thiscall([00F8D39C], &name)`.
  Each releases the old reference in slot `+66Ch`/`+670h`/`+674h`/`+678h` and
  stores `[00F8D394]` vtable `+64h` `(name, 0)`. Identified as colour-remap
  textures by the other caller: `00503510 BSP_FrontEndPreview_Draw` passes
  `ColourRemap.tga` (`00CEB588`) to all four (`00503EC1`, `00503F34`, `00503FA8`,
  `0050401C`). `005098B0` is the third caller and was not read.
* **`00BE4380` `BSP_VFS_OpenStreamInto`**, `__thiscall(out, name, mode)`; stores
  `[0109CEEC]` vtable `+4h` `(name, mode)` into `*out`.
  **`00BE44A0` `BSP_VFS_ReleaseStream`**, `__fastcall(int* slot)`; decrements the
  refcount at `*slot + 4` and calls vtable slot 0 at zero.
* **`00780DB0` `BSP_MultiScore_Construct`**, `__thiscall(obj, 0)`. Installs
  vtables `00D040B8`, `00D040A0`, `00D04098`, `00D04094`, `00D0408C`, sets
  `obj[31h] = 60h` and `obj[15h] = 2`, borrows the Lua state from
  `[[00E188A8]+1A08h]+4`. The class literal `MultiScore` sits at `00D04080`,
  immediately before the vtable block, with `MultiScore_save` at `00D04070`.
  Its placement call matches `docs/SCENE_UNIT_CREATORS.md`:
  `vtable[98h](parent = 0, [game+19CCh], identity 4x4 built from 00D7A24C)`.
  Depth: the constructor's body past the vtable installs was not read.
* **`004BA870` `BSP_SceneRecord_ScatterClouds`**, `__fastcall(record)`. Returns at
  once when `record+C84h < 1`. Otherwise it allocates `record+CA0h` float triples
  and as many floats, and per point rejection-samples a position in the box
  `record+C88h/C8Ch/C90h` to `record+C94h/C98h/C9Ch` (`00BD2F10` is the ranged
  random), picking a kind by the weights at `record+CA4h` and rejecting while any
  placed point is nearer than the two kinds' separations, up to 1000 attempts.
  It then creates the entity with `0046D930(<kind name>, "Cloud", 0)`, sets its
  transform through vtable `+88h` and calls vtable `+D8h`. The kind table is the
  18h-stride array at `00E081C8`: `CloudSmall` 100.0, `CloudMedium` 300.0,
  `CloudBig` 500.0, then the `-1` terminator at `00E08208`. These are procedural
  clouds in addition to the 7078 authored `Cloud` entities
  `docs/SCENE_FILE_READER.md` line 327 counts.

## Corrections to earlier records

* The ledger entry for `004D4DF0` says it "pushes the four terrain scalars at
  `[game+5FCh]+C24h/C2Ch/C34h/C3Ch`". They are not scalars and not terrain: the
  assembly at `004D4F6B..004D4FD0` loads `ECX` from `[00F8D39C]` and pushes the
  record offset as the **argument**, and the callee passes it to the texture
  factory `[00F8D394]` vtable `+64h`. They are four native-string texture names.
  The decompiler dropped the `ECX` on all four sites.
* The ledger entry also says the routine "also touches ... `004BC890(9,0)`" without
  its gate; the gate is the three-way test of row 25.
* `include/bsp/mission_scene_load.hpp` line 39 states "There is no `2_` block".
  That is true of `004DFB70`, but `004D4DF0` opens exactly that block at
  `004D4EEA` and closes it at `004D56BA`, so the numbering has no gap. The header
  is the integrator's file; this packet records the fact rather than editing it.

## Uncertainties

* The readers of `[00E188A8]+19D0h` and `+19D1h` were not identified, so what
  clearing them enables is unknown. `+19CCh` (the world node) is a neighbouring
  field but a different one.
* The `.nav` and `.ema` files are not present loose in the installation; both
  names are only reachable through the VFS packages, so neither extension was
  confirmed against a real file. The names come from `00CE7900` and `00CE7838`.
* The scene-record offsets in this document are all **consumer-side reads**. The
  header pass of `0046DF00` that writes `record+C24h..CB0h` was not opened, so the
  `.scn` keys behind the remap textures, the effect list and the cloud block are
  unknown. `docs/SCENE_RECORD_SIDE_BLOCKS.md` records `+0C24h..` only as "zero
  runs and further sub-objects".
* The constant added to the cloud weight total at `00D7A258` was not decoded, so
  `select_scene_cloud_kind` takes the roll as an input instead of drawing it.
* No run-time evidence: `bsp_game.exe` reaches `load_scene_contents()` only as a
  stub, and this packet does not own the executable or the load walk, so the
  claims here are static. Checklist rule 6 is unmet by construction for this
  packet; the integrator's host wiring is what will exercise it.
