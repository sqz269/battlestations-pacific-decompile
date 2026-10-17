# CC13 fix 5: the follow band's SafeAlt floor (finding R3)

Switch: `kFollowBandSafeAltFloorBound` in `src/game_hosts_units.cpp:5776`, landed **OFF**.
With it OFF the host is unchanged. Grant: the cc9 gameplay lead, 2026-10-17. Scope: the band tail
only (image 009C16E5-009C183B, plus the state+88h value latched by follow Enter 009BED80). The R9
release arm in `run_follow_tick_009c1fd0` is not touched.

## Image, re-read from the listing

Follow Enter `009BED80`:

```
009bed84  CALL 0042E740            ; BSP_GameTuning_GetSingleton
009bed89  MOV  ECX,[ESI+6Ch]       ; the state's Pilot/Follow block
009bed8c  ADD  EAX,380h
009bed92  CALL 009BE150            ; copy singleton+380h.. into the block (float by float)
009bed9a  MOV  EAX,[ESI+6Ch]
009bedb0  FLD  float ptr [EAX+8]   ; block+08h = singleton+388h
009bedbb  FSTP float ptr [ESI+88h] ; state+88h, latched once at Enter
```

singleton+388h is `pilot_follow_safe_alt`, Pilot/Follow/SafeAlt
(`include/bsp/game_tuning_singleton.hpp:270`, loader row `src/game_tuning_singleton.cpp:234`),
authored 60 in this installation's planeglobals.lua line 498. The host already loads it; nothing new is
invented.

The band, `009BFEE0`'s tail:

- 009C16E5-009C16FB: a = float(block+04h + leaderY) (FADD then FSTP float), b = state+88h.
- 009C16FF-009C1725: FCOMIP a,b; JBE -> L = a; else L = b. So L = b only when a > b and ordered;
  an unordered pair keeps a.
- 009C1734-009C1740: c = float(leaderY - 120.0), with 120.0 the **double** at 00D1F3F8.
- 009C1744-009C175A: FCOMIP c,L; JBE keeps L; else L = c. So L = c only when c > L and ordered.
- 009C1779-009C17B9: the ceiling, min(singleton+210h, leaderY + 120.0), unchanged.
- 009C17C6-009C183B: station Y (state+34h) and steer Y (state+48h) clamped into [L, cap], unchanged.

No branch skips 009C171B-009C175A. The tail has three entries in 009BFEE0, all before it: 009C0021
(the latched/hold path, JMP 009C16D2), 009C123C (JMP 009C16C0, the lead-pursuit copy to +60h) and
009C15BB (JMP 009C16D2). A jump census of the whole body (disasm 009BFEE0, every `J* 0x009c16..18`)
finds no other target inside the tail. That settles U4 of docs/CC13_VERIFY15.md: the host's
"-120 only with Phase A" has no image counterpart.

Precision: every intermediate is stored to a float slot before the compare, and leaderY - 120.0 and
leaderY + 10 are exact in double for in-range altitudes, so the result is the same at 24- and 53-bit
precision control. No assumption about the unit job's thread is needed.

## What the switch changes

All three host sites that build this band:

| Site | OFF (unchanged) | ON |
|---|---|---|
| fly-to geometry, `src/game_hosts_units.cpp:9053-9059` | `state_88 = 1e30`; -120 only via `run_phase_a` (kFollowPhaseABound) | `state_88 = gt.pilot_follow_safe_alt`, `band_floor_image = true` |
| hold-arm stand-in geometry, `:8457-8460` | `state_88 = 1e30`, no -120 | same as fly-to |
| catch-up steer point, `:8416-8425` | floor = leaderY + LeaderFollowAlt | floor = min(that, SafeAlt), then max with float(leaderY - 120.0) |

`bsp::plane_follow_geometry_009bfee0` (`src/plane_follow_law.cpp:659-667`) takes a new input
`band_floor_image` (`include/bsp/plane_follow_law.hpp:264-269`, default false). When set, the floor is
the image's two compares with the JBE arms above, and the -120 bound applies whatever `run_phase_a`
is. When clear, the older code runs as before.

The other floats of 009BE150's copy: the band reads only block+04h (LeaderFollowAlt) and block+08h
(SafeAlt). The host reads LeaderFollowAlt from the same singleton offset +384h already, so nothing else
in the copy affects the band.

## What a pair run should show (ON against OFF)

- For a leader above 50 m, L becomes max(60, leaderY - 120) instead of leaderY + 10. Station and steer
  Y below leaderY + 10 are no longer lifted to leaderY + 10: wingmen whose formation station sits below
  the leader keep that station, down to max(60, leaderY - 120).
- For a leader below 50 m, L = leaderY + 10 as before (min picks it), bounded below by leaderY - 120.
- The spent-member done floor (the same tail on the dive-bomb done tick, 009C7278 -> 009C1FD0) is
  max(60, leaderY - 120) for a leader above 50 m, so a spent member can follow down to 60 m.
- Expected movement: follow-state wingman altitude rows on missions with squadrons (torpedo and
  dive-bomb follow ticks). No RNG draw is in this path.

## Uncertainties

- state+88h is latched at Enter; the host reads the singleton each tick. They agree unless the
  Pilot/Follow tuning changes between Enter and the tick (it is loaded once at 007E2A20; no reload path
  was checked).
- The catch-up inline band (`:8416`) clamps only the steer Y, as it did before; the image tail also
  clamps the station Y there. That pre-existing difference is outside this switch.
- The size of the effect depends on the formations' station Y offsets, which were not measured. No game
  run was made (none allowed in this packet).
