# The gun platform's traverse arcs, and how a gun is told to fire

Addresses: `007F5960`, `007F5A10`, `007F5FC0`, `007F60A0`, `007F6530`, `007F6840`, `007F6B10`,
`0072D130`, `0072D2C0`, `0072F6E0`, `00730A20`, `006E3940`, `006E3DC0`, `006E3DE0`, `004F17F0`,
`006FDC90`, `006FDCD0`, `006FE160`, `008598B0`, `00859A20`, `007297B0`, `0085B0F0`.

This packet closes the four "contract: unread" notes `docs/GUN_AIMING.md` left: how the platform
routes a traverse around a blocked window, what fills the window table, who decides to send opcode
`0ADh`, and what the six unread vtable slots do. Read that document first; this one only adds.
Every name below is a hypothesis, not a recovered symbol. Evidence is in
`reports/gun_platform_arc.json`; the rules are in `include/bsp/gun_platform_arc.hpp` and
`src/gun_platform_arc.cpp`.

## The window table is a partition of the circle, not a list of allowed sectors

`docs/GUN_AIMING.md` has the arc record right: `14h` bytes, flags at `+0h`, `MinHorzAngle` at
`+4h`, `MaxHorzAngle` at `+8h`, `MinVertAngle` at `+Ch`, `MaxVertAngle` at `+10h`. What it does not
say is that the records at `platform+3Ch` are contiguous in angle and cover the whole circle. The
producer makes them so.

`platform+3Ch`/`+40h`/`+44h` is a data / size / capacity triple, which `007F5A10`
`BSP_GunPlatform_SplitInsertArc` grows by `2 * capacity + 1` records at a time. Insertion is not an
append. `007F5A10` scans for the last existing window whose horizontal bounds hold **both** ends of
the new window, truncates that window's `MaxHorzAngle` to the new window's `MinHorzAngle`, inserts
the new record after it, and then re-appends the right-hand leftover carrying the **old** window's
flags and elevation bounds. Either leftover narrower than half a degree (`00D08B90`) is dropped
instead of kept.

The consequence is that an authored `Nofire` sector does not add a blocked entry to a list of
allowed ones; it carves a hole out of whatever window already covered that heading. The search
result starts at zero and is dereferenced right after the loop with no null guard, so the list must already hold a covering
window before the first insert: the platform is seeded with one full-circle window somewhere this
packet did not find.

`007F6B10` `BSP_GunPlatform_AddAuthoredArc` is the one caller from the data side, reached from
`0096185E` in `BSP_VehicleClass_ReadLuaFields`. It is `__thiscall(platform)(arc*)`, `RET 4`, and it
normalises before inserting:

| Step | Rule |
| --- | --- |
| 1 | all four angles zero: return without inserting. An unset record is not a window. |
| 2 | `span = abs(MinHorzAngle - MaxHorzAngle)`; when `span < 0.01` (`00D7A238`) both bounds are pushed out by `0.01` (`00D7A358`) and re-wrapped through `00605070`. A single-point arc becomes a 1.1 degree one. |
| 3 | both horizontal bounds are clamped to `+-3.1414795` (`00D08BA8`/`00D08BAC`), a hair inside pi, so an authored 180 degree edge never lands on the seam. |
| 4 | `MaxHorzAngle < MinHorzAngle` means the window straddles `+-pi`: it is inserted twice, as `[min, +pi]` and `[-pi, max]`. |

`0085A4A8` inside `BSP_TurningGun_SetupFromDescriptor` is the other `007F5A10` caller. It builds an
arc record from two `atan2` results (`0085A467`, `0085A488`) and inserts it, so a gun narrows its own
platform's windows at setup. That body is `docs/GUN_AIMING.md`'s `partial: 0085A4x..0085A822` and
was not read further here.

## The four window tests

| Routine | ABI | Answer |
| --- | --- | --- |
| `007F5FC0` | `__thiscall(platform)(h, v) -> bool`, `RET 8` | is there a window with bit 0 set holding the pair |
| `007F60A0` | same | the same against bit 1 |
| `007F5960` | `__thiscall(arc)(h) -> bool` | one record, horizontal bounds only, no flag test |
| `007F6840` | `__thiscall(platform)(h, v) -> arc*`, `RET 8` | **the same walk as `007F5FC0`, returning the record** |

`007F6840` `BSP_GunPlatform_FindTraverseWindow` is byte-for-byte `007F5FC0` with the return changed:
same `00D08B88` epsilon, same `+-pi` clamp on the horizontal angle, same bit 0 test, `0` on a miss.
`0085B0C5` tests only `EAX` against zero (`0085B0CA`), so the aiming document's description of the
effect was right and the predicate holds no surprise. **Correction** to `docs/GUN_AIMING.md` line
211: the predicate is not a different test, it is `007F5FC0`.

## `007F6530`: routing a traverse around a blocked window

`__thiscall(platform)(float horz, float vert, float tHorz, float tVert, float* outStepH,
float* outStepV)`, `RET 18h`. The argument order comes from the listing, not the pushes: the
`00438B10` calls at `007F6547` and `007F6564` each leave `ESP` eight bytes higher, which is what
puts the two out pointers at `[E+14h]` and `[E+18h]`.

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `007F6547`, `007F6564` | `*outStepH = wrap(tHorz - horz)`, `*outStepV = wrap(tVert - vert)`. Both are written before any window is consulted, so everything below can only overwrite them. |
| 2 | `007F65A5`-`007F6608` | find the current window: the first record whose **horizontal** bounds hold the clamped `horz` and whose bit 0 is set. None: return, deltas are the plain shortest path. |
| 3 | `007F6621` | `dir = stepH < 0 ? -1 : +1` against `00D7A218` = `0.0f`. |
| 4 | `007F664E` / `007F673F` | walk circularly from the current window in `dir` until a window holds the clamped `tHorz` horizontally, or a window has bit 0 clear. A full circle with neither: return unchanged. |
| 5 | `007F6694`, `007F669F` | if the walk stopped on a **blocked** window, `*outStepH = -0.0f - *outStepH` (`00D7A208`), which is a negation, and `dir` is reversed. The gun now goes the long way round. |
| 6 | `007F66B3` | if the walk never left the current window, return. |
| 7 | `007F66B9`-`007F681B` | walk again from the current window in the (possibly reversed) `dir` up to but not including the stop window. The first window whose elevation bounds exclude `vert + stepV` replaces `*outStepV` with `wrap(MinVertAngle - vert)` or `wrap(MaxVertAngle - vert)` and returns. |

Two things are worth stating plainly. First, step 5 changes only the **sign**: the magnitude stays
the short-way magnitude, and `0085AD80` clamps it to `rate * dt` anyway, so this is a per-step
steering decision that is re-taken every fixed step, not a path plan. Second, step 7 is what makes a
ship's guns dip or rise as they train past a funnel: the elevation is capped by every window the
barrel sweeps through, not only by the window it ends in.

The blocked-window branch is the one place the routine changes a delta's direction rather than its
size, so it is the branch `tests/math_tests.cpp` pins.

## The fire decision

`docs/GUN_AIMING.md` line 345 asks which routine sends opcode `0ADh`. It is the gun's own fixed-step
tick.

`0072D130` `BSP_Gun_FixedStepTick`, `__thiscall(gun+310h)(float dt)`, body `0072D130`-`0072D2BE`:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `0072D14D`-`0072D178` | decrement the reference count at `gun+420h`; at zero, release `gun+41Ch` through its own vtable and null it |
| 2 | `0072D18C` | `0072AD40(dt)`, the base tick |
| 3 | `0072D1A7`, `0072D1B5` | `gun+478h -= dt`, `gun+450h -= dt`: the two timers `CanFire` test 4 reads |
| 4 | `0072D1C1`, `0072D1CD`, `0072D1D9` | gate on `unit+720h == 0 && gun+3B8h == 0 && gun+5Dh == 0`; any set and the rest of the step is skipped |
| 5 | `0072D1ED`-`0072D227` | for each of `gun+448h` barrels, a timer in `gun+414h` that is not already negative is advanced by `-dt` through `0072CF00` |
| 6 | `0072D22F` | `if (gun+454h == 0) return` |
| 7 | `0072D23A`, `0072D248` | `id = gun+46Ch ? *(uint16*)(gun+46Ch + 174h) : 0` |
| 8 | `0072D252`-`0072D290` | build the opcode `0ADh` message inline (the same field writes `006E3940` makes: vtable `00CF9628`, `msg+4h = 1`, `msg+18h`/`+1Ah`/`+1Ch` zero, `msg+20h = id`) and send it through `BSP_Session_RouteMessage` |

So a gun with its fire latch set emits one `0ADh` message per fixed step, and the handler at
`0072D860` turns each into `vtable[1DCh]` `FireIfReady`, which asks `CanFire` and fires on a yes.
The rate limiting is entirely in `CanFire`'s timers, not in the sender.

`gun+454h` is the latch. A scan of the code sections for a `[reg+454h]` displacement finds exactly two
byte writes to it: the constructor's zero at `0072E620` and `0072D394`. `0072D2C0`
`BSP_Gun_SetFireRequest` is therefore its only run-time writer. It is gun
vtable slot `1E8h` (`00CFBF08`, `00CFE290`, `00CFE4F0`, `00CFE730`), `__thiscall(gun)(bool)`,
`RET 4`:

| Step | Site | Rule |
| --- | --- | --- |
| 1 | `0072D2DC`-`0072D2EC` | a null `gun+3F0h`, a set `unit+5Dh` or a set `gun+5Dh` forces the request to false |
| 2 | `0072D2FE`-`0072D316` | an effective false releases the observer pair at `gun+458h` through `BSP_Observer_UnregisterPair` and nulls its subject `gun+46Ch`. Refusing to fire drops the fire target, exactly as `00836210` does for the director. |
| 3 | `0072D31E` | everything below runs only on a change |
| 4 | `0072D32C`-`0072D37D` | an `MRFSGun` (`vtable[5Ch](23h)`) also sends opcode `0AFh` carrying the new boolean at `msg+20h` |
| 5 | `0072D394` | store the latch |
| 6 | `0072D3B3`, `0072D3B8` | a rising edge seeds `gun+478h` with a random value in `[0, 0.12)` (`00CE81A8`): the stagger that stops a battery firing in lockstep |
| 7 | `0072D3D3` | a falling edge calls `0072B4C0` |

### Where `allowFire` is still missing

The director's `allowFire` byte at `director+3Ch` (`docs/WEAPON_DIRECTOR.md`) does **not** reach the
gun in any body this packet read. The slot `1E8h` dispatches in the image are:

| Site | Containing function | Note |
| --- | --- | --- |
| `009003AB` | `BSP_GunBot_HeadingAimTick` (`008FFF20`) | owned by packet `cc2_gun_bot_ticks` |
| `00959F47`, `00959F5E`, `0095A1B8`, `0095A5AC` | `BSP_Unit_ApplyGunAimMessage` (`00959C20`) | owned by packet `cc2_gun_bot_ticks` |
| `009594B1` | `BSP_Unit_OnDestroyed` | a destroyed unit drops every gun's request |
| `006DF508`, `008FC22C` | no Ghidra function | not decoded |

The dispatch at `006E5878` in `FUN_006E56F0` looked like a seventh but is on the **unit's** vtable,
not the gun's, so it is not a fire request. The `allowFire` path therefore runs through the two
bodies another packet owns: **contract: unread** here by design.

### `gun+358h` is a destruction level

`CanFire` test 3 requires `gun+358h <= 0`. A scan of the executable's code sections for a
`[reg+358h]` displacement finds five gun-side sites: the constructor's zero at `0072E61A`, the
`CanFire` test at `00729AA7`, a descriptor-time branch at `0072EDDF`, `BSP_Gun_Fire`'s test at
`007301A7`, and `007297B3`.

`007297B0` `BSP_Gun_ApplyWreckVisibility` settles it: a zero shows the gun's own node at visibility
`1.0` and hides every child node named `MeshHolder`; a non-zero hides the gun node and shows the
`MeshHolder` children. It is a destroyed-state level, not an ammunition or barrel count.
`BSP_Gun_Fire` lets a non-zero gun fire only when the global at `[00E188A8]+1FE4h` equals `2`.

**Correction** to `docs/GUN_AIMING.md` line 327, which called it "a per-gun counter, contract:
unread". What raises it above zero is still unread: the only write the scan finds is the
constructor's zero, so the damage path most likely writes it as `[reg+48h]` through the `gun+310h`
sub-object, a displacement too common to scan for.

## The six slot helpers

| Slot | Address | ABI | Behaviour |
| --- | --- | --- | --- |
| `188h` base | `004F17F0` | `RET 8` | a shared no-op in 25 vtables, one instruction |
| `188h` turning | `00859A20` | `__thiscall(gun)(stream, level)`, `RET 8` | skips when `[gun+3Ch]->vtable[5Ch](6)` is true or the level is 5 or more; otherwise passes `gun+494h`, `gun+498h`, `gun+488h`, `gun+48Ch`, `gun+49Ch` and `&gun+4BCh` to `00859410` and writes the result through `00779FC0`/`00779F80` |
| `1D4h` | `00730A20` | `ADD ECX,424h; JMP 0072F6E0` | a this-adjusting thunk into the per-target decision table |
| `1E0h` base | `006E3DC0` | `(out, in)`, `RET 8`, `ECX` unused | copies three floats; the unmodified muzzle origin |
| `1E0h` `27h` | `006FE160` | `__thiscall(gun)(out, in)` | offsets the origin by `descriptor+D0h` around a ring at angle `gun+44Ch * 2pi / muzzleCount`, the count being `(descriptor+A0h - descriptor+9Ch) / 0Ch`. This is how a salvo mount spreads its barrels. |
| `1E4h` base | `006E3DE0` | `RET 4` | a shared no-op in 5 vtables |
| `1E4h` turning | `008598B0` | `__thiscall(gun)(out)`, `RET 4` | when `[00F876A8] - gun+4A0h < 0.25` (`00D7A348`), copies the six floats `gun+4A4h`..`gun+4B8h` into `out+78h`..`out+8Ch` and sets `out+90h = 1`: the last shot's pose, published for a quarter second |
| `1E8h` base | `0072D2C0` | above | the fire-request latch |
| `1E8h` `24h` | `006FDC90` | `__thiscall(gun)(bool)`, `RET 4` | calls the base, then clears `gun+4D4h` when the latch came back clear |
| `1E8h` `27h` | `006FDCD0` | same | as `006FDC90`, except a false request is skipped entirely unless `gun+4D8h` is also zero |

`0072F6E0` `BSP_Gun_TargetShotDecision` behind the `1D4h` thunk is a memo table on `gun+424h`: it
returns 1 when `gun+42Ch` is zero, otherwise it looks the target key up in the `0Ch`-byte records at
`gun+430h`/`+434h`/`+438h` and returns the cached byte, and on a miss it takes a fresh answer from
`00729670`, stores it with a random value scaled by `GlobalConfig+8Ch`, and returns it. Neither
`00729670` nor `GlobalConfig+8Ch` was read, so whether the table is an accuracy roll or a target
permission is **contract: unread**.

**Correction** to the aiming document's vtable notes: `00730A20`, `004F17F0`, `006E3DC0` and
`006E3DE0` are not swallowed by neighbouring bodies. `BSP_Gun_Fire` ends at `00730A1D` and
`BSP_ClassId20_IsKindOf` at `006E3D85`; the helpers follow their `INT3` padding. Ghidra simply has
no function at those addresses, which is why they are listed under `no_ghidra_function`.

## `0085B0F0`, the rest of it

The four values stored into `gun+4BCh`..`+4C8h` are not `atan2`-derived. They are `0085B0F0`'s own
four arguments after the `(-pi, pi]` wrap:

| Field | Site | Value |
| --- | --- | --- |
| `gun+4C4h` | `0085B279` | argument 1, the replicated target horizontal angle |
| `gun+4C8h` | `0085B26B` | argument 2, the replicated target vertical angle |
| `gun+4BCh` | `0085B25D` | argument 3, the replicated current horizontal angle |
| `gun+4C0h` | `0085B24F` | argument 4, the replicated current vertical angle |

That is the quad `00859B10` puts in the snapshot at `packet+40h` and `0085BA30` copies back, so the
gun stores the angles it was told about verbatim and uses them below.

The "intermediate angle search" is a seam repair, not a route search. For the target angle and again
for the current angle, `0085B0F0` asks `007F5960` about each window in turn; on a miss it retries the
angle plus `0.008726646` (`00D08B88`) and then minus it, and only if all three candidates fall in no
traverse-enabled window does it return without writing anything. An accepted fallback replaces the
angle. The six call sites are `0085B2CF`, `0085B38A`, `0085B441` for the target and `0085B552`,
`0085B60A`, `0085B6C1` for the current angle.

The tail then writes `gun+480h` and `gun+488h` (`0085B722`, `0085B72A`) and `gun+484h`/`gun+48Ch`
(`0085B78D`, `0085B795`) only when `abs(wrap(replicated - gun+480h))` exceeds `0.8` rad (`00CE3D40`,
read at `0085B4A0`), always writes `gun+494h`/`gun+498h` (`0085B7A3`, `0085B7B3`), and ends in
`00859550` at `0085B7BB`.

## Coverage

| Routine | Coverage |
| --- | --- |
| `007F6530`, `007F6840`, `007F6B10`, `007F5A10` | complete |
| `0072D130`, `0072D2C0`, `0072F6E0`, `007297B0`, `006FE160` | complete |
| `00730A20`, `004F17F0`, `006E3DC0`, `006E3DE0`, `006FDC90`, `006FDCD0`, `008598B0`, `00859A20` | complete, from the raw listing; no Ghidra function |
| `0085B0F0` | complete for the four stored angles, the seam search and the tail writes |
| `006E3940` | complete for the field writes; the message transport is a contract |
| the `allowFire` path | **contract: unread**, in `008FFF20` and `00959C20`, owned by packet `cc2_gun_bot_ticks` |
| the writer of `gun+358h` | **contract: unread** |
| `00729670`, `00859410`, `00779FC0`, `00779F80`, the seed of the window list | **contract: unread** |

Nothing here was run: `bsp_game.exe` does not reach the gun fire chain, so there is no run-time
evidence in this packet and none is claimed.

## Correction from docs/GUN_BOT_TICKS.md (packet cc2_gun_bot_ticks)

`0072D2C0`, called `BSP_Gun_SetFireRequest` above, is named `BSP_Gun_SetTriggerHeld` in the ledger
and in Ghidra: `__thiscall(gun)(char wantFire)`, body `0072D2C0-0072D3A8`, gun vtable slot `1E8h`,
the trigger every gun bot drives. The behaviour described above is unchanged.
