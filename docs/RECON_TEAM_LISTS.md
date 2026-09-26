# The recon slots' published triples (packet `cc9_recon_team_lists`)

`docs/HUD_CAMERA_TEAM_LISTS.md` section 2 found that `004C3CB0` walks the player's recon triples:
own (`+DD8h`, head `+DDCh`), enemy (`+DE4h`, head `+DE8h`) and unknown (`+DFCh`, head
`+E00h`). The host had no recon slot object. Its gunnery contact sweep, the
`008053C0(unit+54h)+0DE8h` read recorded as `Gunnery::recon_slot_contacts_008053c0`, 28 K calls
in E2, was a stand-in: every other-side unit in unit order whose level was above `none`. This
packet builds the five triples the rebuild `008073C0` leaves, from the recon pass the gunnery
host already runs. It feeds them to the sweep under `kReconTeamListsBound` and exposes them to
the HUD through one entry.

## 1. What the triples are (from docs/RECON_SLOT_LISTS.md, not re-read)

For the slot of party `S`, after each rebuild (every 3.0 s, `008079B0`):

| step | rule |
| --- | --- |
| scan `0080749C`..`00807527` | the 22 class ids of `00806480` (`kReconScannedClassIds`), each through the world registry's per-class list, behind the four gate bytes and `IsKindOf(2)` |
| relation `008065FF` | `recon_relation_for_008065ff(S, party)`: own, enemy or neutral (slot 2 sees no enemy) |
| triple 0 `00807529` | every own bucket, in class-id order, before grouping |
| triple 1 `00807615`, drain `00807634` | every enemy bucket; keeps level 2, copies level 1 to triple 3, drops level 0 |
| triple 2 `008076F9`, drain `008077B4` | the same for neutral |
| triple 3 | the enemy blips, then the neutral blips |
| triple 4 `00807933`..`00807966` | copies of triples 0, 1, 2 and 3, unpublished |

Inside a bucket the records follow the scan order, because `008065B0` appends new records and
splices carried ones to the tail. The buckets are walked from 0 to `60h`.

## 2. The host binding

- `publish_recon_triples_008073c0` (`src/game_hosts_gunnery.cpp`) runs after every sensor
  pass. It builds each observed side's five lists as unit indices: world-list order inside
  ascending class ids, the gate through the pass host's `unit_present`, `IsKindOf(2)`, the
  relation rule and the published level.
- **The HUD entry:** `GameGunneryHost::recon_triple_units(int side, int triple,
  std::vector<std::size_t>& out)`, with triple 0 own, 1 enemy, 2 neutral, 3 unknown and 4
  union. It returns false before the first rebuild. It is built on both sides of the switch.
  The HUD worker binds `004C3CB0`'s two call sites to it; no HUD file is touched here.
- **`kReconTeamListsBound`:** the sweep's contact read is the side's enemy triple itself. It
  keeps the stand-in's dead and kind checks and counters, and the `008053C0` row is now
  `done`. Blips (level 1) leave the sweep, and the order becomes class buckets, not unit index.

Substitution: the squadron and convoy aggregates (classes `18h` and `1Ah`, built by the
grouping passes `00805490`/`00805680`) are not built, so a group record never appears.

The summary line `recon triples` gives each list's mean size over the builds, and the final
lists per side follow it.

## 3. Predictions, written before the pair

USN04 4700/4500, `BSP_GUNNERY_RNG_STREAMS=1` and `BSP_DEATH_TABLE=1`, OFF
`local\rtO` against ON `local\rtT`, one tree, only the switch differs. The HUD is unchanged on
both sides, because nothing calls the entry yet, so there are no minimap or pick rows to move.

- **Triples.** The `recon triples` line is identical on both sides: the lists do not depend on
  the switch.
- **Contacts.** `gunnery contacts admit_ship` plus `admit_plane` falls by the blip share, 20 to
  40 percent. In the earlier E2 census, 461 of 1467 detected (side, target) pairs on side 0 were
  blips. `considered` falls further, because the enemy triple no longer lists the undetected
  units the stand-in walked and rejected.
- **Guns.** Blips are mostly beyond gun range, so shots and hits stay within 10 percent, and
  deaths within 3. The first shot is at the same time or later.
- **Order.** The bucket order changes only the tie order between equally scored candidates.

## 4. The pair, and the decision

`local/rtO_usn04.log` against `local/rtT_usn04.log`. Both show `window resolution override fit:
2560x1440 -> 1600x900` and exit 0.

| row | OFF | ON |
| --- | --- | --- |
| recon triples, mean per build (75 builds) | own 55.7, enemy 28.3, neutral 0, unknown 0.3, union 84.2 | identical |
| final lists | side 0: own 30, enemy 6; side 1: own 19, enemy 11 | identical |
| contacts considered | 1,097,368 | 256,883 |
| contacts admitted, ship / plane | 90,252 / 171,430 | 90,252 / 164,896 |
| candidates rejected | 66,881 | 63,875 |
| shots / hits / deaths | 3772 / 469 / 32 | identical |
| gun rows that differ | - | 0 of 726 |

Verdict per prediction:
- **Triples identical on both sides: held.**
- **Admitted contacts fall 20 to 40 percent: failed; they fell 2.5 percent** (plane admits 3.8
  percent, ship admits unchanged). USN04's sensor pass reports 19 blips against 2121
  identified pairs, far below the E2 share the prediction was drawn from. `considered` fell 77
  percent, because the sweep no longer walks and rejects the own side and the undetected units.
- **Shots, hits and deaths: held**, identical to the digit, as is every gun row.
- **Order:** no candidate choice moved.

**Decision: ON.** The sweep reads the image's list, and the `008053C0` row is now `done`. The HUD
entry `GameGunneryHost::recon_triple_units` is in place for the HUD worker's `004C3CB0` call
sites. The E2 pair the brief allows "if the pick changes any order" is not needed: the pick is
the HUD's, still unbound, and USN04 moved no order.
## 5. The squadron and convoy aggregates (packet `cc9_recon_aggregates`)

Addresses: 00805490 00805680 008073C0 00807581 00807877 00807921 008069A0 00806A60 00805430
008040E0 008042B0

### What the grouping does (from the listing and pseudocode of 00805490)

`__thiscall void(List* members, List* groups)`, `RET 8`, body `00805490..00805674`. For every
record of `members`:

* **The skip.** A record with level `+0Ch < 1` is skipped (`008054D7`), and so is one whose unit
  has no squadron (`unit+9D4h == 0`).
* **The group record.** The squadron's record is:
  * found in last pass's carry-over list `slot+F34h` (`D[18h]`), taken out of it with
    `008040E0`, its level reset to 0 and its member list at `+10h` cleared with `008042B0`;
  * or found in `groups`;
  * or created as a `1Ch` record: vtable `00D08E78`, `+4h` the squadron, `+8h` its
    `+1E4h->vtable[1]()`, level 0. It is appended to `groups`, and the pair is registered with
    `00694A60`.
* **The fold.** The member's record is appended to the group's `+10h` list. The group's level
  becomes the maximum of its members' (`00805631..0080564A`). The member record stays where it
  was.

`00805680` (`00805680..00805864`) is the same pass for land vehicles: class `19h` members, back
pointer `unit+738h`, carry-over `slot+F4Ch`, group class `1Ah`.

### Where it runs in 008073C0 (docs/RECON_SLOT_LISTS.md step list)

* **Enemy (step 8, `00807581..00807610`).** Seven `00805490` calls fold classes `10h`, `13h`,
  `12h`, `11h`, `16h`, `15h`, `17h` into `B[18h]`. `008069A0(B[18h])` writes each squadron's level
  to its own detection record. `00805680(B[19h], B[1Ah])` and `00806A60(B[1Ah])` do the same
  for convoys. Then triple 1 walks every bucket in class order, so the groups sit at bucket
  `18h` / `1Ah`, and they drain like any record: level 2 stays, level 1 goes to `unknown`.
* **Neutral (step 10).** The same, into triple 2.
* **Own (step 11, `00807877..0080791C`).** The grouping runs after triple 0's copy, and triple 0
  then takes `A[18h]` and `A[1Ah]` at its end.
* **Retire (step 12, `00807921..0080792E`).** `00805430(18h)` and `00805430(1Ah)` retire
  whatever was not folded this pass. A group record never outlives its members' detection.

### The host binding

`publish_recon_triples_008073c0` builds the three relation arrays as `{unit, level}` records,
runs the fold on enemy, neutral and then own, and assembles the triples in the order above. The
squadron is `plane_squadron_registry().find_by_member_unit(u)->squadron_unit`.

Labelled substitutions:
* This host has no land-vehicle convoy membership, so `00805680` folds nothing. The summary
  counts the class-19h records such a pass would read.
* `008069A0` / `00806A60` (the group's own detection record) are records.
* A carried group record and a new one are rebuilt identically each pass. That is the same
  list, because `00805430` retires the unfolded ones.

The summary line `recon aggregates` counts the group records, members, own groups, identified
and unknown enemy groups, and how many group units the units host calls ship-base or plane-base.

**What the HUD entry now returns** (`GameGunneryHost::recon_triple_units`, the input of
`004C3CB0`'s two call sites, owned by cc9-hud):
* own (0) ends with the side's squadron entities;
* enemy (1) carries identified enemy squadrons at their class-18h place;
* unknown (3) carries blip squadrons.

Those are squadron units, not planes, so the HUD's per-entry handling of a class-18h unit
decides how they draw.

### Predictions (written before the runs)

Same-tree pairs, `BSP_GUNNERY_RNG_STREAMS=1 BSP_DEATH_TABLE=1`, switch OFF against ON.

1. **USN04 4700/4500 (squadrons).**
   * `recon triples` mean own rises by the number of own squadrons with a member present (3
     to 8).
   * Mean enemy rises by the identified IJN squadrons (1 to 6), and mean unknown by 0 to 1.
   * Neutral stays 0.
2. **USN02 9200/9000 (convoys).** The run creates no squadron (`squadrons_created=0`) and has no
   convoy membership, so every line is identical, including the recon triples.
3. **The gunnery sweep.**
   * Squadron units are neither ship-base nor plane-base in the units host, so the sweep
     rejects them by kind. `contact_reject_kind` rises, and every gunnery row (shots, hits,
     damage, deaths, per-gun rows, death table) stays identical on both missions.
   * If `kind_plane` or `kind_ship` in the new summary line is above 0, the sweep admits them.
     That would be a finding, and the rows it moves will be listed.
4. **The native table:** no host method row changes except the three new rows this packet
   adds (`00805490` done, `00805680` and `008069A0` records).

### Results

Pending.
