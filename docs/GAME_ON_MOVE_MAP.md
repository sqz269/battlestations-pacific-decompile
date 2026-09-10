# GGame::OnMove per-frame map (004e4a40)

Addresses: 004e4a40, 00419cc0, 0041e870, 00425d10, 00481640, 004b6260, 004b6e50, 004b9f40,
004bcaa0, 004bec00, 004bf830, 004bfe50, 004c0170, 004c11f0, 004c12b0, 004c1b90, 004c1dd0,
004c40a0, 004c40f0, 004c43c0, 004c6b20, 004c6c30, 004c6c70, 004c6e30, 004ca1f0, 004ca2f0,
004ca440, 004caa90, 004cce50, 004cd0f0, 004d1100, 004d11d0, 004d3ed0, 004d7ea0, 004d80d0,
004d8620, 004d8cd0, 004db030, 004db220, 004db290, 004db920, 004de4b0, 004e1ca0, 004e2200,
004e4430, 004f71f0, 004f7740, 004f8830, 0053c510, 0054e440, 006529e0, 006840f0, 00685170,
00685c80, 00689cc0, 0068a140, 0068c1f0, 0068d850, 0068ec10, 00692580, 006926f0, 00692960,
00692b00, 00692b60, 00692fd0, 006dc1a0, 00740e10, 00757ce0, 00776230, 00778560, 00865ab0,
00865cf0, 00867ee0, 008eb110, 00903670, 00914ef0, 00941140, 0094c8f0, 00987590, 00a919f0,
00a91e20, 00a92aa0, 00a92c40, 00aa0e00, 00aa0e50, 00aa4f80, 00af0450, 00af0c50, 00b0d7b0,
00b19a10, 00b1bf90, 00bbddd0, 00bd1510, 00be3640, 00be3660, 00bf6713

Packet: `game_on_move_map` (analysis only). Anchor `BSP_Game_OnMove` at 004e4a40, called once per
frame by `BSP_Application_RunFrame` (see `docs/APP_RUN_FRAME.md`). ABI `void __thiscall
BSP_Game_OnMove(GGame *this /*ECX*/, float rawDelta /*[esp+4], f32*/)`, `ret 4`.

Evidence base for this document: the complete decompiler export at
`exports/bsp/functions/004e4a40/decompiled.c` (391 lines), the raw disk listing read through
`python tools/bsp.py disasm-raw 004e4a40`, per-callee `lookup` (string immediates, caller counts,
segment keyword clusters) and short read-only Ghidra decompiles of fourteen callees. Ghidra's
stored body for the anchor is still eight bytes (recorded defect,
`reports/game_onmove_body_repair.json`); nothing here was written back to Ghidra.

## Frame object fields used as guards

| Field | Role as established here |
| --- | --- |
| `game+0x34` | Frame phase. 0 = render queue not opened this frame; set to 1 after the queue-open virtual call. `BSP_Game_TryBeginRenderFrame`/`BSP_Game_FinishRenderFrame` also gate on it (phase 2 = rendering). |
| `game+0x5D4` | Game state. Observed values: 1, 2, 4 (front-end/menu group), 0x0C, 0x0D (in mission), 0x11, and `< 10` as a coarse "not in game" test. |
| `game+0x5E0/+0x5E4/+0x5E8` | Pending state-request ring: capacity, head byte offset, pending count. |
| `game+0x5EC` | Re-entrancy flag for the state-request drain. |
| `game+0x21F0` | Scaled frame delta, the value every simulation callee receives. Distinct from the `rawDelta` argument. |
| `game+0x634` / `+0x635` | Cinematic/non-interactive flag and its "still simulate" companion. |
| `game+0x648` | Frame counter, incremented once per OnMove. |
| `game+0x7184` / `+0x7188` | Suspend flag for the simulation block, and the mission-result object checked by 004d7ea0. |
| `game+0x1EE0`, `+0x1EE7`, `+0x1EE1` | One-shot latches inside the in-game path. |
| `game+0x1FE4`, `+0x18CC[+0x18EC]` | Local player count and the active local-player slot, read by the timing helper and the pause gate. |
| `game+0x19E8`, `+0x19F0`, `+0x19FC`, `+0x21A0`, `+0x21D0`, `+0x21D4`, `+0x5B0/+0x5BC`, `+0x5FC` | Subsystem pointers passed as ECX to world/effect updates and the two effect lists. |

## Float delta flow

`rawDelta` arrives on the stack and stays at `[esp+0x48]` for the whole body. It is passed verbatim
to the input update (004e4a72), the modal-screen update (004e4b3b), the front-end updates, the
network tick (004e5036) and the metrics wrapper (004e551e). At 004e4d45 the address of the stack
slot is handed to 004c6e30, which may rewrite it; immediately after, `*(float*)(DAT_01090AB0+4) +=
*(float*)(game+0x21F0)` accumulates the **scaled** delta into a global clock. From that point the
simulation reads `game+0x21F0`, not the argument. Both values are live at once: 00778560 receives
the raw stack copy while the entity/effect updates receive `game+0x21F0`.

## Ordered call sequence

Phase names are mine; the addresses and order are from the listing.

1. **Prologue** 004e4a40-004e4a5e. Pushes the MSVC SEH frame (handler `LAB_00c67a54`), saves ECX as `this`.
2. **Pre-tick** 004e4a5e. `FUN_004e2200()` — string immediates `pause`, `command_line_mission`, `universe/scenes/`, `scenes`, `frames`; sole caller is OnMove.
3. **Input poll** 004e4a63-004e4a72. `004bec00()` returns the input singleton (cache `DAT_00f8bbf8`), `mov ecx,eax`, then `00a92c40(rawDelta)` walks 0x30-byte action records with float hold timers after forwarding the delta to the device backend `DAT_00f8bbf4` vtable+4.
4. **Window close** 004e4a79. `BSP_Game_ProcessWindowCloseRequest()`.
5. **One-shot device bind** 004e4a80-004e4b02, guarded by `DAT_00e188dc == 0 && DAT_00e188d8 != 0`. Three vtable+0x5C capability probes (5, 0x18, 0x0F), then vtable+0x18 produces a handle stored in `DAT_00e188dc` and pushed into a particle-segment object by `00b0d7b0` (writes `+0x1C0`, then `FUN_00b4ec90` if `+0x30` set).
6. **Cinematic GUI layers** 004e4b07-004e4b24, guarded by `game+0x634 != 0 && game+0x5D4 < 10`. `004cd0f0(0,0,1)` — string immediates `Sonar`, `Warnings`, `InGameGUI`, `3DEffect`.
7. **Blocking-screen fast path** 004e4b2a-004e4b8c, **first RET**. If `DAT_00e198bc != 0`: `00689cc0(rawDelta)`; if the screen is active (`+0x3C`) and `BSP_Game_TryBeginRenderFrame()` succeeds, run `BSP_GuiManager_GetOrCreate()` → `00aa4f80(rawDelta,0)` → `BSP_Game_Render()` → `BSP_Game_FinishRenderFrame()` and return. **The whole simulation is skipped while a loading or modal screen is up. This is the title-screen frame.**
8. **Session polls** 004e4b91-004e4b98. `004db290()` (front-end strings `FE_xbox.xsm_profilechanged`, `FE_xbox.xsm_storageremovedauto`, `globals.areyousure`) and `004caa90()` (strings `- - ONLINE - - Update Stats Write Status code %x and %x`).
9. **Front-end states** 004e4b9e-004e4cd0, entered when `game+0x5D4` is 1, 2 or 4. If `game+0x34 == 0`, open the render queue (`BSP_RenderCommandQueue_GetSingleton`, `BSP_RenderCommandQueue_GetRetentionControl`, `DAT_00f8d394` vtable+0xC) and set `game+0x34 = 1`. State 1 → `00685170()`. State 2 → lazy `004db220()` for `DAT_00e198c8`, `0068d850(rawDelta)`, then a GUI branch on `*(char*)(DAT_00f8abe8+1000)` selecting either `BSP_GuiManager_GetOrCreate(1)`/`00aa0e00(1)` plus a `00425d10()` menu-object branch into `004b6e50()` or `004f71f0()`, or the disabled path `00aa0e50()`/`00aa0e00(0)`. All three states then run `004f8830(rawDelta)`, `BSP_GuiManager_GetOrCreate()`, `00aa4f80(rawDelta,0)`, `BSP_Game_Render()`, `BSP_Game_FinishRenderFrame()`, and return unless `*(int*)(DAT_00e188a8+0x5E8) != 0` (state requests pending).
10. **Render-queue open, fallback** 004e4cd6-004e4d02. Same open sequence for every state that reached here.
11. **State-request drain** 004e4d07-004e4d0d. If `game+0x5EC == 0`, `004e4430()` consumes the ring at `game+0x5E0/+0x5E4/+0x5E8`. The producer is `BSP_Game_EnqueueStateRequest` at 004d3ed0 (leased to `agent/init-game-entry`). Then states 1, 2 and 4 return through three separate compares that share one epilogue.
12. **Timing** 004e4d3e-004e4d94. `004c6e30(&rawDelta)` (reads `game+0x1FE4` and the local-player slots at `game+0x18D0`), the global clock accumulate, `004c6b20(rawDelta)`, and for state 0x0D a `DAT_00f88c20` vtable+8 call with the scaled delta.
13. **Profiler label** 004e4d9a-004e4de1, one-shot behind bit 0 of `DAT_00e18b58`. `BSP_NativeString_Assign("GGame::OnMove")` into `_DAT_00e18b54`, then `BSP_SizedStoragePool_GetSingleton`/`BSP_SizedStoragePool_ReturnBlock` release the temporary buffer.
14. **Unconditional per-frame** 004e4de8-004e4def. `0053c510()` (GUI/text segment), `004c0170()`.
15. **In-game effect lists** 004e4df5-004e4fe5, guarded by `game+0x5D4 == 0x0D`. A one-shot at `game+0x1EE7` runs `004b6260()`/`004bcaa0(1)`. When `game+0x634 == 0`: `00987590(scaledDelta)` if `game+0x21E0` is set and the delta is positive; then two `std::list` walks. List `game+0x5B0` runs `004bec00()`/`00a91e20()`/`004b9f40()` per node. List `game+0x5BC` runs `004bf830()`, then either `00a92aa0(id, node+0x18)` with `node+0x20` latched (start) or `00a919f0(id)` (continue), and erases the node through 004d11d0 when its float timer at `node+0x14` reaches zero. Both lists go through the input singleton, so these read as timed controller/feedback effects rather than audio.
16. **Frame bookkeeping** 004e4feb-004e50a5. `004d8cd0()`; `game+0x648 += 1`; the network tick `00778560(rawDelta)` unless (state 0x0D and `DAT_00f876b0 >= 1` and the scaled delta is positive); state 0x0C → `004db920()`; state 0x11 → set/clear `game+0x5EC`, and when no requests are pending `BSP_Game_EnqueueStateRequest(4)` plus `DAT_00e198b0 = (game+0x1EE1 == 0)`.
17. **Profiled block opens** 004e509d-004e50ab. `BSP_Profiler_GetInstance()`, `BSP_Profiler_BeginCounter(DAT_0109db08)`.
18. **Simulation gate** 004e50b0-004e525e. Requires `game+0x634 == 0 || game+0x635 != 0`, then a second one-shot profiler label `"GGame::OnMove::game"` (bit 1 of `DAT_00e18b58`), then `game+0x5D4 == 0x0D && game+0x7184 == 0`. Inside: `004c40a0()`; `004cce50()` behind the `game+0x1EE0` latch; the pause gate reading `game+0x1FE4`, the active player slot, `BSP_InputAction_WasPressedThisFrame(0x4B)` and `(1)`, `004f7740() < 2`, `0068a140()`, `TRIV_body_006529e0()`, `004bfe50()` and three `00425d10()` field tests. The pause branch runs `0054e440()` and `004db030()` twice; the other branch runs `0068c1f0()` (strings `Underwater`, `Cockpit`) when `DAT_00e198c4+0x3C` is set, then `004c40f0()`.
19. **Award/stat trackers** 004e525e-004e52b5. Seven `004e1ca0()` getter calls, each followed by `mov ecx,eax` and one update: `0068ec10(scaledDelta)`, `00692b00()`, `00692b60()` (unit-class ids `BASICSHIP`, `KAMIKAZE`, `TORPEDOBOMBER`, …), `006926f0()` (weapon ids `ARTILLERY`, `TORPEDO_SHIP`, `AAFLAK`, …), `00692580()` (`ENGINE`, `PERISCOPE`, `WATER`), `00692fd0()` (`CAPTURE1STGET`, `LANDING1STGET`), `00692960()` (`SM1STGET`).
20. **World tick** 004e52ba-004e5389, all with the scaled delta and an explicit ECX from a game field: `00914ef0` (ECX `game+0x21A0`), `006dc1a0` (ECX `game+0x21D4`, GUI-highlight/marker segment), `00481640` (ECX `game+0x21D0`, entity segment). When `game+0x19E8 != 0`: `00865cf0()` getter → `00865ab0()`, then `00bbddd0(game+0x19FC, scaledDelta)` with ECX `game+0x19E8` (ocean segment). Then `00740e10(scaledDelta)` (contains `cDecalManager::Update`), `TRIV_body_0094c8f0(scaledDelta)`, `008eb110()` (power-up segment), `004d1100(scaledDelta, game+0x19FC)`, `00867ee0(scaledDelta, game+0x19FC)`, `00903670()` (bot/AI segment), `004d7ea0()`.
21. **Particle step** 004e538e-004e53ad. `f = DAT_00f876a4 * (double)_DAT_00ce47a0` computed on the x87 stack, then `004de4b0()` getter → `00b19a10(f)`.
22. **Non-simulating fallback** 004e53b4. Every path that failed the gate in step 18 runs `004c40f0()` instead.
23. **Post-simulation** 004e53bb-004e549d. For state 0x0D: `00af0450([[game+0x5FC]+0x1054])` with ECX `DAT_00f8c274` (terrain/visibility segment) and `00af0c50(game+0x19FC, camera)` where the camera value is `[[game+0x19F0]+0xA8]+0xC8` or `+0xD0` depending on `+0xCC`. Then `004c6c70()`; `00685c80(rawDelta)` when `DAT_00e198ac != 0`; `004d80d0()` (strings `GGame::MultiInterfaceUpdate() -> SCENE_TERM` / `SCENE_AUTO_TERM`); `006840f0(&pending)`; `DAT_00e18cdc = 0`; **the dropped drain loop, see below**; `004c1b90()` getter → `00941140()`; `004d8620()` (string `FE_main`).
24. **Render** 004e54a2-004e54f0. `BSP_Profiler_EndCounter(DAT_0109db08)`, `BSP_Profiler_BeginCounter(DAT_0109db14)`, `BSP_Game_Render()`, `BSP_Profiler_EndCounter(DAT_0109db14)`, `BSP_Game_FinishRenderFrame()`.
25. **Tail** 004e54f0-004e5535, **second RET**. A `DAT_0109cefc` vtable+4 call whose result is compared against 0x3200000 (50 MiB), then `METRICS_wrapper_00757ce0(rawDelta)` when `DAT_00e1aed4` is set, SEH unlink, `ret 4`.

Simulation versus render split: steps 12 and 15-22 are the simulation; steps 7, 9, 23 (the terrain
and listener half) and 24 are the render side. Steps 7 and 9 each contain their own complete
render, which is why a title-screen or menu frame never reaches the simulation at all.

## Callee table

Status key: `named` = a reviewed ledger name already exists; `untouched` = still `FUN_`, no ledger
record; `stub` = a synthetic classification (trivial body, throw site, CRT tail).

| Address | Current name | Status | Subsystem | What it does | Provisional name | Confidence |
| --- | --- | --- | --- | --- | --- | --- |
| 004e2200 | FUN_004e2200 | untouched | game state machine | Sole caller is OnMove; string immediates `pause`, `command_line_mission`, `universe/scenes/`, `scenes`, `frames`; 28 callees. First call of the frame. | BSP_Game_PreTickSceneRequests | low |
| 004bec00 | FUN_004bec00 | untouched | timing/input | Double-checked lazy singleton, cache `DAT_00f8bbf8`, 0x24 bytes, ctor 00a93da0; the ledger record for `BSP_InputAction_WasPressedThisFrame` already identifies this cache as the input singleton. 94 callers. | BSP_InputManager_GetSingleton | high |
| 00a92c40 | FUN_00a92c40 | untouched | timing/input | ECX = the 004bec00 singleton; forwards the delta to backend `DAT_00f8bbf4` vtable+4, then walks 0x30-byte action records with float hold timers at +0x1C/+0x24. | BSP_InputManager_Update | high |
| 004ca2f0 | BSP_Game_ProcessWindowCloseRequest | named | game state machine | Already reconstructed. | — | — |
| 00b0d7b0 | FUN_00b0d7b0 | untouched | rendering kick | `*(this+0x1C0) = handle; if (this+0x30) FUN_00b4ec90();`. Particle/effect segment. | BSP_ParticleSystem_SetDeviceHandle | low |
| 004cd0f0 | FUN_004cd0f0 | untouched | GUI/HUD | Called `(0,0,1)` when a cinematic is up outside gameplay states. String immediates `Sonar`, `Warnings`, `InGameGUI`, `3DEffect`. 20 callers. | BSP_Gui_SetLayerGroupVisible | low |
| 00689cc0 | FUN_00689cc0 | untouched | GUI/HUD | Sole caller OnMove, takes the raw delta, drives `DAT_00e198bc`; when that object's `+0x3C` is set the frame renders GUI only. Menu-interface segment. | BSP_Game_UpdateBlockingScreen | medium |
| 00aa4f80 | FUN_00aa4f80 | untouched | GUI/HUD | Always immediately follows `BSP_GuiManager_GetOrCreate()`; `(delta, 0)`. Runs on the blocking-screen path and the front-end path. | BSP_GuiManager_Update | medium |
| 004db290 | FUN_004db290 | untouched | network | Two callers. Strings `FE_xbox.xsm_joinotherplayer`, `FE_xbox.xsm_inviteotherplayer`, `FE_xbox.xsm_storageremovedauto`, `FE_xbox.xsm_profilechanged`, `FE.crossplatform_notsupported`. | BSP_Game_PollPlatformSessionEvents | medium |
| 004caa90 | FUN_004caa90 | untouched | network | Strings `- - ONLINE - - WriteStats kaput`, `- - ONLINE - - WriteStats userleft %d`, `- - ONLINE - - Update Stats Write Status code %x and %x`. | BSP_OnlineStats_UpdateWrite | high |
| 004c11f0 | BSP_RenderCommandQueue_GetSingleton | named | rendering kick | Already reconstructed. | — | — |
| 00b1bf90 | BSP_RenderCommandQueue_GetRetentionControl | named | rendering kick | Already reconstructed. | — | — |
| 00685170 | FUN_00685170 | untouched | mission/scenario | Sole caller OnMove, only under state 1. Menu-interface segment. | BSP_Game_UpdateState1Frontend | low |
| 004db220 | FUN_004db220 | untouched | mission/scenario | Sole caller OnMove; lazily populates `DAT_00e198c8` before the state-2 update. | BSP_Game_EnsureFrontendSession | low |
| 0068d850 | FUN_0068d850 | untouched | mission/scenario | Sole caller OnMove, state 2, raw delta. Calls 004f71f0. | BSP_Game_UpdateMainMenu | low |
| 004c12b0 | BSP_GuiManager_GetOrCreate | named | GUI/HUD | Already reconstructed. | — | — |
| 00aa0e00 | FUN_00aa0e00 | untouched | GUI/HUD | Eight callers, always after `BSP_GuiManager_GetOrCreate(0 or 1)`; takes a bool. | BSP_GuiManager_SetEnabled | low |
| 00aa0e50 | FUN_00aa0e50 | untouched | GUI/HUD | Four callers, paired with 00aa0e00 on the disabled branch. | BSP_GuiManager_ClearScreens | low |
| 00425d10 | FUN_00425d10 | untouched | mission/scenario | 57 callers, no-argument accessor returning an object whose bytes at +4, +5, +0x188, +0x218, +0x25C are polled as mode/flag fields. Scene/training segment. | BSP_SceneContext_GetSingleton | low |
| 004b6e50 | FUN_004b6e50 | untouched | mission/scenario | Five callers, state-2 branch partner of 004f71f0. | — | low |
| 004f71f0 | FUN_004f71f0 | untouched | mission/scenario | Three callers; receives the scaled delta on the state-2 branch. Ambient/environment segment. | — | low |
| 004f8830 | FUN_004f8830 | untouched | mission/scenario | Three callers; the shared tail of all three front-end states, raw delta. | BSP_Game_UpdateFrontendCommon | low |
| 004e4430 | FUN_004e4430 | untouched | game state machine | Sole caller OnMove; `while (game+0x5E8) { ... }` over the ring described by `game+0x5E0/+0x5E4`; the producer 004d3ed0 is already named `BSP_Game_EnqueueStateRequest`. | BSP_Game_DrainStateRequestQueue | high |
| 004c6e30 | FUN_004c6e30 | untouched | timing | Sole caller OnMove; receives `&rawDelta`; counts active local players from `game+0x1FE4` and the slot array at `game+0x18D0`. | BSP_Game_UpdateLocalPlayerTiming | medium |
| 004c6b20 | FUN_004c6b20 | untouched | timing | Sole caller OnMove; runs right after the global clock accumulate. | — | low |
| 0041e870 | BSP_NativeString_Assign | named | profiler | Already reconstructed; builds the profiler label strings. | — | — |
| 00419cc0 / 00bd1510 | BSP_SizedStoragePool_GetSingleton / ReturnBlock | named | profiler | Already reconstructed; release the label temporaries. | — | — |
| 0053c510 | FUN_0053c510 | untouched | GUI/HUD | Sole caller OnMove, unconditional. GUI/text segment. | — | low |
| 004c0170 | FUN_004c0170 | untouched | game state machine | Sole caller OnMove, unconditional. Segment keywords include `collectgarbage`, `interface`. | — | low |
| 004b6260 | FUN_004b6260 | untouched | units and AI | Returns a byte; feeds the `game+0x1EE7` one-shot. Unit-class segment. | — | low |
| 004bcaa0 | FUN_004bcaa0 | untouched | units and AI | Called `(1)` from the same one-shot when `game+0x624 == 0`. | — | low |
| 00987590 | FUN_00987590 | untouched | mission/scenario | Sole caller OnMove; scaled delta; guarded by `game+0x21E0` and a positive delta. Segment keywords `entitykilled`, `musicover`, `repair`, `surrender`. | BSP_MissionEvents_Update | low |
| 00a91e20, 004b9f40 | FUN_… | untouched | timing/input | Per-node work on the `game+0x5B0` list, always preceded by the input-singleton getter. | — | low |
| 004bf830, 00a92aa0, 00a919f0 | FUN_… | untouched | timing/input | The `game+0x5BC` list: node fetch, start-with-parameters, continue-by-id; node float timer at +0x14 drives erasure. | — | low |
| 004d11d0 | STL_xlen_throw_004d11d0 | stub | — | Called with `(auStack_14, listHead, node)`; the classification as an `xlen` throw site is inconsistent with that argument shape. See flow breaks. | — | — |
| 004d8cd0 | FUN_004d8cd0 | untouched | game state machine | Sole caller OnMove, immediately before the frame counter increment. | — | low |
| 00778560 | FUN_00778560 | untouched | network | Sole caller OnMove; raw delta; runs unless the in-game simulation will advance. Peer/multiplayer segment. | BSP_Multiplayer_Tick | medium |
| 004db920 | FUN_004db920 | untouched | game state machine | Sole caller OnMove, only under state 0x0C. | — | low |
| 004d3ed0 | BSP_Game_EnqueueStateRequest | named | game state machine | Leased to `agent/init-game-entry`; called here with request id 4 under state 0x11. | — | — |
| 004c1dd0 / 00be3640 / 00be3660 | BSP_Profiler_* | named | profiler | Already reconstructed. Counters `DAT_0109db08` (game block) and `DAT_0109db14` (render). | — | — |
| 004c40a0 | FUN_004c40a0 | untouched | game state machine | Sole caller OnMove; first call inside the in-game block. | — | low |
| 004cce50 | FUN_004cce50 | untouched | game state machine | Sole caller OnMove; behind the `game+0x1EE0` one-shot latch. | — | low |
| 004c43c0 | BSP_InputAction_WasPressedThisFrame | named | timing/input | Already reconstructed; queried with action ids 0x4B and 1 in the pause gate. | — | — |
| 004f7740 | FUN_004f7740 | untouched | units and AI | Sole caller OnMove; result compared `< 2`, reads like a participant count. | — | low |
| 0068a140 | FUN_0068a140 | untouched | GUI/HUD | Four callers; boolean, part of the pause gate. | — | low |
| 006529e0 | TRIV_body_006529e0 | stub | GUI/HUD | Trivial body; boolean in the pause gate, guarded by `DAT_00e188a8+0x19C4`. | — | — |
| 004bfe50 | FUN_004bfe50 | untouched | game state machine | Sole caller OnMove; boolean, final term of the pause gate. | — | low |
| 0054e440 | FUN_0054e440 | untouched | GUI/HUD | Three callers; strings `BASICSHIP`, `BASICPLANE`, `BASICSUB`, `BASICSHIP2`, `BASICPLANE2`, `BASICPLANE3`; front-end button segment. Runs when the pause menu opens. | BSP_PauseMenu_BuildUnitList | low |
| 004db030 | FUN_004db030 | untouched | GUI/HUD | Four callers; called once or twice on the pause branch. | BSP_Game_OpenPauseMenu | low |
| 0068c1f0 | FUN_0068c1f0 | untouched | sound | Sole caller OnMove; strings `Underwater`, `Cockpit`; guarded by `DAT_00e198c4+0x3C`. | BSP_Audio_UpdateEnvironment | medium |
| 004c40f0 | FUN_004c40f0 | untouched | GUI/HUD | Sole caller OnMove, reached from three places: the non-pause in-game branch, the fallback when the simulation gate fails, and the dropped drain loop. Compares `DAT_00e188ae` against `DAT_00e18b34` and clears `game+0x719E`. | BSP_Game_UpdateInterfaceOnly | medium |
| 004e1ca0 | FUN_004e1ca0 | untouched | script callbacks | Double-checked lazy singleton, cache `DAT_00e198d0`, 0xA0 bytes, ctor 004e18e0. 40 callers. Its result is the ECX for all seven tracker updates below. | BSP_AwardTracker_GetSingleton | medium |
| 0068ec10 | FUN_0068ec10 | untouched | script callbacks | Tracker update taking the raw delta. | — | low |
| 00692b00 | FUN_00692b00 | untouched | script callbacks | Tracker update; branches on 004bca50 and 005b5d50 results. | — | low |
| 00692b60 | FUN_00692b60 | untouched | script callbacks | 18 unit-class string ids (`BASICSHIP`, `KAMIKAZE`, `RECONPLANE`, `TORPEDOBOMBER`, `OHKA_PAYLOAD`, …). | — | low |
| 006926f0 | FUN_006926f0 | untouched | script callbacks | Nine weapon ids (`ARTILLERY`, `AAFLAK`, `TORPEDO_SHIP`, `DC_PLANE`, …). | — | low |
| 00692580 | FUN_00692580 | untouched | script callbacks | `ENGINE`, `PERISCOPE`, `WATER`. | — | low |
| 00692fd0 | FUN_00692fd0 | untouched | script callbacks | `CAPTURE1STGET`, `LANDING1STGET`. | — | low |
| 00692960 | FUN_00692960 | untouched | script callbacks | `SM1STGET`. | — | low |
| 00914ef0 | FUN_00914ef0 | untouched | units and AI | ECX `game+0x21A0`, scaled delta. | — | low |
| 006dc1a0 | FUN_006dc1a0 | untouched | GUI/HUD | ECX `game+0x21D4`, scaled delta. Segment keywords `guihighlights`, `entitymarkers`, `positionmarkers`. | BSP_MarkerManager_Update | medium |
| 00481640 | FUN_00481640 | untouched | units and AI | ECX `game+0x21D0`, scaled delta. Segment keywords `entity`, `soldiertypes`, `landvehicleclasses`, `maxentity`. | BSP_EntityManager_Update | medium |
| 00865cf0 | FUN_00865cf0 | untouched | physics | Getter whose result becomes the ECX for 00865ab0. Effects segment. | — | low |
| 00865ab0 | FUN_00865ab0 | untouched | physics | Sole caller OnMove; runs only when `game+0x19E8` is set. | — | low |
| 00bbddd0 | FUN_00bbddd0 | untouched | physics | ECX `game+0x19E8`, args `game+0x19FC` and the scaled delta. Segment keywords `oceanheightmap`, `shorewavetexturesource0..2`. | BSP_Ocean_Update | medium |
| 00740e10 | FUN_00740e10 | untouched | rendering kick | Sole caller OnMove; contains the literal `cDecalManager::Update`; scaled delta. | BSP_DecalManager_Update | high |
| 0094c8f0 | TRIV_body_0094c8f0 | stub | — | Trivial body called with the scaled delta. | — | — |
| 008eb110 | FUN_008eb110 | untouched | units and AI | Sole caller OnMove, no arguments. Segment keywords `pup_gain`, `pum1stget`, `unitclassindex`. | — | low |
| 004d1100 | FUN_004d1100 | untouched | mission/scenario | 11 callers; `(scaledDelta, game+0x19FC)`. | — | low |
| 00867ee0 | FUN_00867ee0 | untouched | physics | Two callers; same argument pair as 004d1100. Effects segment. | — | low |
| 00903670 | FUN_00903670 | untouched | units and AI | Three callers. Segment keywords `pilotbot`, `tailgunnerbot`, `torpedobot`, `thinktimeleft`. | BSP_BotManager_Update | medium |
| 004d7ea0 | FUN_004d7ea0 | untouched | mission/scenario | Sole caller OnMove; guarded by `game+0x5E8 == 0 && game+0x7188 != 0`; calls `BSP_Game_EnqueueStateRequest(0x0F)` when `[game+0x7188]+0x21` is set. | BSP_Game_CheckMissionCompletion | high |
| 004de4b0 | FUN_004de4b0 | untouched | rendering kick | Double-checked lazy singleton, cache `DAT_00f8d420`, 0x1C bytes; its result is the ECX for 00b19a10. | — | medium |
| 00b19a10 | FUN_00b19a10 | untouched | rendering kick | Receives `DAT_00f876a4 * (double)_DAT_00ce47a0`. Particle/emitter segment. | BSP_ParticleSystem_Update | medium |
| 00af0450 | FUN_00af0450 | untouched | camera | ECX `DAT_00f8c274`, arg `[[game+0x5FC]+0x1054]`. Terrain/visibility segment. | — | low |
| 00af0c50 | FUN_00af0c50 | untouched | camera | Two callers; `(game+0x19FC, cameraValue)` selected from the camera object at `[game+0x19F0]+0xA8`. | — | low |
| 004c6c70 | FUN_004c6c70 | untouched | GUI/HUD | Sole caller OnMove; folds the active state of `DAT_00e198ac` and `DAT_00e198b4` (both `+0x3C`) into bit flags. | — | low |
| 00685c80 | FUN_00685c80 | untouched | GUI/HUD | Sole caller OnMove; raw delta; guarded by `DAT_00e198ac`. | — | low |
| 004d80d0 | FUN_004d80d0 | untouched | network | Sole caller OnMove; strings `GGame::MultiInterfaceUpdate() -> SCENE_TERM`, `SCENE_AUTO_TERM`, `FE.multi_terminated`, `ingame.multi_gameclosed_xbox`. | BSP_Game_UpdateMultiplayerInterface | high |
| 006840f0 | FUN_006840f0 | untouched | GUI/HUD | Sole caller OnMove; `__fastcall(bool *out)`; for each of `DAT_00e198ac` and `DAT_00e198b4` compares two index pairs (+1 vs +8, +7 vs +0xE), calls vtable+0x10 to service the difference and sets `*out = 1`. | BSP_MenuInterface_ServicePendingRequests | high |
| 00776230 | FUN_00776230 | untouched | network | Two callers; reached only from the loop Ghidra dropped, with ECX `game+0x1EF0`. Peer/multiplayer segment. | — | low |
| 004c1b90 | FUN_004c1b90 | untouched | GUI/HUD | Double-checked lazy singleton, cache `DAT_00f89b34`, 0x28 bytes; its result is the ECX for 00941140. 12 callers. | — | low |
| 00941140 | FUN_00941140 | untouched | GUI/HUD | Sole caller OnMove. | — | low |
| 004d8620 | FUN_004d8620 | untouched | GUI/HUD | Sole caller OnMove; string `FE_main`; the last update before the render block. | BSP_Frontend_UpdateRoot | medium |
| 004c6c30 / 004ca440 / 004ca1f0 | BSP_Game_TryBeginRenderFrame / Render / FinishRenderFrame | named | rendering kick | Already reconstructed. | — | — |
| 00757ce0 | METRICS_wrapper_00757ce0 | stub | profiler | Called with the raw delta when `DAT_00e1aed4` is set. | — | — |
| 00bf6713 | LIBCRT_unmatched_00bf6713 | stub | — | List-invariant failure paths inside the two effect-list walks. | — | — |

Counts: 95 indexed callees. 19 already carry reviewed names or synthetic classifications; 76 were
untouched `FUN_` entries when this packet started, of which 12 are named below.

## Flow breaks and gaps

1. **The anchor's stored Ghidra body is still eight bytes.** The decompiler traverses the whole
   routine and the export is complete, but `show --asm`, `ghidra disasm` and any body-length query
   see only 004e4a40-004e4a47. Recorded in `reports/game_onmove_body_repair.json`; not touched here.
2. **A live loop is dropped from the pseudocode.** The export opens with
   `WARNING: Removing unreachable block (ram,0x004e5455)` and `(ram,0x004e5460)`. The listing shows
   a real do/while at 004e5455-004e5488: `006840f0` writes a bool into `[esp+0x1B]`, `004e5453`
   branches on it, and the body runs `00776230` with ECX `game+0x1EF0`, then `004c40f0`, then clears
   the flag and re-polls, looping while the flag stays set. Ghidra cannot see that `006840f0` writes
   through its pointer argument, so it proves the branch always taken. `00776230` therefore appears
   in the call graph but in no line of the pseudocode.
3. **The listing's real extent is 2808 bytes, not the 4352 that were dumped.** The body runs
   004e4a40-004e5535 and the `int3` padding starts at 004e5538; everything past that belongs to
   other functions. The two RETs are `ret 4` at 004e4b8c (blocking-screen fast path) and `ret 4` at
   004e5535 (everything else). The six `return` statements in the pseudocode share those two.
4. **004d11d0 is classified as an STL length-error throw site but is called with three arguments**
   (`auStack_14`, the list head, the node) from the erase point of the `game+0x5BC` walk, where a
   node whose float timer has expired is removed. The classification and the call shape disagree;
   the tag looks wrong.
5. **Ghidra loses the `this` register on every getter-then-method pair.** Twenty-odd call sites in
   this function are `call getter; mov ecx,eax; call method`, which the pseudocode renders as two
   unrelated zero-argument calls. Every ECX noted in the ordered sequence above came from the
   listing, not the pseudocode.
6. **Argument counts in the pseudocode are unreliable** for the same reason: `BSP_GuiManager_GetOrCreate(1)`,
   `FUN_00425d10(uVar6)` and `FUN_004f71f0(uVar6)` show arguments that the listing does not push.
7. **The tail comparison at 004e54FD is unexplained.** A `DAT_0109cefc` vtable+4 call is compared
   against 0x3200000 and the only effect is a store of -1 into the SEH try-level slot at `[esp+0x40]`.
   The guarded body appears to have been optimised away; do not read this as a working memory check.
8. **Two float constants are read as doubles into x87 and truncated**: `DAT_00f876a4 * qword
   [0xce47a0]` at 004e538E, and the `5.60519e-45` literal in the pseudocode at the state-0x11 branch
   is the integer 4 reinterpreted, i.e. `BSP_Game_EnqueueStateRequest(4)`.
9. **The index's call graph for this function is nonetheless complete.** All 95 indexed callees were
   confirmed against the listing, and the listing contains no call target that the index lacks. The
   pseudocode is the lossy view here, not the index.

## Proposed follow-up packets

Ordered so that a runnable title-screen frame closes first. Each entry lists the addresses, the
files it would own, its contract and its dependencies.

1. **`game_frame_control`** — 004e2200, 004c6e30, 004c6b20, 004e4430, 004c0170, 0053c510, 004d7ea0,
   004db920. Files `docs/GAME_FRAME_CONTROL.md`, `src/game/game_frame_control.{h,cpp}`,
   `reports/game_frame_control.json`. Contract: produce the scaled delta at `game+0x21F0` and the
   global clock at `DAT_01090AB0+4` from the raw argument, and drain the state-request ring at
   `game+0x5E0..+0x5EC` into the next frame's `game+0x5D4`. Depends on: `app_init_game_entry` (owns
   the producer 004d3ed0).
2. **`game_frame_input_tick`** — 004bec00, 00a92c40, 00a91e20, 00a919f0, 00a92aa0, 004b9f40,
   004bf830. Files `docs/GAME_INPUT_TICK.md`, `src/game/input_tick.{h,cpp}`,
   `reports/game_input_tick.json`. Contract: the input singleton's per-frame poll and the two timed
   effect lists at `game+0x5B0` and `game+0x5BC`, including the node-erase path. Depends on: none.
3. **`game_blocking_screen_frame`** — 00689cc0, 004db220, 00aa4f80, 00aa0e00, 00aa0e50. Files
   `docs/GAME_BLOCKING_SCREEN.md`, `src/game/blocking_screen.{h,cpp}`,
   `reports/game_blocking_screen.json`. Contract: the loading/modal fast path that renders GUI only
   and returns at 004e4b8c, plus the GUI enable/disable pair. Depends on: 1, 2. **This is the
   smallest packet that yields a frame the game can actually run.**
4. **`game_frontend_states`** — 00685170, 0068d850, 004f8830, 004f71f0, 004b6e50, 00425d10,
   0054e440. Files `docs/GAME_FRONTEND_STATES.md`, `src/game/frontend_states.{h,cpp}`,
   `reports/game_frontend_states.json`. Contract: states 1, 2 and 4 including the
   `DAT_00f8abe8+1000` GUI branch and the shared render tail. Depends on: 1, 3.
5. **`game_session_polls`** — 004db290, 004caa90, 004d80d0, 006840f0, 00776230, 004c40f0, 00778560.
   Files `docs/GAME_SESSION_POLLS.md`, `src/game/session_polls.{h,cpp}`,
   `reports/game_session_polls.json`. Contract: per-frame platform notification, online-stats and
   multiplayer-session polling, plus the menu-interface request drain that Ghidra dropped at
   004e5455. Depends on: 1. Must reconstruct the dropped loop from the listing, not the export.
6. **`game_simulation_gate`** — 004c40a0, 004cce50, 004f7740, 0068a140, 004bfe50, 004db030,
   0068c1f0, 004cd0f0. Files `docs/GAME_SIMULATION_GATE.md`, `src/game/simulation_gate.{h,cpp}`,
   `reports/game_simulation_gate.json`. Contract: the state-0x0D entry condition, the pause-key
   gate and its two branches (pause menu versus audio-environment switch). Depends on: 1, 2.
7. **`game_award_trackers`** — 004e1ca0, 0068ec10, 00692b00, 00692b60, 006926f0, 00692580,
   00692fd0, 00692960. Files `docs/GAME_AWARD_TRACKERS.md`, `src/game/award_trackers.{h,cpp}`,
   `reports/game_award_trackers.json`. Contract: the 0xA0-byte tracker singleton and its seven
   update passes with their unit-class, weapon and `*1STGET` string tables. Depends on: 6.
8. **`game_world_entities`** — 00481640, 00914ef0, 006dc1a0, 00740e10, 00903670, 00987590, 008eb110,
   004d8cd0. Files `docs/GAME_WORLD_ENTITIES.md`, `src/game/world_entities.{h,cpp}`,
   `reports/game_world_entities.json`. Contract: the entity, marker, decal, bot and mission-event
   updates driven from `game+0x21F0` with ECX taken from `game+0x21A0/+0x21D0/+0x21D4`. Depends on: 6.
9. **`game_world_ocean_effects`** — 00865cf0, 00865ab0, 00bbddd0, 00867ee0, 004d1100, 004b6260,
   004bcaa0. Files `docs/GAME_WORLD_OCEAN.md`, `src/game/world_ocean.{h,cpp}`,
   `reports/game_world_ocean.json`. Contract: the `game+0x19E8` ocean and `game+0x19FC` effect
   updates plus the `game+0x1EE7` one-shot that arms them. Depends on: 8.
10. **`game_frame_render_tail`** — 004de4b0, 00b19a10, 00af0450, 00af0c50, 004c1b90, 00941140,
    004d8620, 004c6c70. Files `docs/GAME_RENDER_TAIL.md`, `src/game/render_tail.{h,cpp}`,
    `reports/game_render_tail.json`. Contract: everything between the last simulation call and
    `BSP_Game_Render`: particle step, terrain and listener binding, and the front-end root update.
    Depends on: 8, 9; overlaps the render-queue work already owned by `main:frame_bounds_integration`,
    so the render-queue globals must stay read-only in this packet.

Not packetized: 00b0d7b0 (belongs with the particle-system work already leased elsewhere) and
00685c80 (needs `DAT_00e198ac` identified first, which packet 5 will settle).

## Corrections from follow-up packets

- docs/GAME_INPUT_TICK.md: the two containers at `game+0x5B0` and `game+0x5BC` are MSVC red-black trees (`std::_Tree` sets, `{iterator list, _Myhead, _Mysize}` at +0/+4/+8, so the sizes sit at `game+0x5B8` and `game+0x5C4`), not lists. `004b9f40` and `004bf830` are `_Tree::iterator::operator++`, and `004d11d0` is `_Tree::erase(iterator)` (hidden return-iterator pointer plus one checked iterator by value; its `invalid map/set<T> iterator` immediate is the MSVC debug message), not a length-error throw.
- docs/GAME_FRAME_CONTROL.md: the request queue at `game+0x5D8` is an MSVC `std::deque<int>` (four elements per 16-byte block): `game+0x5E0` is the block count and `game+0x5E4` the element offset, not a capacity and a byte offset, and the producer `004d3ed0` takes a `const int&`, so call sites push a stack slot's address. `game+0x64C` accumulates the undilated step while the frame clock at `01090ab0+4` accumulates the dilated one. `0053c510` in step 14 runs with `ECX` = the value of `game+0x19C8` and is a frame-time statistics accumulator, not GUI/text code; `004c0170` throttles the presence-context refresh to three seconds off the frame clock.
- docs/GAME_BLOCKING_SCREEN.md: `DAT_00e198bc` is not a loading or modal gate. Its constructor `00689d90` loads the GUI layout `FE_attract` and its activate virtual `00689e80` plays `movies/PacificTheme.bik`: it is the front-end attract screen, and `00689cc0` is a 45-second idle countdown (any button takes it down) rather than a loading-screen update. `00aa0e50` takes a visibility argument at all four call sites (the decompiler's zero-argument signature is an artefact), and `00aa4f80` iterates a snapshot copy of the screen vector. `00689c00` and `00689bd0` had no Ghidra function and were read from the raw listing.
- docs/GAME_FRONTEND_STATES.md: state 4 is the main-menu shell, reached only through the request drain's case 4 (`004e4000` destroys the title singleton and loads the front-end resources); it gets no per-state update, only the shared screen pump `004f8830` and render tail. The pushes around the GUI calls belong to `00aa0e50` and `00aa0e00`, not to the GUI-manager accessor, which is called with no push at `004e4ca8`. `0054e440` is not a front-end button routine: its only call site in the game update is the in-mission pause branch, where it advances the basic-training hint one step.
- docs/GAME_SESSION_POLLS.md: the dropped do/while at `004e5455` ends on `006840f0`'s out byte, which is set only when a menu channel actually needed servicing, so the loop runs the menu state machine to a fixed point with no iteration bound. `00776230`'s pseudocode `return` after the first queue node is wrong: the listing at `007762ff` jumps back to the loop head, so each locked queue is drained fully. The invite block at manager+32h is an `XINVITE_INFO` (the `REP MOVSD` at `00a4034c` copies 15h dwords), which pins `00a3e470` and `00a3e4b0` as the invitee- and inviter-slot searches.
- docs/GAME_SIMULATION_GATE.md: the 4Bh action chain is a suppression of pause, not a second route into it: every failure inside it and a complete pass both land on the action-1 test the not-pressed case reaches, so its only effect is the early exit at `004e51db`. `004cd0f0` is the sole writer of both cinematic flags and sets `game+635h = game+634h AND allow_simulation`; the pause toggle passes 0, so pausing closes the gate and there is no separate paused flag. The interface-id name table at `00e08cd8` decodes the ids this block reaches (`0068a140` tests INTF_SHIPYARD / INTF_AIRBASE; `004cce50` starts the `<mission>.ema` in-engine movie and switches to INTF_ENGINEMOVIE). The ECX of `00447b80` at `004c40dd` is `game+30h`, not the game object.
- docs/GAME_AWARD_TRACKERS.md: the 0A0h singleton behind `004e1ca0` is the in-game hint system, not an achievement store (`00690fd0` resolves every pushed identifier through the definition map at +30h, the localisation table and the HintSystem Lua binding; the reviewed `BSP_AwardTracker_` prefix was kept). The seven passes are not unconditional: every failing test in the simulation gate at `004e50b0` jumps to `004e53b4` and skips them. The cooldown boundary is `COMISS 0.0` with `JBE`, so a timer at exactly 0.0f survives. `00692fd0` breaks the whole zone walk when a zone passes the landing radius but fails the capture radius; reproduced as written.
- docs/GAME_WORLD_ENTITIES.md: the map records no ECX for four world calls: they are `DAT_00e1aea0` (`00740e10`), `DAT_00f88c30` (`008eb110`), `DAT_00f89b3c` (`0094c8f0`) and `game+0x19CC` (`00903670`). `008eb110` is the power-up manager (it builds the literal `pup_ready`), not a units-and-AI routine. `00903670` runs on the world object at `game+0x19CC`, the same object whose `+0x4AC` byte gates `00481640`, so the proposed `BSP_BotManager_Update` is superseded by `BSP_EntityWorld_FlushActivations`. The decal manager's per-decal loop body is empty in the shipped image and its delta argument is never read.
- docs/GAME_WORLD_OCEAN.md: `004d1100` takes no arguments (a locked lazy singleton getter caching into `00f8765c`); the two pushes at `004e536b`/`004e536c` belong to `00867ee0`, which consumes them with `RET 8`. `004b6260` runs on `game+0x1EF0`, the network session subobject, not the game object. The one-shot at `game+0x1EE7` arms nothing ocean-related: `004b6260` tests the session and `004bcaa0` bumps two mission counters at `game+0x740`/`+0x73C` through `game+0x650`. The ocean and effect updates are gated only by `game+0x19E8 != 0` and the simulation gate; `game+0x19E8` is an owner whose `+0x3C` is the ocean object, and `game+0x19FC` is the scene transform both consumers read, not an effect list.
- docs/GAME_RENDER_TAIL.md: the particle argument is not a delta: `00ce47a0` holds the double 1000.0 and `00f876a4` is the global time float, so `00b19a10` receives the global time in milliseconds (component y of shader constant c33 `Time`; `00af0450` feeds component z). `004c1b90` / `00941140` is a sound cue request queue (six priority slots), not a listener binding. The render-block gate is dead code: `[esp+1Bh]` is written zero at `004e548a` on both incoming paths, so the `jne` at `004e54ba` never fires and `BSP_Game_Render` always runs. `00af0c50` is the camera-facing foliage impostor build. `004d8620` is named `BSP_Frontend_UpdateActiveScreens`, not the suggested `BSP_Frontend_UpdateRoot`.
- docs/GAME_FRONTEND_ENTRY.md: the main-menu shell rests at game state 5, not 4; 4 is only the drain request that enters `BSP_Game_EnterFrontEndShell`, which writes 5 at `004e4279`. The `GILoading::SLM_LOAD_*` strings are VFS file-block names, not a state field, and the load choice has three arms (a set `game+719Ch` with both front-end managers alive skips the block, the config publish and the loading screen). `00a7a460` samples FMOD memory statistics and is renamed `BSP_SoundSystem_SampleFmodMemoryStats`.
- docs/GAME_RENDER_FRAME.md: the begin/finish pair `004c6c30` / `004ca1f0` is D3D9 (`BeginScene` through renderer vtable `00d5f0a8` slot +0Ch = `00b2b200`, device vtable +A4h; slots +10h and +14h both reach `00b2d8e0`), not a lost-device test. The +10h argument is a screenshot path: `004c9a00` formats `%s%04d/scr%04d.png` and the selector `00e188ad` is screenshot capture. The frame job pool `00be2fa0` drains last in first out (it indexes the slot array with the value `InterlockedDecrement` returned), so the camera job runs before the world-view job. The scene branch dispatches on `00e08310`, the interface level `004f7620` latches.
