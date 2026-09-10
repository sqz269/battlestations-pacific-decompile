# Front-end shell entry and the loading screen (packet `game_frontend_shell_entry`)

Addresses: 004e4000, 0057bec0, 0057c250, 0057cb60, 0057cff0, 0057d0d0, 0057c1c0, 0057bff0,
00506c80, 007fbe20, 00a410a0

Scope: the whole body of `BSP_Game_EnterFrontEndShell` (004e4000, listing 004e4000..004e4421) and
the loading screen it raises. `docs/GAME_FRONTEND_STATES.md` read this function only as far as
004e4130 and left "the body past its loading-label setup" open; that is what this packet closes.
`004e4a40` (`BSP_Game_OnMove`) and `004e3aa0` (`GGame::OnInit`) were read only.

## The headline: game state 4 never survives this call

`BSP_Game_DrainStateRequestQueue` stores 4 into `game+5D4h` and dispatches to 004e4000. Inside,
`GGame::OnInit` writes `game+5D4h = 3` as its very first instruction, and the shell writes
`game+5D4h = 5` at 004e4279. So 4 is a *request* value only, never a state any frame observes.

```
004e3ab8: MOV EBX,0x3            (in GGame::OnInit)
004e3ac2: MOV dword ptr [ESI + 0x5d4],EBX     ; unconditional, first store
...
004e4087: MOV EBX,0x3            (in BSP_Game_EnterFrontEndShell)
004e4151: CMP dword ptr [EBP + 0x5d4],EBX     ; compares against 3, not 4
004e41b6: MOV EBX,0x5
004e4279: MOV dword ptr [EBP + 0x5d4],EBX     ; the state the next frame sees
```

The decompiler renders 004e4151 as `!= 3` correctly, but the two `MOV EBX` loads are easy to miss
in the pseudocode, so the listing is the evidence here. The gate at 004e4151 therefore fails only
when `BSP_Game_PollPlatformSessionEvents` (004db290) moved the state off 3 between the two points.
That is the platform abort path: sign-out, invite or storage change. On it the shell calls only
`BSP_LoadingScreen_End` and returns, leaving the managers uncreated.

Two consequences for existing docs. `docs/GAME_FRAME_CONTROL.md` lists drain request 5 as "no
call", so 5 is a resting state, not a request the drain would redispatch: the shell is running.
And `is_front_end_game_state` in `include/bsp/app_frame.hpp` covers 1, 2 and 4; 4 can only be seen
by code reached *from inside* 004e4000, which is exactly what `BSP_Game_ProcessWindowCloseRequest`
and `BSP_Game_PollPlatformSessionEvents` are. Their state-4 arms are reachable, but from the
per-frame path they never fire, because no frame boundary falls while the field holds 4.

## `BSP_Game_EnterFrontEndShell` (004e4000)

`__thiscall`, ECX = the game object, no stack arguments, RET 0, no return value; the drain at
004e44fa ignores it. SEH frame with handler 00c67a22 and the EH state slot at `esp+6Ch`.
`esp+18h` holds the saved `this` (written at 004e4040), `esp+10h` is the raw-allocation scratch.

Ordered steps:

| Listing | Step |
| --- | --- |
| 004e4022 | `*(00F8D394)+18h = 20000000h` |
| 004e4030 | renderer vtable `+70h`("Textures before mainmenu", 00CE82C8) |
| 004e4046 | 00a7a460("Sounds before mainmenu", 00CE82B0) |
| 004e405b | 004c1650() then 0086b0b0("Effects before mainmenu", 00CE8298) |
| 004e4067 | if `00E198C8`: title vtable `+0h`(1), then null the global |
| 004e407f | choose the load block from `game+719Ch` (below) |
| 004e40b0 | build the label NativeString, `BSP_FileBlock_Construct(&block, &label, 1)` |
| 004e40f2 | `BSP_LoadingScreen_BuildDefaultConfig(&cfg)` then `BSP_LoadingScreen_PublishConfig(cfg)` |
| 004e410f | 004d2a80(&cfg), the config destructor |
| 004e4116 | `BSP_LoadingScreen_Begin(0)` — ECX zeroed at 004e4114 |
| 004e4126 | `BSP_FileBlock_Destroy(&block)` |
| 004e4130 | `BSP_LoadingScreen_ReportProgress(0.1f)`, the float at 00D7A2F0 |
| 004e4145 | `GGame::OnInit(*(00E188A8))` — writes `game+5D4h = 3` |
| 004e414c | `BSP_Game_PollPlatformSessionEvents(this)` |
| 004e4151 | **abort gate**: `game+5D4h != 3` leaves through `BSP_LoadingScreen_End` |
| 004e4171 | create `00E198B8` (4Ch, 00689800), then its vtable `+4h` |
| 004e41b0 | create `00E198AC` (78h, 00686170), then its vtable `+4h` |
| 004e41f0 | create `00E198B4` (68h, 006887E0), then its vtable `+4h` |
| 004e422f | if `*(game+1A08h)+4h`: `006b8ad0("collectgarbage(\"collect\")", 0, 0, 2)` |
| 004e424a | if `*(00E198AC)+4h != 4`: `004cc460(1, 0)` |
| 004e425e | `00E198AC` vtable `+8h` |
| 004e426b | `005884a0` with ECX = `*(00E198AC)+58h` |
| 004e4279 | `game+5D4h = 5` |
| 004e427f | if `0067d6e0()`: `004bfc70(this)` — the routine drain request 07h also runs |
| 004e428f | the once-only `GA_HM` award block (below) |
| 004e43e7 | `BSP_LoadingScreen_End()` |
| 004e43ec | if `game+216Dh`: `0076fad0(game+1EF0h, 0)`, then clear the byte |

The three managers share one shape: `if (global == 0) { p = operator new(size); if (p) ctor(p);
global = p; }` followed unconditionally by `(*global->vt[4])()`. The vtable is taken through the
global at 004e41a3, 004e41e3 and 004e4222 even when the allocation returned null, so an
out-of-memory here dereferences null. That is a property of the shipped code, not a decompiler
artifact; the reconstruction keeps the ordering and omits the fault. `docs/GAME_FRAME_CONTROL.md`
gives the matching teardown requests: 16h destroys `00E198B8`, 06h destroys `00E198AC`, 09h
destroys `00E198B4`, each through vtable `+0Ch`.

### The load-block choice, 004e407f..004e40ab

`GILoading::SLM_LOAD_FRONTEND` (00CE8254) and `GILoading::SLM_LOAD_FRONTEND_RETURN` (00CE8274) are
the **only** `SLM_` strings in the image, and 004e4000 is their only referrer. They are not a state
field: the chosen string is wrapped in a NativeString and handed to `BSP_FileBlock_Construct`
(00be0a30), which opens a named VFS file block (`docs/FILE_BLOCK_SETUP.md`,
`docs/VFS_LOAD_PROCESSING_START.md`) that `BSP_FileBlock_Destroy` closes at 004e4126. So `GILoading`
names a resource-grouping scope for the load, and the loading "state machine" the packet contract
asked about is the VFS block registry plus the three-function screen lifecycle below.

There are three arms, not two:

| Condition | Result |
| --- | --- |
| `game+719Ch == 0` | 004e40ab, label `SLM_LOAD_FRONTEND` (cold boot from the title screen) |
| guard set, `00E198B8` and `00E198AC` both non-null | 004e409c jumps to 004e412d: **no block, no loading screen, no config publish** |
| guard set, either manager missing | 004e40a4, label `SLM_LOAD_FRONTEND_RETURN` |

The second test of the guard at 004e40a2 can no longer be false when it is reached, so that branch
is dead; the reconstruction collapses it. The skip arm is the fast return from a mission whose
front-end managers were never torn down.

### The `GA_HM` award block, 004e428f..004e43e2

`GA_HM` (00CE824C) appears nowhere else in the image. The block is a first-visit action:

1. `004c8b80` finds `"GA_HM"` in the `std::map<NativeString,int>` at `game+6F0h`. The iterator's
   `_Mycont` is checked against the container address and its `_Ptr` against the end sentinel at
   `game+6F4h`; **equal to the sentinel means not found**, and only then does the block continue.
2. `0090c5d0` with ECX = `*(00E188A8)+21A0h` must also return true.
3. `006b8da0` with ECX = `00E19900` maps `"GA_HM"` to an integer id, which must satisfy
   `1 <= id <= 99` (`LEA EAX,[ESI-1]; CMP EAX,0x62; JA` at 004e437a).
4. `00a3e520` and `004b44f0`, both with ECX = `00F8ABE8`, must return true.
5. `BSP_OnlineAwards_GrantIfSessionActive(00F8ABE8, id)` grants it, and
   `BSP_AwardTracker_RecordAtLeast(game+650h, "GA_HM", 1)` records it locally.

The tracker's map member is at `tracker+0A0h`, and `650h + 0A0h = 6F0h`, which is why step 1 and
step 5 touch the same container. The tracker object at `game+650h` belongs to packet
`game_award_trackers`; only this one member function is established here.

## The loading screen

One 0x58-byte singleton at `00E194B4`, created and destroyed inside every
`BSP_Game_EnterFrontEndShell` call. It derives from `FrontEndScreen` (004f7180) and from a
singleton-lifetime hook at `+8h`.

| Offset | Evidence |
| --- | --- |
| +0h | vtable 00CEF264 (0057c1c0); the base 004f7180 first stores 00CEAE54 |
| +4h | byte, "wanted"; set at 0057ce8e, cleared at 0057c29e |
| +5h | byte, "active"; set at 0057ce92, cleared at 0057c2a1; gates vtable `+1Ch` |
| +8h | vtable 00CEF28C; the lifetime node passed to 00415350 / 00bcfca0 |
| +0Ch | render-worker descriptor, argument 1 of `BSP_RenderMode_PrepareAndStartWorker` |
| +10h | render-worker context, argument 3 of the same |
| +1Ch | GUI element, vtable `+88h`(index, 0, 1.0f) |
| +20h | GUI element, visible in mode 0 only |
| +24h | GUI element; its `+114h` float is the aspect source in mode 1 |
| +28h | GUI element, vtable `+58h`(rect) in mode 1 |
| +2Ch | int, monotonic max of the 00bf7420 counter |
| +30h | float, monotonic max of the reported progress |
| +38h, +3Ch, +44h | zeroed by 0057c1c0 |
| +40h | set to 1 by 0057c1c0 |

`BSP_LoadingScreen_RegisterSingleton` (0057bff0) assigns `00E194B4 = this - 8` *inside the
constructor*, under the lifetime manager's critical section, which is why 0057cb60 reads the global
straight after 0057c1c0 returns with no store of its own. Fields `+1Ch`..`+30h` are not set by
either constructor; vtable `+10h`, called at 0057ccc2, is the only candidate and was not read.

### `BSP_LoadingScreen_Begin` (0057cb60)

`__fastcall(int mode)`. ECX carries the **mode**, not a `this`: 0057cb7a moves it to EBP, tested at
0057ccc4 and again for 1, and the body never dereferences it. 004e4000 passes 0.

Body order: renderer vtable `+0Ch`/`+14h` twice (both swap-chain buffers cleared and presented);
`0076c190` on `game+1EF0h` when `00E188A8` exists; GUI disable (`004c12b0` then `00aa0e00`) and one
`+0.0f` GUI update (`00aa4f80`, the zero coming from `FLDZ` at 0057cbd5); `004f8ac0` with ECX =
`00E18D48`; tear down the screen at `00E19698` through its vtable `+1Ch` and clear its two bytes;
load `interface/textures/allbutingame.ats`; create the singleton if absent and run vtable `+10h`;
configure for the mode; `0057c990`; set both flag bytes; `004f83b0`; vtable `+18h`; renderer vtable
`+1Ch`; and `BSP_RenderMode_PrepareAndStartWorker(screen+0Ch, 0057ca00, screen+10h, 19h)`.

That last call is the answer to "synchronously or across frames". `docs/RENDER_WORKER.md` and the
ledger record for 00aa4040 have it setting the render command queue to mode 2 and starting a worker
with a callback and a rate. So `BSP_Game_EnterFrontEndShell` **runs to completion inside one call
on the calling thread**, blocking the main loop, while the loading screen is drawn by a render
worker at rate 19h (25) through 0057ca00. Nothing here goes through the loading-queue singleton
(`004fde20` / `00509190`) of `docs/APP_RUN_FRAME.md`; that queue is only pumped by frames that do
run, which these are not.

Modes:

| Mode | Behaviour |
| --- | --- |
| 0 | loads `interface/textures/menu.ats`; `004c1ac0(2,1)` and `00518250(2,1)`; `+1Ch` image index 2 with alpha 1.0f; `+20h` visible; `+24h` and `+28h` hidden |
| 1 | `004c1ac0(1,1)` and `00518250(1,1)`; `+1Ch` image index = the published `00E08798` byte; `+20h` hidden; `00abaed0(&00E08790, 1)` binds the picked image; the `+24h` `+114h` float is compared against 00CEC380 and 00CEC3E8 and both arms call `+28h` vtable `+58h` with a rect; `+24h` and `+28h` visible |
| other | 0057ccc6 falls straight to 0057ce83: the screen is created and the worker started, but no element is configured |

004e4000 only ever uses mode 0, so the `mp.loading_NN` images it publishes one instruction earlier
are consumed by some other caller of mode 1, which was not found. That is the largest open
question in this packet.

### `BSP_LoadingScreen_ReportProgress` (0057bec0)

`__stdcall(float)`, RET 4, no ECX; a null global is a no-op. Both fields only ever rise. Checked on
disk because of the x87:

```
0057beef: fmul qword ptr [0xcef258]        ; 128.0, product left in ST0
0057bef5: movss dword ptr [ecx + 0x30], xmm0   ; stores the UNSCALED maximum
```

Ghidra and the independent decoder agree. The multiply is dead and ST0 is left loaded on return, an
x87 stack leak in the shipped image. Behaviour is `screen->+30h = max(screen->+30h, arg)` and
`screen->+2Ch = max(screen->+2Ch, clock())`.

### `BSP_LoadingScreen_End` (0057c250)

`__cdecl(void)`, RET 0. GUI refresh (`004c12b0` then `00aa2b80`); `0076c230` on `game+1EF0h`; vtable
`+1Ch` when the active byte is set; clear both bytes; `004f83b0`; unregister `screen+8h`; vtable
`+0Ch`(1); null the global. It runs on both exits of 004e4000, so the screen never leaks.

### The published config block at 00E08780

`BSP_LoadingScreen_BuildDefaultConfig` (0057d0d0) fills a caller local and
`BSP_LoadingScreen_PublishConfig` (0057cff0) copies it to the globals.

| Offset | Global | Meaning |
| --- | --- | --- |
| +0h | 00E08780 | never written on this path; unidentified |
| +4h..+0Ch | 00E08784.. | `vector<NativeString>` of image names, 8-byte elements |
| +10h/+14h | 00E08790/94 | the picked NativeString `{int length; char* data;}` |
| +18h | 00E08798 | byte, the mode 1 image index |

The seeded names are `mp.loading_24`, `mp.loading_05` and `mp.loading_12`, each built with an
explicit length of 0Dh. 0057d107 sets `+18h = 1` and then 0057d25a overwrites it with the live
`00E08798`, so the 1 never escapes and the previous load's value is preserved. Nothing on this path
writes `+10h`, so the picked name is whatever the last mode 1 caller left.

## Calling conventions and RET sizes

| Address | Convention | Arguments | RET |
| --- | --- | --- | --- |
| 004e4000 | `__thiscall` | ECX = game | 0 |
| 0057cb60 | `__fastcall` | ECX = mode (int), no `this` | 0 |
| 0057bec0 | `__stdcall` | float progress | 4 |
| 0057c250 | `__cdecl` | none | 0 |
| 0057cff0 | `__thiscall` | ECX = config | 0 |
| 0057d0d0 | `__thiscall` | ECX = config, returns it in EAX | 0 |
| 0057c1c0 | `__thiscall` | ECX = screen, returns it in EAX | 0 |
| 0057bff0 | `__thiscall` | ECX = screen+8h, returns it in EAX | 0 |
| 00506c80 | `__thiscall` | ECX = dst, stack src | 4 |
| 007fbe20 | `__thiscall` | ECX = tracker, stack key and value | 8 |
| 00a410a0 | `__thiscall` | ECX = 00F8ABE8, stack award id | 4 |

## Callers and callees

Sole caller of 004e4000: `BSP_Game_DrainStateRequestQueue` (004e4430) at 004e44fa, from the request
4 case at 004e44f3. 35 callees; the ones this packet establishes are listed above. 0057cb60,
0057c250, 0057cff0, 0057d0d0, 0057c1c0 and 0057bff0 have exactly one caller each within this chain.
0057bec0 is called by 004e4000 and by nothing else that this packet examined.

## Uncertainties

- Field `+0h` of the loading-screen config layout is never written and is unidentified. It could be
  a vtable or an allocator; `00506c80`'s vector members start at `+4h`, which is unusual.
- Fields `+1Ch`..`+30h` of the loading screen are attributed to vtable `+10h` by elimination.
  0057ccc2 was not followed.
- The mode 1 rect computation (0057cdc0..0057ce6d, constants 00CEC380 and 00CEC3E8) was read only
  far enough to see that both arms call vtable `+58h`.
- `004c1650`, `0086b0b0`, `004cc460`, `005884a0`, `0067d6e0`, `004bfc70`, `0076fad0`, `0090c5d0`,
  `00a3e520`, `004b44f0` and `006b8ad0` were identified by call shape and string only.
- The three managers are identified only by size, constructor and teardown request.
- Whether `game+216Dh` can be set on a path that does not enter the front end was not checked.

## Corrections offered to other packets

`00a7a460`'s ledger name is `BSP_SoundSystem_ReportFmodMemoryFailure`. Its own record already notes
that the body only calls `FMOD_Memory_GetStats` and ignores the argument, but the name still reads
as a failure handler. 004e4000 calls it at 004e4051 with `"Sounds before mainmenu"` as one of three
memory checkpoints, so a probe name would fit both call sites. Evidence appended to the record; not
renamed, because the address is outside this packet's lease and the name is referenced by
`docs/APP_INIT_AUDIO_ONLINE.md`.

## Reconstruction

`include/bsp/frontend_entry.hpp` and `src/frontend_entry.cpp`. The load decision, the config build
and publish, the three screen lifecycle routines and the whole 004e4000 sequence are C++ over an
injected `FrontEndShellHost` / `LoadingScreenHost`, one method per native call site, in the style of
`bsp::run_application_frame`. `bsp::FrontEndScreen` is reused from `include/bsp/frontend_states.hpp`.
No global is invented and no pseudocode is compiled. One case was added to `tests/math_tests.cpp`
for the 004e4151 gate, because reading it as "still 4" would build the front end after a sign-out.

## State reached

| Address | State |
| --- | --- |
| 004e4000 | reconstructed, build-tested |
| 0057cb60 | reconstructed, build-tested (the mode 1 rect argument is not modelled) |
| 0057bec0, 0057c250, 0057cff0, 0057d0d0 | reconstructed, build-tested |
| 0057c1c0, 0057bff0 | analyzed; layout and global assignment established, not reconstructed |
| 00506c80 | analyzed; identified as `vector<NativeString>::operator=` |
| 007fbe20, 00a410a0 | analyzed; full bodies read, not reconstructed |
| 00689800, 00686170, 006887e0 | exported only; size and call site, no behaviour |

Nothing here is ABI-compatible or game-validated. Every function named in this packet has a Ghidra
function; none needed defining.

## Follow-up packets proposed

- `loading_screen_elements`: 0057ca00, 0057c990, 0057c4c0, 00abaed0, files
  `docs/LOADING_SCREEN_ELEMENTS.md`, `include/bsp/loading_screen_elements.hpp`. Contract: the four
  GUI elements at screen+1Ch..+28h, the vtable `+10h` initialiser, and the worker callback 0057ca00
  that draws the screen at 25 Hz including its `wave_Icon` element.
- `frontend_manager_trio`: 00689800, 00686170, 006887e0 and their vtable `+4h`/`+8h`/`+0Ch` slots,
  files `docs/FRONTEND_MANAGERS.md`, `include/bsp/frontend_managers.hpp`. Contract: what the three
  singletons at 00E198B8, 00E198AC and 00E198B4 own, what `*(00E198AC)+4h` counts, and why drain
  requests 16h, 06h and 09h destroy them individually.
- `award_grant_path`: 00a41030, 00a40d60, 0090c5d0, 00a3e520, 004b44f0, 006b8da0, files
  `docs/AWARD_GRANT.md`, `include/bsp/award_grant.hpp`. Contract: the award-name to id map at
  00E19900, the 1..99 id range, and the online gating at 00F8ABE8+119h/+11Ah/+11Ch. Coordinate with
  `game_award_trackers`, which owns `game+650h`.
- `loading_image_consumer`: whoever calls `BSP_LoadingScreen_Begin` with mode 1. Contract: find the
  caller that consumes the `mp.loading_NN` list and writes 00E08790, since 004e4000 publishes the
  list but always raises mode 0.

## Corrections from docs/LOADING_SCREEN_ELEMENTS.md

- The vtable `+10h` initialiser is `0057c560`, not `0057c4c0` (slot +10h of vtable `00cef264`, read at `0057ccbb`); `0057c4c0` is the hint-rotation tick.
- Eight named GUI objects are bound, not four: the `FE_loading` page at `+0Ch`, then `hint_Text` `+18h`, `radar_Group` `+10h`, `wave_Icon` `+14h`, a discarded `bg_Group`, `titleLogo_Icon` `+20h`, `frameFlag_Icon` `+1Ch`, `title_Text` `+24h`, `loadingLogo_FrameBox` `+28h`.
- The `mp.loading_NN` strings are localisation keys for a rotating hint line, not textures: `0057c360` sends each through the localisation resolver to `00abaed0` with ECX = the hint widget, so mode 0 consumes the published list through the rotation rather than the mode block. A mode-1 caller is still unidentified.
- The worker callback runs on render worker thread `00b33c20` with ECX = the `radar_Group` widget, gated at 40 ms by the rate `19h`; it issues no render command and takes no lock.

## Corrections from docs/AWARD_GRANT.md

- `00a41030` and `00a40d60` are not grant entry points: they are the compiler-emitted `std::vector<int>::push_back` and `insert` that `00a410a0` uses to append an achievement id to the queue at `manager+360h`, drained by `00a3fa70`, the only caller of `XUserWriteAchievements` in the image.
- `0090c5d0`'s ECX is recorded above as `*(00e188a8)+21A0h`; it is a double dereference, and the body never reads ECX (it probes `<CSIDL_PERSONAL>/Battlestations-Midway/save` and is true only on `ERROR_ALREADY_EXISTS`).
- The award name-to-id map has no names in the executable: `006b9450` parses `Scripts/datatables/Achievements.lua` (52 rows in range, ids 27 to 78; the 1..99 check drops the 22 local-only badges at the -1 default and the `RANK` row at 0).
