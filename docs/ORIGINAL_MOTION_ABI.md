# Original-game motion sampler ABI

The original x86 process has two distinct native Dyn objects. For a unit `U`,
the controller is `C = [U+1018h]`, its body is `B = [C+2Ch]`, and the motion
state is **`M = [B+4h]`**. Linear velocity is three float32 values at `M+00h`;
angular velocity is three float32 values at `M+0Ch`. Both vectors are in world
space. `B+00h` and `B+0Ch` are not those velocity vectors.

`docs/RIGID_BODY_INTEGRATION.md` and `docs/UNIT_CONTROLLER_UPDATE.md` distinguish
`B` from `M`. Their motion-state table is relative to `M`. The prior
`docs/MOTION_DIFFERENTIAL.md` paragraph incorrectly described those offsets as
relative to the body, and the first external sampler followed that paragraph.
New C++ math interfaces and their struct layouts are reconstructed interfaces;
their member offsets alone are not native ABI evidence.

## Assembly evidence

Checked against the saved `C:/Users/sqz269/bsp.gpr` configuration and live
`/battlestationspacific.exe` program on 2026-10-30. This repair makes no Ghidra
edits or native calls. Names below remain descriptive reconstruction names.

- `0092E8CF` sets `ESI=ECX`, the controller. `0092E8F4` and `0092E937` load
  `[ESI+2Ch]` into `ECX` for body accessors. `0092E8D8` uses `C+80h` for the
  smoothed rudder, and `0092E94A` loads the unit back-pointer from `C+1Ch`.
- `00C41556` starts the world body list in `EDX`. **`00C41587 MOV
  ECX,[EDX+4]`** loads the separate motion state. `00C415B7..00C415EB` update
  `M+00h/+04h/+08h` using force times inverse mass and time. Gravity and linear
  damping then update the same vector. `00C419DE..00C419F9` apply angular
  damping to `M+0Ch/+10h/+14h`.
- In the position phase, **`00C5B1EA MOV ESI,[EDI+4]`** again obtains `M`
  from body `EDI`. `00C5B1F8..00C5B29F` add `(M.linear + M.pseudo_linear)*dt`
  to `B+2Ch/+30h/+34h`. `00C5B2A2..00C5B2B9` use
  `M+0Ch/+10h/+14h` plus angular pseudo-velocity to rotate the body's basis.
  This producer/consumer pair establishes the vector meanings independently
  of whether a group of live floats looks plausible.

Pose and order offsets remain documented in `docs/MOTION_DIFFERENTIAL.md`
section 3: pose-valid `U+C8h`, up `U+DCh`, forward `U+ECh`, position `U+FCh`,
ordered throttle/rudder `U+980h/+984h`. The CSV projects linear velocity onto
forward for `fwd_speed` and angular velocity onto up for `yaw_rate`.

## Read-only live cross-check

On 2026-10-30 at 00:51:17 UTC, five unsuspended reads over 1.0022 wall seconds
of the original mission process (PID 32132) gave this stable chain:

`U=2A32B7B0 -> C=17B41CD0 -> B=1A19BFB8 -> M=1A1BD3C8`.

The on-disk original executable SHA-256, confirmed again by schema 2 sampler
discovery at 00:53:21 UTC, is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The first `M` sample had linear velocity `(16.1654, 0.01653, -0.89803)` and
angular velocity `(0.00282666, 0.0000873311, 0.0000852197)`. Its projected
forward speed was `16.1903 m/s` (about `31.47` knots), and its projected yaw
rate was `0.0000874792 rad/s`. Over the read window, the independent pose
secant was `(16.0584, 0.11786, -0.89257)` world units per wall second. These
are a consistency check alongside the integrator assembly, not a claim that
wall time exactly equals engine time. The primary operator's mission/UI
observation is separate evidence.

Raw read evidence is ignored local output at
`local/original-native-motion-abi/live-check.json` in the repair worktree.
The corrected sampler's successful controlled-unit discovery is in
`local/original-native-motion-abi/fixed-discovery.json` there. Python compilation,
whitespace checks, and a focused synthetic indirection/pointer-change check
passed; no reconstructed C++ changed.
The sampler's schema 2 adds `motion_state_ptr`, rechecks `[B+4]`, and includes
the motion-state pointer in its stable identity. Schema 1 captures made by
the earlier sampler read body data as velocity; their velocity, forward-speed,
angular-velocity and yaw-rate columns require recapture.

External reads remain non-atomic and may straddle engine updates. The native
position integrator also adds pseudo-velocity, which this sampler does not
report as physical velocity. `dt` and `engine_dt` stay empty because the engine
frame delta was not observed. This repair establishes native field selection
and read consistency; it does not establish deterministic ticks or parity with
the reconstructed executable.
