# What pumps the front-end screens at game state 5 (packet `cc_main_menu_screens`)

Addresses: 004c40f0, 004c4165, 004c4205, 004c42a6, 006840f0, 00684700, 00683aa0, 00683d10,
00683e90, 005884a0, 004e53b4, 004e5442, 004e5460, 004cc460, 00684600, 00685820, 005cd1e9,
004b6e50, 004f71f0, 004f83b0, 004f8830, 005b6960, 00776230.

`docs/MAIN_MENU_PATH.md` ended at the frame-path boundary: `004E4279` writes `game+5D4h = 5`, and
state 5 is outside the `{1, 2, 4}` set that `GGame::OnMove` runs its front-end branch for. This
document covers what happens from that instant, and closes the four follow-ups that path left open.

Reconstruction: `include/bsp/main_menu_screens_runtime.hpp`, `src/main_menu_screens_runtime.cpp`.
Nothing here is a binary-compatible layout, and descriptive names are hypotheses, not recovered
symbols.

## What was already recovered, and what this packet actually adds

Four of the five routines the packet named were already reconstructed, and re-reading them to the
instruction confirmed the existing models rather than changing them. They are cited here, not
redefined:

| Routine | Where it already lives |
| --- | --- |
| 00684700 activate walk | `activate_front_end_manager_00684700`, `include/bsp/frontend_managers.hpp` |
| 00683AA0 deactivate | `deactivate_front_end_manager_00683aa0`, same header |
| 00683E90 lock | `front_end_request_rejected_00683e90`, same header |
| 006840F0 service pass | `service_pending_menu_requests_006840f0`, `include/bsp/session_polls.hpp` |
| 005884A0 title music | `start_title_music_005884a0`, `include/bsp/main_menu_screens.hpp` |
| the 004E53B4..004E548A frame | `run_render_tail`, `include/bsp/render_tail.hpp` |

The one piece with no body anywhere was **004C40F0 itself**. `run_render_tail` calls it as the
opaque host method `update_without_simulation`, and `SessionPollHost` calls it as
`update_interface_only_004c40f0`. `docs/GAME_SESSION_POLLS.md` describes it in prose; this packet
reconstructs it. That is the substance of the deliverable, and it matters because 004C4165, inside
this routine, is the **only** pump call the main menu gets at state 5.

## The state-5 frame, in call order

The simulation gate at 004E50B0 requires `game+5D4h == 0Dh`, so at state 5 it always fails and
OnMove falls through to 004E53B4. Steps 4..7 repeat while the service pass reports work.

| # | Address | Native call | What it does at state 5 |
| --- | --- | --- | --- |
| 1 | 004E53B4 | 004C40F0 | The non-simulating fallback. Reaches 004F8830 and draws the menu. |
| 2 | 004E5442 | 006840F0(&pending) | Services any manager whose pending record differs from its applied one. |
| 3 | 004E544C | `00E18CDC = 0` | Clears the pump sentinel the registry walk sets. |
| 4 | 004E5462 | 00776230 | `BSP_PeerManager_PumpMessageQueues`, ECX = `game+1EF0h`. |
| 5 | 004E5469 | 004C40F0 | The same interface update again, now over the newly published screen set. |
| 6 | 004E5477 | 006840F0(&pending) | The service pass again; the loop ends when it reports nothing. |
| 7 | 004E5481 | `00E18CDC = 0` | As step 3. |

**The main menu is visible on the frame it is entered, but the drain loop is not what does it.**
00684600 syncs the applied record to the pending one, and it runs twice before OnMove reaches step
2. First inside the shell entry: `BSP_MainMenu_Init` 00686380 calls it at 006868BD with id 4
`INTF_REWARDS` when 00E198B0 is 1 and id 1 `INTF_MAINMENU` otherwise, the returning-from-a-mission
case against the cold boot. EBX holds the 1 there, assigned once at 00686622 and preserved across
the intervening calls. Those two arms interlock with the shell: 004E4250 skips the push at
004E4259 exactly when the applied id is already 4, so the rewards arm skips it and the cold boot
pushes id 1. Second, whichever arm ran, 00684700's replay at 0068477E calls the manager's virtual
`+10h`, 00685820, which calls 00684600 again.

So by 004E5442 the applied and pending records match, step 2 reports nothing, and steps 4..7 never
run on the entry frame. **The level-4 screen set is published by the Activate replay, not by the
service pass.** What makes the menu visible is step 1: the post-drain state test at 004E4D12
compares only against 1, 2 and 4, so state 5 falls through and the same OnMove call carries on into
the render tail, where 004C40F0 at 004E53B4 pumps a set that is already published.

The drain loop is still live code. It matters for later navigation, where a push arrives without a
re-activation and the two records genuinely differ.

## 004C40F0 `BSP_Game_UpdateInterfaceOnly`, reconstructed

`__fastcall(GGame* ECX)`, no stack arguments, `RET` at 004C42F8. Sole caller `BSP_Game_OnMove`, from
004E5259 (the non-pause in-mission branch), 004E53B6 (the fallback state 5 takes) and 004E5469 (the
drain loop). Reconstruction: `run_interface_only_update_004c40f0`.

### The head, 004C40F5..004C412A

```
arm = (00E188AE != 0 || 00E18B34 != 0) && game+719Eh == 0;
game+719Eh = 0;            // 004C4118, unconditional, before either arm
00E18B34 = 00E188AE;       // 004C411E, the previous-frame latch
```

`00E188AE` is the system-UI byte `004CEB40` maintains, so the condition reads "the console guide is
up now or was up last frame". `game+719Eh` is a one-shot suppression byte that forces the normal arm
for exactly one frame; it is cleared before the arm runs, so it can never hold for two.

### Arm A, the system-UI arm, 004C4205

Skipped entirely when `00E198C4` is null, which is the case for every front-end frame. Otherwise it
walks that manager's three sub-objects at **+C8h, +9Ch, +A4h in that order** (registry slots 51h,
37h and 33h per `docs/IN_MISSION_INTERFACE_MANAGER.md`). Each non-null one whose `+5h` byte is set is
either updated through 004F71F0 when `+4h` is set, or closed through 004B6E50 when it is not. The
manager pointer is re-read from the global before each of the three offsets, but the null test is
done only once, at 004C420A.

### Arm B, the front-end arm, 004C4130

```
if (00425D10()->+25Ch == 0) {
    004F8830(game+635h ? game+21ECh : game+21F0h);     // 004C4165
} else {
    <update-or-close the 00425D10 singleton>           // 004C416F
    <update-or-close *(00E19698)>                      // 004C41BD
}
```

The first branch is the main-menu frame. `+25Ch` is the modal-dialog byte
(`bsp::MenuCommandScreen::modal_dialog_active`); with no dialog up, the whole front-end registry is
pumped and nothing else happens. The second branch replaces the registry pump with two hand-held
objects, and 004C41BD dereferences `00E19698` **without a null test**.

Both update-or-close blocks are 004B6E50 inlined: exit virtual `+1Ch`, clear `+4h` and `+5h`, then
commit through 004F83B0. That ordering is the reverse of the enter path, which commits first; both
orderings already appear as `kFrontEndExitCommitsAfterVirtual` and
`kFrontEndEnterCommitsBeforeVirtual` in `include/bsp/frontend_state_machine.hpp`, and this routine
obeys both.

### The tail, 004C42A6, common to both arms

When `game+624h != 0` (the session termination reason) and `00E198C4` and its `+A4h` sub-object both
exist, that overlay is forced visible (`+4h = +5h = 1`), committed, given its enter virtual `+18h`,
and then 005B6960 runs on it as a tail jump. At state 5 the manager is null, so this never fires.

### The delta, and why there are two

Only the 004C4165 pump call chooses between `game+21ECh` and `game+21F0h`. Every 004F71F0 call in
the body, on both arms, uses `game+21F0h` unconditionally. The choice exists because `game+634h`
forces `+21F0h` to 0.0f in cinematic mode (`docs/GAME_FRAME_CONTROL.md`), and `game+635h`, the
"still simulate" companion, redirects the pump to the clamped `+21ECh` so the interface keeps
animating. Reading `+635h` as exactly that companion is taken from the existing frame-control doc,
not re-derived here.

## The activate and deactivate walk

`00683AA0` (`__thiscall(this)`, virtual `+0Ch`) clears `this+3Ch` and then publishes an **empty**
level-4 screen set and an empty level-4 input-context set: `004F8710(0)` and
`004D8C00(*00E188A8, 0)`, both varargs with nothing but the terminator. It destroys nothing and
leaves both interface records intact.

`00684700` (`__thiscall(this)`, virtual `+8h`) walks the registry and calls virtual `+0Ch` on every
entry except `this`, sets `this+3Ch`, and then, unless `this` is the in-mission manager at
`00E198C4`, replays its own virtual `+10h` with the **pending** record `(this+20h, this+38h)`.

The two halves are load-bearing together. Because every deactivation empties the one shared level-4
vector, the walk would leave the screen stack empty if it stopped there; the replay at the end is
what refills it from the activating manager's own pending record. The replay is unconditional, so a
manager that is re-activated with `applied == pending` still republishes its screen set. The
in-mission manager is the single exemption, tested at 0068477E.

## The interface request lock, 00E19894

`00683E90` is `__stdcall(int interfaceId)`, `RET 4`, returning 1 when the request must be dropped:

| Arm | Condition | Result |
| --- | --- | --- |
| 00683E97 | the byte is clear | passes, nothing changes |
| 00683EA0 | id == 2Ch `INTF_MOVIECAMERANEW` | passes, the byte stays set |
| 00683EA5 | id >= 20h | **rejected** |
| 00683EA7 | id < 20h | passes **and clears the byte** |

`classify_interface_request_00683e90` returns those four arms;
`front_end_request_rejected_00683e90` in `include/bsp/frontend_managers.hpp` already returns the
reject bit alone and is unchanged.

The only site that **engages** the byte is 005CD1E9, which stores the 1 that 005CD1A4 put in EBX,
immediately after 005CD1CE pushes interface request 2Ch on the in-mission manager, and only when
`game+1FE4h != 0`. Every other writer clears it: 004DAB5D, 004D2C5F, 004BC493 and 00683EA7 itself.
So the lock means "a movie-camera interface owns the in-session UI", and it is released by the first
front-end request that arrives.

Its only caller, `004CC460`, short-circuits: 004CC478 tests the byte itself and calls 00683E90 only
when it is set, so the routine's own first arm is unreachable from the single call site.

## 005884A0

Already reconstructed by packet `main_menu_screens` (`start_title_music_005884a0`) and re-read here
only to confirm it. It starts the title music on the manager at 00E198AC, is gated on the intro
movie not playing and on `+50h` still being null, and its `this` is unused. `docs/MAIN_MENU_SCREENS.md`
has the instruction-level table. Nothing is added and nothing is corrected.

## Host methods the executable must implement, in call order

`update_without_simulation` on `RenderTailHost` (004E53B6 and 004E5469) is the method that must now
call `run_interface_only_update_004c40f0`. That routine needs `InterfaceOnlyHost`:

| # | Call site | Host method | Native routine |
| --- | --- | --- | --- |
| 1 | 004C4165 | `run_screen_pump_004f8830(seconds)` | 004F8830, the three-pass registry walk |
| 2 | 004C4194, 004C41DB, 004C4234, 004C4267, 004C429A | `screen_update_004f71f0(ref, seconds)` | 004F71F0 |
| 3 | 004C423B, 004C426E, 004C42A1 | `screen_close_004b6e50(ref)` | 004B6E50 |
| 4 | 004C41AE, 004C41F1 | `screen_exit_virtual(ref)` | screen vtable +1Ch |
| 5 | 004C41B8, 004C41FB, 004C42CF | `screen_commit_004f83b0(ref)` | 004F83B0 |
| 6 | 004C42DB | `screen_enter_virtual(ref)` | screen vtable +18h |
| 7 | 004C42EF | `mission_overlay_finalize_005b6960()` | 005B6960 |

Only method 1 is reached on a normal main-menu frame. Methods 3, 4, 6 and 7 are in-mission paths
that a front-end-only executable can implement as hard failures until a mission loads.

The frame around it, already on `RenderTailHost`, must additionally be wired so that:

- `service_menu_requests` (004E5442, 004E5477) runs `service_pending_menu_requests_006840f0` over
  the four channels 00E198AC, 00E198B4, 00E198B8, 00E198C4 and returns its out byte;
- `clear_request_latch` (004E544C, 004E5481) clears 00E18CDC;
- `tick_peer_session` (004E5462) runs 00776230;
- `MenuRequestServicer::service_menu_channel(0, 1, payload)` reaches the main-menu manager's
  virtual `+10h`, 00685820 `BSP_MainMenu_ApplyPendingInterface`, which calls the base 00684600 and
  then publishes the one-element level-4 screen set through 004F8710 and the input-context set
  through 004D8C00.

With those in place the sequence closes: the shell writes state 5, the service pass at 004E5442
publishes screen set `{1}`, and 004C40F0 at 004E5469 pumps it into view on the same frame and again
on every frame after.

## Corrections

- **This document's own first version credited the drain loop with making the menu visible**, saying
  step 2 services the shell's request and step 5 pumps the result. That mechanism is wrong and is
  replaced above; the conclusion that the menu is visible on the entry frame is unchanged. The
  superseded sentence was "The drain loop is why the main menu appears on the frame it is entered,
  not the frame after." It was caught by `agent/cc-exe-2c`, whose 120-frame run showed the service
  pass idle, and the cause is 00684600 syncing applied to pending inside both 00686380 and the
  00684700 replay before 004E5442 is reached. The id polarity at 006868AD, 4 when 00E198B0 is 1 and
  1 otherwise, was settled by the single `MOV EBX,0x1` at 00686622.
- **`006840F0` walks four managers, not two.** The opening sentence of its ledger record, from
  packet `game_on_move_map`, reads "For each of DAT_00e198ac and DAT_00e198b4". A later append from
  packet `game_session_polls` already names all four, and `include/bsp/session_polls.hpp` models
  four channels, so this was a stale first sentence rather than a live error. Re-reading the body
  confirms four identical blocks: 006840F3 (00E198AC), 00684130 (00E198B4), 0068416C (00E198B8) and
  006841A8 (00E198C4). The record now carries the instruction-level confirmation.
- **The 0068412D arm of 006840F0 is dead**, and that is new. It sets the out byte without calling
  the virtual, and is reached only when the applied and pending payloads are equal, which the branch
  at 00684114 has already excluded. The same dead arm repeats in all four blocks.
- **The registry at 00E19898 is a `std::set`, not a `std::map`.** The iterator increment 00683D10
  walks `_Right` then descends leftmost, and tests `+11h` as `_Isnil`; the node layout is the MSVC
  `_Tree` node `{_Left +0, _Parent +4, _Right +8, _Myval +0Ch, _Color +10h, _Isnil +11h}` with a
  four-byte `_Myval`. The comment in `include/bsp/frontend_managers.hpp` calls it a map keyed by the
  manager pointer. The payload offset it states, +0Ch, is right, and the walk order, by pointer
  address, is right; only the container name is wrong, and nothing depends on it.
- **The lock does not gate the shell's push.** `docs/MAIN_MENU_PATH.md`'s follow-up says of the
  `frontend_interface_lock` packet that "the lock decides whether step 9 is a no-op". It does not.
  Step 9 pushes interface id 1, which is below 20h, so it takes the 00683EA7 arm: it always passes,
  and it clears the lock on the way through. No lock state can make step 9 a no-op.
- **The `FUN_00425D10(uVar4)` call in the 004C40F0 pseudocode is an artefact.** At 004C4183 the
  float is pushed, then 00425D10 is called with no arguments to fetch the `this` pointer, then
  004F71F0 is called. The decompiler attributed the pushed float to the singleton getter.

## Uncertainties

- **005B6960 is not reconstructed.** It reads the screen's `+BCh` child, calls that child's vtable
  `+34h` with 1, then writes a zeroed three-float vector and the constant at 00D7A24C. It has one
  caller and is left as a host method; no name is proposed for it.
- **`game+624h` is read as the session termination reason** on the authority of
  `docs/GAME_SESSION_POLLS.md`, not re-derived. The tail block's meaning, forcing the slot-33h
  overlay visible at the end of a mission, follows from that reading and is provisional.
- **`00E19698` is not identified.** It is a front-end screen pointer that several unrelated
  routines clear the `+4h` byte of; this packet only establishes that 004C40F0 dereferences it
  without a null test.
- The system-UI arm and the tail were read to the instruction but never exercised at state 5, since
  both need `00E198C4`. They are reconstructed from the listing alone.

## Follow-up packets

- `in_game_interface_screen_sets` — the level-1 table inside 0068ACA0 with its jump-table arm at
  0068B390, 17 call sites over interface ids 25h..50h. Not started; it was the stretch item and the
  turn went to 004C40F0's body instead. Still needs 0068ACA0, 0068B390 and the registry names.
- `mission_overlay_finalize_005b6960` — 005B6960 and the `+BCh` child whose virtual `+34h` it
  drives. Small, self-contained, and the last unknown inside 004C40F0.
- `front_end_gamepad_prompt` — 00E19698: what allocates it, what its `+4h` byte means to the five
  routines that clear it, and why 004C40F0 may assume it non-null.
- `interface_lock_engage` — 005CD1A0, the only routine that sets 00E19894, and the movie-camera
  interface 2Ch it belongs to.

## `no_ghidra_function`

Every routine read for this packet has a Ghidra function, including 00683E90, 00683D10 and 005B6960,
which have ledger names or labels but no exported pseudocode. No new code ranges were found.

| Start | End (inclusive) | Note |
| --- | --- | --- |
| — | — | none |
