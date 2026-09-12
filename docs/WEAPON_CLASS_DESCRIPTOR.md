# The projectile class descriptor and its Lua field reader

Addresses: 006EA910, 006E8320, 006E8770, 006E1BE0, 008566B0, 006FD400, 00809CB0, 0070C030,
006E9E80, 006E86F0, 006E9B70, 007327B0, 00470B80

This packet reads the object the gun reaches as `gun[+3F8h][+34h]`: the **projectile ("bullet")
class descriptor**. `docs/PROJECTILE_IMPACT.md` established it as the holder of the sub-type at
`+8h`, the gravity flag at `+20h`, the muzzle speed at `+50h` and the blast fields, and called its
`vtable[20h]` the spawn factory. This document adds the constructor, the registry that builds one
descriptor per Lua row, the complete key table of its field reader, and the meaning of `+8h`.
`docs/PROJECTILE_KINDS.md` carries the class family and the per-kind behaviour.

## It is not the gun's class descriptor

The packet brief and `docs/GUN_AIMING.md` place rotation rates at `+88h`/`+8Ch` "on the
descriptor". Those belong to a different object.

| what | was | is | evidence |
| --- | --- | --- | --- |
| `+88h`/`+8Ch` rotation rates | read as fields of the weapon class descriptor | fields of the **gun class** descriptor `gun[+3F4h]`, written by its own reader `007327B0` from `HorzRotSpeed` (`00732836`) and `VertRotSpeed` (`0073284C`) | on the projectile class descriptor `+80h..+8Fh` and `+90h..+9Fh` are the two payload-icon atlas rectangles that `BSP_GUI_ResolveTextureAndAtlasUV` fills at `006E8AFF` and `006E8B4E` |
| the "sub-type table selecting 2, 3, 4, 5 or 7", `docs/UNIT_WEAPON_DEVICES.md` step 7 | left `contract: unread` as a sub-type table | the **weapon type** `gun[+3F4h][+80h]`, not the projectile sub-type: `007302EA` loads `[ECX+80h]` from `gun[+3F4h]` and compares it with 1, 5 and 6 | `007327B0` is the only producer of `+80h` and writes 0..0Bh from the `Function` key; the projectile sub-type at `gun[+3F8h][+34h][+8h]` is loaded separately at `0073039D` and compared with 0Ah, 0Bh, 10h and the group 4..7 |
| `kProjectileClassOffTraceVariant = 0x74` (`include/bsp/projectile_impact.hpp`, site `0084BF00`) | "trace variant" | the `ExplWaterHit` boolean | producer `006E89EA`: `BSP_LuaReference_GetBooleanOrDefault(0)` on the key `ExplWaterHit` stores a byte at `+74h`; nothing else writes it |
| `kProjectileClassOffScoringId = 0xCC` (site `00923A2C`) | "scoring id" | the `Decals.Landscape` decal id, `-1` when the row has no `Decals` table | producer `006E8C6D`/`006E8D0C`; `+D0h` next to it is `Decals.Other`. Provisional: the consumer at `00923A2C` was not re-read, so it is possible that site reads `+CCh` of a different object |

The two enums are easy to confuse because both start at small integers. The gun class's weapon type
is `0` none, `1` AAMACHINEGUN, `2` LIGHTARTILLERY, `3` MEDIUMARTILLERY, `4` HEAVYARTILLERY, `5`
FLAK, `6` LIGHTARTILLERYFLAK, `7` TORPEDO, `8` DEPTHCHARGE, `9` DEPTHCHARGELAUNCHER, `0Ah`
BOMBPLATFORM, `0Bh` CATAPULT (`007327E4`-`00732900`, each `MOV dword ptr [param+80h], n`). That
table agrees with `kWeaponTypeTorpedo = 7` in `include/bsp/unit_weapons.hpp`.

## `006EA910` — one descriptor per Lua `Bullets` row

`__fastcall(int* out, int bulletId)`, body `006EA910`-`006EAE7D`, SEH frame. The ledger name
`BSP_BulletClass_GetOrCreate` is correct and is kept.

| order | site | step |
| --- | --- | --- |
| 1 | `006EA942` | `006E9B70()` singleton cache; `006E86F0(id+1)` grows its vector when `[cache+8h] <= id` |
| 2 | `006EA9A5` | on a cache hit at `[[cache+4h] + id*4]` jump to the tail and return the cached pointer with an `InterlockedIncrement` |
| 3 | `006EA9B8` | `BSP_LuaStateOwner_GetGlobals`, `Bullets`, `[id]`, then the entry's `Type` key (`00CE4780`) as a native string, defaulting to `""` |
| 4 | `006EAAE6`-`006EACE0` | thirteen `BSP_NativeString_EqualsCStringInsensitive` tests in the order Bullet, Artillery, Kamikaze, Bomb, Torpedo, DepthCharge, DummyTarget, DummyKamikazePlane, DummySubmarine, Paratrooper, Flak, Rocket, WaterMine; no match stores a null class pointer and returns |
| 5 | per branch | `operator_new(0D4h)` for Bullet, Artillery and Kamikaze (`006EAA62`, `006EAAA4`, `006EAAF6`), otherwise `00470B80` `BSP_Memory_AllocZeroed` with the size in ECX, then that kind's constructor |
| 6 | `006EACFD` | `class->vtable[18h](luaEntry)` — the **Lua field reader** |
| 7 | `006EAD86` | `class[+0Ch] = id`, then the cache slot takes the pointer, releasing whatever it held |
| 8 | `006EADC1` | the tail: re-read the cache slot, `InterlockedIncrement`, return |

Step 5 gives the descriptor size per kind, which is the only place it is stated:

| kind | size | site |
| --- | --- | --- |
| Bullet, Artillery, Kamikaze | `0D4h` | `006EAA62`, `006EAAA4`, `006EAAF6` |
| Bomb | `0D8h` | `006EAB38` |
| Flak | `0DCh` | `006EAC88` |
| DummyKamikazePlane, DummySubmarine | `0E0h` | `006EABF8`, `006EAC28` |
| DummyTarget | `0E4h` | `006EABC8` |
| Rocket | `0E8h` | `006EACB1` |
| WaterMine | `0ECh` | `006EACDE` |
| DepthCharge | `0F8h` | `006EAB98` |
| Torpedo | `0FCh` | `006EAB68` |
| Paratrooper | `100h` | `006EAC58` |

The only caller found is `BSP_PlaneClass_ReadLuaFields` `007D1F70` at `007D2BFF`
(`docs/PLANE_CLASS_FIELDS.md`), resolving the kamikaze bullet class id; the gun path reaches a
descriptor that is already cached.

## `006E8320` — the base constructor

`__fastcall(descriptor)`, body `006E8320`-`006E83xx`. It writes the base vtable `00CEB130` first and
`00CFA138` second, so `MBullet` derives from a ref-counted base. Non-zero defaults:

| offset | value | note |
| --- | --- | --- |
| `+4h` | `1` | reference count |
| `+8h` | `1` | the sub-type; every derived constructor overwrites it |
| `+54h` | `00D7A248` | `FlyTime` default |
| `+5Ch`, `+88h`, `+8Ch`, `+98h`, `+9Ch` | `00D7A24C` = `1.0f` | the time scale and the two atlas rectangles' diagonals |

Everything else it touches is zeroed. `00D7A24C` is `1.0f`: `006E8493`-`006E84C9` uses the same dword
for the four diagonal entries of the scale-1 matrix that `006E8430` hands to `00923870`.

## `006E8770` — the base field reader (`vtable[18h]`)

`__fastcall(descriptor, luaEntry)`, body `006E8770`-`006E8Dxx`, SEH frame. Thirty-two keys, in call
order. `OrDefault` is `BSP_LuaReference_Get<T>OrDefault`, which returns the listed default when the
key is nil; `number` is `BSP_LuaObject_GetNumber`, which has no default and leaves the constructor's
value. Every key was cross-checked against the installed
`scripts/datatables/classtables/arcade/bulletclasses.lua`.

| key | offset | type | default |
| --- | --- | --- | --- |
| `Name` | `+10h` | string | `""` |
| `Comment` | `+18h` | string | `""` |
| `Mesh` | `+A0h` | string | `""` |
| `ExplWaterEfx` | `+24h` | effect id -> ref | 0 |
| `ExplGroundEfx` | `+38h` | effect id -> ref | 0 |
| `ExplAirEfx` | `+2Ch` | effect id -> ref | 0 |
| `ExplArmorEfx` | `+28h` | effect id -> ref | 0 |
| `ExplLightArmorEfx` | `+3Ch` | effect id -> ref | falls back to `+28h` at `006E8A0C` |
| `TravelEfx` | `+40h` | effect id -> ref | 0 |
| `TravelEfx2` | `+44h` | effect id -> ref | 0 |
| `WaterTravelEfx` | `+48h` | effect id -> ref | 0 |
| `ExplUnderWaterArmorEfx` | `+30h` | effect id -> ref | falls back to `+28h` at `006E8A9B` |
| `ExplDeepUnderWaterArmorEfx` | `+34h` | effect id -> ref | falls back to `+30h` |
| `ExplWaterHit` | `+74h` | bool | false |
| `NoGravity` | `+20h` | bool | false |
| `FlyTime` | `+54h` | float | `00D7A248` (the constructor's value) |
| `SmallPayloadIcon` | `+78h` texture, `+80h` atlas | string -> texture | 0 when the string is empty |
| `BigPayloadIcon` | `+7Ch` texture, `+90h` atlas | string -> texture | 0 when the string is empty |
| `DamageMin` | `+ACh` | float | 0 |
| `DamageMax` | `+B0h` | float | `DamageMin` |
| `WaterDamage` | `+BCh` | float | 0 |
| `FireDamage` | `+C0h` | float | 0 |
| `FireChance` | `+C4h` | float / `00D7A220` | 0 |
| `Mass` | `+4Ch` | float | 0 |
| `V0` | `+50h` | float | 0 |
| `Range` | `+68h` | float | 0 |
| `Blast` | `+6Ch` | sub-table presence -> bool | false |
| `Blast.BlastRange` | `+70h` | number | keeps 0 |
| `Blast.BlastDamageMin` | `+B4h` | number | keeps 0 |
| `Blast.BlastDamageMax` | `+B8h` | float | `BlastDamageMin` |
| `Decals.Landscape` | `+CCh` | string -> decal id | `-1` when `Decals` is nil |
| `Decals.Other` | `+D0h` | string -> decal id | `-1` when `Decals` is nil |

`+58h`, `+5Ch`, `+60h`, `+64h`, `+D8h` and everything from `+D4h` up are untouched by this reader;
the per-kind readers own them.

## The reader chain

`vtable[18h]` is overridden four deep. Each override calls its parent first, so a torpedo row is
read by three routines.

| reader | class | parent call | keys it adds |
| --- | --- | --- | --- |
| `006E8770` | MBullet, MArtilleryBullet, MKamikazePlane | — | the 32 above |
| `006E1BE0` | MBomb and every class below it | `006E8770` at `006E1C02` | `TimeOutEfx` -> `+D4h` |
| `008566B0` | MTorpedo | `006E1BE0` at `008566D3` | `MaxWaterHitVel` `+DCh`, `MaxFall` `+E0h`, `WaterTravelSpeed` `+E4h`, `HeadingTurn` `+E8h`, `WaterSplashEfx` `+D8h`, `HomingHorzTurnSpeed` `+F0h`, `HomingVertTurnSpeed` `+F4h` |
| `006FD400` | MDepthCharge | `006E1BE0` at `006FD422` | `MaxWaterHitVel` `+ECh`, `MaxFall` `+F4h`, `V0RandomFactor` `+D8h`, `DiveSpeed` `+DCh`, `DiveMinDepth` `+E0h` (default `00D7A248`), `DiveMaxDepth` `+E4h` (same default), `WaterSplashEfx` `+E8h`; `+F0h` is computed, not read |
| `00809CB0` | MRocket | `006E1BE0` at `00809CD2` | `VMax` `+D8h`, `Acceleration` `+DCh`, `IgnitionDelay` `+E0h`, `AntiAir` `+E4h` (bool) |
| `0070C030` | MFlakBullet | `006E8770` at `0070C051`, **not** the bomb reader | clears `+D4h`, `MinRange` (integer) -> `+58h` |
| `0085F500`, `00701060`, `007AC780`, `006FF170`, `00700050` | MWaterMine, MDummyTarget, MParatrooper, MDummyKamikazePlane, MDummySubmarine | `006E1BE0` | `contract: unread` |

The flak's `+D8h` fuse time that `0070C3E2` compares against the flight time has **no producer**: no
reader writes it and `00470B80` zeroes the allocation, so on every class built through `006EA910`
the flak fuse is armed from the first tick. Provisional; a writer outside the reader chain would
change this.

## Sub-type `+8h`

`+8h` is set once, by the constructor of the class the `Type` key selected, and never by a reader.
That settles the enum: see `docs/PROJECTILE_KINDS.md` for the full table. The values are
`1` Bullet, `4` Artillery, `9` Bomb, `0Ah` Torpedo, `0Bh` DepthCharge, `0Ch` DummyTarget,
`0Dh` DummyKamikazePlane, `0Eh` DummySubmarine, `0Fh` Paratrooper, `10h` Flak, `11h` Kamikaze,
`12h` Rocket, `13h` WaterMine. `2`, `3`, `5`, `6` and `7` have **no producer**: no constructor
writes them. `4`..`7` is nevertheless a closed group in two consumers (`006E85B4` launch bias,
`0073042F` ammo provider), so the original enum reserved four artillery slots and the shipped
registry uses only the first.

## Host table

One row per native call site the reconstruction does not model.
`include/bsp/projectile_kinds.hpp` names each method.

| site | callee | host method | this / args / ret | gate |
| --- | --- | --- | --- | --- |
| `006E87F1` and its nine copies | the effect-id resolver | `acquire_effect` | `(int effectId)` -> ref | the `0E17BF8` guard; a zero id yields a null ref and the fallback chain runs |
| `006E8AFF`, `006E8B4E` | `BSP_GUI_ResolveTextureAndAtlasUV` | `resolve_payload_icon` | `(&uv, descriptor+80h/+90h, &size, 1.0f)` -> texture | only when the icon string is non-empty |
| `006E8C6D`, `006E8CD9` | the decal-name lookup | `resolve_decal` | `(name)` -> id | only inside the `Decals` sub-table |

## Coverage

| routine | coverage |
| --- | --- |
| `006E8320` | complete |
| `006E8770` | complete for the key table; the storage-pool release blocks between keys are library code and are not modelled |
| `006EA910` | complete |
| `006E1BE0`, `0070C030`, `00809CB0` | complete |
| `008566B0`, `006FD400` | complete except the computed `+F0h` of the depth charge (`LIBCRT_unmatched_00BF7030`) |
| `0085F500`, `00701060`, `007AC780`, `006FF170`, `00700050` | `contract: unread` |
| `006E9E80` | read only far enough to identify it as the base destructor, which is why it is the second writer of `00CFA138` |
