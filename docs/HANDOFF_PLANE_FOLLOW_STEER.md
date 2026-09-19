# Handoff: the plane follow steer direction (packet `cc8_follow_steer`)

Worker `cc8-follow-steer`, 2026-09-19, branch `agent/cc8-follow-steer`, tip `a67b19011`.
Successor of `cc8-follow-law`. Read `docs/PLANE_FOLLOW_LAW.md` first — §5.5-§5.8 and §7 are this
packet's work and are the real brief. This file is only what a cold session needs to resume.

## State

Read and committed this turn:

* **§5.6 the steer direction on the `009C1662` path** — it is the horizontal **perpendicular** of
  the aircraft's nose, `(uz, -ux)` or `(-uz, ux)`. This **withdraws** the predecessor's "carrot
  250 m ahead of the nose".
* **§5.6.1 the `00E0E2F8`-`00E0E2FB` globals** — four never-written `.data` bit constants
  `8/2/1/4`; the `TEST` has the mask in memory and `BL` as the variable.
* **§5.6.2 `BL` is a quadrant classifier** (`009C01D3`-`009C024F`) about `0` and `±π/2`; it picks
  the abeam side and decides the `009C1247` guard.
* **§5.7** — `009C1662` is one guarded regime, not the fall-through. **Withdraws** that too.
* **§5.8 + `tools/callee_effects_009bfee0.json`** — the callee table the frame walk needs.

Not started: item 3 (binding), item 4 (measurement). The host switch is **untouched** —
`kPlaneFormationPlacementEnabled` stays true, and reading the abeam direction made that *more*
clearly right, not less: a station-point substitution is further from the law than the withdrawn
reading suggested.

## Start here, in this order

1. `python tools/bsp.py brief`; claim the lease (`cc8_follow_steer`, see the packet brief).
2. **Do not re-derive the frame walk.** Run it straight from the committed table:
   `python tools/x87trace.py trace 009bfee0 009c1846 <x87trace_argv from tools/callee_effects_009bfee0.json> > local/esp_walk3.txt`
   Then read slots by their canonical `base [ESP+NNh]` annotation, never by the raw `[ESP+NNh]`.
   With an empty call table the walk is **wrong** and looks plausible (§5.8).
3. The listing dump and the predecessor's block-map tool are in this worktree's ignored `local/`:
   `arm_009bfee0.lst` (whole body, 1795 instructions), `blockmap.py`
   (`--mode blocks|calls|writes`, `--lo`/`--hi`), `esp_walk3.txt`, `callee_fx.txt`.

## The four open questions, in dependency order

1. **The producers of `A` and `V`** (§5.6.2). `A` = `base-1Ch` = `SubtractWrappedAngle` at
   `009C01CE`; `V` = `base+0Ch`, sign-tested only. Until these are traced the quadrant is
   unnamed, so the abeam regime has a mechanism but no *meaning* — this is the cheapest next win
   and it is what would let the regime be named (collision avoidance? break-away? overshoot?).
2. **The four remaining `state+44h` regimes** and their guards: `009C10F7`, `009C11D5`,
   `009C1222` (ends `JMP 009C16C0`), `009C1328`, plus the fifth at `009C1552` (ends
   `JMP 009C16D2`). Work backward from each store as in §5.6.
3. **The carrot's altitude offset**, `base+28h` × `base-0Ch` added to `state+34h` at `009C16B2`.
   Note `base-0Ch` is a heavily reused scratch slot (80+ references), so its value at `009C16B6`
   is whatever the last write on the taken path was — trace backward per path, do not assume.
4. **`009BEE30`'s hold arm** `009BEE56`-`009BF9E5` (~700 instructions), untouched: what a member
   already in good position is commanded.

## Traps this packet actually hit

* `tools/x87trace.py` starts with an **empty** call table and assumes `esp+0` for every call.
* `tools/calleefx.py` overruns into the next function where INT3 padding is absent; any callee it
  reports with two RET imms needs its body end from Ghidra and the imm read as bytes there. This
  is how `0042CF10` (7 call sites) looked like RET 0 when it is RET 4.
* A tail `POP ECX` is MSVC freeing a 4-byte local, **not** argument cleanup.
* A write census over a Ghidra listing must match `float ptr` / `double ptr` — x87 stores print
  as `FSTP float ptr [...]` and a byte/word/dword-keyed regex misses nearly all of them.
* `rg` is not on PATH in this PowerShell; use the Grep tool or `Select-String`.
