# The main-menu screen and its update virtual (packet `main_menu_screen_update`)

Addresses: 00599DB0 005905E0 00590D60 00582F30 005861B0 005987F0 0058F560 00584AE0

The screen is the 578h object the front-end manager at `00E198AC` holds at `+58h`, constructed by
`005902E0`, screen id 1 = `INTF_MAINMENU`. `docs/MAIN_MENU_SCREENS.md` recovered its vtable row and
its string sets and listed the update virtual as exported only. This packet reads the update to the
instruction from the listing at `00599DB0..0059A6E2`, plus the other six overrides in that row and
the top-level menu builder `00584AE0`.

Reconstruction: `include/bsp/main_menu_screen.hpp`, `src/main_menu_screen.cpp`.

## Calling conventions

Every override is `__thiscall` with ECX = the screen. RET operands read off the listing:

| Slot | Address | RET | Signature |
| --- | --- | --- | --- |
| +0Ch destroy | 00590D60 | `RET 4` | `void*(this, unsigned char flags)`, the scalar deleting dtor |
| +10h register | 00582F30 | `RET` | `void(this)` |
| +14h bind layout | 005861B0 | `RET` | `void(this)`; the epilogue is at 00588275..0058827B |
| +18h enter | 005987F0 | `RET` | `void(this)` |
| +1Ch exit | 0058F560 | `RET` | `void(this)` |
| +20h update | 00599DB0 | `RET 4` | `void(this, float seconds)` |
| +24h fill list | 005905E0 | `RET 4` | `void(this, void* sink)` |
| — | 00584AE0 | `RET 4` | `void(this, bool select_first)` |

The update's single stack argument is a float: `FLD dword ptr [ESP+0B0h]` at 0059A3BD and
`FLD dword ptr [ESP+0B4h]` at 0059A66B, both forwarded to `00588C70` as its first argument. It is
also the value handed to `00A96F40` at 00599DD8. That is the frame delta in seconds.

`005905E0` is five instructions: `ADD ECX,470h; PUSH ECX; MOV ECX,[ESP+8]; CALL 004D6790; RET 4`.
Ghidra decompiles it as `FUN_004D6790(param_1 + 0x470)` and drops the ECX swap; the call is
`004D6790(ECX = the stack argument, this+470h)`. `004D6790` is also called from
`BSP_Frontend_UpdateActiveScreens` and is tagged `stl_probable`, so `+24h` hands the sub-object at
`+470h` to a container the caller owns. The same `+470h` object is hidden by the register virtual
through its own virtual `+34h`.

## Field layout

Offsets a routine in this packet demonstrably touches. Everything else in the 578h object is
unrecovered. The constructor's highest write is `+574h`, which is what makes the 578h `operator new`
argument of `docs/FRONTEND_MANAGERS.md` consistent.

| Offset | Field | Evidence |
| --- | --- | --- |
| +00h | primary vtable `00CEFC5C` | 005902E0 |
| +08h/+0Ch/+40h | secondary vtables `00CEFC48`, `00CEFC24`, `00CEFC04` | 005902E0 |
| +10h | text-queue flag inside the `+0Ch` sub-object | 00599DD2 `CMP byte [ESI+10h],BL`, then `00A96F40(this+0Ch, delta)` |
| +70h..+10Fh | ten 10h-byte vectors | 005902E0 clears `first`/`last`/`end` of each, proxy first |
| +110h | active mission-group handle | 00599DB0 copies one of +310h..+320h here on every campaign frame; 00582F30 clears it |
| +11Ch/+120h | the pair `00AA6740` returns | 005861B0 |
| +124h..+12Ch | backdrop position float3 | 005861B0 from `00AA6750`; 0059A68E passes `this+124h` to `00AA8240` |
| +130h | map zoom | 00582F30 leaves 0, 005987F0 sets 1.0, 00588C70 clamps to [0.5, 1.5] |
| +134h..+183h | five mission-point vectors, 10h each | 005902E0 vector-ctor iterator `(this+134h, 10h, 5, 004214E0, 0041F5D0)`; 00588C70 indexes `this+134h + index*10h` |
| +184h/+188h/+18Ch | map anchor float3 | 00588C70 stores the `005803E0` result |
| +190h/+194h/+198h | map base offset float3 | 00582F30 writes 0.2, 0.39, 0 |
| +19Ch | second smoothing gate | 00588C70 reads it; no writer in this packet |
| +1A0h | zoom-axis latch byte | 0059A5E5 stores 1 when the zoom delta is non-zero; no clear site found |
| +1A4h/+1A8h/+1ACh | map offset float3 | 00582F30 clears all three; 00588C70 adds them to +190h |
| +1B8h | top-level list box | 00584AE0 drives it; 00588A80 reads the selection |
| +1D4h | page animator sub-object | 0059A6AF `00683820(this+1D4h, byte +56Ch)` |
| +238h | list-entry pool | 00584AE0 `00AAB4C0(this+238h, 0)` per item |
| +2B0h/+2B4h | two text-height derived floats | 00582F30 from `00AB6BD0` and two `00AA6750` extents |
| +2F0h | exit hide target | 0058F560 calls its virtual `+34h` with 0 |
| +310h | `missions_US_Group` | 005861B0 binds the widget by name |
| +314h | `missions_JP_Group` | 005861B0 |
| +318h | `missions_US_DLC_Group` | 005861B0 |
| +31Ch | `missions_JP_DLC_Group` | 005861B0 |
| +320h | `training_Group` | 005861B0 |
| +328h | `bg_01_Icon` | 005861B0; 0059A68E repositions it |
| +32Ch | `background_Icon` | 005861B0 |
| +354h | detail animator sub-object | 0059A3F0 and 0059A41C `00683820(this+354h, 1)` |
| +470h | the `+24h`/register sub-object | 00582F30 hides it, 005905E0 passes it out |
| +500h | async-text request pending | 00590A00 sets it, 0059A041/0059A07A clear it |
| +504h | `std::string` A | 005902E0 initialises buffer 508h, size 518h, capacity 51Ch |
| +520h | `std::string` B | 005902E0 initialises buffer 524h, size 534h, capacity 538h |
| +564h | US campaign selected | 00582F30 clears it; 0059A32A/0059A356 read it |
| +565h | DLC campaign selected | 00582F30 clears it; 0059A31C reads it |
| +56Ch | pad-axis override disabled | 005978A1 writes it; 0059A532 gates the pad poll, 0059A6A7 forwards it |
| +570h/+574h | cleared by the constructor | 005902E0 |

The two strings are the VC8 secure-SCL `std::basic_string` layout: iterator proxy at +0h, 16-byte
`_Bx` at +4h, `_Mysize` at +14h, `_Myres` at +18h, 1Ch bytes total. That is why the constructor
writes 508h/518h/51Ch while the update passes 504h, and 524h/534h/538h while the update passes 520h.

## The update virtual, 00599DB0

Five stages, in listing order.

**1. Text queue, 00599DD2.** When the byte at `+10h` is set, `00A96F40(this+0Ch, seconds)` drains
the queued text events of the `+0Ch` sub-object.

**2. Damaged-content sweep, 00599DEE.** Guarded by `00E0887C != 0` and `00F8A304 != 0`. It walks
`records = 00F8A304[1]`, `count = 00F8A304[2]`, stride 18h, testing the byte at `record+12h`. The
first set byte raises the modal `FE_xbox.xsm_dlcdamaged` (`00CEFF34`) through
`00531B00(2, key, 2, 0, 0, ..., 0, 0)` and breaks the loop. `00E0887C` is cleared either way.

**3. Downloaded-content notice, 00599EF8.** Guarded by the byte at `00F8ABE8+30h`. Its terms, in
order: `00F8A304 != 0`; the menu-command screen's byte `+5h` is zero; `00A3E510` reports a selected
user; `00A3EAD0` returns 2; then `00A3EAD0` returns 2 or 1; then `00F8A304` virtual `+18h`. On a
pass it calls virtual `+24h` and raises `FE_xbox.xsm_dlcdownloaded` (`00CEFF18`). `00F8ABE8+30h` is
cleared either way. The second sign-in test at 00599F44 repeats what 00599F39 already forced, so it
can never fail; the reconstruction keeps both terms.

**4. The async-text arm, 0059A020.** `MOV ECX,[00F8A2FC]; CMP ECX,EBX; JZ 0059A085` with EBX zeroed
at 00599DD0. So the page-0Ah block runs **only when the global is null**, and 0059A0E9 then
dereferences that same global (`mov ecx, dword ptr [0xf8a2fc]`, verified against the disk bytes, not
only Ghidra's listing). This is a null dereference on its face. No site in the image stores an
immediate 0Ah into the page global, so the arm is most likely dead; see "Uncertainties".

When the global is non-null instead, and `+500h` is set, the arm polls: virtual `+44h` true clears
`+500h`; otherwise virtual `+3Ch` returns an object whose field `+34h` is compared against 4 and
then 2, and a 2 clears `+500h`.

Inside the page-0Ah block: `00408720(tmp, " \n\n", 3)`; if virtual `+44h` is true, virtual `+148h`
fills string B at `this+520h`, then `"\nTitle: "` (`00CEFF08`) and `"\n"` (`00CE4390`) are
concatenated around it. Then action 4Bh submits `virtual +70h(this+504h, this+520h, 1)` followed by
`00590A00(this, 1)`, and action 4Ah enqueues state requests 6 and 7 and sets the page to 1.

**5. The page machine, 0059A283.** The page is reloaded from `00E08874` here, so the arm above is
visible to it.

| Page | Action | Effect | Epilogue argument |
| --- | --- | --- | --- |
| 01h | — | nothing | 1 |
| 02h, 03h | 4Bh | `00584AE0(this, 0)` | 1 |
| 0Bh | 4Bh | `00584AE0(this, 0)` | none; 0059A2DC jumps past the epilogue |
| 04h, 05h, 06h, 07h, 08h | 4Bh | `00AA8240(this+328h, this+124h)` then `00584F50` | 3 |
| 09h | 4Bh | `00599340` | 4 |
| 09h | 4Ah | `00594BF0` | 4 |
| 0Ch | 4Bh | `0058C010` | 5 |
| 0Ch | 4Ah | `005885D0` | 5 |
| 0Ch | 50h | `005806A0`, then mode 2 into `manager+70h`, then `005886C0` | 5 |
| anything else | — | returns at 0059A6C8 | none |

The epilogue at 0059A6BC is `004C1E90(id)` then `00427190(ECX = its result)`. `004C1E90` reaches
`BSP_SingletonLifetime_GetManager`/`Register`, and `00427190` names `SCRIPTS/datatables/MPakScenes.lua`,
so the pair selects and updates the backdrop scene for the page.

### Return paths

Ghidra reports sixteen returns and the decompiled form shows eight `return` statements, but the
listing has exactly **one** `RET 4`, at 0059A6E0, reached through the shared epilogue at 0059A6C8.
The three early `return`s the decompiler places inside the page-0Ah block (`_free(pvStack_40)` and
its two neighbours) are artifacts of the inlined string destructors: 0059A214 `JC 0059A22C` falls
back into the same path after the free. Nothing in this function returns early past the SEH unlink.

## Input: the action query 004D92B0

`__thiscall(ECX = the game at 00E188A8, int action)`; Ghidra types the argument as a float, which is
why its constant compares render as denormals. The action scales by 30h into the record table of
`docs/GAME_INPUT_TICK.md`, and the record fields it reads are `+1Ch` previous hold, `+20h` previous
down, `+24h` current hold, `+28h` current down.

- Fresh press: `+28h` set, `+24h > 0`, and not (`+20h` set and `+1Ch > 0`). The per-action deadline
  in the map `004D6900` keys becomes `game+64Ch + 0.4` (`00CE65D0`, a double).
- Repeat: `+28h` set, `+24h >` the float at `00D7A218` (0.0f in this image), and `game+64Ch >`
  the deadline, strictly. The deadline becomes `game+64Ch + 0.1` (`00D7A3A0`, a double).

So the front end has a 0.4 s initial delay and a 0.1 s repeat.

It then raises one UI sound flag on the request queue singleton `004C1B90`:

| Slot | Actions |
| --- | --- |
| +19h | 46h, 47h, 4Ch, 4Dh, 57h, 58h |
| +1Ah | 4Ah, 4Eh, 4Fh, 50h, 51h, 52h, 53h, 54h, 55h |
| +1Bh | 4Bh alone |

**Which index is accept and which is back.** The GUI event handler `005993A0` runs the same page
switch off a one-byte control code at `event+0F0h`, and its arms match the update's one for one:
code A2h on page 0Ch calls `005885D0`, which is what the update does for action **4Ah**; code A3h on
page 0Ch calls `0058C010`, which is what the update does for action **4Bh**; the same pairing holds
on pages 02h/03h/0Bh (`00584AE0`), on page 09h (`00594BF0` against `00599340`) and on pages 04h..08h
(`00AA8240` then `00584F50`). Code A7h matches action 50h. So **4Ah is accept, 4Bh is back**, and
50h is the third, context action. The sound table is consistent with that only if `+1Bh` is the back
sound and `+1Ah` the select sound; the six `+19h` actions form a plausible navigation set.

## The top-level menu, 00584AE0 and 00588A80

`00584AE0` is `__thiscall(this, bool select_first)`, `RET 4`. It opens `FE.main_menu`, sets page 1,
hides `+2F0h`, enables `+1B8h`, and runs the seven-pass loop at 00584BC3..00584C13. Each pass:
`00AAB4C0(this+238h, 0)` allocates the entry, its virtual `+34h` is called with 1, the byte at
`entry+77h` becomes `enableTable[i] == 0` (`SETZ` at 00584BF2 against `00CEF77C[i]`), `00ABBE50`
labels it with `00E087B8[i]`, and `entry+0D8h` receives `i`. The item table and its labels are
already in `docs/MAIN_MENU_SCREENS.md` and in `bsp::kMainMenuItems`.

At 00584C2F the argument decides the highlight: **zero restores `00E194C4`**, the cached selection;
non-zero selects item 0. Both call sites inside this packet pass zero, so the menu remembers where
the player was.

**Activation is not in the update.** Page 1 does nothing in `00599DB0`. The item is dispatched by
`00588A80`, which reads the list-box selection with `00A9C990(this+1B8h)`, caches it in `00E194C4`,
and then:

| Item | Label | What it does |
| --- | --- | --- |
| 0 | `FE.main_singleplayer` | `00584F50`, which sets page 2 |
| 1 | `FE.main_multiplayer` | `005853C0`, which sets page 3 |
| 2 | `FE.main_tacticallibrary` | runs the Lua chunk `luaLoadControlFunctionNames()` through `006B8AD0`, then `005886F0`, which sets page 0Bh |
| 3 | `FE.main_options` | clears `00E19650`, then `BSP_Game_RequestState(game, 6)` and `BSP_Game_RequestState(game, 14h)` |
| 4 | `globals.live` | `XShowGuideUI(0)` |
| 5 | `FE_pc.main_marketplace` | only when `00585B40` and `00585810` both pass; then `XShowMarketplaceUI(BSP_XenonSystemManager_GetActiveUserSlot(0))` |
| 6 | `FE_pc.main_quit` | raises the `FE_pc.main_quit_confirm` modal with callback `0057D3B0`, then `00530C20` on the menu-command screen |

**This corrects `docs/MAIN_MENU_SCREENS.md`.** That doc states that no screen in this range pushes
onto the `game+5D8h` state-request deque and that every producer is in `004D`/`004E`. Two sites in
this packet's classes are producers: 00588AF9 and 00588B06 push 6 and 14h for the options item, and
0059A1EC and 0059A1F9 push 6 and 7 in the update's async-text arm. Against the drain table of
`docs/GAME_FRAME_CONTROL.md`, request 6 runs the manager virtual `+0Ch`, request 14h is
`BSP_Game_RaiseOptionsMenu` (004BAC20) and request 7 is `BSP_Game_RaiseMultiplayerMenu` (004BFC70).
So "Options" is a deque transition, not an interface request, and the async-text page backs out to
the multiplayer menu.

## The five mission groups

Index 0..4 is the value the update keeps in EDI/EBP and hands to `00588C70` as its fifth argument.
It selects the widget handle copied into `+110h` **and** the point list at `this+134h + index*10h`,
so the two are one choice. The widget names come from `005861B0`.

| Index | Field | Widget | Point list | Page | Detail bytes (+565h, +564h) |
| --- | --- | --- | --- | --- | --- |
| 0 | +314h | `missions_JP_Group` | +134h | 04h | 0, 0 |
| 1 | +310h | `missions_US_Group` | +144h | 05h | 0, 1 |
| 2 | +320h | `training_Group` | +154h | 08h | unreachable |
| 3 | +31Ch | `missions_JP_DLC_Group` | +164h | 06h | 1, 0 |
| 4 | +318h | `missions_US_DLC_Group` | +174h | 07h | 1, 1 |

**This corrects the campaign-page naming of `docs/MAIN_MENU_SCREENS.md`**, which read pages 4..7 as
USN, USN-DLC, IJN, IJN-DLC from neighbouring title strings and marked the attribution provisional.
The widget each page binds says pages 4 and 6 are the Japanese lists and 5 and 7 the American ones,
and it identifies the previously unnamed page 8 as the training list. `+564h` is therefore the "US
campaign" byte and `+565h` the "DLC campaign" byte, and the two selectors agree on the four groups
they share.

Page 0Ch also gains a writer: 00594C51 in `00594BF0` stores 0Ch, so the objectives page is what the
mission-detail page's accept action opens. A page writer outside the doc's list is 004B4130 in
`FUN_004B3FC0`.

## The map camera

`00588C70` (one caller, this update) drives the mission map. It accumulates the zoom delta into
`+130h` as `delta * 0.1 + zoom` and clamps to `[0.5, 1.5]` (`00CE3800`, `00CE380C`), scales the
selected point list by the zoom, adds `+190h..+198h` and `+1A4h..+1ACh`, passes the result through
`005803E0`, stores it at `+184h..+18Ch`, and then walks the manager's list at `00E198AC+5Ch` naming
`mission_mappoint_<n>_Icon` widgets.

On the five mission-list pages the delta comes from 0059A517:

- `records+3654h` and `records+3684h` are field `+24h` (current hold) of action records **121h** and
  **122h**, the zoom-out and zoom-in analog holds.
- When `+56Ch` is clear and the vector at `00F8BBF4+94h..+98h` is non-empty, its first element's
  virtual `+24h` is called with 0Ah. A negative result replaces the zoom-out hold with its absolute
  value; a result strictly greater than `00D7A218` (0.0f) replaces the zoom-in hold.
- `delta = zoom_in - zoom_out`. A non-zero delta latches `+1A0h`.

`+1A8h` decays every frame by `offset + (0 - offset) * 0.1`. While `+1A0h` is still clear the map
drifts instead: `-0.2` (`00CE69CC`) when the zoom is above 1.0 (`00D7A24C`), otherwise 0. Once the
player touches the axis the latch is set and the drift never returns, because nothing in this packet
clears `+1A0h`.

The mission-detail page uses a fixed 0.2 (`00CE54A0`) instead, and blends `+1A8h` through
`00414130(ECX = &this+1A8h, &-0.21, &0.1)` (`00CEFF04`, `00D7A2F0`), whose body was not read.

## Callers and callees

The update is reached only through vtable slot `+20h` of `00CEFC5C`, driven by the three-pass pump
of `docs/GAME_FRONTEND_STATES.md`. Its direct callees, beyond the string and pool helpers:
`00A96F40`, `00531B00`, `00425D10`, `00A3E510`, `00A3EAD0`, `00408720`, `00426500`, `00425F80`,
`00BD1A60`, `00BD1E80`, `004D92B0`, `004D7920`, `00590A00`, `00584AE0`, `00588C70`, `00599340`,
`00594BF0`, `0058C010`, `005885D0`, `005886C0`, `005806A0`, `00584F50`, `00AA8240`, `00683820`,
`004BEC00`, `00414130`, `004C1E90`, `00427190`.

`00588A80` has two callers, `00598B60` and `005993A0`. `005993A0` is the GUI event handler that runs
the same page switch off a control code; it was read only far enough to settle the action mapping.

## Uncertainties

- **The page-0Ah null dereference.** The arm is entered only when `[00F8A2FC]` is zero and then
  dereferences it. The disk bytes confirm both the test at 0059A026 and the load at 0059A0E9 address
  the same global. No site stores an immediate 0Ah into `00E08874`; the register writers are
  00580940 (two sites), 00597870, 004B3FC0, 00515BC0 and 00686380, none of which was followed. The
  arm is most plausibly dead, but that was not proved. The reconstruction preserves the shape.
- `00F8A2FC` is an object with virtuals `+38h` (start), `+3Ch` (status, field `+34h` compared with 4
  and 2), `+44h` (busy), `+70h(a, b, 1)` and `+148h(dst, tmp)`. The page composes a body plus
  `"\nTitle: "` plus a name, so it reads as a text-entry or content-detail surface, but its writer
  was not found and the identification is open.
- `00414130`, `00683820`, `004D6790`, `00427190`, `004C1E90` and `005803E0` were used but not read.
  The names above describe the call, not the body.
- `+19Ch` gates the second smoothing branch in `00588C70` and has no writer in this packet, so
  `apply_map_zoom_00588c70` models only the delta term.
- `+1A0h` has no clear site in this packet; where the latch is reset is unresolved.
- `0057D3B0` has **no Ghidra function**; it is inside the stored body of
  `BSP_LoadingScreen_BuildDefaultConfig` (0057D0D0) and is passed to `00531B00` as the quit-confirm
  callback. It also contains the page write at 0057D472 that sets page 1. The orchestrator should
  define a function there before any name is applied.
- `00584AE0`'s `select_first` argument: only the two call sites in this packet were checked, both
  passing zero. `005993A0` and `00598B60` were not enumerated.
- Sound slots `+19h`/`+1Ah`/`+1Bh` on the request queue are named navigate/select/back from the
  action grouping, not from a string.

## What remains

The three page builders `00584F50`, `005853C0`, `005886F0` and the detail pair `00594BF0` /
`00599340` were identified by their call sites but not read. `005993A0` (375 decompiled lines) and
`00598B60` were read only for the action mapping. `005861B0` was read only for the handles it binds;
it is 8 KB and names `FE_briefing`, `FE_briefing_grid` and `FE_worldmap_historical` as well.
`005987F0` was read only for its `+130h` write.

## Follow-up packets

- `main_menu_page_builders` — addresses 00584F50 005853C0 005886F0 00580940 00597870; files
  `docs/MAIN_MENU_PAGES.md`, `include/bsp/main_menu_pages.hpp`, `src/main_menu_pages.cpp`. Contract:
  the five page builders, what each opens, and the parameter 00597870 turns into pages 4..7 and the
  `+56Ch` byte, so the page-0Ah reachability question can be closed.
- `main_menu_gui_events` — addresses 005993A0 00598B60 00588A80; files
  `docs/MAIN_MENU_GUI_EVENTS.md`, `include/bsp/main_menu_gui_events.hpp`,
  `src/main_menu_gui_events.cpp`. Contract: the control-code switch at `event+0F0h`, the widget
  search over `+248h`/`+260h`/`+268h`, and the full item dispatch including the quit callback.
- `front_end_action_query` — addresses 004D92B0 004D6900 004C1B90; files
  `docs/FRONT_END_ACTION_QUERY.md`. Contract: the repeat map keyed by action index, the three sound
  slots, and the action indices the fifteen named actions correspond to.
- `mission_map_camera` — addresses 00588C70 005803E0 00414130 00683820; files
  `docs/MISSION_MAP_CAMERA.md`, `include/bsp/mission_map_camera.hpp`. Contract: the point-list
  element layout, the `mission_mappoint_` widget walk, and the `+19Ch` gate.

## State reached

| Routine | State |
| --- | --- |
| 00599DB0 update | analyzed to the instruction; reconstructed as a host sequence; build-tested |
| 00584AE0 top-level builder | analyzed; the selection rule reconstructed and build-tested |
| 00588A80 item dispatcher | analyzed; the item table reconstructed as data and build-tested |
| 004D92B0 action query | analyzed; the edge and repeat rule reconstructed and build-tested |
| 00582F30 register | analyzed; field writes recorded, not reconstructed |
| 0058F560 exit | analyzed; not reconstructed |
| 00590D60 destroy | analyzed; not reconstructed |
| 005905E0 fill list | analyzed from the listing; not reconstructed |
| 005861B0 bind layout | partially analyzed: the handles it binds only |
| 005987F0 enter | exported; read only for `+130h` |
| 00588C70 map driver | partially analyzed: the zoom accumulator and clamp; the widget walk was skimmed |
| 005993A0 GUI events | partially analyzed: the action mapping only |
