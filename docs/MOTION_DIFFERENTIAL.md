# The motion differential: is the reconstructed ship motion the game's motion?

Addresses: 00825F20, and read-only 00826866, 00826897, 008268A1, 008268AF, 008268F2,
00826900, 00826A3A, 00826A40, 00826A6D, 00826A82, 00826A8A, 0092E8C0, 0082ECB0,
0082E890, 00419010, 00424C40.

Worker `agent/cc-motion-trace`, packet `cc_motion_trace`, 2026-09-11. Ghidra was
read-only for this packet: no renames, no comments, no prototypes, no program save.
The one deviation from that is recorded under "Corrections" below. The installed game
at `I:/SteamLibrary/steamapps/common/Battlestations Pacific` was read but never written,
and was never launched.

**The packet's question is not answered.** Nothing here compares the reconstruction with
the running game, because the game was never run. What this packet does deliver is the
exact blocker record, a differential that was measurable without the game and that found
a real divergence, and the sampling plan plus the comparison tool so the live run can be
done in one sitting once a person can drive the menus.

## 1. The live trace: what was tried, and what blocked it

The Ghidra MCP debugger tools all proxy a standalone server on `127.0.0.1:8099`. In the
order tried:

| tool | result |
| --- | --- |
| `debugger_status` | `{"error": "Debugger server not running at http://127.0.0.1:8099. Start it with: python -m debugger"}` |
| `debugger_launch_offers` | `{"error":"Debugger not active and GhidraMCP could not auto-start a Debugger tool. Open the Debugger tool or enable Window > Debugger in CodeBrowser, then attach to your target process."}` |
| `debugger_attach`, `debugger_set_breakpoint` | the same 8099 error |

The server's backend was missing, not broken. `J:\tools\ghidra-mcp\debugger` needs
`pybag` and `comtypes` (`requirements-debugger.txt`), and neither was installed in any
interpreter on the machine. Installing `pybag==2.2.16` and `comtypes==1.4.16` into a
throwaway virtualenv under the session scratchpad was enough:

```
python -m debugger      # cwd J:\tools\ghidra-mcp
[debugger.engine] INFO: dbgeng COM initialized on worker thread
[debugger.server] INFO: Debugger server starting on 127.0.0.1:8099
```

`debugger_status` then answers `{"state": "detached", "module_count": 0, ...}`. **The
attach path works.** Nothing was attached to, because there was nothing to attach to.

Two things still block the run, and only the second one matters.

**The Ghidra launcher path is dead, so there is no launch offer.** After the server is
up, `debugger_launch_offers` still returns the same error: that tool does not use the
8099 server at all, it needs Ghidra's own Debugger tool and its Trace RMI launcher in the
GUI. `debugger_launch` with `dry_run` confirms the backend is not installed:

```
Debugger launch failed using 'dbgeng extra options': The back-end exited
(code=9009) before receiving a connection.
```

`9009` is the Windows "command not found" code. The packet's instruction was to launch
the game *if a launch offer exists*; none does, so the game was not launched. Starting it
outside the debugger and attaching afterwards is possible in principle - Steam is
running, and `xlive.dll` in the install is an emulator that creates a local user without
a Games for Windows Live prompt, as `xlive_debug.log` from the previous run on
2026-05-09 shows - but see the next paragraph for why that would not have produced a
trace, and note that running the game rewrites `bsp_debug.log` and `xlive_debug.log`
inside the installation this packet is required to leave untouched.

**The mission cannot be reached.** Recording the controlled unit in USN02 needs the game
driven from the main menu through the campaign screen, the mission list, the briefing and
into the mission. This session has no desktop input and no screen capture; the only
computer-use tools available are scoped to a browser. So the scenario is out of reach
regardless of the debugger, and it stays out of reach for any unattended agent. **The
live trace needs a person at the keyboard**, or a scripted input path into the menus that
does not exist yet. Once the game is in the mission and paused, every step in section 3
is an MCP call and needs no further human input.

## 2. The differential that was measurable, and what it found

Two reconstruction harnesses run the same motion code: `bsp_ship_motion_probe.exe`
(`src/ship_motion_probe.cpp`) and `bsp_game.exe` on the real mission
(`docs/GAME_EXECUTABLE.md`, milestone 2i). Neither is the game, so agreement between them
proves nothing about the game - but disagreement is a defect that any future differential
would have inherited, and there is one.

Both were run on `VehicleClass[20] DeRuyter class 1935`, the class USN02's controlled
unit resolves to (`MaxSpeed 16.4622`, `MaxAccel 3`, `Retardation 2`,
`MaxRotAngle 0.122173 rad/s`), at the `0.05 s` fixed step.

**The straight-line path agrees to the logs' own rounding.** Probe at rudder 0 against
`bsp_game.exe --order throttle=1.0,rudder=0.0`, over 6 s, origin-aligned:

| channel | peak absolute delta |
| --- | --- |
| forward speed | `0.0002` m/s |
| heading | `0.0000` deg |
| position | `0.0100` m |

That covers the throttle gate, the acceleration selection in `0092D300`, the target-speed
product and the integration. The residual is the log's two-decimal position field.

**The turn rate disagrees by exactly the rudder-curve denominator.** Probe at rudder 1
against `bsp_game.exe --order throttle=1.0,rudder=1.0`, over 18 s:

| t (s) | `bsp_game.exe` yaw | probe yaw | heading delta (deg) | position delta (m) |
| --- | --- | --- | --- | --- |
| 2.0 | `-0.04232` | `-0.03582` | `0.060` | `0.000` |
| 4.0 | `-0.08732` | `-0.05103` | `-2.533` | `0.032` |
| 6.0 | `-0.12217` | `-0.06109` | `-8.325` | `0.671` |
| 9.0 | `-0.12217` | `-0.06109` | `-18.825` | `5.085` |
| 12.0 | `-0.12217` | `-0.06109` | `-29.325` | `18.247` |
| 15.0 | `-0.12217` | `-0.06109` | `-39.825` | `44.571` |
| 18.0 | `-0.11912` | `-0.06109` | `-50.201` | `87.498` |

`bsp_game.exe` settles at `-0.12217` rad/s, which is `MaxRotAngle` exactly. The probe
settles at `-0.06109`, which is `MaxRotAngle / 2.0` exactly. The factor is the authored
denominator at full throttle.

**The probe is the one that is right.** `0082ECB0` divides `MaxRotAngle` by the curve
`0082E890` interpolates from the gameplay settings singleton `[00F8753C]` at
`+438h..+44Ch` (`00424C40` at `00424C55 MOV EAX,[0x00f8753c]`). Those knots were
recovered in `docs/UNIT_RUDDER_CURVE.md` and are authored in
`<install>/scripts/datatables/shipglobals.lua` under
`ShipGlobals["Navigator"]["TurnMultipliers"]`, verified again here at lines 340-342:

| key | `[1]` | `[2]` | settings fields |
| --- | --- | --- | --- |
| `TurnMultiplierMinSpeed` | `0.0` | `0.4` | `+444h`, `+440h` |
| `TurnMultiplierMedSpeed` | `0.5` | `1.5` | `+44Ch`, `+448h` |
| `TurnMultiplierMaxSpeed` | `1.0` | `2.0` | `+43Ch`, `+438h` |

At full throttle the denominator is `2.0`, so a hull turns at half its `MaxRotAngle`. The
probe's current build reads these knots and prints so in its header. `bsp_game.exe` still
forces all three to `1`, and says so at `local/game_run_r1.log`: *"the rudder curve
settings at 00424c40()+438h..+44Ch were never recovered, so the three denominator knots
are forced to 1 (the same stand-in src/ship_motion_probe.cpp uses)"*. That sentence is
stale on both counts - the knots were recovered, and the probe no longer forces them.

So **`bsp_game.exe` turns the mission's ships at twice the authored rate**, and every
milestone-2i number that depends on heading or on a turning hull's path is wrong by that
factor. Its straight-line distances are unaffected. This is a divergence between two
reconstructions, not a proven divergence from the game; but the game's own authored data
is the tie-breaker, and it agrees with the probe.

## 3. The sampling plan for the live run

Everything below is the unit-relative offset, read from the stored listing. **The unit
base is `ECX - 310h` at the `00825F20` entry**: `00825F2A MOV EDI,ECX`, then
`00825F32 LEA ESI,[EDI-310h]`. The first instruction of the routine is `SUB ESP,0B0h`, so
at a breakpoint placed on `00825F20` itself, `[ESP]` is the return address and `[ESP+4]`
is the incoming frame delta as a float, before any prologue has run.

**Breakpoint.** `debugger_set_breakpoint` at `0x00825F20`, module
`battlestationspacific.exe`. The routine is a virtual reached from three call sites and
runs once per ship per fixed step, so it fires for all 32 units; filter on the controlled
unit by comparing the computed unit base against the pointer the mission logged. For a
non-stopping capture `debugger_trace_function` at the same address with
`convention=__thiscall, arg_count=1` records the delta and `this` without pausing the
game, which is what a 20 s capture at 20 Hz wants; `debugger_read_memory` then pulls the
pose. Reading the whole pose needs the process stopped, so prefer the breakpoint for
fidelity and the trace for a first pass.

**Per-step fields.**

| what | address | evidence |
| --- | --- | --- |
| frame delta `dt` | `[ESP+4]` at entry | first instruction is `SUB ESP,0B0h` |
| unit base | `ECX - 310h` | `00825F2A`, `00825F32` |
| pose block | `unit+0CCh` | `008268F2 LEA EBP,[ESI+0CCh]` |
| pose row 0, lateral | `unit+0CCh..+0D4h` | row stride `10h` |
| pose row 1, up | `unit+0DCh..+0E4h` | the keel point's `down` term |
| pose row 2, forward | `unit+0ECh..+0F4h` | `0082687C FMUL [EBP+20h]`, `00826883 FLD [EBP+24h]`, `0082688C FMUL [EBP+28h]` |
| translation x, y, z | `unit+0FCh`, `+100h`, `+104h` | `00826897 FADD [ESI+0FCh]`, `008268A1 FLD [ESI+100h]`, `008268AF FLD [ESI+104h]` |
| pose-valid byte | `unit+0C8h` | `00826900 MOV byte ptr [ESI+0C8h],1` |
| out-of-action byte | `unit+5Dh` | `008269F4`, zeroes throttle |
| per-unit time scale | `unit+340h` | `00826126` |
| class descriptor | `[unit+538h]` | `00826A8A MOV EBP,[EDI+228h]`, `EDI = unit+310h` |
| order ring base | `unit+838h` | `docs/UNIT_STATE_MESSAGE.md` |
| ordered throttle | `unit+980h` = `ring+148h` | `0080D9E8`, `00826708` |
| ordered rudder | `unit+984h` = `ring+14Ch` | `0080DA3A`, `0082674C` |
| order kind byte | `unit+988h` = `ring+150h` | `00826A82 MOV AL,[EDI+678h]` |
| max speed | `unit+9C0h` | `00826A3A FLD [ESI+9C0h]` |
| engine value, engine jam | `unit+9D8h`, `unit+9E5h` | the engine gate `00826754` |
| rudder jam | `unit+9E4h` | `00826B2E` |
| boost timer, boost kind | `unit+1188h`, `unit+118Ch` | `00826A98`, `00826A90` |
| force controller | `[unit+1018h]` | `00826A40 MOV ECX,[EDI+0D08h]`, `EDI = unit+310h` |

**Through the controller** (`0092E8C0`: `ESI` the controller): unit back-pointer at
`controller+1Ch`, rigid body at `controller+2Ch`, smoothed rudder at `controller+80h`.
**On the rigid body** (`docs/RIGID_BODY_INTEGRATION.md`): linear velocity `body+00h`,
angular velocity `body+0Ch`, maximum linear speed `body+18h`. Forward speed for the CSV
is the linear velocity projected on pose row 2; the yaw rate is the angular velocity's
row-1 component, which is what `0092E8C0` slews.

**Two one-shot reads that settle open questions and need no mission at all**, only a
process sitting at the main menu:

1. `[00F8753C] + 438h .. + 44Ch`, six floats: the turn-multiplier knots as the running
   game actually loaded them. This confirms section 2's tie-break against the live
   process rather than against the Lua source.
2. The x87 control word. `docs/SHIP_MOTION.md` lists the shipped precision as its first
   open question and says a differential run is what would settle it. Read it at the
   breakpoint; `D3DCREATE_FPU_PRESERVE` would leave it at 64-bit, and the reconstruction
   assumes the MSVC default of 53-bit.

**What the capture should record.** One row per motion tick into `local/motion_trace.csv`
with the columns `tools/motion_trace_compare.py` reads:

```
t,dt,pos_x,pos_y,pos_z,fwd_x,fwd_y,fwd_z,fwd_speed,throttle,rudder,yaw_rate
```

`heading` is derived from `fwd_x`/`fwd_z` and need not be captured. 20 s at the `0.05 s`
step is 400 rows. Record the ordered pair at `unit+980h`/`+984h` every row: the authored
`Cruise` command's meaning is *not* recovered (`docs/GAME_EXECUTABLE.md` lists the
`Cruise` command object at `00469610` as open), so what the ring holds under it is itself
a finding, and it is what the probe must be told to reproduce.

**Then compare.** `python tools/motion_trace_compare.py --trace local/motion_trace.csv
--probe local/probe_class20.txt --align-origin --json reports/motion_trace_result.json`,
with the probe run at the class and ordered pair the trace reports. `--align-origin`
rigidly moves the capture's world coordinates onto the probe's frame, which starts at the
origin heading down `+z`. Do not pass `--apply-curve` to a current probe build; it exists
for probe output captured before the authored knots landed and would double-count the
denominator.

## 4. The comparison tool

`tools/motion_trace_compare.py` parses the probe's fixed-width table structurally, parses
the trace CSV, interpolates the probe onto the trace's timestamps with heading unwrapping,
and reports per-step deltas of speed, heading, position and yaw rate plus the first step
at which each channel leaves its tolerance. `--json` writes the full per-step table.

Exercised three ways, all reproducible from `local/`:

* a null differential, the probe's own trajectory fed back as a trace: every channel
  exactly `0.0000` over 41 samples and 20 s, so the parsing, the interpolation and the
  heading wrap are not inventing error;
* probe against `bsp_game.exe` at rudder 0: peak deltas `0.0002` m/s, `0.0000` deg,
  `0.0100` m;
* probe against `bsp_game.exe` at rudder 1: the divergence table in section 2, with the
  first heading divergence past 2 deg at `t = 4.00 s` and the first position divergence
  past 5 m at `t = 9.00 s`.

## Corrections

**To this packet's own protocol.** One mutating Ghidra call was made in error:
`disassemble_bytes` over `00826866..0082692F`, which reported 202 bytes disassembled. That
range is already inside `00825F20`'s existing function body, so it re-disassembled
existing instructions; the listing read back afterwards matches `docs/SHIP_MOTION.md`'s
keel-point arithmetic instruction for instruction (`FLD [EBX+0A0h]`,
`FMUL double ptr [00CEC9E0]`, the three row-2 multiplies, the three translation adds), so
no listing change resulted. The program was not saved. Every later listing read went
through the read-only `bsp.py ghidra disasm`.

**To `docs/SHIP_MOTION.md`, "The probe".** It records the probe's rudder curve and its
integrator as stand-ins: *"The probe forces the three denominator knots to 1.0"* and
*"a stand-in integrator"*. The current build does neither. Its header prints
*"rudder curve +438h..+44Ch from shipglobals.lua {0,0.4} {0.5,1.5} {1,2}"* and
*"integration 00C41550 + 00C5B1B0, the game's own routines"*. The two remaining
stand-ins are the ocean sampler and the gameplay scale. Its uncertainty 3 likewise says
`0082ECB0`'s settings block "remains unrecovered"; `docs/UNIT_RUDDER_CURVE.md` recovered
it, on the settings singleton rather than on the unit.

**To `docs/SHIP_MOTION.md`, the field at `unit+9C0h`.** Its target-speed extract annotates
`scaled = float(maxSpeed * gameplayScale)` with *"unit+9C0h, one store"*. The instruction
at `00826A3A` is `FLD float ptr [ESI+9C0h]`, a load: `unit+9C0h` holds the unit's maximum
speed and is the multiplicand, not the destination. The product is stored to the stack at
`00826A4F`.

**To `docs/GAME_EXECUTABLE.md`, milestone 2i.** Its stand-in note *"the rudder curve
settings at 00424c40()+438h..+44Ch were never recovered, so the three denominator knots
are forced to 1 (the same stand-in src/ship_motion_probe.cpp uses)"* is wrong in both
halves: the knots were recovered in `docs/UNIT_RUDDER_CURVE.md`, and the probe no longer
forces them. Consequently every milestone-2i heading and turning path is out by the
denominator, measured here as exactly `2.0` at full throttle. Its stated result *"with the
player's rudder at 0.5 it settles at a yaw rate of 0.06109 rad/s ... which is exactly
half"* reaches the right number for the wrong reason: at rudder `0.5` with the knots
forced to `1` the rate is `MaxRotAngle * 0.5`, whereas the shipped curve gives
`MaxRotAngle * 0.5 / 2.0 = 0.03054`, which is what the probe now produces at that rudder.
The agreement at `0.06109` is a coincidence of the two factors.

**To `src/ship_motion_probe.cpp`'s summary lines.** Its *"expected yaw rate with the
shipped curve"* line omits the rudder factor: at `--rudder 0.5` it still prints
`0.06109 rad/s (MaxRotAngle / 2.00)` against a measured `0.03054` and concludes
*"heading turns at the curve's rate: no"*. The measurement is right and the expectation is
wrong. No C++ was changed in this packet.

## Follow-up packets

| packet | addresses / files | why |
| --- | --- | --- |
| `game_harness_rudder_curve` | `src/game_hosts_units.cpp` `yaw_rate_target`, `0082ECB0`, `0082E890` | Give `bsp_game.exe` the authored knots the probe already reads, then re-run milestone 2i. Every heading and turning path in that milestone is currently out by the denominator. This is the one follow-up that changes shipped numbers. |
| `motion_live_trace` | `00825F20`, `[00F8753C]+438h..+44Ch` | The live run of section 3, once a person can drive the menus to USN02. The two one-shot reads need only a main menu. |
| `cruise_command_object` | `00469610`, `0046AAB0`, `unit+838h` ring | What the authored `Cruise` order writes into the ring. Section 3 captures it as a side effect; milestone 2i lists it as open, and the probe cannot reproduce the mission's ships without it. |
| `unit_pose_refresh` | `00414DB0`, `00825A64`, `unit+0CCh..+100h` | Already proposed by `docs/UNIT_INSTANCE_UPDATE.md`. This packet pinned the translation at `+0FCh/+100h/+104h` and the three basis rows from the keel-point arithmetic, which is most of it. |
| `x87_control_word` | `00825F20` entry, D3D9 device creation | Whether the shipped executable runs the x87 unit at 53-bit or 64-bit precision. `docs/SHIP_MOTION.md` calls this its first open question; a breakpoint read settles it. |

## no_ghidra_function

none. Every address cited here lies inside a routine that has a Ghidra function.

## Uncertainties

1. **The packet's question is unanswered.** No number here was taken from the running
   game. The section 2 divergence is between two reconstructions, tie-broken by the
   game's authored Lua rather than by its runtime behaviour.
2. Whether the game applies the turn curve to the *ordered* throttle or to the achieved
   speed ratio. `0082ECB0` takes `forwardSpeed / MaxSpeed`, and the Hungarian comment on
   the Lua knots says *gazkar allasa*, the throttle setting. The two agree in steady
   state and differ during the acceleration ramp, which is exactly where the section 2
   table shows the harnesses separating before `t = 6 s`.
3. The handedness of the body basis, unchanged from `docs/SHIP_MOTION.md`.
4. The x87 control word the shipped executable runs with, unchanged.
5. `--align-origin` assumes the capture and the probe start from rest on the same
   heading. A capture taken from a ship already under way needs its initial velocity fed
   to the probe instead, which the probe has no option for.
