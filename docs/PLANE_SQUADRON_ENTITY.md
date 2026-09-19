# The plane squadron as the object the AI commands

Addresses: `007F2C60`, `004F0AD0`, `007F4580`, `007F4B43`, `007F4B55`, `007F4B60`, `007ED610`,
`007ECF80`, `007ECFD0`, `007EDA90`, `009FE080`, `00A2C790`, `007F0030`, `009534A0`, `007F3970`,
`007F3500`, `007F5009`, `0089539A`, `0082388E`, vtable `00D087C0`.

Packet `cc8_plane_squadron_entity`, read-only Ghidra analysis. Every descriptive name is a
hypothesis, not a recovered symbol. Reconstruction: `include/bsp/plane_squadron_entity.hpp`,
`src/plane_squadron_entity.cpp`. Host: `src/game_hosts_ai.cpp`. Report:
`reports/plane_squadron_entity.json`.

The object layout itself is `docs/PLANE_SQUADRON.md` (packet `cc2_squadron`), which read the
constructor, the creator, the plane-spawn attach, the rename fan-out, the removal and the tick to
completion. **None of that is re-derived here.** What this packet adds is the part that decides
whether the AI can command aircraft at all: what `+3D0h` actually holds, what `007EDA90` actually
tests, and which vtable slots carry a squadron-level action down to the member planes.

## 1. `+3D0h` is the member plane array, not a carrier link

`docs/AI_COMMAND_LIFETIME.md` reads `[squadron+3D0h]` as "the squadron's carrier link";
`docs/TORPEDO_RUN_IN_PATH.md` reads `ctl+3D0h` as "its unit array with count `ctl+3CCh`". The
second is right. Three independent witnesses from the listing:

| Witness | Site | What it shows |
| --- | --- | --- |
| the constructor | `007F2DA3..007F2DBB` | zeroes exactly five dwords, `+3D0h`, `+3D4h`, `+3D8h`, `+3DCh`, `+3E0h` |
| the spawn tail | `007F4B55`, `007F4B60` | `MOV [ESI+EDI*4+3D0h],EAX` with `EAX` the plane just created, then `+3CCh += 1` |
| the leader rotation | `007ED621`, `007ED618` | `MOV ESI,[ECX+EDX*4+3D0h]` indexed by `EDX`, bounded by `CMP [ECX+3CCh],EDX` |

A carrier link cannot be indexed by a member counter. `+3D0h` is `members[0]`, and `007ED610`
`BSP_PilotControl_PromoteFlightLeader` exists precisely to move a chosen member into that slot, so
`members[0]` is the **flight leader**.

## 2. `007EDA90` is the kamikaze-lead test

Full body, `007EDA90..007EDABA`, `__thiscall(squadron) -> bool`, `RET 0`:

```
007eda91  MOV ESI,[ECX+3D0h]          ; the FLIGHT LEADER, not a carrier
007eda99  JZ  007edab7                ; no leader -> false
007eda9d  MOV EDX,[EAX+5Ch]           ; the leader's IsKindOf
007edaa0  PUSH 17h                    ; MPlaneKamikaze
007edaa8  JZ  007edab7                ; not a kamikaze -> false
007edaaa  CMP byte [ESI+0C24h],0
007edab1  JNZ 007edab7                ; the byte is set -> false
007edab3  MOV AL,1
```

`17h` is `MPlaneKamikaze` (`docs/ENTITY_CLASS_IDS.md` row 17, class test `009534A0`, whose chain
`009534A4..009534C0` is `{17h, 0Fh, 05, 04, 02, 01, 0}`). `plane+C24h` is the authored `PilotFires`
byte (`docs/ATTACK_GATE_TAILS.md` section "Gate 2": two write sites, both in `FUN_007CD930`, whose
single caller is `007D673E` in `BSP_Plane_ReadPropertyBag`, so it is a load-time constant).

`007EDA90` is therefore the **same shape as the `kamikaze` attack gate `00604A50`**, applied to the
squadron's flight leader instead of to a plane directly. It asks "is this a kamikaze flight whose
pilot does not fire", and nothing about a carrier.

`009FE080 BSP_Entity_IsGroupableCombatant`, body `009FE080..009FE0AA`, read in full:

```
009fe088  PUSH 18h, vtable[5Ch]       ; a plane squadron?
009fe090  JZ  009fe0a0
009fe092  CALL 007EDA90
009fe097  NEG AL / SBB EAX,EAX / ADD EAX,1   ; a logical NOT
009fe09f  RET
009fe0a5  PUSH 6, vtable[5Ch]         ; otherwise: the ship base, and that is all
```

So **every plane squadron is a groupable combatant except a kamikaze flight**, and an individual
aircraft never is: `009FE0A5` pushes `6`, the ship base, and no arm pushes the plane base `0Fh`.

## 3. Squadron layout, the AI-facing fields

`docs/PLANE_SQUADRON.md` section 2 has the whole `0x414` table. The fields this packet reads:

| Offset | Meaning | Evidence |
| --- | --- | --- |
| `+C4h` | class id `18h`, `PlaneSquadronGen` | `007F2DEC` |
| `+348h` | the squadron's own AI command block, `0x22C` bytes | `007F5009` store; the `PUSH 22Ch` at `007F4FE1` |
| `+3C8h` | `WingCount` | `007F4778` |
| `+3CCh` | live member count | `007F4B60` (+1), `007F39ED` (-1) |
| `+3D0h..+3E0h` | five member plane pointers | section 1 |
| `+370h` | attack mode, `1` at construction | `007F2C60`, cleared at `007F31A0` |

`+348h` is **not** in `docs/PLANE_SQUADRON.md`'s table because the constructor never writes it: the
`memset 0` at `004F0B01` leaves it null and a later pass fills it. The store is `007F5009`
(`tools/store_census.py 0x348`), which sits past `007F4580`'s Ghidra body end `007F4B94` in a
routine Ghidra has not defined. Reading the surrounding bytes `007F4FD0..007F500F` shows a
`PUSH 22Ch` immediate, an allocator call, a `__thiscall` taking the squadron in `ESI`, and the
store. **The instruction boundaries there were not established from a function start**, so the two
call targets this packet computed did not survive `tools/verify_report_calls.py` and are withdrawn;
the `0x22C` size and the store are what stand. `contract: unread` for the whole routine.

## 4. How a squadron-level action reaches its planes

Three vtable slots of `00D087C0` carry it. The slot numbers are byte offsets read from
`00D088CC..00D088EB`:

| Slot | Target | Role |
| --- | --- | --- |
| `+114h` | `007ECFD0` | `MOV EAX,[ECX+348h]; RET` — hand back the squadron's AI command block |
| `+128h` | `007ECF80` | **the member fan-out** |
| `+164h` | `007F0030` | the message dispatcher (sole xref `00D08924`) |

### `007ECF80`, the fan-out

No Ghidra function; decoded from the raw bytes `007ECF80..007ECFBF`. `__thiscall(squadron, arg)`,
`RET 4`:

```
007ecf82  MOV EBX,ECX                 ; the squadron
007ecf84  XOR ESI,ESI                 ; i = 0
007ecf86  CMP [EBX+3CCh],ESI
007ecf8c  JLE 007ecfbd                ; no live member -> return
007ecf8f  MOV EBP,[ESP+10h]           ; the one argument
007ecf94  LEA EDI,[EBX+3D0h]          ; &members[0]
007ecfa0  MOV ECX,[EDI]               ; members[i]
007ecfa4  MOV EDX,[EAX+128h]          ; the SAME slot on the member
007ecfaa  PUSH EBP / CALL EDX
007ecfb3  CMP ESI,[EBX+3CCh] / JL 007ecfa0
007ecfbf  RET 4
```

A scan of `.text` for the `MOV r,[vtable+128h]` / `CALL r` pair (the script is
`local/scan128b.py`) finds nine dispatch sites. Two name the slot: `0089539A` inside
`BSP_LuaBinding_SetSkillLevel` (`00895250`) and `0082388E` inside `BSP_UnitInstance_SEntityInit`
(`00822C20`). **Slot `+128h` is the skill-level setter**, and the squadron's override is "apply it
to every live member". The other two squadron-shaped callers are `006D3CDC` in
`BSP_AirField_ReadRunwayProperties` and `0074215C` in `BSP_LandConvoy_ResolvePathReference`, both
container classes with their own member arrays.

So the native's squadron-to-member mechanism is a **virtual broadcast over `+3D0h` bounded by
`+3CCh`**, and `007F3820` (the rename, `docs/PLANE_SQUADRON.md` section 4) is the second instance
of exactly that loop. `coverage: complete` for `007ECF80`; `contract: unread` for what a squadron
does with a `moveto` or `attackmove` descriptor, because the arms of the `+164h` dispatcher
`007F0030` other than `BCh` and `BEh` were not decoded by this packet
(`docs/TORPEDO_ATTACK_MODE.md` decoded `BCh` -> `+370h` and `BEh` -> `007ED610`).

### `007ED610`, the flight-leader rotation

`__thiscall(squadron, int index)`, `RET 4`, body `007ED610..007ED64C`, reached from the `+164h`
dispatcher's `BEh` arm. `007ED614` rejects `index <= 0`, `007ED618` rejects `index >= +3CCh`; the
body saves `members[index]`, shifts `members[0..index-1]` up one slot from the top down
(`007ED630..007ED63D`), writes the saved plane into `+3D0h` and calls `007ED260(squadron)`.
`contract: unread` for `007ED260`.

### What the AI itself reads

`00A2C790`'s per-member chain, `00A2C7F0..00A2C83C`: `member->vtable[+114h]()`, and when that is
non-null it is called twice more, once through `0071EB60` and once through `0071BE40`, and the
group's own command object at `group+564Ch` consumes the pair through its `vtable[+24h]`. For a
squadron `vtable[+114h]` is `007ECFD0`, so what the AI reads out of a squadron is the `+348h`
block of section 3.

## 5. Host methods

One row per native call site this packet models. `src/plane_squadron_entity.cpp` carries the rules;
`src/game_hosts_ai.cpp` binds them.

| Site | In | Callee | Host method | this / args | ret |
| --- | --- | --- | --- | --- | --- |
| `007F4735` | `007F4580` | — | `plane_squadron_wing_count_007f4754` | key absent | `3` |
| `007F4754` | `007F4580` | — | `plane_squadron_wing_count_007f4754` | `prop+0Ch` | `max(1, n)` |
| `007F4B43` | `007F4580` | — | `plane_squadron_attach_plane_007f4b43` | squadron; plane | spawn index |
| `007F4B55` | `007F4580` | — | same, the `+3D0h` store | squadron; plane | — |
| `007F4B60` | `007F4580` | — | same, the `+3CCh` bump | squadron | — |
| `007EDA91` | `007EDA90` | — | `plane_squadron_flight_leader` | squadron | `members[0]` |
| `007EDA90` | `009FE080` | `007EDA90` | `plane_squadron_excluded_007eda90` | lead facts | bool |
| `009FE088` | `009FE080` | vtable `+5Ch` | `plane_squadron_combatant_facts` | squadron | facts |
| `007ED610` | `007F0030` arm `BEh` | `007ED610` | `plane_squadron_promote_flight_leader_007ed610` | squadron; index | bool |
| `007F39ED` | `007F3970` | — | `plane_squadron_remove_plane_007f39ed` | squadron; plane | bool |
| `007ECF80` | vtable `+128h` | member `vtable[+128h]` | `plane_squadron_broadcast_to_members_007ecf80` | squadron; arg | count |
| `004F0AD0` | scene loader | `007F2C60` | `Impl::build_squadrons` | scene entity | squadron |
| `00A2C790` | group tick | member `vtable[+114h]` | `close_member_current_target` (flight leader stands in) | squadron | descriptor |
| `0077D600` | `00A02020`, `00A14A6E` | `0046AAB0` | `Impl::issue_order` / `Impl::fan_out_to_members` | squadron; token, target | bool |

### Substitutions, each labelled at its address

* **`007F4580`'s per-wing loop.** This process creates exactly one unit per scene entity, and a
  scene aircraft entity **is** a `PlaneSquadronGen`, so the unit the loader made is the squadron's
  first wing. `build_squadrons` builds the squadron object and a **one-wing** member array; the
  remaining `WingCount - 1` planes are not spawned, because unit creation belongs to
  `src/game_hosts_units.cpp`, which this packet does not edit. The member array, the class id and
  the leader are real; the wing count is not. `coverage: partial` for `007F4580`.
* **`007EDAAA`, the `+C24h` byte.** No reader for `PilotFires` exists in this process, so the byte
  is read as clear. The consequence is visible and benign: `007EDA90` can only answer true for a
  squadron whose leader is `IsKindOf(17h)`, and the census reports `excluded_007eda90=0` for all
  three missions, which is the correct answer for them (no kamikaze flight is authored).
* **`007ECFD0` / `squadron+348h`.** This process holds no `0x22C` AI command block, so the flight
  leader's own command descriptor stands in wherever `00A2C790`'s chain would read it.
* **The order fan-out.** A squadron has no scene-registry name here, so a squadron-level order is
  delivered as one `0077D600` order per member plane, in `+3D0h` order. That is `007ECF80`'s shape
  applied to an order, and it is a substitution for the unread order arms of `007F0030`, not a
  reconstruction of them.
* **Order de-duplication.** `0077D600` replaces an entity's outstanding order; this process's order
  ring appends. `00A10DC0` calls `00A02020` on every fixed step and its only native gate is the
  squared distance at `00A0205C`, so a follower that never closes the distance would append one
  order per step for the whole mission. The host suppresses a re-issue only when token, target and
  point are all unchanged; any change is always issued, and the census reports what was suppressed.
  This is a host rule, not a native one.
* **Seeding.** A plane a squadron owns is not an AI candidate of its own. The native seeds the
  scene's `PlaneSquadronGen` entities and the planes live only in `+3D0h`; `009FE080` would refuse
  a loose plane anyway. Keeping both would double-order the same aircraft.

## 6. Corrections

Appended, not rewritten, in the docs concerned.

* `docs/AI_COMMAND_LIFETIME.md` lines ~36-60, ~119, ~125 and the row
  `ai_squadron_carrier_link` in its table call `[squadron+3D0h]` the squadron's **carrier link**.
  It is the member plane array and `[squadron+3D0h]` is the flight leader; `IsKindOf(17h)` is
  `MPlaneKamikaze` and `+C24h` is the authored `PilotFires`. Section 1 and section 2 above carry
  the listing. The *logic* that document records is correct and `src/ai_command_lifetime.cpp`
  implements the right rule; only the naming of the three inputs is wrong. Contract for that
  packet's owner: rename `squadron_has_carrier`, `squadron_carrier_is_kind_17` and
  `squadron_carrier_flag_0c24` in `include/bsp/ai_command_lifetime.hpp`. This packet did not touch
  that header.
* `docs/AI_CLOSE_ATTACK_TICK.md` line 56 carries the same wording ("the carrier link passes
  `007EDA90`'s shape inline") for the inline copy at `00A143ED..00A14413`. Same correction.
* `docs/TORPEDO_RUN_IN_PATH.md`'s reading of `ctl+3D0h` as the unit array with count `ctl+3CCh` is
  **correct** and needs no change.
* `docs/TORPEDO_ATTACK_MODE.md` calls `007F0030` "the pilot control block's message dispatcher".
  Its only reference is the `.rdata` dword at `00D08924`, which is offset `+164h` of the plane
  squadron's primary vtable `00D087C0`, so the "pilot control block" it dispatches for is the
  squadron object itself. The document's decoding of the `BCh` and `BEh` arms is unaffected.

## 7. `no_ghidra_function`

Routines read from the raw listing with no Ghidra function, with their inclusive final byte.

| Start | End (inclusive) | What it is |
| --- | --- | --- |
| `007ECF80` | `007ECFBF` (`RET 4` at `007ECFBD`) | the member fan-out, vtable `00D087C0` slot `+128h` |
| `007ECFD0` | `007ECFD6` (`RET` at `007ECFD6`) | `MOV EAX,[ECX+348h]; RET`, slot `+114h` |
| `007EFA70` | `007EFA74` | the five-byte `JMP 00928860` thunk, slot `+98h` (already recorded by `docs/PLANE_SQUADRON.md`) |

The `+348h` store at `007F5009` also sits outside any Ghidra function. This packet did not
establish that routine's entry or its instruction boundaries, so it is **not** listed here with a
start or an end address.

## 8. Validation

`./tools/run_game.ps1`, 3200 frames, `--mission-frames 3000 --mission-frame-seconds 0.05`.
"Before" is this worktree at `f5400e43f`; "after" is the commit this document belongs to.

### Aircraft and squadrons

| Mission | units | aircraft (`0Fh`) | squadrons before | squadrons after | members |
| --- | --- | --- | --- | --- | --- |
| IJN01 | 321 | 33 | 0 | 33 | 33 |
| USN01 | 77 | 20 | 0 | 20 | 20 |
| USN02 | 32 | 0 | 0 | 0 | 0 |

"aircraft" is the world-list census line's list `15` (the plane base `0Fh`); no run of any mission,
before or after, registers a native class-`18h` entity, which is the measurement that says this
process never built a squadron.

### The AI census

| Mission | | squadrons | members in groups | `007EDA90` true | squadron commands | member orders | groups | `served` | `tick_orders` | commands |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 34 |
| IJN01 | after | 33 | 33 | 0 | 141 | 141 | 3 | 0 | 237 | 171 |
| USN01 | before | 0 | 0 | 0 | 0 | 0 | 4 | 0 | 0 | 17 |
| USN01 | after | 20 | 20 | 0 | 477 | 477 | 4 | 0 | 486 | 485 |
| USN02 | before | 0 | 0 | 0 | 0 | 0 | 2 | 616 | 5 | 635 |
| USN02 | after | 0 | 0 | 0 | 0 | 0 | 2 | 616 | 5 | 635 |

Aircraft that receive an attack task, from the `pilot attack` line:

| Mission | ordered before | ordered after | mean distance closed before | after |
| --- | --- | --- | --- | --- |
| IJN01 | 4 | 33 | -149899.7 m | 0.0 m (see below) |
| USN01 | 14 | 14 | -20361.5 m | +1435.0 m |
| USN02 | 0 | 0 | none | none |

Gunnery, from the `gunnery units` line:

| Mission | candidates before | after | assigns before | after |
| --- | --- | --- | --- | --- |
| IJN01 | 52 | 1841 | 200 | 2766 |
| USN01 | not recorded before | 1059 | not recorded before | 1082 |

### Attribution

Every change on IJN01 and USN01 is attributable to one fact: `009FE080`'s `009FE088` arm now has
something to answer for. Before, no entity in the process answered `IsKindOf(18h)`, and the ship
tail `009FE0A5` cannot admit an aircraft, so the aircraft half of every AI arm was empty. USN02
authors no aircraft at all, so nothing about it should change, and it does not: `served`,
`attackmove`, `commands`, `groups_created` and `members_added` are **bit-identical** before and
after, which is this packet's regression evidence.

The group churn the first attempt produced (3002 groups created, 98998 members evicted) was a
host bug of this packet's own making, not a native fact: `evict_invalid_members` called
`units.unit_side_0054` with a squadron candidate index, which is out of the unit table and returns
`-1`, so every squadron was evicted the same pass it was added. It is fixed by routing the side
lookup through the flight leader.

### What did NOT change, and is named

* **`served` is still 0 on IJN01 and USN01.** The close-attack pass `00A13B60` reaches no member.
  It was 0 on both missions **before** this packet as well, with `scored=0`, and USN02 serves 616
  with the same code, so the pass works and this is not a squadron fact. The promoted group on
  those two missions is not the one the squadrons are in, or its member count is zero at the tick.
  Not established. The member gate is `00A143ED..00A14413` and the promotion is `00A2BD00`.
* **`plane+C24h`, `PilotFires`, has no reader in this process**, so `007EDA90` cannot answer true
  here and `excluded_007eda90` is 0 on all three missions. For these three missions that is also
  the right answer, because none authors a kamikaze flight, but the input is a substitution and a
  mission that did author one would be served wrongly.
* **IJN01's `pilot attack` ranges are 0.0 after.** Thirty-three aircraft now receive attack tasks
  where four did before, but the tracker records zero first and last range for them. Whether that
  is a tracker sampling artefact or a target that resolves to nothing was not established; the
  tracker is not this packet's code.

## 9. Follow-up packets

* `007F0030`'s remaining four live arms over `4Bh..BEh`, which is where a squadron's `moveto` and
  `attackmove` handling has to be. The tables are `007F0204` (byte) and `007F01E8` (jump).
* The routine that contains `007F4FDC..007F500F` and the constructor `0084D80E` of the `0x22C`
  block at `squadron+348h`: that block is what `00A2C790` reads, so it is the squadron's real AI
  command object.
* `007F4580`'s per-wing loop in the units host: spawning `WingCount` planes per scene entity rather
  than one, which is the only part of the squadron model this packet left at one wing.
* `007ED260`, called by both `007ED610` and `007F3970`.
* `007F3500 BSP_PlaneSquadron_CloneFrom`'s caller, still unlocated.

## The first of those three is now a packet (`cc8_plane_squadron_members`)

The first bullet above — "spawning `WingCount` planes per scene entity rather than one, which is the
only part of the squadron model this packet left at one wing" — turned out to be the root cause of
the torpedo stream's stall, reached from the opposite direction.

`007EEF30 BSP_PilotControl_IssueReleaseOrders` walks this document's `+3D0h` member array under its
`+3CCh` count to issue release orders, and `007EE7F0 BSP_PilotControl_RefreshArmedFraction` computes
an armed fraction over the same array from each `member+5Ch`, storing it at `+374h`. Both collapse
when the array is empty: `007EE7F0` writes `+374h = 0` at `007EE891`, and `007EEF40`'s count test
`CMP [ESI+3CCh],EBX / JLE` fails on its own. This host has one plane per `PlaneSquadronGen` row and
one plane per air-ops launch, so `+3CCh` is always 0, and USN01's five kind Eh torpedo tasks never
issue an order.

`+374h` is named by its writer. `+390h`, the other operand of `007EEF40`'s float test, has **no
located producer**: zero LEA references image-wide and none of the 39 stores at that offset is a
float store on this class (the `BSP_GameTuning_LoadFromPlaneGlobals` pair is the tuning singleton at
the same offsets, a collision). It is unread rather than absent.

The full brief, including the census multiplier that makes 1566 authored squadrons about 4,800
aircraft and the list of places the one-plane stand-in lives in the host, is the handoff section at
the end of `docs/PLANE_SQUADRON.md`.

## The host packet re-read the spawn tail, and what it changes here

Packet `cc8_plane_squadron_host` re-read `007F4580` mode 1 and `007F2C60` from the listing rather
than from this document. The readings this document depends on all hold: the five-slot array at
`+3D0h`, the count at `+3CCh`, the back pointer `007F4B49`, the spawn index `007F4B43` stamped from
the pre-append count, and the `max(1, authored)` wing count defaulting to 3. The evidence is the
new section in `docs/PLANE_SQUADRON.md`; three things there bear on this document.

1. **`+3ECh` is a byte store** (`007F4B6E MOV byte [ESI+3ECh],1`), which is what
   `PlaneSquadronEntity::dirty` already models.
2. **`+378h` is seeded set** by `007F2D1E MOV byte [ESI+378h],1`. Section 4 leaves the `+378h` arm
   of `007EEF78` unexplained; the constructor answers it. A squadron that has not yet been through
   `007ED3C0` issues a release order to **every** member without consulting
   `007B8AD0`, because `007EEF62 CMP byte [ESI+378h],0 / 007EEF6B JNZ` jumps straight to the raise.
3. **Membership has two more producers than this document lists.** `007ED0D0
   BSP_PlaneSquadron_InsertPlaneSorted` writes `plane+9D4h` at `007ED0E6`, and
   `007D5D20 BSP_Plane_ReadPropertyBag` writes it at `007D68D5` and clears it at `007D694B`. The
   fan-out reading in section 4 is unaffected - it walks `+3D0h` whatever filled it - but a host
   that reconstructs only `007F4580` is reconstructing one of three producers. Both are
   `contract: unread`.

`+390h` stays unlocated. The one float store to the offset that a reader will find, `0079CD36`, is
on the `0x410` object `FUN_0079CBD0` allocates at `0079CC2D`, not on the `0x414` squadron: that
routine replaces a squadron argument with `members[0]` at `0079CC13` before it builds anything, and
the exhaustive `+9D4h` store census (ten writers, all in the plane/squadron band) shows its object
never becomes a `plane+9D4h`. So the sibling is named rather than adopted.
