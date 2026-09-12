# Projectile spawn, flight and impact

Addresses: 0072F830, 0072BF10, 006E8430, 006E7B00, 006E6BB0, 009555A0, 006E6490, 0070C370,
006E65C0, 006E65F0, 006E7670, 006E7760, 0084C430, 0084BF00, 0084BC60, 0098B370, 00926E80,
00926700, 009239A0, 0070C210

This packet joins `docs/UNIT_WEAPON_DEVICES.md` (gun `Fire` `00730160` reaches `0072F830`) to
`docs/UNIT_HIT_PATH.md` (`007BBCF0` `BSP_UnitInstance_OnHit` -> `008777D0`). It reuses the
projectile constructor and the three tick slots from `docs/TICK_ELEMENT_OVERRIDES.md` and does
not re-derive them.

The chain is:

```
00730160 Gun_Fire
  -> 0072F830   muzzle effects, shake, per-pellet loop, fire sound
       -> 0072BF10            one projectile
            -> classDesc->vtable[20h] = 006E8430   allocate, construct, aim
                 -> 006E7B00  BSP_ProjectileTickableEntity_Construct
            -> 009555A0 -> 006E6BB0                bind the owner
  ... fixed steps ...
  006E6750 place pose -> vtable[114h]/[118h]       position integration
  006E6490 tick advance -> vtable[11Ch]/[120h]     velocity integration
       -> 0084C430 -> 0084BF00                     segment sweep
            -> 0084BC60                            impact
                 -> 00926E80                       queue the hit record
  00926700 BSP_DeferredEntityEventQueue_Drain
       -> 009239A0 -> entity->vtable[ECh] = 007BBCF0 OnHit -> 008777D0
```

## Two corrections to existing documents

| what | was | is | evidence |
| --- | --- | --- | --- |
| `0084C430` | "the tracer", `docs/TICK_ELEMENT_OVERRIDES.md` Projectile section | the **collision sweep** for the step's segment; the tracer is not in it | `0084C430` packs six floats and tail-calls `0084BF00`, which traces the segment, queries the spatial index and calls the impact routine `0084BC60` |
| `vtable[1ACh]` as `OnHit` (packet brief) | brief said `007BBCF0` is slot `1ACh` | slot `1ACh` is `AddDamage` (`docs/UNIT_HIT_PATH.md` level 3 already says so); `007BBCF0` is slot **`ECh`** | in all nine vtables that hold `007BBCF0` the dword `0C0h` later is `0095DA00`, the `AddDamage` base; `1ACh - 0C0h = ECh`. The nine slot addresses are `00D0015C`, `00D003F4`, `00D0600C`, `00D06724`, `00D06A0C`, `00D0BB6C`, `00D19E14`, `00D1A0EC`, `00D1A3C4` |

`006E7B00` also needs one clarification: `EBX = ESI + 244h` at `006E7B3B`, so `+244h` is the
**tick-element subobject** (vtable `00CF9D78`), not a scene node, and `[ESI+23Ch] = ESI` at
`006E7B91` is the self-pointer eight bytes below it. The subobject at `+170h` (vtable `00CF9D90`,
`LEA EDI,[ESI+170h]` at `006E7B24`) is the **shot interface**; it is what `006E8430` returns and
what every downstream routine is handed.

## Spawn

### `0072F830` — fire effects and the pellet loop

`__thiscall(gun, float* muzzlePos, float* effectPos, void* a3, int flag)`, `RET 10h` at
`0073015A`, body `0072F830`-`0073015C`. Four stack arguments, so the doc's five-value shape in
`docs/UNIT_WEAPON_DEVICES.md` step 10 is right; the first is the spawn position and the second is
the effect position. `EBX = [ESP+0F0h]` at `0072F882` resolves to the second argument and
`EDI = [ESP+0ECh]` at `0072FEF0` to the first (frame: `PUSH -1 / PUSH / PUSH` then
`SUB ESP,0CCh` then four register pushes, so `ESP0 = entry - 0E8h`).

| order | site | step |
| --- | --- | --- |
| 1 | `0072F850` | team id `gun[+1ACh]`, replaced by `[gun[+3F0h]+1B0h]` when it is `8` and `gun->vtable[5Ch](21h)` holds; compared against `[00E188A8]+18ECh` to make the "local side" flag |
| 2 | `0072F89E` | muzzle point effects from `gun[+3F8h]+10h` and `+0Ch` through `BSP_PointEffect_CreateWithParentMatrix`, on an identity matrix copied from `00E1AE30` with the effect position in its translation row |
| 3 | `0072FCE8` | looped-effect start/stop `00732210` / `00731EF0`, latched in `gun[+470h]` |
| 4 | `0072FD2x` | `gun[+3F4h]+80h` in `{2,3,4,6}` seeds `[gun[+3F0h]+6F8h]` with a random; `7` seeds `+6FCh` from `00836EB0` |
| 5 | `0072FD5x` | first-person camera shake `BSP_Target_AccumulateClampedShake` when `[gun[+3F8h]+1Ch] != 0.0` |
| 6 | `0072FD80`, `0072FDA6` | the spawn flag: starts as the fourth argument; when `gun[+440h] != -1` the word `[[gun[+3B4h]+10h] + gun[+444h]*4]` replaces it with bit 1 and **suppresses the shot entirely when the word is zero** |
| 7 | `0072FDBx` | `gun[+444h] = (gun[+444h] + 1) % ([gun[+3B4h]+8h] * gun[+43Ch])` |
| 8 | `0072FE0x` | in world mode `[00E188A8]+1FE4h == 2` ten sub-types of `[gun[+3F8h]+34h]+8h` (`8`..`0Fh`, `12h`, `13h`) return without spawning |
| 9 | `0072FEEx` | `gun->vtable[1E0h]()` |
| 10 | `00730042`, `00730076` | `0072BF10(gun, muzzlePos, dir, flag)` once, or `gun[+3F4h]+0CCh` times around a cone of radius `gun[+3F4h]+0D0h` built by `BSP_Matrix_BuildLookAt` and `006FDD00` |
| 11 | `007300E4` | `FUN_007290D0(gun, gun[+4D0h])` then the fire sound `0077C7B0` |

### `0072BF10` — one projectile

`__thiscall(gun, float* pos, float* dir, int flag)`, `RET 0Ch` at `0072C160`, body
`0072BF10`-`0072C162`.

| order | site | step |
| --- | --- | --- |
| 1 | `0072BF18`-`0072BFAF` | when `gun[+3FCh] == 0`, the camera unit `00E188D8` passes `vtable[5Ch](6)` and `FUN_004193E0()` is below `00CFE050`, and it is not the gun's own owner, the sixth factory argument becomes `1` (a near-camera shot) |
| 2 | `0072C006` | `classDesc->vtable[20h](0, [gun[+3F0h]+54h], pos, dir, flag, nearCamera, gun[+41Ch])` — **seven** stack pushes (`0072BFE3`, `0072BFE8`, `0072BFEA`, `0072BFEF`, `0072BFF4`, `0072C003`, `0072C004`), and the caller never adjusts `ESP` afterwards, so the callee pops all `1Ch` |
| 3 | `0072C013`, `0072C025` | `projectile+58h = gun[+58h]`, `projectile+54h = [gun[+3F0h]+54h]`, both through the self-pointer `[shot+0CCh]` |
| 4 | `0072C033` | `gun->vtable[1E4h](shot)` |
| 5 | `0072C043` | `gun[+474h] = [00F876A4]`, the fire timestamp |
| 6 | `0072C064` | when `[gun[+3F4h]+95h]`: `owner->vtable[34h]()` returns a float3 that is stored at `shot+48h..50h` **and added into the velocity at `shot+8h..10h`** — the platform's own velocity |
| 7 | `0072C0AE` | when `gun[+43Ch] != 1`: `shot->vtable[20h]((float)gun[+43Ch])` |
| 8 | `0072C0F0` | `shot->vtable[34h](&{1,1,1}, &{0,0,0})` |
| 9 | `0072C0FB`, `0072C14B` | `shot+1Ch = gun[+1ACh]`; when that is `8` and `gun->vtable[5Ch](21h)`, it becomes `[owner[+9D4h][+3D0h]+1B0h]` (or the owner's own `+1B0h`) |
| 10 | `0072C155` | `009555A0(owner, shot)` |

### `006E8430` — the factory ("MBullet")

The class descriptor's `vtable[20h]`. `00CFA158` is the last slot of the vtable
`00CFA138`-`00CFA158`, and the string `MBullet` follows it at `00CFA15C`.
Body `006E8430`-`006E86E4`, SEH frame.

| order | site | step |
| --- | --- | --- |
| 1 | `006E8471` | `operator_new(284h)` then `memset(.., 0, 284h)` |
| 2 | `006E8478` | `BSP_ProjectileTickableEntity_Construct` = `006E7B00` |
| 3 | — | `projectile+174h = classDesc` |
| 4 | — | `FUN_00923870(0, [00E188A8]+19CCh, &matrix)` with a scale-1 matrix template |
| 5 | — | `projectile+54h` = the owner id; `+A4h..+ACh`, `+FCh..+104h`, `+1D0h..+1D8h` and `+1DCh..+1E4h` all take the spawn position |
| 6 | — | velocity: `v = classDesc[+50h] * dir`, written to both `+178h..+180h` and `+94h..+9Ch` |
| 7 | — | `FUN_0085DC80()`, then the child-node chain `[proj+48h]`/`+44h` through `BSP_SceneNode_InvalidateSubtreePose`, then `BSP_EntityPose_RefreshWorld` |
| 8 | — | `BSP_GameWorld_SampleWaterHeight(pos.x, pos.z)`: when the spawn point is at or below the water surface the shot interface's `vtable[28h](0)` runs, otherwise `vtable[24h](0)` — the water/air mode is decided **at spawn** |
| 9 | — | `projectile+1C8h = 0`, `+1CCh` = the gun's spawn flag, `+278h` = the near-camera flag, and on the near-camera flag `shot->vtable[3Ch]()` |
| 10 | `006E86Dx` | returns `projectile + 170h`, the shot interface |

The launch velocity's vertical component carries a one-step bias:

```
v.y = classDesc[+50h] * dir.y
if (classDesc[+8h] in {4, 5, 6, 7})
    v.y -= 0.05f * 9.8103800773621   // 00D0DE84 (float) * 00CF9058 (double)
```

`classDesc[+8h]` is the weapon sub-type that `docs/UNIT_WEAPON_DEVICES.md` leaves undecoded; the
values the code separates here are `4`, `5`, `6`, `7` (biased), `10h` (a decal variant in
`0084BC60`), `11h` (a friendly-fire exemption in `0084BF00`) and the ten values `0072F830` step 8
suppresses.

### `009555A0` and `006E6BB0` — registration

`009555A0` is `__thiscall(owner, shot)`, body `009555A0`-`00955628`.

| order | site | step |
| --- | --- | --- |
| 1 | `009555AF` | `owner->vtable[5Ch](0Fh)`, result unused |
| 2 | `009555B8` | `006E6BB0(shot, owner)` |
| 3 | `009555CB` | `shot+14h = (owner+170h)->vtable[0]()` |
| 4 | `009555D3` | `shot+18h = [owner+54h]` |
| 5 | `009555E0`, `00955607` | `director = owner->vtable[114h]()`, falling back to `owner[+9D4h]->vtable[114h]()` when the owner passes `vtable[5Ch](0Fh)`. `unit->vtable[114h]` is `0080E150` per `docs/UNIT_WEAPON_DEVICES.md`, so this is the weapon director at `unit+738h` |
| 6 | `0095561B` | `[director + 1D0h + classDesc[+8h]*4] = [00F876A4]` — a per-sub-type "last fired" timestamp |

`006E6BB0(shot, owner)`, body `006E6BB0`-`006E6C0x`: when `[projectile+5Dh]` is clear and the
owner is non-null it calls `FUN_006E1B90(owner)`, then unregisters the previous observer pair,
stores `projectile+238h = owner` and registers a new pair, and writes `projectile+204h` from
`00CE3854`. There is **no projectile list**: the shot reaches the fixed step only through the
tick registry group 0 that `006E7B00` joins, and reaches the owner only through this pointer.

## The projectile record

`006E8430` allocates `284h` bytes and zeroes them, so every offset below starts from zero.
Offsets are on the base object; the shot interface a caller holds is `base + 170h` and the tick
element the fixed step drives is `base + 244h`.

| offset | type | writer | meaning |
| --- | --- | --- | --- |
| `+0h` | vptr | `006E7B67` | `00CF9DF0`, the primary vtable (`114h`, `118h`, `11Ch`, `120h` are the motion slots) |
| `+10h` | vptr | `006E7B6D` | `00CF9DD8` |
| `+24h` | vptr | `006E7B74` | `00CF9DD0` |
| `+44h` | ptr | base ctor | sibling link of a child scene node |
| `+48h` | ptr | base ctor | first child scene node; `006E7670` walks `+44h` from here |
| `+54h` | dword | `006E8430`, `0072C025` | owner entity id, `[owner+54h]` |
| `+58h` | dword | `0072C013` | firing gun id, `[gun+58h]` |
| `+5Ch` | byte | unread | gates the per-step collision sweep (`006E64EF`) |
| `+5Dh` | byte | unread | gates re-binding in `006E6BB0` |
| `+94h`..`+9Ch` | float3 | `006E8430` | launch velocity, a second copy |
| `+A4h`..`+ACh` | float3 | `006E8430`, `006E7670`, `006E7760` | local position; the integration writes here |
| `+C4h` | int | `006E7B87` | class id `29h` |
| `+C8h` | byte | `006E8430`, `006E7670`, `0084BC60` | world-pose-valid flag |
| `+FCh`..`+104h` | float3 | `006E8430`, `0084BC60` | world translation; the segment's end point |
| `+10Ch` | byte | `006E8430`, `006E7670` | second pose flag, always cleared with `+C8h` |
| `+170h` | vptr | `006E7B7B` | `00CF9D90`, the shot interface subobject |
| `+174h` | ptr | `006E8430` | class descriptor |
| `+178h`..`+180h` | float3 | `006E8430`, `006E65C0`, `006E65F0` | velocity |
| `+184h` | dword | `009555CB` | owner sub-object id |
| `+188h` | dword | `009555D3` | `[owner+54h]` |
| `+18Ch` | dword | `0072C0FB`, `0072C14B` | team id |
| `+1B8h`..`+1C0h` | float3 | `0072C068` | platform velocity added into the launch velocity |
| `+1C4h` | float | `006E6490` | accumulated flight time |
| `+1C8h` | dword | `006E8430` | zero at spawn; the last collision argument |
| `+1CCh` | byte | `006E8430` | the gun's spawn flag from `0072F830` step 6 |
| `+1D0h`..`+1D8h` | float3 | `006E8430`, `006E7D50` | current pose snapshot |
| `+1DCh`..`+1E4h` | float3 | `006E8430`, `006E7D50` | previous pose snapshot; the segment's start point |
| `+204h` | dword | `006E6BB0` | `[00CE3854]` |
| `+238h` | ptr | `006E6BB0` | owner entity |
| `+23Ch` | ptr | `006E7B91` | self-pointer, how `+170h` and `+244h` reach the base |
| `+244h` | vptr | `006E7B81` | `00CF9D78`, the tick element (flak: `00CFD554`) |
| `+278h` | byte | `006E8430` | near-camera flag; provisional, Ghidra reuses a parameter name there |
| `+27Ch` | float | `006E7B9C` | a per-shot random in `[0, 0.1f]` (`00D7A2F0`) |
| `+294h` | dword | `0070C3C1` | cleared when a flak round detonates |

`+1B8h`..`+1C0h` is read from the interface offsets `+48h`..`+50h` in `0072BF10`; the base offsets
follow from the `170h` shift and are not confirmed by a second writer.

## Flight

One fixed step runs three slots of the tick element `00CF9D78`, all with
`scaled = step * classDesc[+5Ch]`.

### Position, `vtable[114h]` and `[118h]`

Neither has a Ghidra function; both were read from the raw listing. The motion-mode query
`shot->vtable[2Ch]()` picks between them (`006E6750`, already documented).

`006E7670` (`006E7670`-`006E7756`, `__thiscall(projectile, float scaled)`, `RET 4`), taken when
the query is **false**:

```
projectile+C8h = 0 ; projectile+10Ch = 0
for (n = [projectile+48h]; n; n = [n+44h])  0042ED50(n)      // invalidate the subtree pose
pos = snapshot_current                                        // +1D0h..+1D8h
d.x = v.x * scaled
d.y = v.y * scaled
d.z = v.z * scaled
if (classDesc[+20h] == 0)                                     // gravity enabled
    d.y -= (scaled * g) * scaled * 0.5
pos += d ; store to +A4h..+ACh
```

`006E7760` (`006E7760`-`006E7812`) is the same routine with the gravity term and the ordering of
the node walk changed: it has **no** `classDesc[+20h]` test and no half-step term, so the
alternate mode moves in a straight line.

### Velocity, `vtable[11Ch]` and `[120h]`

`006E65C0` (`006E65C0`-`006E65E4`, `RET 4`), query **false**:

```
if (classDesc[+20h] == 0)
    v.y -= scaled * g
```

`006E65F0` (`006E65F0`-`006E666C`, `RET 4`), query **true**:

```
v.x += (-v.x) * scaled
v.y += (-v.y) * scaled
v.z += (-v.z) * scaled
```

which is `v *= (1 - scaled)`: a linear drag with a unit coefficient and no tunable constant.
Together with the straight-line position slot this is the underwater or drag mode; the mode is
chosen once at spawn by the water-height test in `006E8430` step 8.

### Constants

| address | width | value | used by |
| --- | --- | --- | --- |
| `00CF9058` | double | `9.8103800773621` | `006E65C0`, `006E7670`, `006E8430` |
| `00D7A280` | double | `0.5` | `006E7670` |
| `00D0DE84` | float | `0.05` | `006E8430` (the launch bias), the fixed step |
| `00D0B9C0` | double | `1.0e-7` | `0084BF00`, the minimum squared segment length |
| `00D7A2F0` | float | `0.1` | `006E7B97`, the per-shot random range |
| `00D7A218` | float | `0.0` | `0070C3F5`, `0072C130` |

### Expiry, and the flak variant

`006E6490` ends with `flight_time > classDesc[+54h]` -> `00696350(projectile, 0)` (`EDX` zeroed,
so `__fastcall`) then `00926D90(projectile, 2)`.

`0070C370` has no Ghidra function; it was read from the raw listing (`0070C370`-`0070C7AA`,
`__thiscall(tickElement, float step)`, `RET 4` at `0070C3D7` and `0070C7AA`). It is the flak
`vtable[8h]` of `00CFD554` and it **calls `006E6490` first** (`0070C387`), so everything above
still applies, then:

| order | site | step |
| --- | --- | --- |
| 1 | `0070C387` | `006E6490(step)` |
| 2 | `0070C3B5` | `flight_time > classDesc[+54h]`: clear `projectile+294h` and detonate through `0070C210(projectile, 0)`, then return |
| 3 | `0070C3E6` | otherwise require `flight_time > classDesc[+D8h]` (the fuse arming time) and `scaled > 0` |
| 4 | `0070C41C` | segment = world translation minus the previous snapshot, length from `0042B2F0`, then normalised |
| 5 | `0070C52A`..`0070C680` | `008053C0`, four virtual calls, `00427EB0` and `00901C20`: the proximity search, `contract: unread` |
| 6 | `0070C79B` | `0070C210(projectile, ...)` on a proximity hit |

`0070C210` is the flak detonation and is `contract: unread`.

## Collision and impact

### `0084C430` and `0084BF00`

`0084C430` (`0084C430`-`0084C4D5`, `RET 2Ch`) only copies the two by-value vectors into one
`float[6]` and tail-calls `0084BF00`; `CL` and `DL` pass through. `006E6490` supplies
`CL = ([00E188A8]+1FE4h != 2)` and `DL = 1`.

`0084BF00` is `__fastcall(char replayFlag, uint flags, float segment[6], void* shot,
void* owner, void* classDesc, void* extra)`, body `0084BF00`-`0084C429`, SEH frame. The `70h`-byte
staging buffer it zeroes at `0084BF8x` is the thing every later step reads.

| offset | writer | meaning |
| --- | --- | --- |
| `+0h` | `0084BF00` | the shot interface, `projectile+170h` |
| `+4h` | `0084BF00` | the owner, `projectile+238h` |
| `+8h` | `0084BF00` | the class descriptor, `projectile+174h` |
| `+10h`..`+18h` | `0084BF00` | the segment direction, normalised |
| `+1Ch`..`+70h` | `0098B370` | **the hit record**, `54h` bytes; see below |

| order | site | step |
| --- | --- | --- |
| 1 | — | `delta = end - start`; `BSP_Vector3f_Length`; the reciprocal when positive, else `0`, giving the unit direction at `+10h` |
| 2 | — | abort unless `|delta|^2 > 1.0e-7` (`00D0B9C0`) |
| 3 | — | terrain/water trace: `shot->vtable[2Ch]()` and `classDesc[+74h]` choose `0084B380` or `0084B4B0`; both return a `float[4]` whose `+0Ch` byte is the hit flag and whose first three floats are the hit point |
| 4 | — | `FUN_00BD2F10(classDesc[+ACh], classDesc[+B0h])` then `FUN_00428420(t)` — a random in a per-class range |
| 5 | — | `BSP_SpatialIndex_GetSingleton()` then `0098B370(index, segment, owner->vtable[B0h](), &buffer[1Ch])` — the entity sweep, which fills the hit record |
| 6 | — | the entity hit is discarded when `classDesc[+8h] == 11h` and the hit entity shares the owner's `+9D4h` parent (a friendly-fire exemption) |
| 7 | — | `[00E188A8]+1FE4h == 2` plus `shot`'s base passing `vtable[5Ch](2Ah)` sets a replay override that forces the static path |
| 8 | — | entity hit: `0084AFD0` when `CL` is set, then `0084BC60(flags, buffer, 1, extra)` |
| 9 | — | no entity but a terrain hit: `007BC4E0` when the owner branch applies, then `0084BC60(flags, buffer, 2, extra)` if `shot->vtable[2Ch]()` is zero and `vtable[30h]()` is true, otherwise `shot->vtable[24h or 28h](1)` — the water/air mode switch, which is how a shell that crosses the surface keeps flying |

`0084BF00` has a second caller, `007C0910`, and its `[shot+0CCh]->vtable[5Ch](0Fh)` branch
(`local_94`, `piVar11[+306h]`, `piVar11[+2FDh]`) is for that caller: for a projectile
`[shot+0CCh]` is the projectile itself, whose class id is `29h`, so the test fails. That branch is
`contract: other caller unread`.

### `0084BC60` — the impact

`__fastcall`, body `0084BC60`-`0084BEFC`, SEH frame; `ESI` is the staging buffer.

| order | site | step |
| --- | --- | --- |
| 1 | `0084BC8x` | mode `1` is refined to `3` when the hit entity passes `vtable[5Ch](44h)` and to `4` when it passes `(0Fh)`; mode `2` stays |
| 2 | `0084BCBx` | **the projectile is teleported to the impact point**: through `[buffer+0h]+0CCh` it sets `projectile+C8h = 1`, `+10Ch = 0` and `+FCh..+104h` = the hit position |
| 3 | `0084BCFx` | in world mode `1`, and when the projectile passes `vtable[5Ch](2Ah)`: `0084B650()`, then `[..+310h]->vtable[2Ch](&hitPos, extra)`, `0084B000(mode, ..)` and `BSP_Session_RouteMessage(msg, 4, 0)` — the network hit message |
| 4 | `0084BDAx` | when `[buffer[0]+45h]`: `shot->vtable[2Ch](&hitPos, extra)` then `0084B8C0` — the decal |
| 5 | `0084BE00` | `BSP_MissionEntity_Kill(1)` — **the projectile removes itself here**, not on a timer |
| 6 | `0084BE20` | when the hit entity `[buffer+1Ch]` is non-null: `00926E80(&buffer[1Ch], &buffer[10h])` queues the hit record with the impact direction |
| 7 | `0084BE32` | when `classDesc[+6Ch]`: the explosion at `hitPos - direction * [00D7A270]`, radius randomised from `classDesc[+B4h]`/`[+B8h]`, through `0084BAD0(&args, classDesc[+8h] == 10h, owner, buffer[+20h])` |

Steps 3, 4 and 7 are contracts: `BSP_Session_RouteMessage` is the session message,
`0084B8C0` the decal placement and `0084BAD0` the explosion and its effects; none is read here.

### The deferred queue

`00926E80(record, direction)` is `__fastcall`, body `00926E80`-`00926F99`. It takes the critical
section at `[FUN_00924990()+4h]`, appends a node to the `std::list` at `00F899C4`, copies the
record into `node+8h` with `00925050`, writes the direction to `node+5Ch`..`node+64h` and calls
`[record+4h]->vtable[5Ch](29h)`. The copy target and the direction offset together bound the
record at `54h` bytes.

`BSP_DeferredEntityEventQueue_Drain` (`00926700`, already in the ledger, called twice per fixed
step by `00875BB0`) pops the front node, requires the subject `[node+8h]` to have `+5Eh` and
`+5Fh` clear, and calls `009239A0(subject, node+8h, node+5Ch)`.

### `009239A0` — the dispatcher

`__thiscall(entity, record, direction*)`, body `009239A0`-`00923AE7`. **This is the producer of
the `007BBCF0` call `docs/UNIT_HIT_PATH.md` lists as unread.**

| order | site | step |
| --- | --- | --- |
| 1 | `009239Bx` | require `[entity+5Eh]` and `[entity+5Fh]` clear |
| 2 | `009239D4`, `009239E3` | the shot `[record+4h]` must pass `vtable[5Ch](29h)` (a projectile) or `(2Ah)` |
| 3 | `009239F6`, `00923A0D` | `source = shot[+174h]` for `29h` (the class descriptor) or `shot[+314h]` for `2Ah` |
| 4 | `00923A2C` | when the hit entity passes `vtable[5Ch](44h)` and `source[+0CCh] >= 0`: `entity->vtable[24h](source[+0CCh], &record[+8h], direction)` — the scoring/attribution hook |
| 5 | `00923A70` | `while (entity->vtable[ECh](record) == 0) entity = entity[+3Ch]` — **`vtable[ECh]` is `007BBCF0` `BSP_UnitInstance_OnHit`**; the hit walks up the parent chain until one handler accepts it, and returns when the chain runs out |
| 6 | `00923A7D` | then walk up again while the entity is not `vtable[5Ch](0Fh)` and its parent is not `(44h)` |
| 7 | `00923Ax` | when the entity passes `vtable[5Ch](2)`: `0077CE60(record)` |

### The hit record, producer side

Base is staging buffer `+1Ch`; `0098B370` fills it. `0098B370` (`0098B370`-`0098B446`) is a thin
multi-segment loop over `0098ADD0`, which walks the spatial index's AABBs; the narrowphase that
writes the part and hull fields is `contract: unread`, so rule 4 of
`docs/WORKER_VERIFICATION_CHECKLIST.md` is satisfied only for the first five rows.

| offset | writer | meaning |
| --- | --- | --- |
| `+0h` | `0098B370` chain | the hit entity; the drain's subject and `009239A0`'s `this` |
| `+4h` | `0098B370` chain | the shot: the projectile base, `IsKindOf(29h)`. `docs/UNIT_HIT_PATH.md` reads `vtable[58h]()` and `vtable[108h]()` on it from the consumer side |
| `+8h`..`+10h` | `0098B370` chain, or `0084BF00` on the static path | the hit position |
| `+0Ch` | same | this is the hit position's **y**, which is why `008777F8` treats a negative value as "below the surface" and selects the alternate armour |
| `+14h`, `+24h`, `+28h`, `+2Ch`, `+34h`, `+38h`, `+3Ch`, `+40h` | narrowphase, unread | the damage and part fields of `docs/UNIT_HIT_PATH.md` |
| `+48h` | `008778BF`, `00877A24` | written back by the consumer |
| `+54h` | `00926F48` (in the queued copy only) | the impact direction, appended by `00926E80` |

## Host table

One row per native call site the reconstruction models.

| site | callee | host method | this / arguments | ret | gate |
| --- | --- | --- | --- | --- | --- |
| `0072C006` | `classDesc->vtable[20h]` (`006E8430`) | `create_projectile` | classDesc; `0`, ownerId, pos, dir, flag, nearCamera, `gun[+41Ch]` | shot interface | always |
| `0072C033` | `gun->vtable[1E4h]` | `gun_on_projectile_created` | gun; shot | — | always |
| `0072C064` | `owner->vtable[34h]` | `owner_velocity` | owner; out buffer | `float*` | `[gun[+3F4h]+95h] != 0` |
| `0072C0AE` | `shot->vtable[20h]` | `shot_set_scale` | shot; `(float)gun[+43Ch]` | — | `gun[+43Ch] != 1` |
| `0072C0F0` | `shot->vtable[34h]` | `shot_set_colour` | shot; `{1,1,1}`, `{0,0,0}` | — | always |
| `0072C155` | `009555A0` | `register_projectile` | owner; shot | — | always |
| `009555B8` | `006E6BB0` | `bind_owner` | shot; owner | — | always |
| `009555E0` | `owner->vtable[114h]` (`0080E150`) | `weapon_director` | owner | director | always |
| `006E8430` step 8 | `BSP_GameWorld_SampleWaterHeight` | `water_height` | `pos.x`, `pos.z` | float | always |
| `006E8430` step 8 | `shot->vtable[24h]` / `[28h]` | `set_medium` | shot; `0` | — | air vs water |
| `006E7694` | `0042ED50` | `invalidate_node_pose` | node | — | per child node |
| `006E652D` | `00414DB0` | `refresh_world_pose` | projectile | — | `[proj+C8h] == 0` |
| `006E657F` | `0084C430` | `sweep_segment` | — ; previous, current, shot, owner, classDesc, `00F87574`, `proj+1C8h`; `CL`, `DL` | float | `[proj+5Ch] != 0` |
| `006E659F` | `00696350` | `expire_projectile` | projectile; `0` | — | `flightTime > classDesc[+54h]` |
| `006E65A9` | `00926D90` | `release_projectile` | projectile; `2` | — | same |
| `0084BF00` step 1 | `BSP_Vector3f_Length` | `segment_length` | delta | float | always |
| `0084BF00` step 3 | `0084B380` / `0084B4B0` | `trace_static` | segment | `float[4]` | always |
| `0084BF00` step 5 | `0098B370` | `sweep_entities` | index; segment, mask, record | bool | always |
| `0084BF00` step 5 | `owner->vtable[B0h]` | `owner_collision_mask` | owner | mask | owner non-null |
| `0084BF00` step 9 | `shot->vtable[24h]` / `[28h]` | `set_medium` | shot; `1` | — | surface crossing |
| `0084BC60` step 3 | `BSP_Session_RouteMessage` | `route_hit_message` | message, `4`, `0` | — | world mode `1` and `IsKindOf(2Ah)` |
| `0084BC60` step 4 | `0084B8C0` | `place_decal` | decal, hitPos, extra | — | `[shot+45h] != 0` |
| `0084BC60` step 5 | `BSP_MissionEntity_Kill` | `kill_projectile` | `1` | — | mode `1` |
| `0084BC60` step 6 | `00926E80` | `queue_hit` | record, direction | — | hit entity non-null |
| `0084BC60` step 7 | `0084BAD0` | `spawn_explosion` | args, `classDesc[+8h] == 10h`, owner, `record[+20h]` | — | `classDesc[+6Ch] != 0` |
| `00926F65` | `[record+4h]->vtable[5Ch]` | `shot_is_projectile` | shot; `29h` | bool | always |
| `00923A2C` | `entity->vtable[24h]` | `credit_hit` | entity; sourceId, position, direction | — | `IsKindOf(44h)` and `source[+CCh] >= 0` |
| `00923A70` | `entity->vtable[ECh]` (`007BBCF0`) | `deliver_hit` | entity; record | bool | loop over the parent chain |
| `00923A7D` | `0077CE60` | `notify_hit_listener` | record | — | `IsKindOf(2)` |

## Coverage

| routine | coverage |
| --- | --- |
| `0072F830` | complete for the spawn path; the point-effect and shake calls are contracts |
| `0072BF10` | complete |
| `006E8430` | complete; the trailing two of seven factory arguments are inferred from the push order |
| `006E7B00`, `006E6BB0`, `009555A0` | complete |
| `006E6490`, `006E6750`, `006E7D50` | reused from `docs/TICK_ELEMENT_OVERRIDES.md` |
| `006E65C0`, `006E65F0`, `006E7670`, `006E7760` | complete, raw listing |
| `0070C370` | partial: `0070C370`-`0070C3E6` and the tail at `0070C79B`; the proximity search `0070C4A4`-`0070C795` is `contract: unread` |
| `0070C210` | `contract: unread` |
| `0084C430` | complete |
| `0084BF00` | complete for the projectile caller; the `007C0910` branch is `contract: other caller unread` |
| `0084BC60` | complete; five effect and message callees are contracts |
| `00926E80`, `00926700`, `009239A0` | complete |
| `0098B370` | complete; `0098ADD0` broadphase read, narrowphase `contract: unread` |
| `0084B380`, `0084B4B0`, `0084B8C0`, `0084BAD0`, `0084B000`, `0084AFD0`, `007BC4E0` | `contract: unread` |

## Correction from docs/HIT_NARROWPHASE.md (packet cc2_hit_narrowphase)

The third argument of the segment query `0098ADD0` that `0084BF00` passes is the **exclude
entity** (compared with `!=` against each candidate's owner), not a collision mask; the query is
`__thiscall(index; from, to, excludeEntity, record, kindFilter)`, RET 14h, over a 150x150 grid at
`index+84h` and a loose array at `index+8h`. The bytes `projectile+5Ch` / `+5Dh` are the scene-node
base's active and torn-down flags (written by `00922F30`, `00922F80`, `00922FD0`, `009263C0`,
`00925F20`, `009274CE`), not projectile fields. A segment hit carries hull damage only: the record's
damage `+28h`, centre and radius `+24h` come from the blast path `00904470` / `0084BAD0`.

## Corrections from docs/PROJECTILE_KINDS.md (packet cc2_projectile_kinds)

The spawn table above mislabels two of `006E8430`'s seven arguments: arguments 5 and 6 land in
`+278h` and `+1CCh` respectively (the reverse of the reading above), the spawn hook fires on
argument 5, and arguments 1 and 7 are never read. The descriptor's `+74h` is `ExplWaterHit` and
`+CCh`/`+D0h` are the two decal ids (provisional: the consumer at `00923A2C` was not re-read).
The sub-type at descriptor `+8h` is set by the class constructors, not by Lua: 1 Bullet,
4 Artillery, 9 Bomb, 0Ah Torpedo, 0Bh DepthCharge, 0Ch DummyTarget, 0Dh DummyKamikazePlane,
0Eh DummySubmarine, 0Fh Paratrooper, 10h Flak, 11h Kamikaze, 12h Rocket, 13h WaterMine.

## Correction from docs/PROJECTILE_HELPERS.md (packet cc2_projectile_helpers)

- **Was:** __fastcall(char replayFlag, uint flags, float segment[6], shot, owner, classDesc, extra), five stack arguments (docs/PROJECTILE_IMPACT.md, copied from Ghidra's signature)
  **Is:** six stack arguments: the segment list, shot, owner, classDesc, a const float3 direction and a sixth pointer
  **Evidence:** RET 0x18 at 0084c427 is six dwords. The prologue reads six slots: 0084bf71 [ESP+0DCh], then at push depth 0Ch 0084bfca [ESP+0ECh], 0084bfd1 [ESP+0F0h] and 0084bfd8 [ESP+0F4h], which are base+0E0h, +0E4h and +0E8h. 0084c430 pushes five forwarded dwords plus the buffer (0084c446, 0084c44e, 0084c456, 0084c47c, 0084c4a2, 0084c4b6) and 007c0910 passes six. The fifth is 00F87574 for a projectile and the sixth projectile+1C8h.
- **Was:** the third argument is a float[6] holding one segment
  **Is:** a 0C4h-byte segment list: eight {float3 from; float3 to} records at +0h and the count at +0C0h
  **Evidence:** 0084c430 does SUB ESP,0xc4, writes six floats at base+0h..14h and MOV [ESP+0xd8],1 at 0084c4bd, which is base+0C0h after six pushes. 0084b388 reads [EDI+0C0h] as the count and 0084b38e scales it by 18h. 0C0h/18h = 8. 007c0910's local_c4 is the same 0C0h block, filled with one segment per collision point.
- **Was:** step 3 [..+310h]->vtable[2Ch](&hitPos, extra) and step 4 shot->vtable[2Ch](&hitPos, extra)
  **Is:** slot 2Ch takes no arguments; the two dwords pushed before each call are the following helper's
  **Evidence:** 0084b000 is RET 10h, four stack arguments, but only PUSH EAX and PUSH EBP follow the virtual call at 0084bd81. 0084b8c0 is RET 0Ch, three, but only PUSH EAX follows the call at 0084bde2. The pushes at 0084bd67/0084bd77 and 0084bdc8/0084bdd8 make up the difference. The three other call sites of slot 2Ch, 0084c12d, 0084c364 and 0084c3e1, push nothing at all.
- **Was:** place_decal, the decal placement (docs/PROJECTILE_IMPACT.md host table and coverage row)
  **Is:** the impact-effect dispatch: a switch on (mode, medium) over six class-descriptor effect slots
  **Evidence:** Every arm of 0084b8c0 calls 0084b6f0, which builds a matrix and calls BSP_PointEffect_CreateFromMatrix on the effect manager [00E188A8]+19ECh. No decal call exists anywhere in 0084b6f0's body.
- **Was:** body 0070C370-0070C7AA
  **Is:** body 0070C370-0070CAD3
  **Evidence:** 0070c3e6 and 0070c400 jump to 0070c808, past the RET 4 at 0070c7aa; 0070c7c1 and 0070c7de jump there too. 0070c89b calls 0084c430 with the same eleven-dword shape as 006e657f. The last RET 4 is at 0070cad1 and int3 padding follows to 0070cadf.
- **Was:** step 5 lists 008053C0, four virtual calls, 00427EB0 and 00901C20 without attributing arguments
  **Is:** the four dwords pushed at 0070c656-0070c66e belong to 00901C20, not 00427EB0
  **Evidence:** 00427eb0 BSP_EntityPose_GetWorldPositionRefreshed is __fastcall(ECX) with a plain RET at 00427ec8. 00901c20 is RET 10h at 0090227e, and its SUB ESP,0xcc / COMISS XMM0,[ESP+0xd0] at 00901c2e reads the first of the four, the muzzle speed classDesc[+50h].
- **Was:** kScoringTarget = 3 and kUnit = 4
  **Is:** mode 3 is a Landscape hit and mode 4 a plane hit
  **Evidence:** 0084bc99 pushes 0x44 and 0084bcb0 pushes 0x0f to the hit entity's vtable[5Ch]. docs/ENTITY_CLASS_IDS.md maps class id 44h to Landscape and 0Fh to the plane base. The effect table confirms it: mode 3 selects the terrain slot d+38h and mode 4 the aircraft slot d+3Ch.
- **Was:** 0x54, the owner id
  **Is:** 0x54, the owner's party index
  **Evidence:** 0070c512 loads [proj+54h] into ECX for BSP_Recon_EnsureSlot, whose table at 00F874BC has exactly three slots (docs/FIXED_STEP_COUNTDOWN.md). docs/BOT_FIRE_TARGET.md names the same field on a unit the owner's party index at 009f5d30.
- **Was:** the [shot+0CCh] IsKindOf(0Fh) branch is contract: other caller unread
  **Is:** the caller is 007C0910, the plane's swept collision test, and the branch binds the plane's crash effect
  **Evidence:** 007c0910 passes p+72Ch as the shot, so [shot+0CCh] is the plane, whose class id is 0Fh, the plane base. 0084bf3f and 0084bf66 set EBP to that object and 0084c34b calls 007bc4e0 on it.

## Correction from docs/KILL_CREDIT.md (packet cc2_kill_credit)

- **Was:** the projectile is 284h bytes
  **Is:** the base class is 284h; the flak subclass allocates and zero-fills 298h, which is what makes +290h and +294h legal
  **Evidence:** 0070cc4e PUSH 0x298 into 00bf55be and 0070cc5c..0070cc67 memset(obj, 0, 0x298); 006e844b PUSH 0x284 for the base
