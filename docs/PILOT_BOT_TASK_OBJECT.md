# The bot task object is the planner's object, shifted by four

Addresses: `009998A0` `BSP_PilotBot_Update` (body `009998A0`-`009999C2`, 87 instructions, read in
full), `0099D300`, `0099B450`, `0099C270`, `0099B740`, `009A17D0`, `009FC7C0`, `009FD0E0`,
`009A2F40` `BSP_BotTask_MakeCloseToShip`, `009A2730`. Global `00F876B8`.

Packet `cc7_pilot_bot_task_object`. Ghidra **read-only**. **Exported / read only** — the finding is
a layout identity rather than a new law, so no C++ was added and no tests. Mod-artefact caveat
carried: scans cover `battlestationspacific.exe` only.

## Headline

`0099ACD0`'s **task** and `0099D300`'s **bot** are the **same object**. `009998A0` calls the planner
with `ECX = task + 4`:

```
009998CD  LEA EDI,[ESI + 4]          ; ESI = the task
00999907  CALL 0099D300              ; ECX = EDI
009999AA  CALL 0099D300              ; the other arm, same EDI
```

So every offset in `docs/PILOT_BOT_PLAN_CONTROLS.md` is **bot-relative = task-relative minus 4**,
and a host does not have to invent a task shape: it already has the layout, displaced.

Three independent confirmations rather than the `LEA` alone:

1. **The state byte.** `009998A4` tests `[task+270h] == 2`; the planner tests `[bot+26Ch] == 2` at
   `0099D309`. Same byte.
2. **The `+9C2h` pointer.** `009998AD` loads `[task+274h]` and indexes `+9C2h`; the planner loads
   `[bot+270h]` at `0099D337` and indexes `+9C2h` the same way. Same field.
3. **The unit.** `0099995C` loads `[task+2F4h]` and reads its `+C24h` — the
   `Platforms[].PilotFires` byte recovered in `docs/PILOT_BOT_PLAN_CONTROLS.md`; the planner loads
   the unit from `[bot+2F0h]`. Same field, and it is demonstrably a plane unit.

## The offset map

| task | bot (planner) | what |
| --- | --- | --- |
| `+270h` | `+26Ch` | the state byte; both routines gate on `== 2` |
| `+274h` | `+270h` | the second entity pointer, `+9C2h`-indexed |
| `+278h` | `+274h` | **the plan slots** — five 12-byte `{prev, desired, active}` records |
| `+2E4h` | `+2E0h` | set to `0` on one arm and `0FFh` on the other |
| `+2F4h` | `+2F0h` | the plane unit |
| `+2F8h` | `+2F4h` | the vehicle class descriptor |
| `+304h`/`+308h` | `+300h`/`+304h` | the update-interval pair |

`include/bsp/pilot_command_path.hpp`'s `task+278h` for the plan slots is therefore correct, and it
is the same array `include/bsp/plane_ai_control.hpp` calls `kPlanSlotBase = 0x274` on the bot.

## `009998A0` in full

```
009998A4  if (task+270h == 2 && task+274h != 0
009998BE      && [[task+274h] + idx*8 + 9C2h] != 0) return        ; the early-out
009998D2  0099B450(task+4)                                        ; seed the plan slots
009998D9  ok = 0099C270(task)
          if (ok) {
009998FB      task->vtable[64h](dt) ; task+2E4h = 0
00999907      0099D300(task+4, dt)
              return
          }
00999912  if (task+308h >= dt) {                                  ; FCOMI / JC at 00999926
0099992C      task+308h += task+304h - dt
0099993C      0099B740(task)
          } else {
009999B5      task+308h = dt - task+308h
          }
0099995A  task->vtable[64h](dt) ; task+2E4h = 0FFh                ; 00999950
0099995C  if (unit[+C24h] != 0) 009FC7C0(task+314h, dt)           ; PilotFires
0099998C  009FD0E0(task+38Ch, dt)
0099999B  009A17D0(task, dt)
009999AA  0099D300(task+4, dt)
```

Both arms end in the planner, which is what makes `0099D300` the single producer of the plan slots.
The two arms differ in the `+2E4h` stamp (`0` versus `0FFh`) and in the three extra sub-object ticks
on the slow path.

## A correction for the host interface

`PilotCmdBotTickHost` takes `task_step_flag_clear(task, idx)`. **The native index is not a
parameter**: `009998B7` reads it from the global word `[00F876B8]`, and the planner reads the same
global at `0099D316`. So it is one shared per-frame index, not something the caller chooses, and a
host that lets the two disagree would desynchronise the early-out from the planner's own gate.

The scaling matches on both sides: `009998BE` uses `ECX*8` directly, and `0099D31D`-`0099D329`
reaches `×8` through three `ADD EAX,EAX`.

## What a task is constructed with

`BSP_BotTask_MakeCloseToShip` (`009A2F40`), taken as the representative of the thirteen factories:

```
009A2F58  operator new(554h)                     ; 1364 bytes
009A2F7D  009A2730(obj, ECX_of_factory, EDX_of_factory)
```

so a task is a **`554h`-byte object** built by a two-argument constructor, and the factory forwards
its own two register arguments unchanged. The plan slots at `+278h`..`+2B3h` and every field above
sit comfortably inside that.

## What this means for wiring `0099ACD0`

The task does **not** need to be modelled from guesswork. What the tick reads is the planner's own
object at a four-byte displacement, and that object is already characterised. What a host still
cannot supply without more reading:

* `0099C270`, the arm selector whose boolean picks between the two paths — **unread**.
* `task->vtable[64h]`, the per-state step — **unread**, and it is a virtual, so it is per task kind.
* `0099B740`, `009A17D0`, `009FC7C0`, `009FD0E0` — the slow-path ticks, all **unread**.
* `009A2730`, the task constructor, and what the factories pass it — **unread** beyond the size.
* The remaining `554h` bytes: only the fields above are attributed.

So a host can bind the **fast path** — early-out, seed, planner — from what is now known, and must
refuse the slow path until `0099C270` and the state virtual are read. That is a real partial bind
rather than a modelled shape, which is the outcome the brief asked for.
