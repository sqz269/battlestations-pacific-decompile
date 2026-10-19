# The killer-less Yorktown_sqn05 death in the step-tail pair (packet `cc13-steptaildeath`)

Follow-up to `docs/CC13_COMPPAIR.md` item 2. The pair is cc9-physics1 `p23c_ab_u04` (OFF) and `p23c_abc_u04` (ON,
`kSquadronGeneralStepTailBound`), USN04, reference env. In the ON log, Yorktown-class01_sqn05 dies at 230.71 s with
`killed_by -`. It is the only killer-less death in the pair. Read-only: no game runs, no code.

**Verdict.**
- The death is a **mid-air collision** with the A6M Zero #7.2 that the squadron was dogfighting.
- The image kills both planes the same way, so this is not a host defect and needs no switch.
- The step tail's part is indirect: it shifts timing, and does not place the squadron.

## 1. The death path (ON log, lines 53771-53806)

| line | record |
|---|---|
| 53771 | `plane crash sweep: plane=Yorktown-class01_sqn05 class=99 point=2 victim=A6M Zero #7.2 t=230.71 at=(8726.3 836.2 -9579.3) width=12.0 bound=1 (007C0910 -> 0084BF00)`: the right wing point's swept segment hits the Zero at 836 m altitude |
| 53772 | the Zero's death row: `killer=Yorktown-class01_sqn05 ... killer_blast=1` |
| 53773 | `plane crash impact: ... class=99 blast=350..500 range=20 reached=1 dealt=154.4 victim_health=0.0 (0084BC60 / 007BBA10)` |
| 53774 | Yorktown_sqn05's death row: `first_damage=-1.00 killer=-`. Its table row reads `taken 0, health 280`: undamaged, at full health |
| 53778 | `plane death mode: unit=Yorktown-class01_sqn05 ... mode=powerlost` |

The plane was not lost to terrain, water, fuel or a landing crash. The death-row writer is not omitting a killer
either: the plane dies by its own impact's Kill(1), which has no attacker.

## 2. The image does the same

The gate block 007CEEC1-007CEF67 lies inside 007CE040, with ESI = plane+310h and EDI = the plane. The sweep runs when all
of these hold:
- plane+5Ch is set (007CEEC1);
- 007BC5B0 BSP_Plane_IsDownOrHumanHeld answers true (007CEED0);
- the step is positive (007CEEE6);
- plane+520h is clear (007CEEF3);
- the plane is not the live controlled player plane (007CEF00-007CEF21).

For an AI plane with +61h clear, 007BC5B0 tests flight state +900h in {7, 6, 4} and +AA0h <= 0. It then answers true
when the invincibility timer +150h <= 0 (007BC5DE-007BC603). It does not test health. A healthy, non-invincible AI
fighter in free flight is therefore swept every fixed step.

After 007CEF67 CALL 007C0910, the caller (007CF02D-007CF0CC) drops an entity hit (mode 1) or a plane hit (mode 4)
only when plane+9E0h is set. Only the control-apply pass writes that byte (`unit+9E0h = 1` at 007D174D,
docs/PILOT_COMMAND_PATH.md); it is clear for single-player AI planes. Every other hit sends message 0CAh through
0077C2A0. Its handler 007BBA10 applies the KamikazeBulletClass impact at the point and Kill(1) to the plane for modes
1, 3 and 4.

The host's `run_plane_crash_sweeps_007c0910` (`src/game_hosts_gunnery.cpp`, `kPlaneCrashSweepBound`, cc9 lua53)
reproduces that outcome:
- the Zero takes the blast, credited to the plane;
- the plane is killed with no attacker.

Gates the host does not model (none applies here):
- 007BC5B0's invincibility arm: an AI plane with +150h > 0 is not swept in the image;
- plane+520h (its meaning is not read);
- plane+5Ch.

cc9 owns `include/bsp/plane_crash_sweep.hpp` and will note these there.

## 3. The step tail's part

| time (s) | ON event (log line) |
|---|---|
| 168.06 | first free-attack tick, head 0 (36477) |
| 170.71 | step tail: `0084E595 moveto-self` (37067) |
| 171.06 | free-attack override 00E08F58 on A6M Zero #8.2 replaces the head, 0.35 s later (37188) |
| 192.46 | #8.2 gone; override on A6M Zero #7.2 (41478) |
| 230.71 | the dogfight closes 3031 m to 11.3 m; `aim -> avoid_roll d=20.1` (53743), `avoid_roll -> maneuver d=9.9` (53757); collision (53771) |

The loiter neither placed the squadron at the collision point nor set its altitude.

In the OFF log, the squadron's first free-attack tick is at 183.06 s. It takes the stop-head path onto D3A Val #7.1
(cc13-stophead) and never meets #7.2. That divergence is the RNG-coupled shift cc9 recorded for this pair
(OCEAN_WAVE_FIELD 95, "USN04: MOVED ... RNG-coupled"). Closing to under 10 m inside the dogfight's avoid states is a
question about the dogfight bodies (cc9), not about the step tail.

## 4. Uncertainty

- plane+520h is not read. The host's sweep gate does not test it.
- Whether the image's dogfight avoid would have kept the two planes apart at 9.9 m is not established here.
