# 007C7800: the plane caller of the random wander (cc9 RNG question)

Worker cc13-dyn1, 2026-10-12, on main 7ce0d6898. Read only from the exports, the Ghidra listing and the
PE on disk; nothing was run. Question from cc9: `007C7800` calls `0042C1B0`
(BSP_RandomWander_Retarget, 2 to 4 draws from the shared gameplay stream 0, `00BD2F10` with ECX=0; draw
table in `docs/CC13_SEGMENT2_CAMERA_MATH.md`). Is it gameplay, how often does it run, does the host model it?

## Answer

**Presentation, not plane AI.** `007C7800` is the player's cockpit-camera gun shake. It writes only two
fields of the camera passed as its argument and nothing of the plane. It runs for the **player's plane
only**, up to twice per frame, and it draws stream 0 **only on the frame the shake switches on**. That is the
first frame on which the forward-gun latch is set on a plane with a pilot-fired gun, while a cockpit camera
is attached. The host does not model it. In the image, then, stream-0 consumption depends on the
player's view and trigger. An idle-player harness run never reaches the draw. A run where the player fires
from the cockpit draws 2 to 4 extra values per burst start, plus the per-period retargets that the cockpit
step `0042DAF0` makes while the budget lasts. After such a burst, every later gameplay draw in the image is
offset relative to a host that skips it. The fix belongs to the host's RNG rule, not to gameplay. If the
cockpit shake is ever wired, it takes a separate presentation stream or stays behind an OFF switch.

## The function

`007C7800 __thiscall(plane ECX, camera [ESP+4])`, `RET 4` (`007C7892`). Assembly `007C7800..007C7892`:

```
tuning = 0042E740()                      ; BSP_GameTuning_GetSingleton, 13 dwords copied from +84h
camera+3ACh = -(plane+9ECh * tuning+94h) ; 007C7828 FMUL [ESP+20h] = copy[+10h] = tuning+94h; 007C7836
v = (plane+9FAh && plane+C24h) ? 1.0 [00D7A24C] : -1.0 [00D7A260]     ; 007C783C..007C7858
rec = camera+428h                        ; 007C786C, wander record 0 of the cockpit camera
if (v > 0 && rec+2Ch <= 0) 0042C1B0(rec) ; 007C7863 COMISS, 007C7874 COMISS, 007C787C CALL (ECX = rec)
rec+2Ch = v                              ; 007C7888
```

- `plane+9FAh` is the forward-gun latch (`docs/PLANE_GUNFIRE.md`, `docs/DOGFIGHT_GUN.md` list `007C7800`
  among its readers). `plane+C24h` is "a gun platform has `PilotFires`" (`docs/ATTACK_GATE_TAILS.md`, "Gate 2").
- `camera+428h` is `wander[0]` of the plane cockpit camera (`include/bsp/segment2_camera_cc13.hpp:291`,
  stepped by `00434810`). `+2Ch` is its budget. The retarget fires only on the edge from <= 0 to > 0.
- Tuning `+94h` lies inside the 13-dword `PlaneCamera` block that starts at `+84h` (CockpitFOVMul,
  `docs/GAME_TUNING_SINGLETON.md`). It scales `plane+9ECh` into the camera's `+3ACh` angle. The field
  is not named here.
- The function writes nothing to the plane.

## Callers (rel32 + absolute-dword scan of the PE on disk)

Scanned with `local/dy1_xscan.py` (E8/E9 rel32 over `.text`, plus every aligned or unaligned dword equal to
the target in every section). For the positive control, `0042C1B0` returns its two known callers
(`0042DB62` in `0042DAF0`, `007C787C` here) and two more that `ghidra callers` misses (below).

| target | hits |
|---|---|
| `007C7800` | `CALL 0060AC48`, `CALL 0079639A`; abs `00D08B3F` (unaligned, in `.rdata`; taken as a chance match: no aligned pointer exists) |
| `0042C1B0` | `CALL 0042DB62`, `007BDC47`, `007BDC96`, `007C787C` |
| `007BDC30`, `007BDC60` | none |

1. **`0060AC48` in `0060ABD0`.** `0060ABD0` has no direct caller. Its only reference is slot `+20h` (`00CF43CC`)
   of vtable `00CF43AC`, the HudPlaneEffectsScreen (screen id 3Fh, per the ledger name of slot `+00h`, `00606220`). It takes
   one float and returns `RET 4`. It runs once per frame while the player-plane HUD screen is active. It
   first calls `007C3CB0(dt)` on the screen's unit `+B8h` (`0060AC1A`). If the screen's camera `+98h` is
   non-null and the unit's virtual `+1C0h` answers non-zero, it calls `007C7800(unit, camera)`
   (`0060AC1F..0060AC48`). So the plane is the HUD's unit, which is the player's.
2. **`0079639A` in `00795EE0` BSP_GameCameraModifier_Evaluate.** The modifier's fps byte `+78h` must be set, and
   the parent must answer `IsKindOf(0Fh)` (vtable `+5Ch`, `PUSH 0Fh`). The call is then
   `007C7800(parent, modifier+8)` (`0079637B..0079639A`). This is the "fps attach" step of
   `docs/CC13_CAMERA_CONTROLLERS.md` item 4, the player camera.

AI planes never reach it. The draw needs a `wander[0]` budget edge, so the steady state is 0 draws per
frame. The edge is +1 retarget (2 to 4 draws) when the latch and the PilotFires condition both go true.

### The two extra `0042C1B0` callers are dead out-of-line copies

`007BDC30 __thiscall(record, float)` and `007BDC60 __thiscall(camera, bool)` sit between INT3 padding, and
Ghidra has no function at either. They are the same edge setter, by record and by camera: `007BDC60` is the
tail of `007C7800` from the `1.0 / -1.0` select onward. Neither address has a rel32 or absolute reference
anywhere in the image, so the linker kept them but nothing calls them. No functions were created for them.

## Host

`git grep -i "007c7800\|0042c1b0\|0060abd0"` over `src/` and `include/`:
- `random_wander_retarget_0042c1b0` and `plane_cockpit_camera_step_00434810` (`src/segment2_camera_cc13.cpp`)
  are reconstructed, build-tested and fixture-free. They take a caller-supplied `WanderRandom` and never
  touch stream 0. They are not called from any game host.
- `CameraControllerHost::fps_attach_007c7800` (`include/bsp/cc13_camera_controllers.hpp:229`) is a pure
  virtual that `src/cc13_camera_controllers.cpp:537` calls. No class in `src/` or `include/` overrides it.
- `0060ABD0` and `007C3CB0` have no host counterpart (`docs/SCENE_CONTENTS_HOSTS.md` 12.6 and 13).

So the host skips `007C7800`, and every draw it makes, completely.

## Names

| address | name | status |
|---|---|---|
| `007C7800` | `BSP_Plane_SyncCockpitCameraGunShake` | provisional, Ghidra and ledger |
| `0060ABD0` | `BSP_HudPlaneEffectsScreen_Update` | provisional, Ghidra and ledger (slot `+20h` of `00CF43AC`, the float argument) |

## Open

- The unit virtual `+1C0h` gate in `0060ABD0` is not identified.
- Tuning `+94h` (the `+3ACh` scale) is not named.
- No other image writer of `camera+428h+2Ch` was found by a `disp32 49Ch` scan (`?? 9C 04 00 00`). That
  scan covers only literal disp32 operands, so a writer that goes through a base register plus 2Ch would be
  missed. Writers of `wander[1]` and `wander[2]` were not looked for.
