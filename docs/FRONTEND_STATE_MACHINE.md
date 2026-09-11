# The front-end screen state machine

Addresses: 004f7180, 004f71a0, 004f71d0, 004f71f0, 004f7570, 004f7580, 004f7590, 004f75a0,
004f75b0, 004f75c0, 004f75d0, 004f75e0, 004f8830, 004f83b0, 0068d8d0, 0068d850, 0068d8a0,
004c9a70, 0067cfb0, 004d7920, 004e4430, 004e4000

Packet `cc_frontend_states`, read-only analysis. The base vtable and the four empty virtuals were
read from the disk image with `bsp.py disasm-raw`; everything else comes from the stored Ghidra
listing. Ghidra was not modified.

Scope: the registry lifecycle and the state machine over the two flag bytes, plus the title
handover that carries a cold boot into the first interactive page. It deliberately does not restate
the pump body, the visibility commit, the screen sets, the title bring-up or the press-start flow:
those have reconstructions already, listed under "What this builds on".

## The base screen vtable, 00CEAE54

`004f7180` installs this table. Ten slots were read; the two cells that follow, `00CEAE7C` and
`00CEAE80`, point at `004f8b50` and `0042b110`, both outside the `004f7570..004f75e0` block this
class occupies, and are not claimed as members.

| Slot | Target | Body | Role |
| --- | --- | --- | --- |
| +00h | `00bf698e` | `__purecall` | screen id, used as the registry index |
| +04h | `004f7570` | `XOR AL,AL` / `RET` | self-managed visibility, false in the base |
| +08h | `004f7580` | `XOR AL,AL` / `RET` | unnamed predicate, false in the base |
| +0Ch | `004f75e0` | scalar deleting destructor | inlines `004f71a0` |
| +10h | `004f71d0` | register | |
| +14h | `004f7590` | `RET` | unnamed, empty, no arguments |
| +18h | `004f75a0` | `RET` | enter, no arguments |
| +1Ch | `004f75b0` | `RET` | exit, no arguments |
| +20h | `004f75c0` | `RET 4` | update, one stack argument |
| +24h | `004f75d0` | `RET 4` | collect children, one stack argument |

Four of the six behavioural slots are a single `RET`, so **the base class does nothing on enter,
exit, update or collect** and every observable screen behaviour is a leaf override. The `RET` sizes
are the argument counts: enter and exit take none, update and collect take one each. The `+04h`
predicate is the exemption `004f7620` tests twice per live slot before it may write `+4h`
(`docs/FRONTEND_SCREEN_SETS.md`), and in the base it is false, so an ordinary screen's visibility is
owned by the screen set rather than by itself.

## Registry lifecycle

The registry is the fixed pointer array `00E18B60..00E18CD8`, 95 slots. Both loop guards compare
against `00E18CDC` with `JL` (`004f71bd` and `004f88bf`), and `00E18CDC` is a separate byte that the
pump clears at `004f8881`.

**`004f7180`, construct.** `__thiscall`, ECX = the screen, no stack arguments, `RET 0`, returns
this in EAX. Stores the vtable and clears `+4h` and `+5h` from the same zeroed CL, so a screen is
born neither wanted nor active and is not yet in the array.

**`004f71d0`, register.** `__thiscall`, no stack arguments, `RET 0`. The whole body is
`id = this->vtable[0](); *(00E18B60 + id*4) = this`. Three consequences:

- The screen id *is* the slot index, and it comes from the leaf's pure virtual, so no id-to-screen
  table exists anywhere in the image.
- The store at `004f71d9` is unchecked. A leaf returning an id outside `0..94` writes past the
  array. The reconstruction bounds it instead, because reproducing the overrun would corrupt its own
  caller; the discarded case is recorded here.
- Registering twice under the same id silently replaces the occupant, and slots are not reference
  counted.

**`004f71a0`, unregister.** `__thiscall`, no stack arguments, `RET 0`. Restores the base vtable,
then walks all 95 slots and nulls every one holding `this`. It scans rather than using the id, so a
screen registered under several ids is removed from all of them, and it does **not** touch the flag
bytes: a screen destroyed while active leaves no exit call behind. `004b6e50`
(`bsp::close_front_end_screen`) is the routine that clears them.

**`004f71f0`, update trampoline.** `__thiscall(float)`, `RET 4`. Loads the vtable, pushes the float
and tail-calls `+20h`. It is how both the title object and the state 2 branch reach a screen's
update without walking the registry.

## The two flag bytes as a state machine

`+4h` is what the screen sets want and `+5h` is what the registry has applied. The four
combinations and the pump pass that acts on each:

| `+4h` | `+5h` | Phase | Pass | Pump body |
| --- | --- | --- | --- | --- |
| 0 | 0 | Hidden | none | skipped by all three sweeps |
| 1 | 0 | Entering | enter | `004f88d0..004f88ff` |
| 1 | 1 | Shown | update | `004f8906..004f8930` |
| 0 | 1 | Exiting | exit | `004f8890..004f88c5` |

`004f8830` makes three **full** sweeps in that order, so every exit in the frame completes before
any enter runs, and every enter completes before any update runs. Only `004f7620` writes `+4h`
outside the screens themselves; `+5h` is written by the pump, by `004b6e50` and by `0068d8d0`.

The commit order differs between the two edges and it is load bearing:

- **Exit**, `004f88a2`. The exit virtual `+1Ch` runs **first**. Only if `+5h` survived that call
  does the pump clear it and run `004f83b0`. A screen whose exit virtual already ran `004b6e50` on
  itself is therefore not committed twice.
- **Enter**, `004f88e2`. `+5h` is set and `004f83b0` runs **before** the enter virtual `+18h`, so an
  enter body observes itself as already visible.

`004f8830`'s own guard, `004f8830..004f887f`: when the menu command screen's modal-dialog byte
`+25Ch` is set, the pump updates only that screen (through its `+20h`, and only if it is active)
and returns, **unless** both `game+1FE4h` and `00E198C4` are nonzero, in which case it falls
through into the three sweeps. On that fall-through the menu screen is updated twice in one frame,
once by the guard and once by the update sweep, because it is also in the registry.

## The title handover, 0068d8d0

This is the only place a front-end screen is made visible without going through the pump's enter
pass, and it is the bridge from `GGame::OnInitTitle` into the registry. Vtable `00CF7A98` of the
`0x44` byte title object at `00E198C8`: `+4h` is `0068d8d0`, `+8h` is `0068d850`.

`__thiscall`, ECX = the title object, no stack arguments, `RET 0`, SEH handler `00C7E0CB`.

```
if (00E198CC /* skipTitle */ != 0) {
    00bd3450(*0109cecc);                 // 0068d8f8, reset storage availability
    if ((*00E188A8)->+5E8h != 0) return; // 0068d903, a request is already queued
    004d7920(game, 4);                   // 0068d90e, ECX is the game from 0068d8fd
    (*00E188A8)->+5ECh = 0;              // 0068d918
    return;
}
p = operator new(0x18);                  // 0068d931
if (p) 0067c840(p);                      // 0068d94b, BSP_PressStartScreen_Construct
this->+40h = p;                          // 0068d954, stored before it is tested
p->vtable[+10h]();                       // 0068d966, register; 0067ca80 for this leaf
s = this->+40h;
s->+4h = 1; s->+5h = 1;                  // 0068d96f, 0068d972, both from the same AL
004f83b0(s);                             // 0068d975
s->vtable[+18h]();                       // 0068d981, enter
```

The storage reset runs **before** the queue guard, so it happens even when the enqueue does not.
The allocation result is written into `title+40h` and then dereferenced at `0068d957` with no null
test; a failed allocation is a crash there, and the reconstruction returns instead.

Because the routine sets both bytes itself, the screen is already Shown when the next pump sweep
sees it, so **pass B never runs for the press-start screen** and its enter virtual is called exactly
once, from here.

`0068d850`, the title update, is two forwarding calls: the singleton at `00F8BBF4` through its
vtable `+4h`, then `004f71f0(title+40h)`. Both receive the **raw** delta, not the scaled one.
`00F8BBF4` is built at `00a908f6` and is still unidentified.

## The cold-boot path into the first interactive page

| Step | Native | Where it is reconstructed |
| --- | --- | --- |
| 1. `GGame::OnInit` leaves the state at 3 | `004e3ac2` | `bsp::kGameStateFrontEndInit` |
| 2. Title bring-up writes the state 2, loads `allbutingame.ats`, selects `FE_frame` and `FE_frame_title`, builds the attract and title objects | `004c9a70` | `bsp::run_title_init` |
| 3. Title activate builds the `0x18` byte press-start screen, registers it into slot `5Ch`, marks it wanted and active, commits and enters it; the leaf's register override loads `FE_initial` | `0068d8d0`, `0067ca80` | `bsp::activate_title_screen_0068d8d0` |
| 4. Each frame, state 2 updates the title object | `004e4c24`, `0068d850` | `bsp::update_title_screen_0068d850` |
| 5. Each frame, the shared tail pumps the registry | `004e4c9b`, `004f8830` | `bsp::run_front_end_screen_pump` |
| 6. The update sweep reaches the press-start body | `0067cfb0` | `bsp::press_start_screen.hpp` |
| 7. Every press-start exit funnels through the title skip, which enqueues state 4 when the ring is empty and then clears `game+5ECh` | `0068d8a0`, `004d7920` | `bsp::TitleHandoverHost::request_game_state` |
| 8. The drain pops 4 and runs the shell entry, which destroys the title object, loads the main-menu resource sets and settles the state at 5 | `004e4430`, `004e4000` | `bsp::drain_state_requests_004e4430`, `bsp/frontend_entry.hpp` |

**Which screen set is built first, and which page.** The title bring-up loads exactly two layouts
through `00518250`, `FE_frame` and `FE_frame_title`, plus `FE_attract` from the attract screen's
constructor. The first interactive page is `FE_initial`, loaded by the press-start screen's own
register override at step 3. No interface id and no screen set is applied on this path: raising the
main-menu screen set belongs to the state-4 entry `004e4000`, not to the title.

**What the press-start edge waits on.** Input action `4Eh` through
`BSP_InputAction_WasPressedThisFrame` (`004c43c0`), and only on the no-device arm; the sign-in,
storage and message-screen arms reach the same exit by other routes
(`docs/PRESS_START_SCREEN.md`). All four exits call `0068d8a0`, so the machine leaves the title only
through the state-4 request.

`bsp::next_front_end_boot_step` encodes steps 4 through 8: the two frame steps repeat until the page
accepts an input, and a request already in the ring suppresses the enqueue for another frame
(`0068d8bc`, the same guard as `0068d903`).

## Host methods the executable must implement, in call order

`bsp::TitleHandoverHost` in `include/bsp/frontend_state_machine.hpp`. One method per native call
site; nothing has a default implementation.

| Order | Method | Native site | Native callee |
| --- | --- | --- | --- |
| 1 | `reset_storage_availability` | `0068d8f8` | `00bd3450`, ECX = `*0109cecc`; skip path only |
| 2 | `request_game_state` | `0068d90e` | `004d7920`, ECX = the game, value 4 |
| 3 | `create_press_start_screen` | `0068d931` | `operator new(18h)` then `0067c840` |
| 4 | `screen_register` | `0068d966` | vtable `+10h`, `0067ca80` for this leaf |
| 5 | `screen_commit` | `0068d975` | `004f83b0` |
| 6 | `screen_enter` | `0068d981` | vtable `+18h`, `0067cb40` for this leaf |
| 7 | `update_title_owner` | `0068d866` | `*00F8BBF4` vtable `+4h`, per frame |
| 8 | `update_press_start_screen` | `0068d873` | `004f71f0` on `title+40h`, per frame |

Steps 1 and 2 are the skip path; 3 through 6 are the normal path; 7 and 8 are the per-frame update.
The registry routines themselves need no host: `bsp::register_front_end_screen_004f71d0` and
`bsp::unregister_front_end_screen_004f71a0` operate on the table directly, because the native bodies
make no calls except the id virtual, which is the caller's own screen id.

## What this builds on

Reused rather than restated: `bsp::run_front_end_screen_pump` and `bsp::run_front_end_state_frame`
(`docs/GAME_FRONTEND_STATES.md`), `bsp::commit_front_end_screen_visibility_004f83b0` and
`bsp::recompute_front_end_screen_requests_004f7620` (`docs/FRONTEND_SCREEN_SETS.md`),
`bsp::run_title_init` (`docs/GAME_TITLE_INIT.md`), the press-start body
(`docs/PRESS_START_SCREEN.md`), the queue and drain (`docs/GAME_FRAME_CONTROL.md`) and the shell
entry (`docs/GAME_FRONTEND_ENTRY.md`). `FrontEndScreen`, `FrontEndScreenTable` and
`kFrontEndScreenSlotCount` are taken from `bsp/frontend_states.hpp`, not redeclared.

## Corrections

**To `docs/GAME_FRONTEND_STATES.md`.** That doc lists the base vtable's `+00h`, `+10h`, `+18h`,
`+1Ch`, `+20h` and `+24h` and calls `004f71a0` "the destructor". `004f71a0` is not the destructor:
`+0Ch` of the same table, `004f75e0`, is the scalar deleting destructor and it inlines the same
registry scan. `004f71a0` is the unregister half. The four remaining slots, `+04h`, `+08h`, `+0Ch`
and `+14h`, are filled in above.

**To `docs/GAME_EXECUTABLE.md`.** Recorded in `docs/APP_FRAME_GAME_STATE.md`: the
`PlatformLoopCallbacks::pretranslate` host method is cited against `00bec20a`, which is the
loop-finished store; the call is at `00bec1d8`.

## No Ghidra function

These have no function in the saved project, so the integrator must define them before they can be
named. End addresses are inclusive and were read from the disk image; each is followed by `int3`
padding.

| Start | End | Bytes | Body |
| --- | --- | --- | --- |
| `004f7570` | `004f7572` | 3 | `XOR AL,AL` / `RET` |
| `004f7580` | `004f7582` | 3 | `XOR AL,AL` / `RET` |
| `004f7590` | `004f7590` | 1 | `RET` |
| `004f75a0` | `004f75a0` | 1 | `RET` |
| `004f75b0` | `004f75b0` | 1 | `RET` |
| `004f75c0` | `004f75c2` | 3 | `RET 4` |
| `004f75d0` | `004f75d2` | 3 | `RET 4` |

All seven are currently swallowed by the enclosing candidate `FUN_004f7430`.

## Reconstruction

`include/bsp/frontend_state_machine.hpp` and `src/frontend_state_machine.cpp`.

No new tests. The registry routines are array bookkeeping and the boot rule is a switch; the
existing `reconstructed_math` suite covers the arithmetic they touch, which is none.

## State reached per routine

| Address | State |
| --- | --- |
| 004f7180, 004f71d0, 004f71a0 | analyzed, reconstructed, build-tested |
| 0068d8d0, 0068d850 | analyzed, reconstructed, build-tested |
| 004f7570, 004f7580, 004f7590, 004f75a0, 004f75b0, 004f75c0, 004f75d0 | analyzed; empty bodies, reconstructed as constants |
| 004f75e0 | analyzed to the point that it is the deleting destructor; the tail was not read |
| 00CEAE54 | ten slots read from the image; slots past +24h not established |
| 004f8830, 004f83b0, 004c9a70, 0067cfb0, 0068d8a0 | analyzed by earlier packets; only the transition rules are restated here |

None of this is ABI-compatible or game-validated.

## What remains

- The identity of `00F8BBF4`, updated by `0068d850` through vtable `+4h`, built at `00a908f6`.
- Whether `00CEAE7C` and `00CEAE80` extend this vtable or begin the next one. `004f8b50` lies
  inside `BSP_MoviePlayer_Construct`'s Ghidra range with no function of its own, which is the only
  reason to doubt the simple reading.
- The role of base slots `+08h` and `+14h`: both are empty in the base and no override was traced.
- The screen ids. The registry is indexed by a per-leaf pure virtual, so the 95 slots stay unnamed
  apart from `5Ch`, the press-start screen.
- Whether any leaf overrides `+04h` to true, which would exempt it from the screen sets entirely.
