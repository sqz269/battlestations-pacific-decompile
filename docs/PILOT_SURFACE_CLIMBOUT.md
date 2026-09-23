# A live aircraft held under the surface: what holds it, and what the image does

Addresses: `007BCAA0` (the plane's `vtable[7Ch]`, the destroyed hook), `007F3970`
(BSP_Squadron_RemovePlane), `009FBA50` (the cruise altitude command), `007CB7F0` (the water
contact), `007CE3A7` (the depth kill), `009C7270` (the dive-bomb `done` tick's member placement,
host side).

Packet `cc9_pilot_surface_climbout`, 2026-09-23. Ghidra was read, not written. All names are
hypotheses. Nothing here is ABI-compatible or game-validated.

## 1. What the image does with a live aircraft at the surface

- **The contact does nothing** for a live AI aircraft of a class with a non-zero MinWaterSpd.
  `007CB7F0` returns at `007CB9C2` (`docs/WATER_SURFACE_LAW.md` 2.1). The aircraft stays in free
  flight, and the free-flight arm keeps calling the contact handler while the aircraft is under
  the line.
- **There is no altitude floor in the pilot's altitude command.** `009FBA50` clamps the
  commanded altitude only against the ceiling: `Dynamics/Ceiling` minus a margin, or the
  squadron's own limit when that is lower (`009FBA82`-`009FBAF7`). No term keeps it above the
  water. The host's census shows commanded altitudes of -39.1 m and -20.3 m for the Lexington
  fighters, reproduced from that rule.
- **The only end under the water is the depth kill.** `007CE3A7` calls
  `BSP_MissionEntity_Kill(unit, 1)` below -30 m (`docs/WATER_SURFACE_LAW.md` 2.4).
- **So the image neither climbs a live aircraft out nor holds it.** It keeps flying the pilot's
  commands, and it dies if they take its origin below -30 m. Nothing in the read makes a climb-out
  the rule. I did not find one, and I am not assuming one.

## 2. What holds it in the host: a dead leader's formation station

Current main, USN04, 9000 frames, RNG option on (`local\sP_9000.log`). A once-a-second census
of any live aircraft below 5 m, the `surface probe` lines, shows D3A Val #3.1|.-2:

| t (s) | altitude | vy | pitch | commanded altitude | commanded pitch |
| --- | --- | --- | --- | --- | --- |
| 293.80 | 4.80 | -0.08 | -0.0012 | 212.9 | -0.1716 |
| 294.80 | -20.34 | -0.08 | -0.0012 | 212.9 | -0.1716 |
| 295.80 to 304.80 | -20.34 | -0.07 to -0.08 | about -0.001 | 212.9 | -0.1716 |

- The altitude falls 25 m in one second at a vertical speed of -0.08 m/s. It then stays at
  -20.34 to two decimals for ten seconds, although the integrated vertical speed stays negative.
  So the position is being written, not flown.
- The writer is a per-tick station placement, most likely the dive-bomb `done` tick
  `run_dive_bomb_done_prepare_tick_009c7270`; the follow ticks place the same way. For a
  wing member it PLACES the member on its formation station every tick, `007F23A0` from the
  squadron's `members[0]`, standing in for the unread follow law `009BFEE0`/`009BEE30`.
- `members[0]` is D3A Val #3.1. It died at 281.60 s in the power-lost mode, glided to the water,
  and touched at 294.35 s. It is then frozen in state 6 at -2.16 m. Its formation station puts
  the member about 18 m lower, at -20.34.
- The member is therefore pinned under the surface on a dead leader. It is not deep enough for
  the depth kill.
- The earlier movieval|.-2 case (`local\dT_9000.log`, about 203 s at -0.80 m) has the same
  shape. Its leader, movieval, died at 184.11 s and touched the water at 204.35 s.

**The image does not keep a dead leader.** When the destroy flush sets `unit+5Dh`, it
dispatches `vtable[7Ch]`, which for a plane is `007BCAA0` (`00D19D28 + 7Ch = 00D19DA4`; the same
pointer sits in nine plane vtables).
- `007BCAA0` draws `unit+C14h` from `DeadEngineFireDelay`: `00BD2F10(ECX=0, [00E18718],
  [00E18714])`.
- It stores the slot at `+748h`, then calls `00959450`.
- At `007BCADE`-`007BCAEB` it calls `007F3970(unit+9D4h, plane, 0)`, BSP_Squadron_RemovePlane.
  That memmoves the member array at `squadron+3D0h` over the dead plane, decrements `+3CCh`,
  clears `plane+9D4h` and re-deals the formation indices through `007ED260`.
  (`docs/PLANE_SQUADRON.md` section 5 has the sequence.)
- `members[0]` is the flight leader (`docs/PLANE_SQUADRON_ENTITY.md`), so the next live member
  leads. No member is ever placed on a dead plane's station.

## 3. The binding (`kSquadronRemovesDeadBound`, default true)

At the death, in the plane-branch hunk where `kPlaneDeathModesBound` detects it, the dead plane's
entry in its squadron record becomes `kPlaneSquadronNoUnit`. Every reader of `member_units`
already skips that value, so this is the compaction. The formation indices are re-dealt on the
next placement (`formation_indices_assigned = false`). Each removal is logged as
`squadron remove plane`.

**Substitutions:**
- The destroy flush and the `vtable[7Ch]` dispatch are taken to happen at the death step.
- `007BCAA0`'s other effects, the engine-fire delay `C14h`, `+748h` and `00959450`, are not
  bound.

**The draws, step 4 of the packet.** The death-mode draws now go through the gunnery host:
`GameGunneryHost::death_mode_draw_00bd2f10`.
- The mode choice uses stream 1, the shared generator the gunnery draws also use.
- The ExplosionExplosionDelay draw uses stream 0, a separate generator. SUBSTITUTION, labelled:
  its seed is not the image's.
- Under `BSP_GUNNERY_RNG_STREAMS=1`, each (stream, unit) key gets its own generator.

## 4. Predictions, written before the runs

**Pair: USN04 9000, RNG option on, `kSquadronRemovesDeadBound` off against on, same binary
otherwise.** The draws now come from the gunnery host's keyed streams, so the modes differ from
`local\sP_9000.log`'s. Both sides of the pair draw the same modes.
- **The pinned member.** If a member's leader dies, the member leads its remaining wing. No
  `surface probe` line shows a constant altitude with a non-zero vertical speed.
- **D3A Val #3.1|.-2, or whichever member is pinned in the control.** It flies its own `done` or
  `goaway` from the leader's death on. It either survives above the water or is shot down. It
  does not sit at -20 m.
- **Halvings.** At or below the control's count. They are already low in this tree, at 5397
  against 10879 before the faithful dive set.
- **Friendly-hull hits.** Zero-damage hits on friendly ships around a pinned member fall toward 0.
- **Category 1 shots.** They fall if a pinned live member drew fire in the control.
- **Deaths and hits by entity.** The rows that move are the removed planes' squadron-mates, whose
  formation and leader change, and whatever they then attack. Expect movement in every squadron
  that lost a leader before its members' `done`.

**Draws moved to the shared stream (option off, measured only on the re-baseline).** Option-off
rows move through the coupling: each plane death now consumes one draw (the mode choice) from the stream
the gunnery draws share. So every later gunnery draw shifts, and the re-baseline rows differ from
any earlier ones for that reason alone. With the option on the draws are keyed per unit. The
pair above therefore isolates the squadron removal, but its modes differ from
`local\dT_9000.log`'s, whose draws came from the units-host generator.
