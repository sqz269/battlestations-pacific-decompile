# The pick screen's squadron members (packet `cc9_pick_squadron_members`)

Addresses: 00526E58 (read: 00526A40's squadron arm 00526F2E..00526F43, 00526FB2..00526FD3).

Worker cc9-side-ai, 2026-09-26, from `docs/SIDE_AI_SCHEDULER_HANDOFF.md` section 6. Ghidra was
read only. Names are hypotheses.

## 1. The image's arm, and the census

00526A40 walks game+1974h, then game+19BCh. Each list entry goes through this sequence:
- **00526E3D.** `IsKindOf(1)`. A failing entry is dropped (00526E4E zeroes ESI).
- **00526F33.** `PUSH 18h; CALL [vtable+5Ch]` on the entry. A squadron sets `EDI = entry`, and
  00526E58 reads the first member `[EDI+3D0h]`.
- **Otherwise.** The entry is its own single candidate, with `EDI = 0`.

The candidate tests run on each member in turn (00526E70..00526FAD):
- the grey-arrow test, 008DDF90;
- `!= exclude`;
- `+5Dh` clear;
- `IsKindOf(5)`;
- then grey, or not 1Ah and not 19h.

After each member, 00526FB7 increments EBP and the loop continues while all of these hold:
- `EDI != 0`;
- `EBP < [EDI+3CCh]` (00526FBE);
- `EBP <= 4` (00526FC6);
- the next slot `[ECX]` is non-zero (00526FCF, 00526FD1).

`bsp::unit_pick_00526a40` (`src/hud_warning_screen.cpp`) already matches this shape.

**Census.** `pick list census` lines in `UnitPickBinding::list_size` print when a list's shape
changes. Run `local\psm_census_usn04.log`: USN04 4700/4500, pre-switch build, both RNG and
death-table variables, fit line and module line in this tree.

```
local-player unit lists rebuilt: walk0_ships=18 walk0_rest=18 ships=0 squadrons=0 airfields=0 shipyards=0 land_forts=0 merged=0
local-player unit lists sources: recon_bound=1 triples own=18 enemy=0 unknown=0 (-1 = not published) at mission clock 0.05
pick list census: list=1970 entries=18 kind18=0 kind5=18 squadron_units=0 squadron_members=0 registry_squadrons=1 classes: 7h:9 9h:2 Ah:7
pick list census: list=19b8 entries=0 kind18=0 kind5=0 squadron_units=0 squadron_members=0 registry_squadrons=1 classes:
```

Each line prints once, so neither list changes for the rest of the mission. The image builds
both lists once after the load: 004C3CB0 is latched by game+193Ch
(`docs/RECON_CALL_SITES.md` section 1).

**Answer to the handoff's question.** No entry answers IsKindOf(18h), and no entry is a
squadron at all:
- **The own list** is 18 ships, of classes 7h, 9h and Ah.
- **The one registry squadron** at build time is the authored `movieval` group, a Japanese Val
  `PlaneSquadronGen`. It is enemy and undetected at clock 0.05, so the enemy triple is empty
  and it enters neither list. The allied Kingfisher row was rejected at generation.
- **Carrier squadrons** are launched from air ops later (the first at log line 7801) and never
  enter a list that is not rebuilt.

**A second difference, for the case where a squadron does reach a list.** The host's recon
group record for a squadron is `plane_squadron_registry()`'s `squadron_unit`, which is the fused
wing-0 leader. The leader answers its plane chain. The `ai diag close` lines show
`squadron_18h=0` for the leader. The native group record is the 414h container, class 18h
(`include/bsp/plane_squadron.hpp`, +C4h). So without a substitution the pick would treat such an
entry as one plane and never reach the member arm.

## 2. The binding (`kPickSquadronMembersBound`, `src/game_hosts_hud.cpp`, committed OFF)

- **00526F33.** `UnitPickBinding::is_kind_of(entry, 18h)` is true when the entry is a registry
  `squadron_unit`. **Substitution:** the fused leader stands for the container. It only changes
  18h answers, and 00526F33 is the only 18h test on the pick path. `firing_unit_004b4b00` tests
  kind 5 first, so a fused leader still answers itself. The new row is
  `UnitPickScreen::squadron_entry`.
- **+3CCh.** `member_count_3cc` returns the record's `live_count()`.
- **+3D0h[k].** `member_3d0` returns the k-th member whose unit exists, as index + 1. The image's
  array holds only planes that exist. The host keeps a no-unit slot for a wing that never became
  a unit, so the k-th live member is the image's slot k. The rows are
  `UnitPickScreen::squadron_members` (done) and `squadron_member_empty`.
- **OFF** keeps the old record and 0.

## 3. Predictions (written before the pairs)

Same tree, `local\bin\psm_off` against `local\bin\psm_on`, `BSP_GUNNERY_RNG_STREAMS=1`,
`BSP_DEATH_TABLE=1`, USN04 4700/4500 and E2 = USN04 9200/9000. USN02 builds no squadrons.

- **The census lines** are identical ON and OFF: the list shapes do not depend on the switch.
- **The squadron rows.** `UnitPickScreen::squadron_entry` and `squadron_members` show 0 calls ON,
  and the OFF record is absent too. No list entry is a squadron unit, so the arm is never
  reached.
- **Pick candidates per squadron entry: 0 added.** The rows are unchanged ON against OFF:
  - `grey_arrow_set`: USN04 18 per update, 164,880 in `bs_on_usn04.log`;
  - `gui_extent`;
  - `owner_140`;
  - `ray_pick_*`.
- **Gameplay.** The whole native table, every per-entity row, the gunnery damage lines and the
  death table are identical, with a zero clock offset. No resolved pick can change, so nothing
  reaches the weapon-group fire path.
- **Ignored counters.** `ship avoidance search refills` and `pretranslate` are ignored.

## 4. The pairs and the verdict

One tree, 54013982d, with `local\bin\psm_off` against `local\bin\psm_on`. The two builds differ
only by the switch. Both variables were set. All four logs show the 1600x900 fit line, the
module directory in this tree and the final COM release.

| row | USN04 OFF | USN04 ON | E2 OFF | E2 ON |
| --- | ---: | ---: | ---: | ---: |
| native table rows | 1,524 | 1,524, all equal | 1,526 | 1,526, all equal |
| `UnitPickScreen::squadron_entry` / `squadron_members` | absent | absent | absent | absent |
| `UnitPickScreen::grey_arrow_set` | 164,880 | 164,880 | 326,880 | 326,880 |
| `UnitPickScreen::gui_extent` | 155,669 | 155,669 | 308,669 | 308,669 |
| deaths / hit records | 41 / 727 | 41 / 727 | 51 / 836 | 51 / 836 |
| death rows | 41 | 41, equal | 51 | 51, equal |
| pick list census lines | 2 | 2, equal | 2 | 2, equal |

**Every prediction held. Pick candidates added per squadron entry: 0.** No list entry is a
squadron, so the arm is not reached.

A whole-log diff with heap addresses masked leaves only these lines:
- the render thread id;
- the front-end press-start text's blink alpha in E2 (0.83 against 0.87), before the mission
  starts;
- the ignored `ship avoidance search` refills counter.

**Verdict: ON.** The arm now answers as the image does whenever a squadron group record reaches
a pick list. It is unexercised in these two missions. The leader-as-container substitution is
labelled in the code and in section 2.

**Next steps, not taken here:**
- A mission with an own squadron at load time, or a detected enemy one at clock 0.05, would
  exercise the arm. An authored allied `PlaneSquadronGen` that is generated would do.
- USN04's allied Kingfisher row is rejected at generation (`scene class PlaneSquadronGen seen=2
  generated=1 rejected=1`). Why is a separate question.
