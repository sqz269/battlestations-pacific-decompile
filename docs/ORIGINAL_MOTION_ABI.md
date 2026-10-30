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
report as physical velocity. Schema 2 left `dt` and `engine_dt` empty because
the engine frame delta was not observed. This repair establishes native field selection
and read consistency; it does not establish deterministic ticks or parity with
the reconstructed executable.

## Clock fields for the sampler handoff

The clock lane documents its writer evidence and coherent-read contract in
[`ORIGINAL_GAME_CLOCK.md`](ORIGINAL_GAME_CLOCK.md). Its address/width handoff is:

| Field | Native storage | Producer evidence |
| --- | --- | --- |
| Game pointer | uint32 at `00E188A8` | Loaded at `00875BB1`. |
| Game time | float32 at `00F876A4` | Reset `MOVSS 0087464C`; step stores `FSTP 00875C21`, `00875C80`, `00875F26`. |
| OnMove frame | uint32 at `game+648h` | `ADD DWORD 004E4FF5`. |
| Engine dt | float32 at `game+21F0h` | `FSTP 004C6F7F`; time controls `004C700D`, cinematic zero `004C7020`. |
| Fixed-step count | uint32 at `00F876B0` | `ADD DWORD 00875C6E`; reset `0087466B`. |
| Fixed-step dt | float32 at `00D0DE84` | Loaded at `00875C8D`. |

`sample_game_dt` is the difference between sampled game-time values. It is
distinct from the native `engine_dt` field and the fixed-step dt/count. The
motion breakpoint's `engine_dt_bytes` remains the float argument at `ESP+4`
for that particular native call; it is not a wall-clock interval.

## Guard thread and x87 capture protocol

The guard identifies the main thread as the thread that actually reaches the
original PE entry breakpoint. `processes[].initial_thread_id` comes from the
process-creation debug event; `original_entry_thread_id` and `main_thread_id`
come from the original entry event. `entry_thread_matches_initial` records
their equality, and the top-level `root_initial_thread_id` retains the
suspended `CreateProcessAsUser` thread ID. This does not infer thread identity
from a small numeric ID or from a control word.

After the mission is visible, the operator writes the private stage marker
`discover-motion.request`. The recommended content is:

```json
{"hits":20,"interval_ms":50,"timeout_seconds":30}
```

Empty or legacy plain-text markers use those defaults. An integer such as
`20` is also accepted. UTF-8 BOMs are supported. The JSON accepts only the three
shown keys: `hits` is 1–100, `interval_ms` is 10–1000, and `timeout_seconds` is
1–120. The marker is renamed to `discover-motion.ack` when inspected; that
acknowledges the request, not its completion. Read the launch receipt's
`processes[].motion_session.status` for `complete` or `expired`. Invalid or
busy requests are acknowledged and recorded in `motion_request_rejected`
without changing an active request. Wait for the previous request to finish
and its breakpoint to be restored before writing another marker. A process
is limited to 32 requests and 1000 recorded hits.

Each `motion_captures[]` record retains `thread_id` and adds:

| Field | Meaning |
| --- | --- |
| `event_thread_id` | Thread ID from this motion exception event; equals `thread_id`. |
| `initial_thread_id` | This process's initial thread ID. |
| `original_entry_thread_id`, `main_thread_id` | The actual original-entry thread, defining the main-thread comparison. |
| `is_main_thread` | Whether this event's thread equals `main_thread_id`. |
| `x87_control_word`, `x87_control_word_hex` | This event thread's `FloatSave.ControlWord`, read with its integer/control context through `Wow64GetThreadContext`. |
| `request_sequence`, `hit_index` | Request identity and one-based recorded-hit index. |
| `unit`, `controlled`, `matches_controlled_pointer` | Existing `ECX-310h` unit hypothesis and controlled-pointer equality; no ship/team inference. |

`motion_session.histogram` groups recorded hits by thread and x87 control word
and includes the main-thread equality. `unit_counts` and `unique_units` show
the actual spread across unit addresses. These are sampled calls: debugger
stops perturb scheduling, the rearm cadence is wall time, and the histogram
is not a frequency estimate for every engine call. A request can complete
with repeated units; inspect `unique_units` and recapture if the live sample
does not cover several units. Thread IDs are meaningful only with that PID
and run. Historical receipts lacking the entry/main identity cannot establish
whether an old `027Fh` hit came from the main thread.

The entry byte and EIP are restored for every trap. Already queued traps from
other threads are genuine calls and can fill the bounded request; excess
queued traps are rewound without adding records. Rearming occurs at a later
debug stop after scanning every tracked live WOW64 thread. Windows holds the
affected process's threads until the debugger continues the event; see
[Microsoft's debugging-event contract](https://learn.microsoft.com/en-us/windows/win32/debug/debugging-events).
A thread at the
entry or entry+1, or an unavailable context, defers arming. `CREATE_THREAD` and
`EXIT_THREAD` events maintain that inventory. No thread is single-stepped.
The interval has a deterministic 0–30 ms stagger to avoid one fixed frame
phase. At the requested count, no further rearm occurs. Expiry requests a
debug stop to restore an armed byte even if no motion call arrives. The
single debugger, source write-lock protections and owned-job cleanup remain
in force.

The focused manifested x86 probe uses two synthetic units and native threads,
with its main thread explicitly set to `007Fh` and worker to `027Fh`. It checks
six bounded hits against their actual Windows thread IDs, then rearms an idle
request to check expiry/restoration. These fixture settings are not evidence
about the retail game's thread precision. Live histogram results belong in a
separate run report; no PC24 classification changes follow from this tool
update.
