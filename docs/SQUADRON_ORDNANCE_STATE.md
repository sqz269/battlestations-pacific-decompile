# The squadron's ordnance state, 007EDAD0 (packet `cc9_squadron_ordnance_state`)

Addresses: `007EDAD0`, `007B91C0`, `007B9320`, `007B93E0`, `007B93F0`, `007B9400`, `007B94F0`,
`007B9500`, `006E4060`, `006E4640`, `006D1F40`, `007F16D0`

cc9-lua12, 2026-09-29. The consumer is the ship AI's rank 4, `009FFEB0`'s carrier arm
(docs/SHIP_AI_OPEN_ITEMS.md 32). That arm is bound by a ships worker, not here.

## 1. `007EDAD0 BSP_PlaneSquadron_AmmoType`

`007EDAD0`-`007EDB7C`, `__fastcall(squadron)`, `RET`, read whole from the listing. **coverage:**
complete.

For each of the `+3CCh` planes at `+3D0h`, in order, it calls six predicates on the plane (`ECX`)
with loadout `1`. The first one to answer decides the result:

| order | predicate | kind | result |
| --- | --- | --- | --- |
| 1 | `007B93F0` | `2Bh` MTorpedo | 2 |
| 2 | `007B94F0` | `2Ch` MDepthCharge | 3 |
| 3 | `007B9400` | `33h` MRocket | 4 |
| 4 | `007B9500` | `31h` MParatrooper | 5 |
| 5 | `007B93E0` | `2Fh` MDummyKamikazePlane | 6 |
| 6 | `007B9320` | `2Ah`, and none of `2Ch`, `31h`, `2Bh`, `33h`, `2Dh` | 1 |

No plane answers any: 0. The class names come from docs/ENTITY_CLASS_IDS.md.
- **Correction.** `31h` is `MParatrooper`, not a level bomb. docs/ORDNANCE_KIND_IDENTITY.md's
  table labels `007B9500` "levelbomb", which is the command class `007ED830` uses it for, not the
  ordnance.

`007B91C0` (`RET 8`, `(kind, loadout)`) walks the weapon controller's `+994h` slots at `+974h`.
For each slot it takes `d = slot->vtable[220h](loadout)` and answers true when
`d->vtable[8](kind)`. The slots are the plane's bomb platforms (docs/PLANE_DEVICE_WALK.md).

**What `vtable[220h]` answers:**
- **Single rack, `006E4060`** (`MBombPlatform`, `00CF98C8`): the descriptor at `+314h` of the
  first child at `+48h` (next `+44h`) that answers `IsKindOf(2Ah)`, i.e. the round hanging on the
  rack.
  - Without such a child, and with a non-zero loadout, the fallback needs the owner's
    `vtable[1E4h]` true (`006D1F40`: the byte `[00E17BF2]`) and the gun's descriptor `+3F8h+34h`
    to be a bomb.
  - `[00E17BF2]` is 0 in single player (docs/BOMBER_AFTER_TASK.md, "global_e17bf2 = false is well founded"), so a rack with no round
    attached answers null.
- **Multi rack, `006E4640`** (`00CF9B38`): `006E4060` first, then the first non-null of the forty
  entries at `+514h`, through `+188h` and `00472520`.

So a squadron answers 0 when no member has a round on any rack. That is the "spent bombers" case
the stand-in never produces.

## 2. The binding, behind `kSquadronOrdnanceReaderBound` (committed OFF)

`GameUnitsHost::squadron_ammo_type_007edad0(unit)` takes the squadron unit or a member.
- **Each rack's kind:** the unit row's BSPGun platforms with category BombPlatform, in order.
  `DeviceClass[dev].Bullet[1].Bullet` gives the `Bullets` row, and its `Type` gives the class
  (Bomb `2Ah`, Torpedo `2Bh`, Depthcharge `2Ch`, DummyKamikazePlane `2Fh`, Paratrooper `31h`,
  Rocket `33h`).
- **The ancestry** each descriptor answers: `2Bh`, `2Ch`, `31h` and `33h` also answer `2Ah`; `2Fh`
  also answers `2Dh`.
- **LABELLED substitutions:**
  - A single rack holds a round while its ammo is above 0: `rack_ammo_per_rack`, else `rack_ammo`,
    else the authored Ammo. An unauthored rack always holds one.
  - A multi rack's `+514h` entries are not modelled, so it always answers.
  - Other bullet types (WaterMine, Kamikaze, Bullet) answer no kind.

With the switch OFF the function answers the leader-class stand-in the AI host used:
TorpedoBomber `11h` -> 2, DiveBomber `12h` -> 1, LevelBomber `10h` -> 5, Kamikaze `17h` -> 6,
anything else 0. With it ON, a census logs `squadron ordnance <name>: ammo type a -> b
(stand-in s)` whenever a squadron's answer changes.

`GameUnitsHost::issue_return_to_base_007f16d0(unit, source)` places `returntobase` (`00E08F98`,
flags 1, zero target) on each member plane through `issue_script_command`. That is the path the
Lua binding and the AI selling tick take, and it runs `007F16D0`'s resolution and the land-task
install.

## 3. Predictions for JM05, USN13 and JM08 at 3200/3000, written before any ON run

From this tree's OFF runs (`local\l12_ordoff_<row>.log`), which record no rack drop in any of
them:

| mission | squadron planes | predicted ammo type | stand-in |
| --- | --- | --- | --- |
| JM05 | TBD Devastator (type 112, 11h), 12 planes | 2 | 2 |
| JM05 | SB2C Helldiver (108, 12h), 8 | 1 | 1 |
| JM05 | the fighter squadrons (101, 135, 150) | 0 | 0 |
| USN13 | type 167 (10h, flown on torpedo tasks), 36 | **2** | 5 |
| USN13 | B5N Kate (162, 11h), 24 | 2 | 2 |
| USN13 | A6M (26) | 0 | 0 |
| JM08 | type 174 (16h), 4 | **1** (bombs) | 0 |
| JM08 | fighters (133, 152, 154) | 0 | 0 |

**No squadron runs dry in 3000 frames on any of the three rows.** None drops a round (0 torpedo
releases and 0 dive-bomb releases on JM05 and USN13, and no bomber task row on JM08), so every
answer holds from the first census to the end. The rows where the reader and the stand-in
disagree are USN13's type-167 squadrons and JM08's type-174 squadron. The switch changes no
gameplay by itself: its consumer (`009FFEB0`) is not bound.

## 4. The runs and the verdict (cc9-lua12, 2026-09-29): ON

OFF is this tree's build of `fd28a0931` (`local\l12_ordoff_<row>.log`). ON flips
`kSquadronOrdnanceReaderBound`.

**The first ON run (`fd28a0931`) was a mechanism failure.** Every fighter squadron answered 1: 21
squadrons across JM05, USN13 and JM08, against a stand-in of 0.
- **Cause:** their bomb platforms have no entry in `Equipments[DefaultEquipment]`. F4F (`[101]`),
  F6F (`[26]`) and F2A (`[133]`) author `DefaultEquipment = 0`, so the rack is not loaded. The
  reader had taken a rack without an authored Ammo as loaded.
- **Fix (`0fdb01ab3`):** each rack now carries its default equipment's Ammo, and a rack with no
  entry holds nothing.
- Those first-run logs were overwritten; the table above is from the run's own census lines.

**The corrected run (`0fdb01ab3`, `local\l12_ordon2_<row>.log`):**

| row | `pair_diff` | census |
| --- | --- | --- |
| JM05 | 1, gameplay identical | fighters 0; SB2C and the carriers' dive squadrons 1; TBD squadrons 2. Every answer equals the stand-in |
| USN13 | 1, gameplay identical | nine fighter squadrons 0; fifteen `bruh` squadrons 2, nine of them over a stand-in of 5 (type 167) |
| JM08 | 1, gameplay identical | fighters 0; four H6K Mavis squadrons 2 over a stand-in of 0 |

- **Held:** the fighters, JM05 and USN13's type 167 (2 where the stand-in says 5).
- **Missed: JM08's Mavis.** It answers 2 (torpedo), not the predicted 1: its rack carries a
  torpedo (Mavis rack drops, docs/RELEASE_ISSUE_STAGE.md).
- **Held: nobody runs dry.** No squadron's answer changes after its first census on any row.
- **Verdict: ON.**
  - The mechanism holds on the corrected build.
  - Gameplay is identical because nothing consumes the answer yet.
  - The stand-in is wrong for the type-167 and Mavis squadrons.
  - The ships worker's `009FFEB0` binding is where the answer starts to matter.
