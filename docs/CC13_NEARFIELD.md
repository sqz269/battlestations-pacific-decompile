# `007F0280`'s candidate list against the host's walk (packet `cc13-nearfield`)

Worker cc13-reltimer, branch `agent/cc13-nearfield`, 2026-10-19. This is a read-only analysis of
STUB census row 16 (`docs/CC13_STUB_CENSUS.md`). `007F0280` lies in cc9's struck range
`007EE000-007F2FFF`, so nothing there was built, recorded or leased. The bytes come from the export
of `/battlestationspacific.exe`. The probe body and its accumulator are already documented in
`docs/BOT_PROBE_007F0280.md` and `docs/NEAR_FIELD_PROBE.md` section 1.

**Verdict.** There is no live mismatch.
- **Order does not matter.** The accumulator keeps per-axis minima, so the result is independent of
  candidate order.
- **Membership matches.** The host's membership equals the image's except for the image list's 3 s
  refresh lag. That lag can change an answer only when a candidate closes from beyond 1080 m to inside
  the box (at most 140 m) within 3 s. For a candidate already in the world, that needs a closing
  speed above about 313 m/s, which no plane class in the census missions reaches in level flight.
  Closing speed is not the only route, though. An entity that enters the world list, or becomes
  active, within 1080 m after the last refresh is missed by the image for up to 3 s at any speed,
  and the reverse holds for one that lands or is hangared (section 4, deck traffic row). That case
  reaches only deck traffic at the land/line probe `009B0632`.
- **The label stays.** The census label "substitution" remains accurate; the row becomes
  `substitution, no live effect`.
- **Speed bound.** No plane class in the seven probe missions closes faster than about 200 m/s
  head-on.

## 1. How the image builds the candidate list

The list is one heap vector, created by `operator new` at `007F0385`, grown to `2n+2` at
`007F03EE`/`007F04B2`, and freed at `007F0B00`. It is filled in one of two ways, chosen by the byte
mode (arg5, `007F038D CMP [ESP+138h],0`):

- **Mode != 0, with `[arg0+C50h]` set** (`007F039F`-`007F03DC`): the list walks the neighbour
  object's `+30h` vector (`begin` `+30h`, `count` `+34h`) in vector order.
  - Null entries are skipped (`007F03C9`). These are the observer slots of destroyed entities.
  - It keeps an element only if `vtable[5Ch](0Fh)` answers true (`007F03D8`), so only aircraft pass.
  - There is no distance, party or death test here.
- **Mode == 0, or mode != 0 with a null `+C50h`** (`007F0481`-`007F049C`): the list walks
  `ECX+3D0h` for `ECX+3CCh` entries, skipping the element equal to arg0 (`007F049A`).
  - At every one of the eighteen call sites `ECX` = `[approach+0Ch]`, the squadron, except
    `009B4DBF MOV ECX,EAX`. For example, `009C42B5`, `009D0854` and `009D1A90` load
    `ECX,[EAX+0Ch]` / `[EDI+0Ch]` just before the call.
  - There is no kind test in this branch.

The `+30h` vector is built by `007E11D0` BSP_PlaneNeighbours_Refresh (`docs/PLANE_GUNFIRE.md`
section 3):
- It walks the world list `[[00E188A8]+19CCh]+58h`.
- It takes entities other than the owner that answer `vtable[5Ch](6)` or `(0Fh)`, are not already
  listed, and lie within `R = (3.0 + 3.0) * 180.0` = 1080 m.
- Members at or beyond R are dropped at each refresh.
- The update `007E2010` runs it every `+88h` = 3.0 s. Its clock starts at `U(0, 3) + 3 >= 3`, so the
  first refresh comes on the first update.
- The refresh phase is per plane and random. The constructor `007E1E20` seeds the clock `+7Ch`
  with `U(0, 3)` from `00BD2F10` at `007E1F94`. `007E2010` counts the clock up and subtracts 3 on
  each refresh (`007E2058`-`007E2061`), so each later refresh lands `3 - U` s after a whole
  3 s step.

The object at `+C50h` is built by `007D61C0`-`007D621F` when `[game+1FE4h]` is 0 or 1 and
`unit+54h` differs from the scene's `DummyAIEnabled`. That key is 3 when absent, and no
single-player scene authors it (`docs/TERRAIN_ARM_US_FIGHTERS.md`, `docs/CC13_VERIFY11.md` S16). In
single player every plane therefore has the object, and mode 1 always takes the `+30h` walk.

## 2. The host's walks

Both walks are in `src/game_hosts_units.cpp`.

- **`nf_probe_007f0280` (mode 1).** It loops over every slot in slot order. It skips self, any slot
  failing `df_slot_live`, and any slot that is not `IsKindOf(0Fh)`. It then applies a sphere
  prefilter of radius `e0 + e1 + e2`.
  - `df_slot_live` requires `+5Ch` set and `+5Dh`, `+5Eh` and `+60h` clear.
  - The sphere encloses the box, whose half-diagonal is `sqrt(e0^2 + e1^2 + e2^2) <= e0 + e1 + e2`,
    so the prefilter removes nothing the box would keep.
- **`nf_probe_squadron_007f0280` (mode 0).** It walks the squadron registry's `member_units`,
  skipping `kPlaneSquadronNoUnit` and self, and applies the same prefilter.

## 3. Does order matter? No

The accumulator (`near_field_probe_007f0280`, `src/near_field_probe.cpp`, from `007F06AF`-
`007F0AF6`) handles each candidate inside the box the same way:
- It sets `hit` (`007F06C0`).
- It may replace each component by the tie-break value `+-f`. That value depends only on that
  candidate and on self's `+9D0h`.
- It folds the result into `pos_min[i]` / `neg_min[i]`, kept as running minima (`!(p > min)`
  stores).

A minimum is commutative. On a tie the stored value is equal whichever candidate came first. Nothing
else in the output depends on which candidate set a minimum. So the vector order does not matter:
the image's append order in the `+30h` vector against the host's slot order, or the `+3D0h` seat
order against the registry's `member_units` order. The only order-sensitive case would be a NaN
component, which `!(p > min)` stores; no NaN position reaches the probe in the host.

## 4. Membership

| difference | mode | effect |
|---|---|---|
| The image's `+30h` list is a snapshot up to 3 s old; the host reads live positions. | 1 | A candidate missing from the snapshot was beyond 1080 m at the last refresh. To be inside the box now (|x| < e0 <= 100, |y| < e1 <= 70, |z| < e2 <= 140, `docs/BOT_PROBE_007F0280.md` section 4), it must have closed at least 940 m in at most 3 s, about 313 m/s. The plane classes in the seven probe missions were read from their `unit hull input ... type_id=` lines and checked against `vehicleclasses.lua` with `local/nf_speeds.py`. Their fastest `MaxSpd` is 100.0 (USNOS: F2G Super Corsair, AD-2 Skyraider); the rest are at most 83.3. A level head-on pass therefore closes at most about 200 m/s. Only the Funryu (343.1), Ohka (194.4) and the jets (Kikka and P-80 at 125.0, plus an F-86 platform row at 307.2 in this modded installation) could exceed 313 m/s, and none appears in those missions. A steep dive is the one case without a bound (labelled). |
| Dead or destroyed entities. | 1 | They match. The image nulls a destroyed entity's observer slot (skipped at `007F03C9`). The host drops `+60h` / `+5Dh` slots: the death path sets both together (`0092639B`, `00926CD5`; `game_hosts_units.cpp`, `kPlaneDeathFlagsBound` and `kStockReturnDestroyBytesBound`). A wreck that is still active stays a candidate in both. |
| Kind. | 1 | Both keep `IsKindOf(0Fh)`; the list's kind-6 entries are dropped at `007F03D8`. |
| Self. | 1 and 0 | The image list excludes the owner (`007E11D0` "other than the owner"; `007F049A` in mode 0). The host skips `process_index`. |
| Party. | 1 | Neither filters by party. |
| Inactive (`+5Ch` clear) entities, such as hangared planes. | 1 | The host excludes them. Whether the image's world list `+58h` holds an inactive entity is not read, so this is an uncertainty. Hangared planes are not flying near a probing plane, except perhaps during the land/line probe at `009B0632` near a deck. |
| Deck traffic: entities that appear or disappear between refreshes. | 1 | Some entities enter the world list, or become active, within 1080 m after the last refresh: a carrier launch, a GenerateObject or squadron placement, or a plane leaving the hangar. The image's snapshot misses these for up to 3 s at any speed (the refresh phase is random per plane, section 1), while the host includes them as soon as `+5Ch` is set. The reverse holds for a plane that lands or is hangared: the image's snapshot keeps it until the next refresh, unless the observer slot is nulled, while the host drops it when `+5Ch` clears. The reach is deck traffic at the land/line probe `009B0632`. It is not counted: the host logs no per-call candidate list, and in cc13-decklaunch's USNOS census log (`p23h_off_os.log`) the only land/line entries are two Enterprise_sqn02 wingmen at 1498.89 s and 1499.19 s (`009B02E0`, mode 2). That log has no line that ties a probe answer to a launch or a landing at the deck. |
| A dead member's seat. | 0 | Under `kSquadronRemovesDeadBound`, the host clears a dead leader's seat to `kPlaneSquadronNoUnit` before `remove_member_unit_007f3970` erases it. In that window the host skips the member, while the image still lists the pointer until `007F3970` compacts `+3D0h`. The window is inside one death step, so its reach is that step's probes only. |

## 5. Reach in the logs

The census reach is 7 missions and 148,665 calls. The missions are the ones with `near field ... calls=` lines: USN04, USN21, USN01, BSM09, BSM01, IJN08 and USNOS. The host prints `near field <plane> calls= hits=`
per plane. In the census USN04 and BSM09 logs, the D3A formations hit on every call. For example,
`near field D3A Val 1 calls=131 hits=131` in BSM09 and `near field D3A Val #1.1 calls=95 hits=95` in
USN04: the hits are the plane's own wingmen, holding formation inside the box. Those wingmen are
within 1080 m at every refresh, so the image's snapshot holds them too, and the answer is identical
for both lists. Nothing in the logs records closing speeds, so the staleness case is bounded by the
class speeds above, not counted.

No adoption note is needed. If a mission with Ohka, Funryu, or diving formations crossing head-on
is ever added to the WINS rows, rerun this check against it.

## 6. Uncertainties

- Inactive entities in the world list `+58h` (section 4) are not read.
- `MaxSpd` is taken as m/s (the plane spawns log 69.44 for 250 km/h). A dive can exceed `MaxSpd`.
- The NaN case of section 3 is argued from the host's inputs, not proved for the image.
