# Kill credit and attribution (packet `cc2_kill_credit`)

Addresses: 0077ce60, 0091bda0, 0077cc80, 006e22d0, 00779a00, 009ffd20, 0090e850, 0090ede0,
0090c170, 0090ec40, 0070cc30 (read only), 00959450 (caller only).

Ghidra was **read-only** for this packet. Every name below is a hypothesis, not a recovered
symbol. Follow-up of `docs/SCORING_BODIES.md`, which established the two kill trees and left
`0077CE60`'s `+2DCh` fallback and two thirds of `0091BDA0` unread; both are read here.
`docs/SCORING_BINDING_TABLE.md` owns the 284h record and its manager arithmetic,
`docs/UNIT_HIT_PATH.md` the hit record, `docs/AWARD_GRANT.md` the achievement registry,
`docs/ENTITY_CLASS_IDS.md` the class ids every `vtable[5Ch]` test below names.

## 1. `0077CE60` — the attribution writer

`__thiscall(Unit* victim /*ECX*/, HitRecord* hit)`, `RET 4` at `0077D19D`, body
`0077CE60..0077D19F`. Coverage: **complete**.

### The gate and the damage test

| site | step |
| --- | --- |
| `0077CE66`..`0077CE88` | require `victim+5Ch != 0`, `victim+5Dh == 0`, `victim+60h == 0`, `victim+5Eh == 0`; otherwise return with nothing done |
| `0077CE95` | `damage = (hit+34h == -1) ? 00470740(hit, 0.0f) : 00470510(hit, 0.0f)` — `hit+34h` is the hull segment index `docs/UNIT_HIT_PATH.md` names |
| `0077CEBE` | `COMISS` against `[00D7A218]`; `JBE` skips the whole attribution block and goes straight to the tail |
| `0077CED7` | `victim+2E4h = [00F876A4]`, the mission clock stamp, written before the shot is fetched |

The four gate bytes are tested a second time at `0077CEFB`..`0077CF1D`, after the virtual call
that resolves the source record, because that call can re-enter.

### The source record

`shot = [hit+4h]`; `src = shot->vtable[108h]()`. Both must be non-null. `src` is the shot's
own attribution source, the object whose base constructor is `006E22D0` (section 3): its
`+98h` is a weak reference with `+14h` = the owning unit and `+18h` = a `u16` owner id, its
`+ACh`/`+B0h` are those two fields seen from the object, `+CCh` is an entity whose `+54h` is a
side, `+1Ch` is a player slot and `+4h` is the weapon class descriptor.

### The rule table

Every row is a store on the victim. Rows the routine does not reach leave the field unchanged.

| Field | Site | Written from |
| --- | --- | --- |
| `+2E4h` float | `0077CED7` | `[00F876A4]`, the mission clock |
| `+2D4h` | `0077CF29` | `[[src+4h]+8h]`, the ordnance kind |
| `+2D0h` | `0077CF3E` | `[[src+CCh]+54h]`, the attacker side |
| — | `0077CF44` | `word [src+B0h] == 0` abandons everything below |
| `+2E8h` | `0077CF58` / `0077CF73` | `1` when the prior value is negative; else `0` when `+2C4h` is set and differs from `[src+ACh]`; else unchanged |
| `+2C4h`, `+2C8h` | `0077CF84` | `0077CC80(&victim+2B0h, &src+98h)` re-points the weak reference at `[src+ACh]` and copies the `u16` at `[src+B0h]` |
| `+2CCh` | `0077CFBA` | the attacker's unit class, `(*(void***)(attacker+170h))[0]()` |
| `+2D8h` | `0077CFC5` | `attacker+180h`, or `[src+1Ch]` when `attacker+180h > 8` |
| `+2DCh` | `0077CFDD` | `[src+1Ch]` when the attacker `IsKindOf(0Fh)` (a plane) |
| `+2DCh` | `0077D010` | `attacker[+1ACh + 00779A00(src+4h)*4]` when the attacker `IsKindOf(06h)` or `IsKindOf(1Bh)` |
| `+2DCh` | `0077D02E` | `attacker+1B0h` when the attacker `IsKindOf(0Fh)` and that value is `<= 7` |
| `+2D8h` | `0077D067` | a land fort (`IsKindOf(1Bh)`) whose credited slot is above 7 and whose `+71Ch` is set takes `[[fort+71Ch]+180h]` |
| `+2E0h` `u16` | `0077D07E` | `word [[src+ACh]+174h]`, the attacker's type id |
| `+2D8h` | `0077D08A` | `[src+1Ch]` on the arm where there is no attacker unit at all |
| `+2CCh` | `0077D0DE` | forced to `0Ch` on the kamikaze branch, which also sets `[src+1Ch] = [shot+1ACh]` |

**The `+2DCh` fallback `docs/SCORING_BODIES.md` left unread is the `0077D001`..`0077D010`
arm.** It is taken when the attacker answers `IsKindOf(06h)` or `IsKindOf(1Bh)`, and the index
comes from `00779A00([src+4h])`, a jump table over the ordnance kind. The register the
decompiler lost is `EBX`; filtering the whole listing for it (rule 8) gives four writes —
`0077CECD` `[hit+4h]`, `0077CFA3` `attacker+180h`, `0077CFC2` `[src+1Ch]`, `0077D07A` the
reload of the `[hit+4h]` spill at `0077CED3` — so `EBX` at `0077D090`..`0077D0D5` is the shot
entity on both the attacker and the no-attacker path.

The kamikaze branch condition, all on the shot entity `[hit+4h]`:
`(IsKindOf(0Fh) && 007B93E0(shot, 0)) || IsKindOf(17h) || (IsKindOf(06h) && 00779AA0(shot))`.
`00779AA0` is `bool __fastcall(unit)`: `desc = unit[+538h]`, true when `desc[+510h] > 0` or
`desc[+514h] > 0`.

### The parent propagation

`0077D0E8`..`0077D122`: `parent = victim->vtable[140h]()`; when it is non-null, is not the
victim and answers `IsKindOf(02h)`, the same `0077CC80` copy runs into `parent+2B0h` and
`parent+2D8h = [src+1Ch]`. Only those two fields propagate.

### The tail — it runs on every hit, not only damaging ones

`0077D12E` is the target of the `JBE` at `0077CEBE`, so the three calls below run whenever the
four gate bytes pass, whatever the damage was.

| site | call |
| --- | --- |
| `0077D13C` | `00915F20([game+21A0h], victim, hit)`, the damage-trace recorder |
| `0077D149` | `00988510([00F8A0C4], victim, hit)`, the `hit` mission event |
| `0077D193` | `0090EC40([game+21A0h], [hit+4h], 0)` when `[hit+4h]` answers `IsKindOf(0Fh)`, does **not** answer `IsKindOf(17h)`, and the victim does not answer `IsKindOf(0Fh)` |

`0090EC40` is a per-slot tally at `record+12Ch`, indexed by `[shot+1B0h]` (rejected above 7)
and keyed by `word [shot+174h]`, abandoned when the shot's own class is `0Ch`. Coverage:
**partial**, only its entry conditions and the record offset were read.

## 2. `0091BDA0` — the kill writer

`__thiscall(ScoringManager* /*ECX*/, Unit* victim)`, `RET 4` at `0091C551`, SEH frame, body
`0091BDA0..0091C553`. Coverage: **complete**. Its sole caller is `00959450` at `00959519`,
with `ECX = [[00E188A8]+21A0h]` and one pushed argument, the destroyed unit.

### Entry conditions

| site | rule |
| --- | --- |
| `0091BDC4` | return when `[game+1FE4h] == 2` |
| `0091BDD8`..`0091BDF9` | when `manager+1484h` is set, return if the victim's `+54h` equals `[game+18CCh + [game+18ECh]*4]+28h`, the local player's side |
| `0091BE29`, `0091BE3F` | `0090C170`, a `find` on the `entity -> int` map at `manager+1488h`, keyed by `victim->vtable[140h]()`. A hit returns. Nothing in this body inserts into that map |
| `0091BE4D`..`0091BE65` | a victim answering `IsKindOf(1Bh)` whose `vtable[140h]()` is not itself returns |

### The loss counters

`0091BE90`..`0091BF7A`, eight iterations with `EBX = 18CCh + slot*4`, `EBP = slot` and
`EDI = manager + 1FCh + slot*284h`. Per slot:

* the victim's `+54h` must be `0` or `1`; any other side counts nowhere;
* count when `009FFD20(victim) == slot`, or when `[game+18CCh + slot*4]+28h` differs from the
  victim's side;
* the map is `record[slot]+1ECh` for side `0` and `record[slot]+1F8h` for side `1` — the
  allied and Japanese loss maps `docs/SCORING_BINDING_TABLE.md` named — indexed by
  `005070C0` with `victim->vtable[28h]()` as the key, then `ADD dword ptr [EAX],1`.

`009FFD20` is `int __fastcall(unit)`, body `009FFD20..009FFD5C`: `-1` when `unit+180h` is
exactly `8`, `unit+180h` when `<= 7`, `-1` in multiplayer otherwise, and in single player `0`
when the unit's side matches the local player's and `4` when it does not.

### The kill trees

Unchanged from `docs/SCORING_BODIES.md`: slot `[victim+2D8h]` rejected above 7 at `0091BF97`,
tree `+B4h` when `[victim+2D8h] == [victim+2DCh]` and `+C0h` otherwise, three keys pushed in
the order relative party, attacker class, victim class, `ADD dword ptr [EAX],1` at `0091C046`.

### The per-type tallies

`0091C057`..`0091C0CF`. Both need `[victim+2C4h]` non-null and `[victim+2DCh] <= 7`, both are
`int -> int` maps indexed by `00667EB0` with `word [victim+174h]` as the key, and both are
addressed with the **originating** slot, not the credited one.

| map | record offset | condition | sites |
| --- | --- | --- | --- |
| ship kills by victim type | `+1A4h` | attacker `IsKindOf(06h)` | `0091C061`, `0091C085`, `0091C091` |
| plane kills by victim type | `+198h` | attacker `IsKindOf(0Fh)` | `0091C09F`, `0091C0C3`, `0091C0CF` |

### The awards

The registry is `docs/AWARD_GRANT.md`'s achievement table at `[00E19900]+10h`, looked up with
`0050FC30`; the threshold is `vector::at(0)` (`00481750`) on the row's `+44h`, the only parsed
column that document leaves without an offset, so `+44h` is `Params` — a hypothesis, not a
read of `006B9450`. The grant is `0090EDE0`, `__thiscall(manager, slot, NativeString*, value,
0)`, `RET 10h` at `0090F0DD`: it applies locally through `0090C000` and forwards a type-`14h`
session message when the slot is not the local player, `[game+5D4h] == 0Dh` and
`[game+1FE4h] == 1`.

| award | condition | value | sites |
| --- | --- | --- | --- |
| `Counter_RUA_DU` in the `name -> int` map at `record+6Ch` | attacker `IsKindOf(07h)` (`MDestroyer`), `[victim+2D4h] == 0Ah`, `[victim+2D0h] != [victim+54h]` | `+1` | `0091C0DD`, `0091C0E9`, `0091C0FC`, `0091C125` |
| `RUA_DU` | that counter, **after** the increment, equals `Params[0]` of the `RUA_DU` row exactly (`CMP` then `SETZ`) | `1` | `0091C194`, `0091C1E5` |
| `RUA_BU` | `[victim+2DCh] <= 7`, attacker `IsKindOf(0Dh)` (`MBattleship`), victim `IsKindOf(06h)`, kill distance strictly greater than `Params[0]` of `RUA_BU`, and `00803510(victim+54h, victim+2D0h) == 1` | `CVTTSS2SI` of the distance | `0091C276`..`0091C2FD`, `0091C36A` |
| `GA_WY` | attacker `IsKindOf(0Dh)`, `(07h)`, `(0Ah)` or `(09h)`; `[victim+54h] != [[victim+2C4h]+54h]`; `[[victim+2C4h]+1B0h] != 0`; and `00923BE0(attacker) * 100.0 <= Params[0]` of `GA_WY` | `1` | `0091C3F8`..`0091C4B8`, `0091C517` |

The kill distance is `0042B2F0` on `victim+FCh..104h` minus `attacker+FCh..104h`, both poses
refreshed first with `00414DB0` when their `+C8h` byte is clear (`0091C205`..`0091C271`).
`00D7A220` is the double `100.0`. Note that `0091C2EC` calls `00803510` with the victim's side
as the subject and the attacker's as the reference, the opposite of the kill-tree call at
`0091BFD8`; for the `1` (`ENEMY`) answer the order does not matter.

### The kill list at `record+1BCh`

`0091C392`..`0091C3F5`, reached whenever `[victim+2C4h]` is non-null. The key is
`(word [victim+174h] << 16) | word [victim+2E0h]`, that is victim type id over attacker type
id. `0090E850` is the `uint32 -> 2Ch-byte entry` `operator[]`; it inserts a default entry
whose prototype is the `REP MOVSD` of `0Bh` dwords at `0090E8CB`. Three fields are written:

| entry offset | value | site |
| --- | --- | --- |
| `+20h` | `[victim+2D8h]`, the credited slot | `0091C3CB` |
| `+18h` | `[victim+2D4h]`, the ordnance kind | `0091C3DC` |
| `+2Ah` byte | `[victim+2E8h] != 0`, the sole-attacker flag | `0091C3F5` |

## 3. The two projectile flags

### `[shot+45h]`, the impact-effect gate

Producer: **`006E2353`**, inside `006E22D0`, the shot base constructor. `MOV byte ptr
[ESI+0x45],AL` with `AL = 1`, twelve instructions after the same routine initialises the
`+98h` weak-reference record (`006E2304`..`006E2321`, vtable `00CF938C`, `+10h` byte `1`,
`+14h` and `+18h` cleared) and clears `+C8h` and `+CCh`. `006E22D0` sets `[this] = 00CF93B0`
and is called by `BSP_BombProjectile_Construct` (`006E2670`) and
`BSP_ProjectileTickableEntity_Construct` (`006E7B00`).

The identification rests on three offsets the constructor and the two consumers share: `+98h`
(the record `0077CC80` copies into `victim+2B0h`, whose live fields are exactly the `+14h` and
`+18h` the constructor seeds), `+CCh` (`0084BCBD` writes the hit position through it,
`0084BF3F` loads it) and `+45h` itself.

Meaning: the default is **enabled**. `0084BDB0` `CMP byte ptr [ECX+0x45],0x0` with
`ECX = [staging+0h]` skips `0084BDB6`..`0084BDEA`, the whole impact-effect dispatch through
`0084B8C0`, when it is clear. **No other write to `+45h` exists.** Method: `scan-bytes` over
`.text` for `C6 xx 45`, `88 xx 45` and `8D xx 45` across every non-SIB `modrm` with `mod=01`,
and for the wide forms `C7 xx 44` and `66 C7 xx 44` that would cover the byte. The only other
hits in the projectile range belong to the replicated-state block at `[unit+DECh]`
(`007B8DA0`, `007B8DC0`, `007B8DE0`, `007C5F60`), a different class with `+11h`, `+45h`,
`+48h` and `+4Ch`. A `memcpy` or an assignment operator could still carry the byte; that class
of writer was not scanned.

### `[proj+290h]`, the flak burst bias

Producer: **`0070CE65`..`0070CE6B`**, inside `0070CC30` `BSP_FlakBulletClass_CreateProjectile`:
`FLD float ptr [ECX+0x70]; FMUL float ptr [EDX+0x4]; FSTP float ptr [ESI+0x290]`, where
`ECX = [ESP+10h]` is the flak bullet class descriptor spilled at `0070CC53` and `EDX` is the
routine's seventh stack argument (`RET 1Ch` at `0070CE90`; the argument slots are `[ESP+74h]`
through `[ESP+8Ch]` at the `0070h` baseline). `classDesc+70h` is `Blast.BlastRange` in
`docs/WEAPON_CLASS_DESCRIPTOR.md`, so the bias is the blast range scaled by a float the caller
supplies. The seventh argument itself is `contract: unread`.

**The derived class is the flak bullet projectile and it is `298h` bytes**, not `284h`:
`0070CC4E` `PUSH 0x298` into `00BF55BE` and `0070CC5C`..`0070CC67` a `memset(obj, 0, 0x298)`.
`006E8430 BSP_ProjectileClass_CreateProjectile` does the same with `284h`. So `+290h` and
`+294h` are the two dwords past the base projectile, and every byte of both classes starts at
zero.

## 4. `00959450` as the caller

Body `00959450..00959608`. Its gate, already recorded in its plate comment and in
`docs/UNIT_INSTANCE_UPDATE.md`, is: `[00F876A4] > 0.0f`, `unit+70h == 1`, and
`[[game+19CCh]+4ACh]` set. The two arguments at `00959519` are confirmed:
`ECX = [[00E188A8]+21A0h]` at `00959512` and `PUSH EDI`, the destroyed unit, at `00959518`.

The other calls on the destroy path: `00878990` unconditionally at `0095945B`; `009813A0`, the
`kill` mission event, at `00959507`; `004B4B00` at `0095951E` whose result is compared with the
unit; then, only when they match and the mission state at `[00E198C4]+20h` is none of `29h`,
`2Bh`, `2Ch`, `2Dh`, `00565FB0`, `0068A120` and `004CC460(34h, 0)`; and the tail jump to
`004C0890` with `ECX = 0`. None of those is a scoring record.

## 5. Readers of the attribution block

Scan method: `scan-bytes` for `?? ?? <disp32>` over `.text` restricted to `0076A000-00A40000`
for each of `+2C4h`, `+2CCh`, `+2D0h`, `+2D4h`, `+2D8h`, `+2DCh`, `+2E0h`, `+2E4h`, `+2E8h`,
with SIB (`24`) and long-jump (`0F 8x`) encodings filtered out. Hits at `0099B260`,
`0099B450`, `0099D300`, `009A4DC0`, `009A5000` and the rest of the `0099`-`009C` range are the
pilot-bot classes at the same displacements, a different object.

| routine | fields | note |
| --- | --- | --- |
| `0077EED0` `BSP_UnitOwnerEntity_Construct` | writes `+2C4h`, `+2CCh`, `+2D0h`, `+2D8h`, `+2E4h`, `+2E8h` | the block's initialiser; `+2E8h` starts here, which is what makes `0077CF4F`'s negative test meaningful |
| `00915F20` | reads `+2C4h`, `+2D0h`, `+2D4h`, `+2DCh`, `+2E0h` | the damage-trace recorder `0077CE60` itself calls; body `00915F20..00916957`, `contract: unread` |
| `0091BDA0` | the kill writer | this packet |
| `009813A0` | reads `+2D8h` at `0098181E` and `00981948` | the `kill` mission-event reporter |
| `009744C0` | reads `+2C4h` at `00974523`, compares `+2D8h` at `009747F0` | `contract: unread` |
| `00906180` | reads `+2CCh`, `+2D0h`, `+2D4h`, `+2DCh`, `+2E0h`, `+2E8h` | six of the nine fields in one body; `contract: unread`, a likely serialiser |
| `00907510` | reads `+2C4h` | `contract: unread` |
| `0077C160` | reads `+2D8h` | `contract: unread` |
| `00877B90` `BSP_UnitInstance_SetHealth` | writes `+2E8h` at `00877C20` | resets the sole-attacker flag |

## Corrections to earlier documents

| document | was | is | evidence |
| --- | --- | --- | --- |
| `docs/SCORING_BODIES.md` | `+2D8h` is `attacker+180h` falling back to `[hit+1Ch]` when it exceeds 7 | it falls back only when `attacker+180h` is **above 8**, and the fallback source is the shot's source record `[src+1Ch]`, not the hit record | `0077CFB7` `CMP EBX,0x8` then `0077CFC0` `JBE` past the store; `0077CFC2` `MOV EBX,[EBP+0x1c]` with `EBP` the `vtable[108h]` result |
| `docs/SCORING_BODIES.md` | `+2D0h` is `[[hit+CCh]+54h]` and `+2DCh` is `[hit+1Ch]` | both read the shot's source record, not the hit record | `0077CF2F` `MOV ECX,[EBP+0xcc]` and `0077CFDA` `MOV ECX,[EBP+0x1c]`, `EBP` set at `0077CEF1` from `shot->vtable[108h]()` |
| `docs/SCORING_BODIES.md` | `00959450` reaches `0091BDA0` "only on the branch where the owner answers `+18h(17h)` or `+18h(6)` with a set byte; otherwise the kill goes to `009813A0` and no tree is touched" | that branch gates `009813A0` alone. `0095950C` is the fall-through of the `009813A0` call as well as the target of the `JNZ`, so `0091BDA0` runs on both arms | `009594FE` `JNZ 0x0095950C` and `00959507` `CALL 0x009813a0` followed by `0095950C` in program order |
| `docs/SCORING_BODIES.md` | `0091BDA0`'s per-slot string counters are "over `game+18CCh..18ECh`" | that range is the loop cursor over the player-record pointers; the counters live in the eight **scoring records**, at `record+1ECh` and `record+1F8h` | `0091BF6E` `ADD EDI,0x284` next to `0091BF68` `ADD EBX,0x4`, with `EDI` seeded `manager+1FCh` at `0091BE85` |
| `docs/MISSION_EVENTS_UPDATE.md` | the `entityKilled` channel `00986480` has `0077CE60` as its native producer | `00986480`'s only caller is `0077D1A0` `BSP_UnitInstance_DestroyAndBroadcast`, the function that starts one byte after `0077CE60`'s body ends. `0077CE60` produces the `hit` channel only | `ghidra callers 00986480`; `0077CE60`'s body is `0077CE60..0077D19F` |
| `docs/PROJECTILE_HELPERS.md` | the producers of `[shot+45h]` and `[proj+290h]` were not found | `006E2353` and `0070CE6B` | sections 3 above |
| `docs/PROJECTILE_IMPACT.md` | the projectile is `284h` bytes | the base class is; the flak subclass `0070CC30` allocates and zeroes `298h` | `0070CC4E` and `0070CC5C` |

## Open questions

- What inserts into the `entity -> int` map at `manager+1488h` that `0091BE3F` consults. The
  kill writer only reads it, and `0091C560 BSP_MissionScoring_ResetForNewMission` was not
  re-read for this packet.
- Whether `+44h` of an achievement row really is the `Params` column. The offset is settled by
  its consumer, not by `006B9450`, so rule 4 is **not** satisfied for it.
- `0070CC30`'s seventh argument, the float multiplier on `Blast.BlastRange`.
- `007B93E0`'s ABI. The ledger records `void __cdecl(int loadout)`, but `0077D0A1`..`0077D0A3`
  passes the shot in `ECX` and one stack dword. Its body was not read here.
- `00915F20` and `00906180`, the two largest readers of the attribution block.
- Whether a `memcpy` or assignment operator ever clears `[shot+45h]`; only displacement forms
  were scanned.

## Routine coverage

| address | name | coverage | status |
| --- | --- | --- | --- |
| 0077ce60 | `BSP_Unit_RecordDamageAttribution` | complete | analyzed, reconstructed, build-tested |
| 0091bda0 | `BSP_MissionScoring_RecordUnitKill` | complete | analyzed, reconstructed, build-tested |
| 0077cc80 | `BSP_AttributionRef_Assign` | complete | analyzed |
| 006e22d0 | `BSP_ProjectileShotBase_Construct` | partial: the `+45h`, `+98h` and `+B4h`..`+CCh` seeding only; `006E2356..006E2409` unread | analyzed |
| 00779a00 | `BSP_OrdnanceKind_ToCategory` | complete, including both tables | analyzed, reconstructed, build-tested |
| 009ffd20 | `BSP_Unit_LossCountingSlot` | complete | analyzed, reconstructed, build-tested |
| 0090e850 | `STL_KillListMap_Index` | partial: the search and the default-entry prototype; `0090E8D0..0090E8F9` unread | analyzed |
| 0090ede0 | `BSP_MissionScoring_GrantAward` | partial: the ABI, the local apply and the message gate; `0090EEA0..0090F0DF` unread | analyzed |
| 0090c170 | `STL_EntityIntMap_Find` | complete | analyzed |
| 0090ec40 | `BSP_MissionScoring_RecordShooterHit` | partial: the entry conditions and `record+12Ch`; `0090ECCD..0090ED39` unread | analyzed |
| 0070cc30 | `BSP_FlakBulletClass_CreateProjectile` | partial: the allocation size and the `+290h` store | analyzed |
| 00959450 | `BSP_Unit_OnDestroyed` | complete for the gate and the call list | cited, not re-derived |

## Reconstruction

`include/bsp/kill_credit.hpp` and `src/kill_credit.cpp`. The attribution rule, the loss pass,
the per-type tallies, the three award rules and the kill-list key are pure functions over
explicit inputs; `kill_credit_record_unit_kill_0091bda0` is the sequence over
`KillCreditHost`, one virtual per native call site. `ScoringKillAttribution`,
`ScoringKillCredit` and `scoring_relative_party_00803510` are reused from
`include/bsp/scoring_bodies.hpp`, `kFlakOffBurstDistanceBias` from
`include/bsp/projectile_helpers.hpp`. Build-tested on the Win32 MSVC build with warnings as
errors; the existing `reconstructed_math` suite passes. Not fixture-tested against the binary
and not ABI-compatible.
