# Handoff: the plane follow regimes (packet `cc8_follow_regimes`)

Worker `cc8-follow-regimes`, 2026-09-19, branch `agent/cc8-follow-regimes`, tip `6fe098150`.
Third on `009BFEE0`, after `cc8-follow-law` and `cc8-follow-steer`. Read
`docs/PLANE_FOLLOW_LAW.md` §5.10-§5.12 and its §7 block map first; they are the real brief.
This file is only what a cold session needs to resume.

## State

Read, bound and committed this turn:

* **§5.10 the reference direction** — the leader's heading LAGGED by `rate * T`, with `T` a time
  ramped 0.5 s at 100 m to 4.0 s at 500 m from `Pilot/Follow/LeaderHeadingSpdTime|Dist/1,2`.
  This closes §5.6.2's "still open" on what `A` and `V` mean: `V` is the CROSS-TRACK offset from
  the station about that lagged track and `A` is the heading error against it.
* **§5.11 a RETRACTION of §5.6.2's conclusion** — `BL` is rewritten nine times, not five; the
  quadrant classifier's value never reaches the dispatch. The five values that do are
  1 / 2 / 4 / 10 / 12, and the abeam side is the saved sign of `V`, which makes that branch a
  **break-away**, not a rejoin.
* **§5.12 the lead-pursuit regime**, end to end, and `009C10F7`/`009C11D5`/`009C1222` are three
  **stages of one point**, not three regimes.
* The pure functions: `plane_follow_geometry_009bfee0` in `src/plane_follow_law.cpp`.
* **The host switch**: `kPlaneFormationPlacementEnabled` is now `false` and
  `kPlaneFollowLawEnabled` `true` — a pair, exactly one true, so a before/after is one binary.

## Start here, in this order

1. `python tools/bsp.py brief`; claim the lease (`cc8_follow_regimes` addresses, see below).
2. Do **not** re-derive the frame walk; `tools/callee_effects_009bfee0.json` and its ready-made
   argv are committed. The listings and `local/cfg.py` (a compact CFG prober: `edges` and `back`,
   which prints the transitive predecessor cone with each terminator — that is how the regime
   guards were found) are in this worktree's ignored `local/`.
3. `local/cmp.ps1 -Log <log>` prints the run metrics whose column meanings come from the
   printing code at `src/game_hosts_units.cpp:2217-2229`.

## The open questions, in dependency order

1. **`009BEE30`'s HOLD arm**, `009BEE56`-`009BF9E5`, ~700 instructions, still untouched. This is
   the biggest remaining hole and it is what a member in good position is commanded. Its call
   census: `00438B10` x7, `00414DB0` x5, `004142E0` x4, `00419010` x4, `00B63D50` x3,
   `00415620` x2, `007C47F0` x2, plus nine `call eax`. No `009F9E40`/`009F9ED0` appears, so the
   commands go through those indirect calls.
2. **Phase A**, `009C0251`-`009C0EE0`, ~1200 instructions. It now reaches the dispatch through
   only **two** channels (§5.12.1), so this is bounded: the regime selector and `base-0Ch`. Two
   leaf guards are already read (`009C08C7` / `009C0B96` / `009C0BC1`, `JA` on `e > 0.05 * p`);
   what is missing is what `e` and `p` are, which is the whole point.
3. **The `009C1552` subtree**, `009C1455`-`009C1560`, never entered by this packet.
4. `007D7DA0`'s body — a coordinated-turn rate from bank, pitch and TurnRollSpd; only its
   dimension is established.

## The five substitutions the host now carries, each labelled at its use site

| what | where | why it is the honest default |
|---|---|---|
| regime selector | always lead pursuit | its `D -> 0` limit is `station + 250*U`, i.e. formation flight |
| good-position gate | fly-to arm always | that arm's own `d = 0` limit is the leader's speed and `stationY` |
| `007D7DA0` rate | 0 | the lag law's zero-turn-rate limit; costs the lag only while the leader turns |
| `state+88h` | 1e30 | the named half of the band floor wins; the floor can only RISE |
| `007C47F0`, `classDesc+188h` | `plane_max_spd*0.9`, `plane_stall_spd` | §5.4 shows the ramp reaches only 0.39 toward the first |

## Traps this packet actually hit

* **`00419510` returns `out` in EAX**, so the `LEA EAX,[EDI+0CCh]` before the call at `009C0F13`
  and `009C100F` is DEAD. Reading it as the leader's pose row 0 gives a wrong, plausible law.
* **A register census must cover the whole body.** `BL` looked settled from five writes; there
  are nine, and the four extra ones invert the meaning of the dispatch.
* `Measure-Object -Line` does not count blank lines, so it under-reports a Markdown file's
  length — `docs/PLANE_FOLLOW_LAW.md` looked 100 lines shorter than it is.
* `python local/cfg.py` must not plain-`import blockmap`: that file calls `main()` at module
  scope.
* I rebuilt twice while one of my own runs held `bsp_game.exe`. Both builds failed at compile so
  nothing relinked and the run was unaffected, but that was luck, not method.
