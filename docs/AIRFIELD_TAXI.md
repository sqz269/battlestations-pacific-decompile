# Airfield hangar paths and the plane's taxi state (packet `cc2_airfield_taxi`)

Read-only analysis. Every name below is a hypothesis, not a recovered symbol; the offsets and
call sites are the evidence. This packet closes the open question of `docs/LAND_AND_STRUCTURES.md`
("who consumes the airfield's `EntryPath`/`ExitPath` hangar list") and the open question of
`docs/PLANE_GROUND_OPS.md` (which object the plane follows in flight state `5`, `Runway on Path`).

**The answer in one line:** the hangar list is consumed by an *air-operations launch-site* object
that the air-operations block owns at `block+3Ch`; the plane holds a reference to that block at
`unit+BF4h`, and the plane bot reads the site's slot `+2Ch` every step to get the taxi target.

## 1. The chain from the plane to the hangar list

| Step | Expression | Established by |
| --- | --- | --- |
| the plane's ground-contact holder | `unit+BF4h` | `007B8E80` is its only writer outside the constructor |
| the air-operations block | `holder+4h` | `006CF9F0` passes `unit+72Ch`; `007C5F60` reads `+3Ch`/`+7Ch`/`+80h` off it |
| the launch-site object | `block+3Ch` | `007C5F60` at `007C5F79`, `009CD540` at `009CD5EB` |
| the owning unit | `block+7Ch` | already in `docs/AIR_OPERATIONS.md`; the site mirrors it at `site+44h` |
| the airfield's hangar vector | `airfield+830h` base, `+834h` count, stride `0Ch` | `006D5220` builds it, `006D2640`/`006D2780`/`006D2730` walk it |

`006BCD20 BSP_AirOps_GetBlock` already established that the block is `owner+72Ch` for `MAirfield`
and `owner+1188h` for `MMothership`. `006CF3E0` (vtable slot `+4Ch`) is the site's own accessor for
the same thing, `site+44h + 72Ch`, which is the independent confirmation that `site+44h` is the
airfield unit.

`007B8E80 BSP_Plane_SetGroundContactSite` (`__thiscall(plane, holder*)`, `RET 4`, Ghidra body
`007B8E80`-`007B8ED2`, complete) is the attach/detach:

| Site | Rule |
| --- | --- |
| `007B8E8E` | equal to the current value: no-op |
| `007B8E96`-`007B8E9F` | detach: `006952A0 BSP_Observer_UnregisterPair(ECX = (old+4)->+7Ch, EDX = plane+10h)` |
| `007B8EB1`-`007B8EB4` | `byte ((old+4)->+3Ch)+1Ch = 1`, i.e. the released site is marked |
| `007B8EBC`-`007B8EC5` | attach: `00694A60 BSP_Observer_RegisterPair(ECX = (new+4)->+7Ch, EDX = plane+10h)` |
| `007B8ECA` | `plane+BF4h = new` |

So `unit+BF4h` is **not** the surface object itself. `include/bsp/plane_ground_ops.hpp` calls it
`kGroundContactOwner`, "the object the plane rests on"; it is one level further out than that -- a
holder whose `+4h` is the owner's air-operations block. The doc's own dereference chain
`(unit+BF4h)->+4h->+3Ch->vtable[28h](unit)` was already correct.

## 2. The launch-site class

Vtable `00CF89F8`, stored by the two constructors `006CF3C0` and `006CFABC`
(`mov dword ptr [edx], 0CF89F8h`). The data immediately before it is this class's serialized field
names, `lastLandingTime` (`00CF89D8`) and `readyPlane` (`00CF89E8`).

| Field | Meaning | Evidence |
| --- | --- | --- |
| `+4h` | an embedded observer node | `006CF190`, `006CF9F0` use it as the `EDX` of the register/unregister pair |
| `+18h` | the observed ready-plane target, cleared once the plane is placed | `006CF190`, `006CFA43`-`006CFA54` |
| `+20h` | a base subobject: the `+28h` slot is an adjustor thunk `ADD ECX,20h` | `006CF180` |
| `+34h`, `+38h` | the occupancy vector: base pointer and count of the planes on the site | `006CF5B0` walks `[+34h] .. [+34h] + [+38h]*4` |
| `+44h` | the owning `MAirfield` unit | `006CF3E0`, `006CF520`, `006CF9F0`, `006CF420` |

Slots this packet identified:

| Slot | Routine | What it does |
| --- | --- | --- |
| `+14h` | `006CF9F0 BSP_AirOpsSite_PlacePlaneOnSpot` | place a plane on the base and lock it |
| `+28h` | `006CF180` thunk -> `006CEF80` | the plane's ground-surface query (`docs/PLANE_GROUND_OPS.md`) |
| `+2Ch` | `006CF420 BSP_AirOpsSite_TaxiTargetPoint` | **the state-5 taxi target** |
| `+30h` | `006CF3F0` | returns `true`, `RET 0` |
| `+34h` | `006CF5B0 BSP_AirOpsSite_SpotIsClear` | the separation test against the other planes on the site |
| `+40h` | `006CF730 BSP_AirOpsSite_PoseQueuedPlane` | the queue pose, used by `007C5F60` |
| `+44h` | `006CF520 BSP_AirOpsSite_QueueOriginLocal` | the head of the queue in airfield-local space |
| `+4Ch` | `006CF3E0 BSP_AirOpsSite_GetAirFieldBlock` | `site+44h + 72Ch` |
| `+20h` | `006CF400` | returns `true`, `RET 4` |

The remaining slots (`+0h` `006CFA60`, `+4h` `006CF980`, `+8h` `006CEBE0`, `+0Ch` `006CF1C0`,
`+10h` `006CF2A0`, `+18h` `006CDF60`, `+1Ch` `006CE4A0`, `+24h` `006CED90`, `+38h` `006CEBF0`,
`+3Ch` `006CEC00`, `+48h` `006CE230`, `+50h` `006CEC10`) are **contract: unread**.

## 3. The hangar record and its producer

`006D5220 BSP_AirField_ReadHangarAndMarkerProperties` (`MAirfield` vtable slot `+0A4h`, **not**
`+0A0h`) fills a hand-rolled vector of a **12-byte record**, three pointers:

```
struct Hangar {          // stride 0Ch
    Entity*    object;    // +0h  the hangar building
    ScenePath* entry_path;// +4h  "EntryPath"  / "entryPathID"
    ScenePath* exit_path; // +8h  "ExitPath"   / "exitPathID"
};
// airfield+830h base pointer, +834h size, +838h capacity
```

There is no occupancy field in the record; occupancy lives on the launch site (`site+34h`/`+38h`).

Three source branches, keyed on `*(airfield+0C0h) + 4h`:

| Value | Source | Keys |
| --- | --- | --- |
| `1` | the scene property bag | `"Hangar %d"` `00CF8F08`, `"Object"`, `"EntryPath"` `00CF8EF4`, `"ExitPath"` `00CF8EE8` |
| `2` | a compiled table | count at `src+138h`, a `ushort` array at `src+13Ch` with **stride 6**: three ids per hangar |
| `3` | the Lua reader | `"hangars"`, `"entityID"`, `"entryPathID"`, `"exitPathID"` |

In branch `2` each `ushort` is resolved through the global record map
(`(id - DAT_00F89A60) * 10h + 0Ch + DAT_00F89AA8`, or the low-range pair
`_DAT_00F89A0C`/`DAT_00F89A54`) to an entity pointer. In all branches the two path fields go
through `007AC9D0 BSP_Entity_PathInterfaceForKind`, so a hangar's `EntryPath`/`ExitPath` is stored
as a **path interface**, not as the authored entity. That is what `007AF800
BSP_ScenePath_TransformPointToWorld(ECX = path, out, index)` later indexes.

The park-slot array is a **different** structure and does not overlap: `006D3250` reads the inline
`vec3` at `+83Ch + i*0Ch` with the count at `+884h`, six slots (`884h - 83Ch = 48h = 6 * 0Ch`).

## 4. Picking a hangar: two mirrored searches

Both walk the vector, transform each hangar object's world position (`object+0FCh`) into the
airfield's local frame through the cached inverse at `airfield+110h`
(`00B63D50 BSP_Matrix_BuildOrthogonalScaledAffineInverse` of `airfield+0CCh`, latched by the byte
at `+10Ch`) and keep one extreme of the local **Z**.

| Routine | Gate | Extreme | Sentinel | Returns |
| --- | --- | --- | --- | --- |
| `006D2780 BSP_AirField_PickHangarEntryPath` | object non-null | maximum | `00CF8E44` = `-9999.0f` | `record[1]`, the EntryPath |
| `006D2640 BSP_AirField_PickHangarExitPath` | object non-null **and** `object+370h > 0.0f` | minimum | `00CE4C04` = `+9999.0f` | `record[2]`, the ExitPath |

The comparison senses are from the listing, not the pseudocode: `006D280B`-`006D2817` pushes the
running best then the candidate and takes `JBE` to skip, so it updates on `candidate > best`;
`006D26DD`-`006D26E9` pushes the candidate then the best, so it updates on `best > candidate`.

`006D2640`'s `object+370h` gate is the hangar-failure rule: a hangar whose `+370h` is not above
zero is skipped, and with no hangar left `006CF420` falls back to the plane's own position, which
makes the taxi delta zero. `006D2780` has no such gate.

`006D2730 BSP_AirField_FindUsableHangarObject` is a third walk that returns the first object
passing the four-byte liveness test `+5Ch` set with `+5Dh`, `+60h`, `+5Eh` clear -- the same test
`007C16F0` applies to the plane at `007C1776`-`007C178C`.

## 5. The taxi cursor: `006CF420`, slot `+2Ch`

`__thiscall(site, vec3* out, unit* plane)`, `RET 8`, Ghidra body `006CF420`-`006CF517`, complete.

1. `site+44h` non-null and `006D2640(airfield)` non-null (`006CF431`), else step 5.
2. Latch the airfield's inverse world matrix at `+110h` (`006CF44D`, `006CF465`).
3. The point count is `(path+0Ch - path+8h) >> 2`; the index used is **that minus one**.
4. `007AF800(path, scratch, lastIndex)` at `006CF49B`, then `004142E0
   BSP_Vector3f_TransformAffinePoint` at `006CF4AE` into `airfield+110h`; the result is `*out`.
5. Fallback: `*out` = the plane's own world position at `plane+0FCh`.

The path is not cached: `006D2640` is called three times in this one body, at `006CF431`,
`006CF46D` and `006CF494`, so the hangar is re-chosen for the null test, for the count and for the
sample.

**There is no per-point cursor.** The state-5 target is always the *last* point of the chosen
hangar's exit path, expressed in the airfield's local frame. The path is re-chosen every step, so a
hangar dying mid-taxi changes the target on the next step.

## 6. The state-5 driver: the plane bot

Two sibling bot-task steps run the same law. Ghidra has a function for neither.

| Routine | Body | Vtable | Target site from |
| --- | --- | --- | --- |
| `009CD540 BSP_PlaneBot_TaxiStep` | `009CD540`-`009CDC92`, `RET 4` | `00D21150` slot `+0Ch` | `plane+BF4h` (already attached) |
| `009B22C0 BSP_PlaneBot_ApproachBaseStep` | `009B22C0`-`009B2C68`, `RET 4` | `00D1FF60` slot `+0Ch` | `(task+4)+34h` (the destination base) |

`009CD540` in order:

| Site | Rule |
| --- | --- |
| `009CD564`-`009CD586` | fail the task (`byte task+18h = 1`) unless `plane+900h == 5` **and** `plane+BF4h != 0` |
| `009CD58C`-`009CD5B5` | if `(holder+4)->+7Ch` is null or its byte `+5Dh` is set: `007B9000(plane)` then `007C1680` (leave the path) and return |
| `009CD5B8`-`009CD5E2` | request neutral controls on `(task+4)+18h`: `+2C4h = 0.0f`, `+2CCh = 1`, `+29Ch = 0.0f`, byte `+2A0h = 1`, `+2D0h = 0` |
| `009CD5E8`-`009CD5FF` | `site->vtable[2Ch](scratch, plane)` = `006CF420`: the taxi target |
| `009CD62F`-`009CD643` | `dx = target.x - plane+A4h`, `dz = target.z - plane+ACh` |
| `009CD657` | `plane->vtable[38h]()`, a per-plane scalar, into the local at `+38h` |
| `009CD752`-`009CD7A0` | the required rate: `tuning+2B0h RunwayYawTurnSpdMul * classDesc+1B0h YawSpd`, floored by `00415550 BSP_Math_MaxFloatByRef` against `tuning+188h`, `1.5` divided by it, then scaled by `tuning+2A8h` |
| `009CD7D9` | `BL = 1` when the requirement is not met by free steering |
| `009CD7E1`-`009CD802` | `SETE CL` on `plane+900h == 5`; when `CL != BL`, `007C16F0` if `BL` else `007C1680` |
| `009CD95E`-`009CD997` | **only when `plane+900h != 5`**: `site->vtable[34h](plane, 1, {x, y})` = `006CF5B0`; a false answer zeroes the speed request |
| `009CDC48`-`009CDC7F` | write the steering request `(task+4)+18h+284h` with byte `+288h = 1` and `+2D4h = 0`, and the speed request `+2B4h` with byte `+2B0h = 0` and `+2D8h = 1` |

So the 4 <-> 5 switch is **not** a threshold on the path: it is a steering-authority test. The bot
joins the path exactly when the turn it needs is sharper than the runway yaw band of
`docs/PLANE_GROUND_OPS.md` section 5 allows at this speed, and leaves it as soon as free steering
suffices. `007C16F0` and `007C1680` carry no path argument, which is why they never appeared to
take one: the path is always re-read from the site.

The tuning fields (`+188h`, `+2A8h`, `+2B0h`) and the class field (`+1B0h YawSpd`) are the same
ones the runway steering band reads, which is the cross-check that this is the ground law's
partner and not an air manoeuvre.

**Coverage: partial.** The x87 arithmetic between `009CD6AB` and `009CDC48` -- the speed reference,
the `00419010 BSP_Math_InterpolateClamped` five-argument ramps and the final yaw blend -- was not
reduced to closed form. Only the structure, the call sites and the constants are established.

## 7. Launch from a hangar

1. `LaunchAirBaseSlot` (`00896750`) is a request: message `83h` plus one air-operations update
   (`docs/AIR_OPERATIONS.md`). The authoritative launch is inside `006CDC70`, whose members are
   **contract: unread**.
2. The plane becomes ready; `006CF190 BSP_AirOpsSite_SetReadyPlaneObserver` is the site's watch for
   it, held at `site+18h`.
3. `006CF9F0 BSP_AirOpsSite_PlacePlaneOnSpot(site, plane)` fires. It requires `site+44h` and the
   block owner at `unit+7A8h` alive, then calls `007C5F60(plane, unit+72Ch)`, `007C3C90(plane, 1)`,
   `0042ED50` and `(plane+310h)->vtable[8](0.0f)`, and finally unregisters and clears `site+18h`.
4. `007C5F60 BSP_Plane_PlaceOnLaunchSpotLocked` poses and locks:
   - `block+3Ch ->vtable[40h](out, plane)` = `006CF730` gives the pose. That routine samples points
     `0` and `1` of `006D2780`'s **entry** path, builds a frame from their difference through
     `0085DAD0`, and slides the origin along it by
     `(classDesc+158h + 1.0) - plane+9D8h * 00CE3D10`, so `plane+9D8h` is the queue index and
     successive planes line up behind each other on the entry path.
   - the attitude comes from `cos`/`sin` of `classDesc+200h GroundPitch`, with `classDesc+1FCh`
     added to the vertical.
   - `block+7Ch ->vtable[5Ch](45h)` at `007C5FE2` splits the airfield arm from the carrier arm:
     different throttle (`plane+9DCh`) and different gear/flap requests on `plane+0DECh`.
   - `007B8E80(plane, *(block+80h))` attaches the ground-contact holder -- so **`block+80h` holds
     the holder whose `+4h` points back at the block**.
   - `007C6260`-`007C628A`: `plane+900h = 2` (`Locked`), `plane+C04h` stamped, `007C11E0` notified.
   - the tail runs `007D9C80`, an optional `vtable[0DCh](0.0f)` on `plane+9D4h`, and
     `008073C0 BSP_Recon_RebuildSlotLists`.
5. From `Locked` the plane goes to `3` (launching, full throttle) and then to `4`/`5` by the
   transitions already tabulated in `docs/PLANE_GROUND_OPS.md`. The taxi out is the bot of
   section 6 driving toward the exit path's last point.

**Contract: unread.** What inside `006CDC70` calls `006CF9F0`, and which slot or stock entry picks
the hangar. `006CF9F0`'s only reference is the vtable at `00CF8A0C`, so the dispatcher was not
reached from a direct call site.

## 8. Return to a hangar

Established: the approach is `009B22C0 BSP_PlaneBot_ApproachBaseStep`, which targets a base held at
`(task+4)+34h` rather than `plane+BF4h`, reads the queue origin through `006CF520` (keeping only
its Z) and the taxi target through `006CF420`, and runs the identical 4 <-> 5 switch at
`009B271A`-`009B273A`. Its gate at `009B22DC`-`009B2306` fails only when the plane is already
attached (`byte plane+BF8h` set and `plane+BF4h` non-null) and the destination's owner is dead.

Also established: `007B8E80` is what detaches a plane from a base, and it sets
`byte ((old+4)->+3Ch)+1Ch = 1` on the way out, which is the site being told its plane left.

**Contract: unread.** The step that puts the plane back to state `1` `Inside` and frees the
air-operations slot. `docs/PLANE_GROUND_OPS.md` already assigns the slot release to
`006C65B0 BSP_AirOps_ReleaseSquadronSlot` through `007F1B70`, and the state-1 writer to `007CC820`
on plane message sub-kind `1`; the **sender** of that message was not traced in this packet.
The park slots of `006D3250` were not reached from any routine read here.

## 9. `006D2510 BSP_AirField_TickAdvance`

`__thiscall(this = unit+310h, float dt)`, `RET 4`, body `006D2510`-`006D2551`, complete. The whole
body, from the listing:

| Site | Call |
| --- | --- |
| `006D251B` | `00953CC0(dt)` with `ECX = unit+310h`, the inherited base advance |
| `006D2520`-`006D2532` | `unit->vtable[1A8h]()` = `006D40F0` |
| `006D2534` | gate: `byte[unit+5Dh]` clear (`ESI-2B3h`) |
| `006D254B` | `006CDC70(unit+72Ch, dt)` (`LEA ECX,[ESI+41Ch]`) |

It touches **no** hangar, park slot or launch slot itself. Everything in this document that runs
per step runs either inside `006CDC70` or inside the plane bot, not here. This confirms the
summary already in `docs/LAND_AND_STRUCTURES.md`.

## 10. Corrections

| Document | Was | Is | Evidence |
| --- | --- | --- | --- |
| `docs/LAND_AND_STRUCTURES.md` | the `EntryPath`/`ExitPath` consumer "was not found"; the link is open | the consumer is the launch-site object at `block+3Ch`: `006D2780` feeds `006CF520`/`006CF730` (the launch queue) and `006D2640` feeds `006CF420` (the state-5 taxi target) | `006CF55C` and `006CF73A` call `006D2780`; `006CF432` calls `006D2640`; `009CD5EB`-`009CD5FF` reaches `006CF420` through `(plane+BF4h)->+4h->+3Ch->vtable[2Ch]` |
| `docs/LAND_AND_STRUCTURES.md` | `006D5220` walks the hangar bags "into the `0xC`-stride vector at `+830h`" (one branch described) | three branches: the property bag, a compiled `ushort` table with stride `6` at `src+13Ch` counted by `src+138h`, and the Lua reader `hangars`/`entityID`/`entryPathID`/`exitPathID` | the `iVar5 == 1`, `== 2` and `== 3` arms of `006D5220` keyed on `*(airfield+0C0h)+4` |
| `docs/PLANE_GROUND_OPS.md` | `unit+BF4h` is named `kGroundContactOwner`, "the object the plane rests on" | it is a holder; `holder+4h` is the owner's air-operations block (`owner+72Ch` / `owner+1188h`), `block+3Ch` the launch site, `block+7Ch` the owner unit | `007B8E80` dereferences `(value)+4h ->+7Ch` and `->+3Ch`; `006CF9F0` passes `unit+72Ch` to `007C5F60`, which reads `+3Ch`, `+7Ch` and `+80h` |
| `docs/AIR_OPERATIONS.md` | the block field table lists `+0Ch`, `+40h`, `+4Ch`, `+50h`, `+54h`, `+58h`, `+7Ch` | add `+3Ch`, the launch-site object, and `+80h`, the ground-contact holder handed to `007B8E80` | `007C5F79` (`[EDI+3Ch]`), `007C6239` (`[EAX+80h]`) with `EDI`/`EAX` = the block passed by `006CFA01` as `unit+72Ch` |
| `docs/AIR_OPERATIONS.md` | the byte at `block+7Ch ->+5Dh` is "contract: unread" | it is the owner's out-of-action byte: `006CF9F0` refuses to place a plane when it is set, and `009CD596` drops a taxiing plane off the path when it is set | `006CFA0A`, `009CD596`; the same byte gates `006D2534` in the airfield tick |
| the packet brief | the hangar reader is `vtable[0A0h]` | it is `vtable[0A4h]`; `vtable[0A0h]` `006D3C10` is the `RunwayWidth`/`RunwayLength` reader | `docs/LAND_AND_STRUCTURES.md` line 293 and the string refs of `006D5220` |

## 11. Open questions

* What inside `006CDC70` invokes the site's `+14h` slot, and which slot or stock entry chooses the
  hangar for a launch. `006CF9F0` has no direct call site.
* The routine that returns a landed plane to state `1` `Inside` and the sender of plane message
  sub-kind `1`.
* Whether `006D3250`'s six park slots at `+83Ch` are used at all: nothing read in this packet
  reaches them.
* The twelve unidentified slots of the launch-site vtable, in particular `006CDF60` (`+18h`) and
  `006CE4A0` (`+1Ch`), which are the largest and the most likely home of the landing bookkeeping
  that `lastLandingTime` names.
* `plane->vtable[38h]`: a per-plane scalar the taxi law and `006CF5B0` both read. Its body was not
  opened.
* Why the launch queue uses the hangar with the **maximum** local Z and the taxi target the hangar
  with the **minimum**; in authored content they may be the same hangar, but nothing enforces it.
* The frame of `plane+A4h`/`+A8h`/`+ACh`. `006CF420` and `006CF520` both return their point in the
  airfield's local frame, and `006CF730` samples the path in **world** space before building the
  queue frame (its tail past `006CF8C0` was not read, so it may transform at the end as `006CF520`
  does). The subtraction at `009CD62F` only makes sense if the plane's `+A4h` triple is
  parent-relative and the plane is a child of the airfield node while it is on the ground. That
  parenting was not verified here; `007C5F60` writes the same triple at `007C5FE8`-`007C6030` by
  adding the `GroundPitch` offsets to it.
