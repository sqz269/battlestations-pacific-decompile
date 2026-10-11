# CC13 input rebinding and the segment-6 scene classes

Worker cc13-settings2 (Claude lineage cc13), successor of cc13-settings1. Two parts: the two
input-settings routines settings1 surveyed but did not reconstruct (segment 29), and partition
segment 6 (`00465610..0047C680`).

Every name here is a hypothesis, not a recovered symbol. Status words: *exported* (pseudocode in
`exports/`), *reconstructed* (C++ in `src/`), *build-tested* (compiles in the MSVC Win32 build),
*game-validated* (observed in a run). Nothing in this document is game-validated.

## Input rebinding and conflicts

Host: `include/bsp/input_rebind.hpp`, `src/input_rebind.cpp`, over the `InputSettings` model in
`include/bsp/input_settings.hpp`. Status: reconstructed, build-tested, not ABI-compatible.

### `006A9A40` `BSP_InputSettings_SetBindingSlot`

`__thiscall`, `ECX` = settings, stack: device name, input name, slot, 14h-byte binding pointer,
float scale; `RET 14h` (`006A9E62`). Only caller `00560430` (`BSP_ControlBindingScreen_Update`).

1. Returns at once when settings+50h (DEVINPUTS) is set (`006A9A58`).
2. `devices[device]` (`0055C110` on settings+8), then `runtime_bindings[input]` (`006A45C0` on
   devRec+0Ch) slot `slot` receives the five dwords of the binding (`006A9AB1..006A9AE8`).
3. The slot's bit in `runtime_reverse[input]` (`006A4CA0` on devRec+6Ch) is set when
   `COMISS 0.0, scale; JBE` is not taken, that is when `scale < 0`; NaN clears it
   (`006A9AC1..006A9AF3`).
4. Walks the device's AxisPair vector (devRec+5Ch, begin +60h, end +64h, 10h-byte elements, each
   a vector of at least two pooled strings). A pair names the input when its first string equals
   it (length compare, then `_stricmp`), giving the second as partner, or else when its second
   string equals it (`BSP_NativeString_EqualsInsensitive`), giving the first. The loop does not
   stop at a match.
5. The partner name is copied (`006A9C24`) and `00699BF0` (axis test: class 1 key >= 8, class 2
   key >= 3Ch, unsigned) is applied to the **new** binding (`006A9C60`, `ECX = EBX` = the binding):
   - axis: the partner's same slot receives the same binding; its reverse bit is set when
     `COMISS scale, [00D7A218]; JC` is not taken. `00D7A218` holds `00 00 00 00` (0.0f), so the
     partner is reversed when `scale >= 0` and NaN clears it. The two halves of a pair read one
     physical axis in opposite directions.
   - not an axis: when the partner's same slot holds an axis, it becomes `{-1, 0, 0, -1, 0}`
     (`006A9DC4..006A9DD2`; the byte at +10h is written from a stack dword whose low byte was
     zeroed) and its reverse bit is cleared. A partner holding a button is left alone.

### `006A9F70` `BSP_InputSettings_IsInputInAxisPair`

`__thiscall`, `ECX` = settings, device name, input name; `RET 8`, `AL`. True when any AxisPair
of `devices[device]` has the input on either side. Only caller `006AAC50` (`006AAF8C`).

### `006AAC50` `BSP_InputSettings_FindBindingConflict`

`__thiscall`, `ECX` = settings, stack: device name, input name, slot, out string pointer (may be
null); `RET 10h`, `AL` (`006AB659` returns 1, `006AB67D` returns 0). Ghidra's `__fastcall`
prototype shows a spurious `EDX` parameter; the four stack arguments are `param_3..param_6`.
Callers `0055C200`, `0055C5A0`, `0055D380` (the control binding screens).

1. DEVINPUTS set: returns 1 (`006AAC68..006AAC82`).
2. The slot's binding; class -1 returns 0. The slot's reverse bit is captured (`006AACF9..
   006AAD0A`, `0048F230`).
3. Finds the first group in Conflicts.Groups (settings+30h, begin +34h) holding an entry
   `(device, input)`; none returns 0. `00552250` is the 10h-element vector size.
4. Builds a list: that group, then for each Conflicts.Pairs row (settings+40h, begin +44h) the
   other index of a row that names the group (first column checked first) (`00442190` push_back).
5. `paired = 006A9F70(device, input)`.
6. For each listed group, for each entry `(device2, input2)`:
   `same_device`, `same_input` by case-insensitive compare; `shares_axis` only when `paired` and
   `same_device`: some AxisPair of `devices[device]` holds the input and `input2`
   (`006AB0F0..006AB2BE`). Then for slots 0 and 1 (`stride 14h, < 28h`): the other binding
   `devices[device2].runtime_bindings[input2][s]` conflicts when it is bound, it is not excluded
   (at `s == slot`, excluded when `same_device && same_input`, or when `shares_axis` and the
   other binding is an axis), its class, +4 and key equal this binding's (+8 and the byte at
   +10h are not compared), and its reverse bit equals this binding's.
7. On the first conflict: when `out` is non-null it becomes `input2 + "|. (|" + device2 + "|.)|"`
   (`00CF81D0`, `00CF81D8`; concat order from `006AB508..006AB551`, where the first left operand
   is the input slot and the second the device slot of the stack frame); returns 1.

### Divergences and uncertainty

- Native indexes every map with `operator[]` (inserting a missing key) and fails the checked
  vector index on a short vector (`00BF6713`). The host uses `at()` and throws
  `std::out_of_range`.
- The partner-reverse rule (`!(scale < 0)`) and the own-slot rule (`scale < 0`) differ at
  exactly 0.0: a zero scale leaves the slot unreversed and the partner reversed.
- The `|. (|`/`|.)|` markup reads as "input (device)" in the GUI text system; its rendering was
  not observed.
- `include/bsp/input_settings.hpp`: `InputNameRecord::primary`/`secondary` were renamed to
  `device_type`/`key`, on the `0069E9E0` evidence settings1 recorded (`0069EDBB..0069EDCC`).

## Segment 6: survey

`python tools/bsp.py segment 6` reports 428 candidates; the index lists 551 functions in
`00465610..0047C680`, 379 of them still `FUN_` before this packet. Most of those are `std::_Tree`
and `std::vector` instantiations (`STL_xlen_throw_*` callers, node allocators, `_Lbound`,
`_Insert`, erase-range walkers), skipped under the usual rules. The scene-file side
(`BSP_SceneFile_*`, `BSP_SceneDatabase_*`, the `SceneBrowserGroups` block) is already covered by
`docs/SCENE_FILE_READER.md`, `docs/SCENE_BROWSER_GROUPS.md` and `docs/SCENE_ENTITY_FACTORY.md`.
The new work is the upper half of the segment, `00470700..0047B4E0`: five Lua-described visual
classes, each with its own registry singleton, lazy loader, reader, entity factory and entity
vtable. Names below were added to the ledger and applied to Ghidra.

## Segment 6: the visual class registries

Host: `include/bsp/scene_class_registry.hpp`, `src/scene_class_registry.cpp`. Status of the
loaders, the two readers and the two reset routines: reconstructed, build-tested, not
ABI-compatible. Everything else in this section is exported and named only.

### Common shape

All five classes derive from the named class base (`00489E60` constructor, base vtable
`00CE660C`, `00489E80` destructor; `docs/NATIVE_SOLDIER_CLASS_RESOLUTION_ORCH4.md`): name
NativeString at +4h, mesh resource +10h, FakeFireEfx handle +14h, fire point +18h..+24h, read by
`NamedClassBase_ReadLua_0048F670`. Class vtable: +0 scalar deleting destructor, +4 read from a
Lua row, +8 create an entity of the class, and for the three name-keyed classes +0Ch, a constant
`MOV AL,1; RET` the loader calls on every lookup and ignores (the SoldierClass loader `004B1400`
does the same).

Each registry is a 10h-byte object (vtable, then a `std::map`) published in a global by a
double-checked singleton getter under the `BSP_SingletonLifetime_GetManager` critical section and
registered with `BSP_SingletonLifetime_Register`. The destructor body erases the map nodes and
frees the sentinel; it does not delete the mapped classes.

| Table | Class bytes / vtable | Key | Getter / global | Construct | Destruct body | Index | Loader | Read | Create entity | Entity id / bytes / vtable |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `AnimClass` | 2Ch / `00CE5B44` | name | `00472200` / `00E18688` | `00472100` | `00472170` | `00471F60` | `004722D0` | `00470820` | `00470C90` | 3Eh / 194h / `00CE5A20` |
| `Bullets` | 2Ch / `00CE5CB4` | bullet class | `00473AF0` / `00E18690` | `004739F0` | `00473A60` | `004738C0` | `00473BC0` | `0048F670` | `004728F0` | 39h / 18Ch / `00CE5B90` |
| `BuoyClass` | 28h / `00CE5E14` | name | `00475640` / `00E18698` | `00475540` | `004755B0` | `004753A0` | `00475710` | `0048F670` | `00474160` | 3Ch / 18Ch / `00CE5CF0` |
| `CloudClass` | 38h / `00CE5FB4` | name | `004798A0` / `00E186A0` | `004797A0` | `00479810` | `00479600` | `00479970` | `004774A0` | `00476360` | 3Dh / 1ACh / `00CE5E68` |
| `VehicleClass` | 2Ch / `00CE6254` | vehicle class | `0047B2F0` / `00E186DC` | `0047B1F0` | `0047B260` | `0047B0C0` | `0047B3C0` | `0048F670` | `00479FE0` | 38h / 190h / `00CE6130` |

Class ids 3Dh (Cloud) is the scene-file class of `docs/SCENE_ENTITY_FACTORY.md`; 38h, 39h, 3Ch
and 3Eh are not registered from scene files. Their `IsKindOf` slots (`vtable+5Ch`) were already
named; all five chain to 37h (`00470BE0`), so 37h is the shared visual-entity base.

### Name-keyed loaders `004722D0`, `00475710`, `00479970`

`__fastcall`, `ECX` = name NativeString, `RET`, `EAX` = class. Registry `operator[]` by name
(case-insensitive, `BSP_NativeString_LessCaseInsensitive` in the tree walk). On a null cell: Lua
globals (`00B67980`), the class table (`00B67800` with `"AnimClass"`, `"BuoyClass"`,
`"CloudClass"`), its row by the name (`00B67800` with the C string for AnimClass and BuoyClass,
`00B68100` with the NativeString for CloudClass), `operator new`, the class constructor, a copy of
the name, `vtable+4(row)` (`00479A5F` for CloudClass), then the registry is fetched again and the
cell stored. `vtable+0Ch` follows on hit and miss.

Native creates the null cell before the Lua read, so a read that throws leaves a null entry that
the next call retries; the host inserts only after the read and treats a null cell as a miss,
which is the same observable retry.

Callers: AnimClass `00498D60`; BuoyClass `004A41C0`, `004A5850`; CloudClass
`BSP_SceneCloud_RegisterClass` (`004E9920`) and `BSP_SceneCloud_Instantiate` (`004E9E40`), which
`docs/ENTITY_IDENTITY.md` already traced into `00479970`.

### Source-keyed loaders `00473BC0`, `0047B3C0`

`__fastcall`, `ECX` = source class object, `RET`, `EAX` = class. The registry key is the
pointer (unsigned compare in `004738C0`/`0047B0C0`). On a miss: `operator new(2Ch)`, the copy
constructor (`00472FF0` Bullets: name from the source's string data at +14h; `0047A800` Vehicle:
from +54h; both store the source at +28h), the cell is stored (`00473C38`, `0047B438`) **before**
the Lua read, then `Bullets[[source+0Ch]]` / `VehicleClass[[source+70h]]` through `00B677E0`
(`00473C7D`, `0047B47D`) and `vtable+4` = `0048F670`. No `vtable+0Ch` call. The host keeps the
publish-then-read order.

`00473BC0` has one static caller, `006E5C39`, which Ghidra does not place inside a function
(segment 33, `docs` for `006EA910 BSP_BulletClass_GetOrCreate` describe the bullet class it
starts from); the caller then calls the visual class's `vtable+8` and keeps the entity.
`0047B3C0` callers: `00487470`, `004A0410`, `004A3610`, `004A4B70` and
`BSP_LandingShipClass_ReadLuaFields` (`0074C9F4`; `docs/VEHICLE_CLASS_LUA_LOAD.md` left this call
undecoded).

### Readers

`00470820` AnimClass: `NamedClassBase_ReadLua_0048F670(row)`, then `Radius` through
`00B66270` (GetNumber, no default) stored by `FSTP` to +28h.

`004774A0` CloudClass (does not call the base reader):

| Step | Sites | Effect |
| --- | --- | --- |
| `CloudType` | `004774C0..004774EB` | `00B66380` default 0, stored to +34h |
| zero: `Texture` | `00477501..00477556` | `00B662B0` string to +2Ch (empty for null) |
| zero: `ParticleMultiplier` | `0047756E..00477594` | `00B66330` with `FLD1` default, kept on the stack |
| zero: clear | `004775B1` | +10h = 0 |
| zero: `Mesh` | `004775A5..004775FE` | `[[00E188A8]+19E8h]` then `00BBDDF0` (`MOV EAX,[ECX+3Ch]`), `00BBD6C0(mesh, &texture, multiplier)` to +28h |
| non-zero: `Mesh` | `0047764A..004776AA` | `BSP_Resource_LoadWithGameFactory` (`007188A0`) to +10h |

So a particle cloud (type 0) carries a particle system built from the mesh, texture and
multiplier, and a mesh cloud keeps only the resource.

### Entities

`CreateEntity` (class `vtable+8`, `__thiscall` on the class, `RET`, `EAX` = entity): `operator
new` and `memset 0`, base constructor `00489F30`, the three vtable pointers (+0, +10h, +24h), class
id at +C4h, `BSP_ClassEntity_SetClass` (`0047B9E0`, +170h = class), and
`BSP_SEntity_PushPending`. The cloud entity also stores the class at +188h and clears +1A4h and
the byte +1A8h.

Overrides the five entity vtables share (the 37h base): +34h `00470BA0` writes the constant vector
at `00F87574` (zero velocity), +60h `00470BD0` returns 0, +B0h `004707D0` returns +178h, +114h
`0047B9D0` stores +174h. Per class `+A0h` (the SEntityInit slot, see `docs/SUBMARINE_MODEL.md`):
Bullets and Buoy tail-jump to the base `0048DBD0`; Anim `00471080` calls the base, then
`00B6DC90` on +178h and stores a `BSP_Random_UniformFloatRange` result to +18Ch, which its +D8h
`004710D0` reads; Vehicle `00479BD0` calls the base and stores to +18Ch; Cloud is
`BSP_CloudEntity_ApplySceneProperties` below. Vehicle +ECh `0047A660` reads
`BSP_HitRecord_HullDamage`/`BSP_HitRecord_WorstPart` (`00470510`, `004706D0`). These slot readings
are survey-level.

`00477710` `BSP_CloudEntity_ApplySceneProperties` (547 pseudocode lines, survey only): after the
base `0048DBD0`, when the class's CloudType or entity +1A4h is non-zero, the property bag at +C0h
has kind 1, and byte +1A8h is clear:

- CloudType 0: `InsideColor`, `Ambient`, `Diffuse`, `Specular` (each a property with `R`, `G`,
  `B` sub-properties at `00CE60EC/E8/E4`, integer or float) feed the particle-system setters
  `00BBE540`, `00BBE4B0`, `00BBE4E0`, `00BBE510` with the constant at `00D7A24C`; `Density`
  `00BBE570`, `BlurRadius` `00BBE590`, `Scale` `00BBE470`, `Alpha` `00BBE490`, `AffectedByWind`
  `00BBE5C0`, then `BSP_SceneNode_SetVisibilityFactor` and `00BBD470`.
- otherwise: `xBaseCol`, `xSkyCol`, `xSunCol` (R, G, B, and `A` at `00CE608C` for the base
  colour), `xScale`, `xRimPower`, `xBackPower` become material parameters `cBaseColor`,
  `cSkyColor`, `cSunColor`, `cRim_Back`, `cDiffuseColor_BaranyScale` through
  `BSP_Material_RegisterFloat4ArrayParameter` on the generated model's geometry.

`004772C0` and `004773A0` are byte-identical `ECX` = property bag, C-string name, `RET 4`
helpers returning `BSP_ScenePropertyBag_Find(name)`; both are also called by
`BSP_Landscape_InitAttachTerrain`.

### Lua table resets

`00474200` and `00476410` run `"BuoyClass=nil\n"` / `"CloudClass=nil\n"` through `006B8AD0(chunk,
0, 0, 2)`. Both are called from `0045F600` (`0045F6DF`, `0045F6E4`) and from
`BSP_Game_LoadMissionScene` (`004E0AB9`, `004E0ABE`) in a run of other class-table resets
(`006E9C50`, `004B1890`, `004AF930`, `004A7120`, `006D74B0`). The registries themselves are not
cleared there, so a class read in one mission stays cached across a reset of its Lua table; whether
another path clears them was not traced.

## Segment 6: engine-movie actions `00465610..004663A0`

The bottom of segment 6 holds the constructors of the engine-movie (in-engine cutscene) actions.
Chain: `BSP_Game_BeginEngineMovie` (`004CCE50`) calls `00460B80`, which fills the action type
table, a vector of 58h-byte records at `00E18550` (begin `00E18554`), and `004602C0`, the movie
file parser (`ActionList`, `PointList`, `Entitypath`, `Animations`, `Movielen`, `AspectR`).
Its action reader `0045E5E0` reads `Action`, `ActivateTime`, `Type`, `Num1..3`, `Enum`,
`ReferPath`, `OffsetX/Y` and `Comment`, matches the action name against the table names
(record +4, `_stricmp`) and passes the index to the factory `0045DE50`, a switch that allocates
44h bytes (48h for type 20) and calls the type's constructor; the `Comment` token is then copied to action+8.
These callers are in segment 5 and were read, not renamed.

`00460B80` pushes 32 records (`00460AD0` calls at `00460E37` .. `00464285`); the authored
name of each is the string copied just before its push (`00460E08` `sSetTarget` ..
`0046424C` `aHideEntity`). Record 4 is built from empty strings only and the factory has no
case 4. Each record also carries parameter labels and option lists (`Destroy|Blown|Kill`,
`OpenFire|HoldFire`, `All|AA_Flak|Artillery|Torpedo|DC`, `Surface|Periscope|Underwater|Maxdepth`,
`JetEngineBig|RealGiantSmoke|GiantSmoke`, `CameraPos|LookAtPos|CameraAndLookAt`,
`Binocular|Periscope|OFF`, ...), apparently for an editor. The lowercase prefix (`s`, `p`, `u`,
`c`, `a`) reads as ship, plane, U-boat, camera, any; that is a hypothesis.

Every constructor is `__thiscall(this, float activate time)`, `RET 4`, calls the base
`BSP_MovieAction_ConstructBase` (`00465B60`) and stores its vtable. The base calls
`BSP_MovieTimedItem_Construct` (`00465850`: +4 time, +8 string, which the factory fills with the comment), writes the base vtable
`00CE5498` {`_purecall`, `004656C0`}, and clears +14h, the strings at +18h and +2Ch, +20h..+28h
and +34h..+3Ch. `cBlackOut` passes +20h and +24h to the blackout arm, so +20h..+28h are
the three numeric parameters. Action vtables have two slots: slot 0 runs the action, slot 1 is
the no-op `004656C0` except for `aParticleEffect`.

| id | name | constructor | vtable | slot 0 (run) | slot 1 |
| --- | --- | --- | --- | --- | --- |
| 0 | `sSetTarget` | `00465C40` | `00CE54A4` | `004650F0` | `004656C0` |
| 1 | `cSetParent` | `00465C70` | `00CE54AC` | `00464870` | `004656C0` |
| 2 | `pEngineFailure` | `00465CD0` | `00CE54BC` | `00465D00` | `004656C0` |
| 3 | `pForceRelease` | `00465DB0` | `00CE54D8` | `004648F0` | `004656C0` |
| 5 | `cUnParent` | `00465CA0` | `00CE54B4` | `00464350` | `004656C0` |
| 6 | `sForceDepthCharge` | `00465DE0` | `00CE54E0` | `00465180` | `004656C0` |
| 7 | `pSetGunFire` | `00465E10` | `00CE54E8` | `00464910` | `004656C0` |
| 8 | `uTogglePeriscope` | `00465E40` | `00CE54F0` | `004649B0` | `004656C0` |
| 9 | `sForceArtilleryAngle` | `00465E70` | `00CE54F8` | `00464390` | `004656C0` |
| 10 | `sForceArtilleryTarget` | `00465EA0` | `00CE5500` | `004643A0` BSP_FireTargetHolder_Apply | `004656C0` |
| 11 | `uForceTorpedo` | `00465ED0` | `00CE5508` | `00465210` | `004656C0` |
| 12 | `aSetGunFireEnabled` | `00465F00` | `00CE5510` | `00465290` | `004656C0` |
| 13 | `aSetGunFireDisabled` | `00465F30` | `00CE5518` | `00465350` | `004656C0` |
| 14 | `aSetGunFireTargetOnly` | `00465F60` | `00CE5520` | `00465410` | `004656C0` |
| 15 | `aTurnGun` | `00465F90` | `00CE5528` | `00465450` | `004656C0` |
| 16 | `uSetDepth` | `00465FC0` | `00CE5530` | `004654D0` BSP_EntityCommand_SetSubmarineDepth | `004656C0` |
| 17 | `sForceAAFlak` | `00465FF0` | `00CE5538` | `00465570` | `004656C0` |
| 18 | `cFOV` | `00466020` | `00CE5540` | `004643D0` | `004656C0` |
| 19 | `cBlackOut` | `00466080` | `00CE5550` | `004660B0` | `004656C0` |
| 20 | `aParticleEffect` | `00466170` | `00CE5558` | `004656D0` | `004661A0` |
| 21 | `aDont_Move` | `004661C0` | `00CE5560` | `004649F0` | `004656C0` |
| 22 | `cFOV_Now` | `00466050` | `00CE5548` | `004643E0` | `004656C0` |
| 23 | `pSetGear` | `004661F0` | `00CE5568` | `00464A30` | `004656C0` |
| 24 | `cCameraShake` | `00466220` | `00CE5570` | `00464AB0` | `004656C0` |
| 25 | `cBinocular` | `00466250` | `00CE5578` | `00464C40` | `004656C0` |
| 26 | `pTakeOffHack` | `00466280` | `00CE5580` | `00464D70` | `004656C0` |
| 27 | `pWingTipEffect` | `004662B0` | `00CE5588` | `00464D80` | `004656C0` |
| 28 | `sMoveToPoint` | `004662E0` | `00CE5590` | `00464DC0` | `004656C0` |
| 29 | `cWideBlackBars` | `00466310` | `00CE5598` | `00464E00` | `004656C0` |
| 30 | `sUseCatapult` | `00466340` | `00CE55A0` | `00464E50` | `004656C0` |
| 31 | `aHideEntity` | `00466370` | `00CE55A8` | `00464EE0` | `004656C0` |

Run bodies inside segment 6:

- `00465D00` `pEngineFailure`: `[ESP+4]` = entity, `RET 4`; `entity->vtable[194h]("explosion_delayed")`
  (`00465D58`). `docs/GUNNERY_OPEN_ITEMS.md` identifies that slot as the named death call
  (`006D2210(name, 0)` for an airfield). The action's own parameters are not read.
- `004660B0` `cBlackOut`: `BSP_HudNarrativeScreen_ArmBlackoutFade(+20h, +24h, "")`.
- `004656D0` `aParticleEffect`: refreshes the entity pose, acquires the effect by name, creates
  a point effect on the entity's matrix, pre-runs `BSP_Effect_UpdateChildren` in 0.1 s steps
  (`00CE54A0`) up to `StartTime` (+20h) when positive, and keeps the effect at +44h;
  `004661A0` (slot 1) releases +44h through `008673B0` and clears it.

The other run bodies are in segment 5 (`00464350..004654D0`); two are already named
(`BSP_FireTargetHolder_Apply`, `BSP_EntityCommand_SetSubmarineDepth`). Status: exported and
named; nothing here is reconstructed.

## Segment 6: shared helpers named in passing

| Address | Name | Evidence |
| --- | --- | --- |
| `00467050` | `BSP_Matrix_SetRotationFromEuler` | `__thiscall(float* m, a, b, c)`, `RET 0Ch`; `fsin`/`fcos` of the three angles into elements 0..2, 4..6, 8..10 of a 4x4 matrix; the hosts already call it for the yaw-only placement rotation (`apply_scene_yaw_00467050` in `src/game_hosts_lua.cpp`) |
| `00467310` | `BSP_FormatToBuffer32` | `cdecl(out, format, ...)` = `_vsprintf_s(out, 20h, format, va)`; `PlaneStock %d` in `docs/AIROPS_LOAD_FROM_SCENE.md` |
| `004674F0` | `BSP_PodPointerArray_Reserve` | `{data, size, capacity}` reserve, `RET 4`, minimum 1, copy and `_free` of the old block |
| `00468560` | `BSP_CommandTarget_InitPosition` | `RET 8`; the moveto position descriptor of `docs/AIROPS_LAUNCH_TICK.md` |
| `00470440` | `BSP_GameplayModifiers_PowerupMultiplier` | `__thiscall(owner, kind, unit)`, `RET 8`; `FLD1` unless `00E0C978` is set and owner+88h+kind*0Ch is non-zero, then a tail jump to `BSP_GameplayModifiers_ProductForUnit` (`0047045E`) |
| `0046BF20` | `BSP_SceneEntity_RegisterStockFromTypeProperty` | `__thiscall(scene database, bag, class name)`, `RET 8`; `Type` present and class id != 47h: `BSP_SceneEntity_RegisterHiddenStockUnit(Type +0Ch)`; `src/game_hosts_scene_contents.cpp` logs this call as unimplemented `SceneGate::register_multiplayer_stock` |

`0046C450`, which the same host logs as `append_deferred_entity_record`, is a
`std::vector<void*>::push_back` instantiation (capacity check, store, else the grow path
`0046B4A0`).

## Segment 6: divergences and uncertainty

- The class roles (`AnimClass` animated scenery, `Bullets` visual for a bullet class, `BuoyClass`,
  `CloudClass`, a `VehicleClass` visual for a land-vehicle class) come from the Lua table names
  and the loaders' callers; none of the five entity types was observed in a run.
- `00BBD6C0` was read only for its role: the CloudType-0 reader passes it the mesh name, the
  texture string and the multiplier and stores the result at +28h. Calling it a particle system
  rests on the `ParticleMultiplier` key and on `00477710` feeding particle-style properties
  (`Density`, `BlurRadius`, `AffectedByWind`) to setters on the same object.
- The host loaders do not model the base fields `0048F670` fills; `read_named_class_base` is a
  hook, and the class objects are host-owned (`std::unique_ptr`) where native leaks them when the
  registry is destroyed.
- Host Lua access is an interface (`SceneClassLuaTables`/`SceneClassLuaRow`); no implementation
  against the game's Lua state exists yet, so nothing calls these functions.
- Movie action slot roles (slot 0 runs, slot 1 stops) rest on `aParticleEffect`, the only action
  with a non-trivial slot 1; who calls the slots (the movie player tick) was not traced.

## Handoff

State at the end of this packet (cc13-settings2): input items 1 done (reconstructed,
build-tested); segment 6: the five visual class families, the engine-movie action constructors
and six shared helpers named (86 ledger names, 3 of them for the input routines, applied to Ghidra with evidence comments), loaders,
two readers and two resets reconstructed in `src/scene_class_registry.cpp`. Report rows in
`reports/cc13_scene_browser.json` pass `tools/verify_report_calls.py`.

Open, in priority order:

1. The engine-movie run bodies in segment 5 (`00464350..004654D0`) and the movie parser
   `004602C0`/`0045E5E0`/`0045DE50`/`00460B80`: the action field layout (+14h, +18h, +2Ch,
   +34h..+3Ch against `Enum`, `ReferPath`, `OffsetX/Y`), the slot callers, and a reconstruction of
   the type table and factory. Needs a segment-5 lease.
2. `00477710` `BSP_CloudEntity_ApplySceneProperties`: a full reconstruction needs the x87 colour
   paths and the property-kind tests (`property+4`); the setter targets `00BBE470..00BBE5C0` are
   in segment 93.
3. Who clears the five class registries (only the Lua tables are reset by `00474200`/`00476410`
   and their siblings); `BSP_SingletonLifetime` teardown is the likely owner.
4. Entity slots of the 37h visual base (`+34h` zero velocity, `+B0h`, `+114h`) against the
   SEntity vtable docs, and the Anim entity's `00B6DC90`/+18Ch animation clock (`00471080`,
   `004710D0`).
5. The `0046F940..004702E0` vector-math cluster has no static references; it is probably reached
   through a vtable not yet tied to a class.
6. A Lua implementation of `SceneClassLuaTables` over the existing GUI/mission Lua readers would
   let the scene hosts call the loaders (`BSP_SceneCloud_RegisterClass`/`Instantiate` already
   reach `00479970` in `docs/ENTITY_IDENTITY.md`).

## Engine-movie actions: file format, parser and run bodies (segment 5)

Second packet (`cc13_settings2_movie`). Host: `include/bsp/engine_movie_actions.hpp`,
`src/engine_movie_actions.cpp`: the type table, the `Action` block reader and four run-body
decisions. Status: reconstructed, build-tested, not ABI-compatible, not wired into any host (the
movie actions drive mission cutscenes, so wiring is left to the owner of the mission flow).

### The `.ema` files

The parser `BSP_EngineMovie_ParseFile` (`004602C0`, only caller `BSP_Game_BeginEngineMovie`)
reads the engine-movie files. This installation has 39 of them under `universe\` (`*.ema`, for
example `scenes\missions\COTP-IJN\PRCPIJN\ijn_02_force_z.ema`, dated 2024-07-13 like the rest
of the bulk content). Grammar from the string pushes of `004602C0` and the files themselves:

```
Animations {
  Movielen: 50.0100
  AspectR: "1.78:1"
  Animation {
    Entitypath: "CameraLookat 01"
    PointList { Point { PointType: 1 ActivateTime: ... Comment: "" PointPos: x y z Alpha: ... } ... }
    ActionList {
      Action { Type: cSetParent ActivateTime: 1.0100 Comment: "" ReferPath : "EngineBetty2|"
               Num1 : 0.0000 OffsetX : 0.0000 OffsetY : 0.0000 OffsetZ : 0.0000 }
    }
  }
}
```

### `0045E5E0` `BSP_MovieAnimation_ReadAction`

`__thiscall`, `ECX` = owner, `[ESP+4]` = tokenizer, `[ESP+8]` not traced, `RET 8`
(`0045EC94`). Token calls in order: `expect("Action")` `0045E626`, `expect("{")`, `expect("Type")`,
`read` (the `:`), `read` (the type name, kept), `expect("ActivateTime")`, `read`, `read_float`,
`expect("Comment")`, `read`, `read` (the comment). The type name is matched against the 32 table
records (`_stricmp` at `0045E7A0`; a record with an empty name matches only a null token), and the
index, time and comment go to `BSP_MovieAction_Create` (`0045DE50`, `__stdcall`, `RET 10h`),
which allocates, constructs and copies the comment to action+8. The optional groups follow, each
gated by a flag of the type record (record offset in brackets): `ReferPath` [+10h] to +18h,
`Enum` [+48h] to +2Ch (both through `ReadNonemptyString`), `Num1`/`Num2`/`Num3`
[+1Ch/+28h/+34h] to +20h/+24h/+28h, and `OffsetX`/`OffsetY`/`OffsetZ` [+54h] to
+34h/+3Ch/+38h: the second float read is stored after the third. Then `expect("}")` and
`0045A210` appends the action to the owner. An unmatched type leaves the action pointer null,
which the field reads then dereference; the host returns no action instead.

### The type table `00460B80` (now `BSP_EngineMovie_Construct`)

Each record is built by `BSP_MovieActionType_Construct` (`0045D480`) from the name, then a label
and a flag per field (ReferPath, Num1, Num2, Num3), the Enum option list, the Enum label and flag,
and the Offset label and flag. The arguments are built right to left, so the eight string pushes
before each `00460AD0` call give the fields in reverse. In every record a field's flag is set
exactly when its label is non-empty, and the field sets used by the 39 `.ema` files agree with
that reading for every type they use:

| id | type | ReferPath | Num1 / Num2 / Num3 | Enum (label: options) | Offset | uses in `.ema` |
| --- | --- | --- | --- | --- | --- | --- |
| 0 | `sSetTarget` | Target: | | | | 0 |
| 1 | `cSetParent` | Parent: | ConvTime: | | Offset: | 34 |
| 2 | `pEngineFailure` | | | Type: Destroy\|Blown\|Kill | | 8 |
| 3 | `pForceRelease` | | | | | 7 |
| 4 | (empty) | | | | | — |
| 5 | `cUnParent` | | ConvTime: | | | 34 |
| 6 | `sForceDepthCharge` | | PlatfrmID: | | | 0 |
| 7 | `pSetGunFire` | | | On/Off: OpenFire\|HoldFire | | 0 |
| 8 | `uTogglePeriscope` | | | In/out: Out\|In | | 0 |
| 9 | `sForceArtilleryAngle` | | V.angle: / H.angle: | | | 0 |
| 10 | `sForceArtilleryTarget` | Target: | | | | 6 |
| 11 | `uForceTorpedo` | | PlatfrmID: | | | 0 |
| 12 | `aSetGunFireEnabled` | | | Weapon: All\|AA_Flak\|Artillery\|Torpedo\|DC | | 26 |
| 13 | `aSetGunFireDisabled` | | | Weapon: (same) | | 26 |
| 14 | `aSetGunFireTargetOnly` | | | On/Off: TargetOnly\|AllEnemy | | 0 |
| 15 | `aTurnGun` | | GunID: / H.Angle / V.Angle | | | 0 |
| 16 | `uSetDepth` | | | Depth: Surface\|Periscope\|Underwater\|Maxdepth | | 4 |
| 17 | `sForceAAFlak` | | PlatfrmID: | | | 1 |
| 18 | `cFOV` | | FOV: | | | 24 |
| 19 | `cBlackOut` | | Alpha: / Time: | | | 158 |
| 20 | `aParticleEffect` | | StartTime: | Name: JetEngineBig\|RealGiantSmoke\|GiantSmoke | Offset: | 1 |
| 21 | `aDont_Move` | | | Stay/Go: Stay\|Move | | 3 |
| 22 | `cFOV_Now` | | FOV: | | | 0 |
| 23 | `pSetGear` | | | Gear: Auto\|Down\|Up | | 34 |
| 24 | `cCameraShake` | | Frequency: | ShakeIt: CameraPos\|LookAtPos\|CameraAndLookAt | Max: | 10 |
| 25 | `cBinocular` | | | Action: Binocular\|Periscope\|OFF | | 30 |
| 26 | `pTakeOffHack` | | Angle: | | | 22 |
| 27 | `pWingTipEffect` | | | Effect: On\|Off | | 0 |
| 28 | `sMoveToPoint` | | | MoveTP: Enable\|Disable | | 64 |
| 29 | `cWideBlackBars` | | | Bar: Enable\|Disable | | 62 |
| 30 | `sUseCatapult` | | Dir: | | | 0 |
| 31 | `aHideEntity` | | | H/S: Hide\|Show | | 74 |

This corrects the previous section: `cBlackOut`'s Num1 is `Alpha:` and Num2 `Time:`, matching
`BSP_HudNarrativeScreen_ArmBlackoutFade(level, duration, callback)` (`005B9BA0`,
`include/bsp/mission_blackout.hpp`); a typical movie opens with `cBlackOut Num1 1.0 Num2 0.0`.

### Run bodies

The 18 run bodies Ghidra had not defined were defined from the disk listing with
`tools/ghidra_define_function.py` (record `reports/cc13_settings2_defined_functions.json`). Each
run takes the entity in `[ESP+4]` and `RET 4`; `ECX` is the action.

| type | run | effect |
| --- | --- | --- |
| `sSetTarget` | `004650F0` | when the resolved target (+14h) is set, `BSP_Entity_IssueCommand(00E08F78 for IsKindOf(6), else 00E08EF8, ..., 1)` |
| `cSetParent` | `00464870` | movie player `[00E188A8]+38h`: tracks of the entity and of the ReferPath target (`0045C690`, track+11Ch), then `00458D50(track, ConvTime, target track, offset)` |
| `pEngineFailure` | `00465D00` | `entity->vtable[194h]("explosion_delayed")`; the Enum is ignored |
| `pForceRelease` | `004648F0` | `BSP_Unit_RequestOrdnanceRelease` when plane byte +9C0h+[00F876B8]*8 is set |
| `cUnParent` | `00464350` | `00458A70(track, ConvTime)` |
| `sForceDepthCharge` | `00465180` | parts (+48h list, next +44h) with IsKindOf(1Eh) and (24h), launcher type 8 and id +38Ch == Num1 (or Num1 0): `vtable+1F0h()` |
| `pSetGunFire` | `00464910` | squadron (IsKindOf 18h): each plane (+3CCh count, +3D0h array, at most 5) byte +9B0h = OpenFire ? 1 : -1; plane (0Fh): its own |
| `uTogglePeriscope` | `004649B0` | byte +1235h = (Enum == Out) |
| `sForceArtilleryAngle`, `cFOV`, `cFOV_Now` | `00464390`, `004643D0`, `004643E0` | `RET 4`: no effect here; the camera FOV types are presumably read by the player itself (not traced) |
| `sForceArtilleryTarget` | `004643A0` | already named `BSP_FireTargetHolder_Apply` |
| `uForceTorpedo` | `00465210` | parts with IsKindOf(1Eh) and (20h): every barrel's reload timer to 0 (`BSP_Gun_SetBarrelReloadTimer`), then `vtable+1D8h(0, [00CE5444], [00CE5444])` |
| `aSetGunFireEnabled` / `Disabled` | `00465290` / `00465350` | IsKindOf(5): the disabled-category mask at unit+634h, AA_Flak 1, Artillery 2, Torpedo 4, DC 8; All clears the low byte / ORs FFh (reconstructed) |
| `aSetGunFireTargetOnly` | `00465410` | `[unit+6DCh]+59h` = (Enum == TargetOnly) |
| `aTurnGun` | `00465450` | matching parts (id == Num1 or Num1 0): `BSP_TurningGun_SetTargetAngles(Num2, Num3)` |
| `uSetDepth` | `004654D0` | already named `BSP_EntityCommand_SetSubmarineDepth` |
| `sForceAAFlak` | `00465570` | parts with IsKindOf(1Eh) and (20h), type 5 or 1, id match: `vtable+1D8h(0, 0, 0)` |
| `cBlackOut` | `004660B0` | blackout arm (level Num1, duration Num2) (reconstructed as data) |
| `aParticleEffect` | `004656D0` / `004661A0` | see the previous section |
| `aDont_Move` | `004649F0` | byte +62h = (Enum == Stay) |
| `pSetGear` | `00464A30` | Auto `007B8CE0(0,0)`, Down `(1,1)`, Up `(2,1)`; `007B8CE0` stores plane+C08h and, for a non-zero mode, plane+C1Ch = (mode == 1) (reconstructed) |
| `cCameraShake` | `00464AB0` | the Enum selects position, look-at or both; reads Num1 and the Offset triple on the player track from `0045DCA0`; read only in part |
| `cBinocular` | `00464C40` | three overlay toggles on `[00E198C4]+84h` (+20h, +6Ch, +70h, `vtable+34h`) for Binocular / Periscope / OFF |
| `pTakeOffHack` | `00464D70` | entity+64h = Num1 |
| `pWingTipEffect` | `00464D80` | byte +96Ah = (Enum == On) |
| `sMoveToPoint` | `00464DC0` | byte +748h = (Enum == Enable) |
| `cWideBlackBars` | `00464E00` | Enable `005CB0F0`, else `005CB130` |
| `sUseCatapult` | `00464E50` | session message 7Bh with Num1 * [00CE3D28] / [00CE3D20], `BSP_Session_RouteMessage(msg, 2, 0)` |
| `aHideEntity` | `00464EE0` | IsKindOf(5): byte +68h = (Enum == Hide) |

Who calls slot 0 and slot 1, and when relative to `ActivateTime`, was not traced; the movie player
tick in segment 5 (`0045xxxx`, around `0045DCA0`/`0045C690`) is the place to look.

## Cloud property pass `00477710` (second packet)

Host: `include/bsp/cloud_scene_properties.hpp`, `src/cloud_scene_properties.cpp`. Status: the branch
gate and the CloudType-0 branch are reconstructed and build-tested (ledger fragment); the two mesh
branches are only read through.

`__thiscall`, `ECX` = cloud entity (`EDI`), `RET`. After the base `0048DBD0`:

- **Gate** (`0047773C..00477780`): class CloudType (class at +188h, field +34h) non-zero or the
  instance +1A4h non-zero; the property holder +C0h present with kind (+4) 1; byte +1A8h clear.
  CloudType 0, 1 and 2 select the branches (`00477780`, `00477C3C`, `0047889A`); every path ends
  with `this->vtable[D8h]()` (`00478A2A`).
- **Property values**: `BSP_ScenePropertyBag_Find` on the holder's bag (+8) returns a record whose
  kind +4 is 0 for an integer (read with `CVTSI2SS`), 3 for a boolean byte, and otherwise a float,
  value at +0Ch. A colour property's +0Ch is a nested bag with `R`, `G`, `B` (`00CE60EC/E8/E4`);
  the colour passed on is `{R, G, B, 1.0}` (`00D7A24C`).
- **CloudType 0, particle** (`00477786..00477C37`): `InsideColor` `00BBE540`, `Ambient` `00BBE4B0`,
  `Diffuse` `00BBE4E0`, `Specular` `00BBE510`, `Density` `00BBE570`, `BlurRadius` `00BBE590`, all
  read without a null check, each with `ECX` = the instance +1A4h (`0047782D`); then optional
  `Scale` `00BBE470`, `Alpha` `00BBE490` and `AffectedByWind` `00BBE5C0` (the byte of a kind-3
  property, otherwise 0), each tested first with `004772C0`. Finally the cloud manager
  `[[00E188A8]+19E8h]+3Ch` (`00BBDDF0`): `BSP_SceneNode_SetVisibilityFactor(instance,
  (float)manager+98h byte, 0)` (`00477C13`) and `manager->00BBD470(instance)` (`00477C32`).
- **CloudType 1, shaded mesh** (`00477C3C..00478895`): `xBaseCol` (R, G, B, A; defaults of 1.0 into
  the globals `00E186C4..00E186D0` when absent), `xSkyCol`, `xSunCol`, `xScale`, `xRimPower`,
  `xBackPower`; then on the generated model's geometry the material parameters `cBaseColor`
  (`BSP_Material_RegisterFloat4ArrayParameter`), `cSkyColor`, `cSunColor` (`00B18AE0`), `cRim_Back`
  and `cBrad_Scale` (`BSP_Material_RegisterFloat2Parameter`), two shadow textures
  (`BSP_Material_SetTextureSlot`), and a vertex loop over `BSP_LogicalVertexStream_ReadColour` /
  `ReadPosition` with `00476550`; it also registers the instance with the manager.
- **CloudType 2, diffuse mesh** (`0047889A..00478A1B`): `Diffuse` (R, G, B) and `Scale` into
  `cDiffuseColor_BaranyScale`.

The first-packet summary of this function put `cDiffuseColor_BaranyScale` in the CloudType-1 list;
it belongs to CloudType 2. Host divergence: a missing mandatory property throws
`std::out_of_range` where native dereferences null.

## Handoff (second packet)

Done in the second packet: segment-5 movie parser and run bodies (18 functions defined, 31 named,
the type table, the `Action` reader and four run decisions reconstructed in
`src/engine_movie_actions.cpp`), and the gate and particle branch of `00477710`
(`src/cloud_scene_properties.cpp`). Nothing is wired into a game host.

Open, in priority order:

1. The movie player tick that calls action slot 0 and slot 1 against `ActivateTime` (segment 5,
   around `0045DCA0`/`0045C690`, owner of the `0045A210` list), and the `PointList`/`Entitypath`
   readers `0045D8A0`, `0045ECA0`, `0045EFB0`. With those, `.ema` playback could be modelled end
   to end against the 39 files in this installation.
2. Why `cFOV`, `cFOV_Now` and `sForceArtilleryAngle` have empty run bodies: the camera track
   probably reads the FOV actions directly.
3. `00477710` mesh branches (CloudType 1 and 2) and the per-vertex colour loop.
4. The open items 2..6 of the first handoff still stand (registry teardown, visual-base slots,
   the unreferenced vector-math cluster, a Lua implementation for the class loaders).

## Engine-movie playback: clock, tracks and action dispatch (third packet)

Host additions in `src/engine_movie_actions.cpp`: the clock (`0045F970`, the clock part of
`0045C5B0`) and the action dispatch (`004560E0`). Status: reconstructed, build-tested, not wired.

**Correction.** `00460B80` is the constructor of the 74h-byte movie object
(`BSP_EngineMovie_Construct`; `BSP_Game_BeginEngineMovie` allocates 74h, calls it with `ECX` = the
object and stores it at game+38h, `docs/GAME_SIMULATION_GATE.md`). Filling the global type vector
`00E18550` is part of that constructor; the earlier name `BSP_MovieActionTypes_Build` is replaced.

**Object.** Movie: +0 `Movielen`, +4 time, +8..+54h the last 20 frame deltas, +60h state (1 set by
`BSP_EngineMovie_Start` `0045DB40`, 2 half speed, 0 and 3 idle, 5 the failed-load state the start
routine tests), +68h/+6Ch the track vector. Tracks are created per `Animation` block by
`BSP_EngineMovie_CreateTrack` (`0045EFB0`): `Entitypath "CameraPosition 01"` gives a 1F4h camera track
(`0045BD00`, vtable `00CE4DCC`), anything else a 1D4h entity track (`0045B100`, vtable `00CE4DB8`).
Track vtable: +4 bind (`00456DD0` resolves `Entitypath`, a squadron to its first plane, and every
`ReferPath` into action+14h), +8 rewind (`00456290`), +0Ch update (`00459020` entity,
`0045BD90` camera). Track fields used here: +108h the action list, +11Ch the entity, +120h the
entity path (`0045D8A0`), +144h the action cursor.

**Per frame.** `BSP_Game_UpdateCutscenePlayback` (`004C6B20`) calls `BSP_EngineMovie_Advance`
(`0045F970`, `__thiscall`, float delta, `RET 4`):

1. `delta > 1.0` becomes 0.01f (`00D7A238`); NaN is kept (`COMISS`/`JBE`).
2. States 0 and 3 return. State 2 multiplies by 0.5 (`FMUL double [00D7A280]`).
3. The step is the mean of the 20 stored deltas and this one: the history is summed left to right
   through a float temporary, the delta added and the result divided by 21.0 (`00CE4D78`) on the
   x87 stack, stored as float. So the clock is smoothed over 21 frames and lags a delta change.
4. The history shifts left and takes the (scaled) delta; `time = float(time + step)`.
5. `time >= Movielen` (`FCOMIP`, `JC` at `0045FACD`; NaN keeps running) calls
   `BSP_EngineMovie_Stop` (`0045F600`); otherwise `BSP_EngineMovie_UpdateTracks` (`0045C560`,
   formerly tagged `STL_inst`) calls every track's `vtable+0Ch(time)`.

The entity track's update `00459020` moves the entity along its `PointList` (`004581E0`) or with its
parent track, then calls `BSP_MovieTrack_RunDueActions` (`004560E0`): from the cursor, while
`time > ActivateTime` (`FCOMIP`/`JBE` at `00456148`, so an action fires on the first frame strictly
after its time and NaN stops the walk), run the action's slot 0 with the entity when the entity
exists and its bytes +5Eh and +5Dh are clear (`004561A0`), and advance the cursor whether or not
it ran. An action whose entity is gone at its time is skipped for good. `BSP_EngineMovie_Rewind`
(`0045C5B0`, at the end of the parse) calls every track's rewind and zeroes time and history.

Slot 1 (`aParticleEffect`'s stop) was not found among these callers; the track or movie
destructor is the likely caller. The camera track's update `0045BD90` is not a Ghidra function yet.

## Handoff (third packet)

Open, in priority order:

1. `BSP_EngineMovie_Stop` (`0045F600`): what it tears down, in which order it calls the Lua entry
   points and the class-table resets, and whether it runs action slot 1. It reaches mission Lua
   (`BSP_Game_CallLuaEntryPoint*`), so it belongs with whoever owns mission flow (cc9) before any
   wiring.
2. The camera track (`0045BD90` update, `00457160` bind) and how it reads `cFOV`/`cFOV_Now`
   (their run bodies are empty), and the entity track's pose interpolation `00459020`/`004581E0`.
3. `00477710` mesh branches (CloudType 1 and 2), unchanged from the second handoff.
