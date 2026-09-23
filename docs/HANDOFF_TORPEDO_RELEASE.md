# Handoff: packet `cc8_torpedo_release` (the torpedo aspect release gate)

Worker `cc8-torpedo-release`, branch `agent/cc8-torpedo-release`, worktree
`J:\PROG\battlestations-pacific-decompile-cc8-torpedo-release`, from main `b0b3696a1`.
Read `docs/TORPEDO_RELEASE_GATE.md` first; this file is only what a successor needs that the
document does not already say.

## State at handoff

**The finding is complete and measured. The only thing outstanding is one confirmation pair.**

1. **Item 1 is done and the mechanism is proved.** `approach+84h` gates the *release*, not just the
   aim-solution byte (`docs/TORPEDO_RELEASE_GATE.md` 1.1). It is seeded verbatim from the robots row
   `+18h`, this installation authors `0.7` on `SPNormal`, and it is now bound.
2. **The packet's real finding was not the constant.** The gate's aspect input was stubbed to zero,
   so `009D1FED` returned its `y1` on every tick whatever the aspect. Binding `0.7` on the blind gate
   moved every release from a flat 450 m to a flat 315 m - agreeing with the blind-gate model to
   0.4 m, which is what proves the diagnosis (section 2.6). The aspect is now bound from the ordered
   target (2.7) and USN01 confirms the clamp works (2.7.1).
3. **Item 2 produced no code, by design.** The image's step-6 exemption is Kamikaze-gated and cannot
   fire for a torpedo; the host already matches it. The observation for the formation owner is in
   section 3.3: the two aircraft are 4-8 m apart and torpedo each other.

## The one thing left (FIRST ITEM)

**The gate is landed and measured. This is a faithfulness refinement, not a fix.**

`target_is_kind_vtable5c` is committed as `aim_target() != nullptr`, labelled a substitution in the
source. It can be bound exactly, because `src/game_hosts_units.cpp:2180` records that
`bsp::unit_is_kind_of` is the same `0074E400` model slot `5Ch` uses:

    bool target_is_kind_vtable5c(int query) override {
        const GameUnitSlot* const t = aim_target();
        return t != nullptr && bsp::unit_is_kind_of(t->class_id, query);
    }

Kind 6 is the ship base (`kKindKamikazeTargetShip`, `kKillCreditKindShipBase`,
`kUnitGunneryKindShipBase`). For a ship target the two forms are identical by construction and every
ordered target in both missions is a ship, so the numbers should not move - but that is why it needs
a confirming pair rather than an assertion. Make the edit, then:

    ./scripts/build.ps1
    ./tools/run_game.ps1 -Log local\rel_usn01_aspect2.log -WaitSeconds 2400 -- --frames 3200 \
        --press-start-frame 30 --menu-select USN01 --mission-frames 3000 --mission-frame-seconds 0.05
    ./tools/run_game.ps1 -Log local\rel_usn04_aspect2.log -WaitSeconds 2400 -- --frames 5000 \
        --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05

and confirm both match `rel_usn01_aspect.log` and `rel_usn04_aspect.log` **to the digit**. If they do,
swap the committed predicate for the bound one and say in the commit that the numbers are unchanged.
If anything moves, the "identical by construction" argument is wrong and that is itself the finding -
report it rather than adjusting anything.

**The landing condition has already been met** (integrator's, 14:16): one binary sent the two
missions in opposite directions. USN01's four clamped rounds bit-identical with Mav2 -7.8 m, and
USN04 down to 303-317 m with the `|cos| = 0.355` control back at 435.1 m and its closest approach
back at 167.9 m. Sections 2.7.1 and 2.7.3.

## Logs, all in `local/`, all finished with `native renderer final COM release`

| log | binary | what it is |
| --- | --- | --- |
| `rel_usn01_before.log` | 13:40:13 | before, census column only, `aspect_scale_84 = 1.0` |
| `rel_usn04_before.log` | 13:40:13 | before |
| `rel_usn01_after.log` | 13:56:22 | `0.7` on the BLIND gate - the falsifier, keep it |
| `rel_usn04_after.log` | 13:56:22 | same |
| `rel_usn01_aspect.log` | 14:06 | aspect bound, unbound kind predicate - USN01 passes |
| `rel_usn04_aspect.log` | 14:06 | aspect bound, unbound kind predicate - **read this first** |

## Traps this packet actually hit, so you do not

* **The census's `abs_cos_aspect` is NOT the gate's input.** It is the gunnery host's own computation
  from real unit headings, sampled at the drop. The gate runs every tick on `f2c`
  (`src/torpedo_aim_tick.cpp:122`). Assuming they were the same cost me a wrong prediction.
* **`clamped_interpolate_00419010` clamps its OUTPUT** to `[min(y0,y1), max(y0,y1)]`, not its input
  `x` to `[x0, x1]` (`src/unit_rudder.cpp:27`). The effect below the knee is the same, but do not
  reason about it as an input clamp.
* **`00CE4D70` is a DOUBLE** (`FADD double ptr`). Read as a float it is `0.0`.
* **`00F8A30C` is a pointer global**, not the robots table base (`009F9D18` loads through it), so the
  rows cannot be read out of the PE there.
* **Grep the docs before the listing.** `bsp.py docs-for 0084C203` named
  `docs/TORPEDO_AFTER_THE_DROP.md` 15.3, which already had the step-6 answer, in my first query on
  that address. I read the listing anyway and had to retract a claim to the integrator.

## Still open, and explicitly NOT done here

* `unit_is_kind_vtable5c` (the own-unit probe at `009D175F`/`009D176E`, kinds `10h`/`16h`) is still
  `return false`, so the 0.9 tighten at `009D1750`-`009D178A` never fires. It could be bound the same
  way, from `s_.class_id`. Left alone deliberately: it is a second behavioural change and would have
  muddied the gate measurement. **This is the obvious next packet.**
* The `+40 m`-ish residual on misses is still unexplained, and
  `docs/TORPEDO_AIM_LEAD.md` section 6's model does not survive the corrected `R` unchanged - with
  the measured 435 m and the same `s = 30.87 m/s` its `t_cpa` comes out 9.84 s against 7.40 s
  recorded. Probably the unmodelled air fall between release and water entry. Recorded, not fitted.
* The formation co-location of section 3.3 belongs to whoever owns formation, not here.

## Housekeeping

* Lease `cc8_torpedo_release`: 10 addresses, files `docs/TORPEDO_RELEASE_GATE.md` and
  `docs/TORPEDO_AIM_LEAD.md`. Release with
  `python tools/bsp.py lease release --packet cc8_torpedo_release` when done.
* Three ledger fragments added (`009d1fed`, `009d049d`, `0084c203`); shards
  `config/reconstruction/009d0000.jsonl` and `00840000.jsonl` are staged with the code.
* Shared files were edited by hunk under the integrator's arbitration of 2026-09-19:
  `src/game_hosts_units.cpp` (torpedo task hunk) and `src/game_hosts_gunnery.cpp` (torpedo drop
  census). Every commit message must say so.
* `python tools/const_width_sweep.py --all --load-sites` reports `A-WRONG 0`. The two new constants
  are authored Lua values with no image address, so they are correctly absent from its table.

## Correction from docs/TORPEDO_KIND_PROBE.md (2026-09-22)

Packets `cc9_torpedo_kind` and `cc9_torpedo_kind_land`. **Both kind probes of `009D15F0` are now
bound exactly**, so the "one thing left" and the first "still open" item above are done.

- The target probe (`009D1714`, kind 6) is `bsp::unit_is_kind_of(t->class_id, query)`. Both
  missions reproduced their controls to the digit, as this handoff predicted.
- The own-unit probe (`009D175F`/`009D176E` and `009D1E08`/`009D1E17`, kinds 10h and 16h) is
  `bsp::unit_is_kind_of(s_.class_id, query)`. It fires only for USN01's five H6K Mavis
  (LargeReconPlane). It scales `F14` and `F0C` by exactly 0.9 and moves their release ranges
  433-438 m to 460-486 m. USN04's Kates are TorpedoBomber and USN04 is unchanged.
- **The `:2180` reference above is stale.** The file has moved, and the note that
  `bsp::unit_is_kind_of` is the slot-5Ch model now sits in the torpedo aim binding of
  `src/game_hosts_units.cpp`. `bsp::unit_is_kind_of` models all 88 compiled slot-5Ch bodies, and
  `0074E400` is the plane's body among them, not the routine every class reaches.

The measured values are in `docs/TORPEDO_KIND_PROBE.md` sections 6 and 8.
