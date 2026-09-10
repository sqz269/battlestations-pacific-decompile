# The multiplayer menu's six screens and its online refresh

Addresses: 005E6D90, 005EA8C0, 005E3290, 00574240, 005D2F90, 005CFDA0, 00689310, 00689510,
005EA720, 005EA7C0, 005E1820, 0056F320, 005D2440, 005CF260.

`docs/FRONTEND_MANAGERS.md` establishes the multiplayer menu manager at `00E198B4`: a 68h object
built by 006887E0, initialised by `GVMultiMenu::Init` (00689540) and torn down by 006888E0, whose
`+40h..+54h` hold six screen pointers. This packet reads those six classes, the unidentified `+58h`
member, the vector at `+5Ch`, and the refresh 00689510 reaches.

## What a multiplayer screen is

All six derive from the front-end screen base 004F7180 (`docs/GAME_FRONTEND_STATES.md`), whose
vtable `00CEAE54` has exactly **ten** slots: `00CEAE54+28h` is already the next vtable, and every
leaf vtable here is ten slots wide as well. Slot `+00h` is `__purecall` in the base and each leaf
supplies a two-instruction `mov eax, <id>; ret`; `BSP_FrontEndScreen_Register` (004F71D0) calls it
and uses EAX as the index into the 95-slot registry at `00E18B60`.

Five of the six chain the base, overwrite `+00h`, `+08h` and `+0Ch` with their own vtables (the
intermediate pair `00CEB0FC`/`00CEB110` is written first and immediately replaced), and embed one
or three copies of the 00683610 block. 00683610 is `__thiscall(this)` with no callees: it clears
dwords at `+00h`, `+20h` and `+28h`, zeroes eight floats (`+0Ch`..`+1Ch`, `+24h`, `+2Ch`,
`+34h`..`+3Ch`, `+54h`) and clears the bytes at `+48h` and `+58h` — a fade/slide animation record.

`00689540` allocates each screen with `operator new` (00BF681B), runs its constructor, stores the
pointer, and calls the screen's virtual `+10h`, the register override. It never reports loading
progress and commits `INTF_MULTIMAINMENU` through `00684600(0Fh, 0)` at the end.

## Screen id is not interface id

The registry index a screen returns from slot `+00h` and the interface id in the `00E08CD8` name
table are two different namespaces. `BSP_MultiMenu_MapInterfaceToScreen` (00687330, owned by the
screen-sets packet; read only here) is the translation: a jump table at 006873EC covering ids
3..1Fh. Read back from the image, the six rows that concern this packet are

| Interface id | `00E08CD8` name | Screen id | Constructor |
| --- | --- | --- | --- |
| 0Fh | `INTF_MULTIMAINMENU` | 10h | 005E6D90 |
| 10h | `INTF_MULTIMODESELECTOR` | 11h | 005EA8C0 |
| 14h | `INTF_MULTIGAMELOBBY` | 16h | 005E3290 |
| 1Ah | `INTF_MULTISESSIONBROWSER` | 1Ch | 00574240 |
| 1Bh | `INTF_MULTIERROR` | 1Dh | 005D2F90 |
| — | — | 15h | 005CFDA0 |

The table is decoded by reading `00E08CD8` as 4-byte pointers: index 0Fh is `INTF_MULTIMAINMENU`
(00CF75B8) and index 17h is `INTF_MULTIINGAME` (00CF7500), the two anchors
`docs/GAME_SIMULATION_GATE.md` already fixed, and every entry in between decodes in order. **Screen
id 15h is not a target of 00687330**, so no interface request can select the chat screen: it is an
overlay the other multiplayer screens drive, and 00772083 reaches it by loading `00E198B4+54h`
directly.

## The six classes

### 005E6D90, F8h, screen 10h, `INTF_MULTIMAINMENU` — `BSP_MultiMainMenuScreen_Construct`

`__thiscall(this)` returning `this` in EAX, RET, SEH handler 00C75113. Base 004F7180, vtables
`00CF2708`/`00CF26F4`/`00CF26D4`, one animation block at `+6Ch`. Slot `+00h` is 005E6E00
(`mov eax,10h; ret`).

Register 005EA720 (`__thiscall(this)`, RET): 004F71D0, virtual `+14h`, clear `+E0h`/`+E4h`/`+F0h`,
`+ECh = *(00AA6750(scratch) + 4)`, `+F4h = -1`. It binds no GUI page itself; the widget bind is
virtual `+14h` (005E7B10), whose literals are `Main_Listbox`, `slider_FrameBox`, `rank_Text`,
`rank_Icon`, `arrow_top_Icon`, `achievement_0_Icon`, `select_FrameBox`, `rankgraf_FrameBox`,
`rank_prog_Icon`, `rank_prog_new_Icon`, `rank_var_Text` and `playername_var_Text`. The two layout
labels `FE.multi_player_title` (00CF2730) and `FE.multi_ranked_title` (00CF2748) sit immediately
after the vtable; that adjacency is a hint, not a traced call argument.

### 005EA8C0, 100h, screen 11h, `INTF_MULTIMODESELECTOR` — `BSP_MultiModeSelectorScreen_Construct`

Same shape, SEH handler 00C75503, vtables `00CF2B44`/`00CF2B30`/`00CF2B10`, one animation block at
`+6Ch`. Slot `+00h` is 005EA930 (`mov eax,11h; ret`).

Register 005EA7C0 is the shortest of the six: 004F71D0, virtual `+14h`, `+F4h = -1`, `+10h = 0`.
Its only callee is 004F71D0. The page comes from virtual `+14h` (005EADC0), which references
`Main_Listbox`, `mode_Text`, `missionpicture_Icon`, `min_rank_t_Text`, `private_t_Text`,
`max_rank_t_Text`, `players_t_Text`, `textbox_Group`, `test_Clipbox`, `scroll_right_Icon`,
`arrow_top_scroll_Icon`, `textx_scroll_bg_Icon`, `textx_scroll_FrameBox` and `rank_Icon`.

### 005E3290, 300h, screen 16h, `INTF_MULTIGAMELOBBY` — `BSP_MultiGameLobbyScreen_Construct`

SEH handler 00C74F9B, vtables `00CF23E4`/`00CF23D0`/`00CF23B0`, **three** consecutive animation
blocks. Slot `+00h` is 005E10A0 (`mov eax,16h; ret`). After the blocks the constructor clears the
vector triples at `+1F8h`, `+234h`, `+244h` and `+2E4h`, the bytes at `+270h` and `+271h`, and the
dword at `+1Ch`; publishes `DAT_00E19594 = *(DAT_00E188A8 + 60Ch)`; then walks eight entries through
00576B10 and 00575E80, reading `DAT_00E198AC+5Ch` (the main menu manager) and calling 005C49C0,
005C6850 and 005D7070. That loop was not followed further.

Register 005E1820 binds two GUI pages through `BSP_GuiManager_GetOrCreate` (004C12B0): `FE_lobby`
into `+14h` and `FE_lobby_settings` into `+18h`, both as 00AA5840 handles. It then calls virtual
`+14h` and 005E12A0 and writes `-1` at `+2F0h` and `+2F4h`. `UniqueID` (00CF240C) and a family of
`FE.multi_*_desc` labels sit next to the vtable.

### 00574240, 2D0h, screen 1Ch, `INTF_MULTISESSIONBROWSER` — `BSP_MultiSessionBrowserScreen_Construct`

SEH handler 00C6F20D, vtables `00CEF134`/`00CEF120`/`00CEF100`, three animation blocks. Slot `+00h`
is 00573BB0 (`mov eax,1Ch; ret`). 0056A430 allocates a list sentinel stored at `+27Ch`; its byte
`+11h` is set and its three links at `+00h`, `+04h` and `+08h` are all pointed back at itself, which
is the MSVC `std::list` proxy shape. 0056A090 then clears the list through `sentinel[1]` and the
sentinel is relinked a second time. The constructor also writes `+270h = DAT_00CEECEC`,
`+274h = -1`, and clears `+280h`, `+284h`, `+288h`, `+28Ch`, `+294h`..`+29Ch`, `+2A4h`..`+2ACh` and
`+2B0h`.

Register 0056F320 is the largest of the six (15 callees). It chains 004F71D0 and then builds the
browser's texture names: `gui/radarshots/` concatenated with `_C.tga`, `_D.tga`, `_E.tga`, `_S.tga`,
`_ICS.tga`, `_ICM.tga`, `_ICL.tga` and `_ICH.tga`, and references `globals.no_server_found`. Its
body past the registration was not read. `Connection_Group`, `players_Text`, `mode_Text`,
`server_Text` and `RANK` are the adjacent literals.

### 005D2F90, 5Ch, screen 1Dh, `INTF_MULTIERROR` — `BSP_MultiErrorScreen_Construct`

SEH handler 00C73E69, vtables `00CF1BF4`/`00CF1BE0`/`00CF1BC0`, no animation block. Slot `+00h` is
005D2F40 (`mov eax,1Dh; ret`). The EH vector constructor iterator builds **two** 10h-byte elements
at `+30h` with constructor 004324A0, then 00450540 pushes the pooled labels `globals.yes` and
`globals.no`: a two-button confirm dialog. Register 005D2440 binds the GUI page `_MultiError` into
`+10h` and clears `+54h`.

This address carried the automatic tag `cg_array_ctor_helper` with a rename suggestion. That tag is
wrong: the EH vector iterator call inside it constructs a two-element member, it is not itself an
array helper. The reviewed name supersedes it.

### 005CFDA0, 90h, screen 15h, no interface — `BSP_MultiChatScreen_Construct`

SEH handler 00C73A69. The odd one out twice over: it has **two** vtables, not three (`00CF1998` at
`+00h`, `00CF19C0` at `+08h`, with `00CF19C0` sitting immediately after the ten slots of
`00CF1998`), and it chains a **second base constructor** 00A97260 after 004F7180. Slot `+00h` is
005CFE40 (`mov eax,15h; ret`), and slot `+04h` (005CFE50) is overridden too, which no other screen
here does. It then builds three subobjects: `+6Ch = 005CEE20()`, `+78h = 005CEE40()` and
`+84h = 005CEE40()`, each followed by a cleared dword at `+70h`, `+7Ch` and `+88h`.

Register 005CF260 binds the GUI page `_Chat` into `+3Ch`, clears the bytes at `+8Ch` and `+8Dh`, and
calls virtual `+14h` last.

## `00E198B4+58h`

Unidentified, and now bounded. It is written by **nothing**:

- the constructor 006887E0 zeroes only `+5Ch`, `+60h` and `+64h`;
- Init 00689540 writes `+40h`..`+54h` and stops (00689678, 00689689, 0068969F, 006896B5, 006896CB,
  006896E1 are the six stores);
- the destructor 006888E0 pushes exactly `+40h`, `+44h`, `+48h`, `+4Ch`, `+50h` and `+54h` into its
  scratch vector at 0068891A..00688998 and never reads `+58h`;
- the class has only five virtuals (`00CF7820` is ten bytes short of a sixth: the literal
  `GVMultiMenu::Init` starts at 00CF7834), and none of 006892F0, 00689540, 00689510, 00687320 or
  00687800 touches it;
- a wildcard scan of the image for `mov <reg>, [00E198B4]` followed by any `[<reg>+58h]` or
  `[<reg>+5Ch]` access returns no matches, so no outside reader reaches either member through the
  global.

A four-byte hole cannot sit between pointer members by accident, so `+58h` is a real declared member
that is dead in this build — most plausibly a seventh screen slot the shipped Init never fills.
Its type is unrecoverable from the image and it stays provisional.

## The vector at `00E198B4+5Ch`

`{first, last, end}` at `+5Ch`/`+60h`/`+64h`. The constructor zeroes all three. The destructor
frees the buffer at 00688A98 with a plain `free` and no element-destructor pass, then zeroes the
three again — so the element type is trivially destructible, which rules out strings and refcounted
handles and leaves raw pointers or a small POD. Nothing in this packet or in
`docs/FRONTEND_MANAGERS.md` fills it, and the scan above shows no outside writer. It is a live
member with no traced producer.

## The online refresh

### 00689510 `BSP_MultiMenu_Activate`

`__thiscall(this)`, RET, no stack arguments, the manager's virtual `+8h`.

```
00689510 PUSH ESI / MOV ESI,ECX / CALL 00684700          ; base activate
00689518 MOV ECX,[00F8A2FC] / CALL [[ECX]+68h]           ; the gate, bool in AL
00689527 JZ 00689531                                     ; AL == 0 -> return
00689529 MOV ECX,ESI / POP ESI / JMP 00689310            ; tail jump, this in ECX
```

The client pointer is dereferenced with no null check, unlike 004BFC70 and 004D8100 which both test
it first.

### 00689310 `BSP_MultiMenu_RefreshOnlinePlayerList`

`__thiscall(this)`, RET (`add esp,0B4h; ret` at 0068950C), no stack arguments, SEH handler
00C7DB79. **ECX is never read**: the body loads the client into ECX at 0068935E before touching the
incoming value, so the manager is not used and Ghidra's `void FUN_00689310(void)` is right about the
parameter list. Ghidra has no function entry at the six screen-id virtuals, so those were decoded
from disk bytes; 00689310 itself decompiles, but its `unaff_ESI`/`unaff_EDI` are artefacts of the
unknown convention on the virtual at `+7Ch` — both stand for the same stack slot `[ESP+14h]`, the
list head. The listing is authoritative.

In order:

| Site | Call | Effect |
| --- | --- | --- |
| 00689380 | client virtual `+70h`(out, query, 1) | the local record, returned by value |
| 006893BF | client virtual `+7Ch`(out) | fills an MSVC `std::list` of the same record |
| 006893D3..0068945B | 004B3FC0, 006883B0 | scan the whole list, no early exit |
| 00689466 | — | `DAT_00E08701 = 1` |
| 00689476 | client virtual `+1A0h`(0) | notify |
| 0068947C, 00689486 | 00688780, `free` | destroy the list, free the head proxy |

The record type is 3Ch bytes as traced: an MSVC `std::string` at `+04h` (buffer `+04h`, `_Mysize`
`+14h`, `_Myres` `+18h`) and scalars at `+20h`, `+24h`, `+28h`..`+2Bh`, `+2Ch`, `+30h`, `+34h` and
`+38h`..`+3Bh`, which is exactly the set 006883B0 copies after assigning the string through 00408120.
The list nodes are `{_Next, _Prev, record}`, so the record starts at node`+08h`.

The comparison at 00689419 passes `min(len(local), len(entry))` to 004B3FC0 and then 00689425..
00689438 requires the two lengths to be equal, so it is a full string equality on the name.
Each match is copied by 006883B0 into a record built on the stack at `ESP+40h` (whose only non-zero
initialiser is the byte `+3Ah = 1`), and **that record is destroyed at the epilogue without ever
being read**. The observable effects of the whole scan are therefore the dirty byte and the
notification; the copy is either a dead inlined result or the tail of a search whose consumer was
optimised away. The reconstruction returns the last match instead of discarding it, and that return
has no counterpart in the original.

`DAT_00E08701` is a dirty flag, not a mode: 0057B530 — in segment 19, the session browser's own
segment — calls client virtual `+94h`, tests the byte at 0057B547, clears it at 0057B551 and then
rebuilds its own list (`(end - begin) / 0A0h` elements) from the returned collection. 005E6F18 and
005E71CD, both inside the main-menu screen class at 005E6xxx, are the other two writers and were not
read.

## Calling conventions and RET sizes

| Address | Convention | Return |
| --- | --- | --- |
| 005E6D90, 005EA8C0, 005E3290, 00574240, 005D2F90, 005CFDA0 | `__thiscall(this)` -> `this` in EAX | RET |
| 005EA720, 005EA7C0, 005E1820, 0056F320, 005D2440, 005CF260 | `__thiscall(this)` | RET |
| 005E6E00, 005EA930, 005E10A0, 00573BB0, 005D2F40, 005CFE40 | no arguments | RET, id in EAX |
| 00689510, 00689310 | `__thiscall(this)`, `this` unused by 00689310 | RET |
| 00683610 | `__thiscall(this)` | RET |

## Functions Ghidra has not defined

The six screen-id virtuals **005E6E00, 005EA930, 005E10A0, 00573BB0, 005D2F40 and 005CFE40** have no
Ghidra function; each is five bytes of `mov eax, imm32` plus `ret`, followed by `int3` padding, read
with `disasm-raw`. They are named in this document but no ledger name was recorded for them, because
the integrator must define the functions first. Defining them is worth doing: they are the only
recovered evidence for the registry index of each class.

## Uncertainties

- `+58h` of the manager: a declared member with no writer, no reader and no recoverable type.
- The element type of the vector at `+5Ch`: trivially destructible, otherwise unknown.
- The identity of the online client at `00F8A2FC` and of its virtuals `+68h`, `+70h`, `+7Ch`, `+94h`
  and `+1A0h`. `docs/GAME_SESSION_POLLS.md` shows `+28h` firing on `XN_FRIENDS_*`, which is why the
  refresh is named for the player list, but no accessor here was traced into its body.
- Why 00689310 copies each match into a record it discards.
- Which of `FE.multi_player_title` and `FE.multi_ranked_title` a run selects: adjacency in `.rdata`
  is all the evidence there is.
- The eight-entry loop in the lobby constructor (00576B10, 00575E80, 005C49C0, 005C6850, 005D7070)
  and `DAT_00E19594`.
- The body of 0056F320 past its registration, and the update virtuals (`+20h`) of all six screens,
  which the budget did not reach. No screen's `Activate`/`Enter` (`+18h`) was read, so which
  XenonSystemManager accessors each screen consumes and which requests it enqueues through the
  deque of `docs/GAME_FRAME_CONTROL.md` is still open.
- The second base of the chat screen (00A97260) and the three subobject constructors 005CEE20 and
  005CEE40.

## Follow-up packets

| Id | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `multi_menu_screen_enter` | 005E8B20, 005EC5B0, 005E51D0, 0057A680, 005D26D0, 005CE740 | docs/MULTI_MENU_SCREEN_ENTER.md | The `+18h` enter virtual of each of the six screens: which XenonSystemManager accessors and session-poll notifications it reads and which interface requests it pushes. |
| `multi_menu_screen_update` | 005EA2D0, 005ECD80, 0057B530, 0057B600, 005D1460 | docs/MULTI_MENU_SCREEN_UPDATE.md | The `+20h` update virtuals, including the session browser's `DAT_00E08701` consumer 0057B530 and the 0A0h-byte session record it builds. |
| `online_client_accessors` | 00F8A2FC virtuals +68h, +70h, +7Ch, +94h, +1A0h | docs/ONLINE_CLIENT_ACCESSORS.md | Identify the online client class, its vtable and the five accessors the multiplayer menu uses; fixes the record type shared with 006883B0. |
| `front_end_screen_animation` | 00683610, 00683380, 00683790 | docs/FRONT_END_SCREEN_ANIMATION.md | The animation block every front-end screen embeds and the two helpers the widget binds call; shared by the main-menu and multiplayer screen families. |

## State reached

| Address | State |
| --- | --- |
| 005E6D90, 005EA8C0, 005E3290, 00574240, 005D2F90, 005CFDA0 | analyzed; layout and ids reconstructed, build-tested |
| 00689310, 00689510 | reconstructed, build-tested |
| 005EA720, 005EA7C0, 005E1820, 005D2440, 005CF260 | analyzed |
| 0056F320 | analyzed to the registration and its texture names only |
| 00683610 | analyzed (field init only; the block's use is not traced) |
| 005E6E00, 005EA930, 005E10A0, 00573BB0, 005D2F40, 005CFE40 | exported from disk bytes; no Ghidra function |
| 00687330, 00687800, 00689540, 006888E0 | read only; owned elsewhere |
