# The torpedo attackrun's squadron probe: 007F0280 in mode 0 (packet cc9_attackrun_squadron_probe)

2026-09-25. Names are hypotheses. This binds the list read in docs/E2_RELEASE_BISECT.md section 4 and
replaces the substitution of docs/PLANNER_HEADING_WRITES.md section 4 (probe product 0). The switch is
committed **OFF**; the pair has not run (the session is disconnected).

## 1. What the image does

- `009D07B0`, on the re-roll arm only (`009D07ED` skips it while `dt < +1Ch`), calls `007F0280` at
  `009D0857`:
  - with `ECX = [approach+0Ch]` (`009D0854`), the plane squadron;
  - with arg0 the unit (`[approach+4h]`, `009D084B`);
  - with extents 80 / 60 / 120 (`009D07F6`, `009D0809`, `009D0826`) and weights 0 / 0 / 0
    (`009D0839`-`009D0845`);
  - with mode 0 (`PUSH 0` at `009D07FF`).
- Mode 0 walks `this+3D0h` for `this+3CCh` entries and skips arg0 (`007F0481`-`007F049A`). No
  `vtable[5Ch]` kind test applies; that is the mode-1 arm at `007F03D8`.
- `+3CCh`/`+3D0h` are the squadron's member count and array. The only stores are `007F2D9D`
  (construct), `007F39ED` (`BSP_Squadron_RemovePlane`, at death) and `007F4B60`/`007F4FCE` (spawn).
- The box is `|p.x| < 80 && |p.y| < 60 && |p.z| < 120` in the unit's frame
  (docs/BOT_PROBE_007F0280.md 0.3). With no member inside it the product is exactly 0.
- `009D085C`-`009D0868` form `-out3.x * out2.y * out2.z`, the dive-bomb product 8 bytes deeper on the
  stack. `009D0877` multiplies by pi/6 (`00CEC730`), and the bias and wrap follow
  (docs/PLANNER_HEADING_WRITES.md 3.1).

## 2. The binding

`kAttackRunSquadronProbeBound` (`src/game_hosts_units.cpp`), gated with `kNearFieldProbeBound`:

- `nf_probe_squadron_007f0280` takes its candidates from
  `plane_squadron_registry().find_by_member_unit(self)->member_units`, the host's `+3D0h`. It skips
  the unit itself and unresolved slots (`kPlaneSquadronNoUnit`). Dead members have already left
  through `007BCAA0 -> 007F3970` (packet cc9_val_squadron_registry).
- The formation-index tie-break, the local frame and the box test are the existing
  `near_field_probe_007f0280`. The reach prefilter is the same host shortcut the mode-1 probe uses.
- The product goes through `near_field_attackrun_sampler_009c42bd` into
  `torpedo_attack_run_heading_009d07b0`, which negates it and scales it by pi/6.
- A new summary line, `summary mission torpedo attackrun squadron probe`, gives units, calls,
  non-zero products and the largest raw offset.

## 3. Predictions, written before the pair

The pair is `kAttackRunSquadronProbeBound` OFF against ON on the landed tree (planner gate and heading
writes ON), E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`. The expectations come from the landed run
`local\PHW1_9000.log`:

- Only the four leaders reach the attackrun: #2.1, #4.1, #6.1 and #8.1, with 275-302 thinks each.
  Their wingmen stay in follow.
- The dive-bomb attackrun's mode-1 probe finds a formation mate in 26-45% of its calls, with products
  up to 0.56.

| row | OFF | ON prediction |
| --- | --- | --- |
| probe units / calls | 0 / 0 | 4 / 200-320 (one call per 0.4 s re-roll over about 27 s of attackrun per leader) |
| non-zero products | 0 | 15-60 % of calls: the leaders whose wingmen sit within 80 m laterally and 120 m fore and aft |
| largest raw offset | 0 | 0.05-0.30 rad (products up to about 0.56, times pi/6), never above 0.524 |
| heading bias per re-roll | 0 | within +-0.3 rad of the bearing, sign from the formation-index tie-break |
| Kate deaths / Val deaths | 16 / 16 | 12-16 / 12-16 |
| hit records | 588 | 450-700 |
| torpedo releases / dive-bomb releases | 4 / 0 | 2-8 / 0 |
| Lexington moved | 5.93 km | 5.5-7.5 km |
| planner yaw base zeroed thinks | 10,071 | 7,000-13,000 |
| mission end | none | none |

The leaders weave, so their aim entry shifts. Every later draw shifts with it, so the kill rows are
judged by band.
