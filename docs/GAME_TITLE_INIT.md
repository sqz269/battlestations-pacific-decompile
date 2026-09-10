# Title-screen bring-up (`GGame::OnInitTitle`, 004c9a70)

Addresses: 004c9a70, 007fdb20, 004c1ac0, 00518250, 0067c8f0, 0067c970, 00916980, 004f83b0,
00685060, 00685070, 00685170, 0067c840, 0067ca80, 0068d8d0, 0068d8a0, 00517d10

Packet `game_title_init`, worktree `agent/game-title-init`. Ghidra was read-only for this packet.
This document covers the whole of `GGame::OnInitTitle`, the routines it runs that carried no
reviewed name, and the resolution of the open question `docs/GAME_FRONTEND_STATES.md` left about
where the logo sequence's advance lives.

## Result in one line

The logo sequence has no timed advance. It advances from the end-of-movie callback `00685060`,
which `BSP_LogoSequence_AdvanceOrFinish` installs on the movie player before it starts each entry;
the float at `logo+78h` is the minimum age before a manual skip is accepted, not a timeout.

## `GGame::OnInitTitle` in order

`__thiscall`, ECX = the `GGame` object (mirrored at `00E188A8`), no stack arguments, `RET` with no
stack cleanup, no return value. The body `004c9a70-004c9c8f` is wrapped in one SEH scope with
handler `00C6546E`. `ESI` holds the game throughout.

1. `004c9a9f` builds the pooled native string `GGame::OnInitTitle` (`00CE7680`, 0x12 bytes) and
   `004c9ad1` passes it to `BSP_FileBlock_Construct` (`00be0a30`). The matching
   `BSP_FileBlock_Destroy` (`00bdcb30`) runs at `004c9c79`. The label is a scope marker, not a file
   that is opened; it is the same literal that names the routine.
2. `004c9afa` `LEA ECX,[ESI+650h]` then `CALL 007fdb20`. **The decompiler renders this as a
   no-argument call; it is `__thiscall` on `game+650h`.** See below.
3. `004c9b0d` `MOV dword ptr [ESI+5D4h],2`, the front-end title state.
4. `004c9b09` builds `interface/textures/allbutingame.ats` (`00CE765C`, 0x23 bytes) and `004c9b52`
   passes it to `BSP_TextureAtlas_Load` with `ECX = [00F8C26C]`, the atlas manager.
5. `004c9b7b` `PUSH 1; PUSH 0; CALL 004c1ac0; MOV ECX,EAX; CALL 00518250`. **The two pushes are the
   arguments of `00518250`, not of `004c1ac0`**; MSVC hoisted them above the `this` evaluation.
   `004c1ac0` takes no arguments and returns the singleton. The real call is
   `00518250(004c1ac0(), 0, 1)`.
6. `004c9b8a` when `00E198BC` is null: `operator new(4Ch)`, `BSP_AttractScreen_Construct`
   (`00689d90`), then the object's virtual `+4h`. A failed allocation stores null and the code
   still dereferences it.
7. `004c9bc7` when `00E198CC` (the `skipTitle` / `.scn` flag) is set: `BSP_TitleScreen_Skip`
   (`0068d8a0`), which enqueues state 4. Otherwise, when `00E198C8` is null: `operator new(44h)`,
   `BSP_TitleScreen_Construct` (`0068d760`), then the object's virtual `+4h` = `0068d8d0`.
8. `004c9c13` when `*(00E188A8 + 21A0h)` is non-null: `00916980` with that object in ECX.
9. `004c9c2b` `MOV dword ptr [00E198B0],EBX`, an unconditional clear to 0.
10. `004c9c31` `CALL 0067c8f0`; when AL is zero, `00a40020`
    (`BSP_XenonSystemManager_ResetSignInState`, `ECX = [00F8ABE8]`) then `0067c970`.
11. `004c9c4a` closes the movie screen `00E18D48`: run its virtual `+1Ch` when byte `+5h` is set,
    then clear bytes `+4h` and `+5h`, then `004f83b0` with `ECX` = that screen. This is
    `close_front_end_screen` (`004b6e50`) inlined; it is the same shape
    `BSP_AttractScreen_Deactivate` uses at `00689c00`.
12. `004c9c79` closes the file block and returns.

## Calling conventions and RET sizes

| Address | Convention | Stack arguments | RET |
| --- | --- | --- | --- |
| `004c9a70` | `__thiscall`, ECX = game | none | `RET` |
| `007fdb20` | `__thiscall`, ECX = `game+650h` | none | `RET` |
| `004c1ac0` | no arguments, singleton in EAX | none | `RET` |
| `00518250` | `__thiscall`, ECX = the 164h singleton | `int set`, `char commit` | `RET 8` |
| `0067c8f0` | no arguments, result in AL | none | `RET` |
| `0067c970` | no arguments, void | none | `RET` |
| `00916980` | `__thiscall`, ECX = `game+21A0h` | none | `RET` |
| `004f83b0` | `__thiscall`, ECX = a front-end screen | none | `RET` |
| `00685070` | `__thiscall`, ECX = the logo object | none | `RET` |
| `00685170` | `__thiscall`, ECX = the logo object | none | `RET`, tail `JMP 00685070` |
| `00685060` | no arguments, static trampoline | none | `RET` or tail `JMP 00685070` |
| `0068d8d0` | `__thiscall`, ECX = the title screen | none | `RET` |

`00518250`'s `RET 8` is inferred from the two pushes with no caller-side `ADD ESP` at `004c9b85`;
the stored body ends past the export window and was not read to its epilogue.

## `007fdb20`, the player-profile reset at `game+650h`

Five callers: `GGame::OnInitTitle`, `00590e60`, `0067cc60`, `007fee20`, `007ff100`. `0067cc60` is
the sign-in handler behind the title screen's press-start element: it runs `007fdb20` first, then
resets several managers, then reads the XUID through `00a3eb00` into `game+698h`, formats it with
`sprintf("%llx")` as the save name, and ends in `BSP_TitleScreen_Skip`. So `007fdb20` is what runs
before a fresh profile session, in both the bring-up and the sign-in path.

Writes, in body order:

| Offset | Value | Evidence |
| --- | --- | --- |
| `+34h/+38h` | the empty string, through `00436710` | `007fdb42` |
| `+3Ch/+40h` | `globals.newplayer` | `007fdbac` |
| `+50h/+54h` | the empty string (`00CE3A0C`) | `007fdbd0` |
| `+59h` | 1 | `007fdbee` |
| `+5Ch`, `+60h` | 1, 1 | `007fdbf1`, `007fdbf4` |
| `+64h` | destroyed with `007fd780` + `free`, replaced by `new(24h)` + `00920e10` | `007fdbf7` |
| `+68h/+6Ch` | the empty string | `007fdc44` |
| `+74h`, `+80h`, `+8Ch` | three `std::list`s cleared through `004cec60` | `007fdc6d` on |
| `+98h` | a list cleared through `007fa880` | `007fdd56` |
| `+A4h` | a list cleared through `0058b520`, twice | `007fdceb`, `007fdd89` |
| `+ACh..+B4h` | a range erased through `004954f0` | `007fdd3f` |
| `+E0h`, `+E4h` | 1, 1 | `007fdd94` |
| a keyed map | `map["RANK"] = 1` through `005070c0` | `007fdde3` |
| `+20h..+2Ch` | 0, 0, 0, 9 | `007fde19` |
| `+18h` | a list rebuilt with 9 entries, each carrying the byte `index < 5` | `007fde60` loop |
| `+30h` | `-1` | `007fdeaf` |
| `+ECh`, `+F0h` | 0, 0 | `007fdeb6` |
| `+D8h`, `+DCh` | 0, 0 | `007fdecc` |
| `00F88980` | `008d4820` then `008d41c0` reset its flags | `007fded8`, `007fdee2` |

The segment 49 keyword set is `unlocks, selectedmissionid, selecteddifficulty, seenunlocks,
savedlobbyfilters`, which matches those fields. `+30h = -1` reads as a cleared selected-mission id
and the 9-entry list with an `index < 5` byte reads as the campaign slots, but neither is proved.

**Decompiler artifact.** Ghidra marks `free` (`00BF65AC`) no-return, so the pseudocode emits a
`return` after every `free` and drops the code behind it. `bsp.py disasm-raw` shows `ADD ESP,4` and
a fall-through at `007fdc0b` and at `007fde49`, so neither site returns; the first replaces the
object at `+64h` and the second is a list-node free loop.

## `004c1ac0` and `00518250`, the front-end frame layout sets

`004c1ac0` is a double-checked lazy singleton: under the singleton-lifetime manager's critical
section it allocates 0x164 bytes, constructs with `00517d10`, registers `instance+8h` and publishes
`00E18D80`. Ten other callers, among them `BSP_Game_OnInitOnce`, `BSP_LoadingScreen_Begin` and six
routines in its own segment.

`00517d10` chains `BSP_FrontEndScreen_Construct` (`004f7180`), so the singleton is itself a
front-end screen: vptr `00CEC39C` at `+0h`, a second vptr `00CEC398` at `+8h`, and the screen id
virtual right after it returns 0x0C. **It writes nothing between `+0Ch` and `+160h`**, so the
handle arrays and the current-set word start as uninitialised heap. `00518250` reads `+160h` as its
early-out guard, so the first bring-up loads the set only when that word does not happen to be 0.
Flagged as an uncertainty rather than a claim: no other initialiser was found.

`00518250(this, set, commit)` holds three parallel handle arrays and three `.rdata` name tables:

| Array | Handles | Names | Slots per set |
| --- | --- | --- | --- |
| backdrop | `this+0Ch`, set stride 34h | `00E08500` | 13 |
| panel | `this+110h`, set stride 8h | `00E08604` | 2 |
| title | `this+138h`, set stride 8h | `00E0862C` | 2 |

Body: return at once when `set == this+160h`; when `commit` is set, walk all three arrays slot-major
and release every handle belonging to a set other than `set`; then load the requested set's null
slots; then, still only when `commit` is set, store `this+160h = set`. A release goes through
`BSP_GuiManager_GetOrCreate()` then `00aa31f0` when the word at `layout+4h` is exactly 1, and
otherwise through `InterlockedDecrement(layout+4h)` with the layout's virtual `+0h` at zero. A load
is `BSP_GuiManager_GetOrCreate()` then `00aa5840(&name, 1, 1)`.

The tables as they stand in the image:

| Set | backdrop | panel | title |
| --- | --- | --- | --- |
| 0 | all null | `FE_frame` | `FE_frame_title` |
| 1 | all null | `FE_frame` | `FE_frame_title` |
| 2 | slot 6 `FE_options_bg`, slot 12 `FE_main` | `FE_frame` | `FE_frame_title` |
| 3 | all null | `GUI_pause` | `GUI_pause_title` |
| 4 | all null | null | null |

So the whole title bring-up loads exactly two layouts through this path, `FE_frame` and
`FE_frame_title`, plus `FE_attract` from the attract screen's constructor and `FE_initial` from the
title screen's activate. The atlas is `interface/textures/allbutingame.ats`. No Lua script, sound
bank or movie is loaded by `OnInitTitle` itself.

## `0067c8f0`, `0067c970` and `00916980`

`0067c8f0` returns true only when `00E19880` is non-zero, `00a3e510` reports a signed-in user, and
`BSP_XenonSystemManager_FindSlotForInvitee` equals `BSP_XenonSystemManager_GetActiveUserSlot`. A
false answer costs a sign-in reset plus `0067c970`.

`0067c970` takes the first element of the checked vector at `[00F8BBF4]+B8h`, passes it to
`00a90ee0` when it is non-null, then calls `00a917e0(2, 1, 0)` (the same input-slot bring-up
`BSP_Game_OnInitOnce` runs three times) and sets `00E1987C = 1`. It re-creates the primary input
binding.

`00916980` runs `00915760` over eight records at `this+4h` with stride 0x284, clears byte
`this+1484h`, then empties the red-black tree at `this+1488h/+148Ch`, destroying each value through
`0063ad70`. `game+21A0h` is written by `BSP_Game_OnInit` (`004e3e9f`) and cleared by `004cccc0` and
`BSP_Game_OnDestroy`, so it exists only while a mission is loaded and the title bring-up resets it
on the way back out. `00915760` lives in segment 59, whose keywords are all bot AI
(`pilotbot`, `torpedobot`, `thinktimeleft`), and its other caller is `008bc540`. What the eight
0x284-byte records hold is not established.

## `004f83b0`, the screen visibility commit

`__thiscall` on a front-end screen. It calls the screen's virtual `+24h` to fill a stack vector of
child pointers, walks it, and for every non-null child calls that child's virtual `+34h` with the
byte at `screen+5h`, then frees the vector. `docs/GAME_FRONTEND_STATES.md` already models slot
`+24h` as "fills the list `004f83b0` walks" and `docs/APP_INIT_FONTS_GUI.md` has `+34h` as the
visibility setter, so this pushes the screen's own active byte down to its children. It is the tail
jump of `close_front_end_screen` (`004b6e50`) and the commit step of every enter and exit in this
packet.

## The logo sequence, resolved

The 0x80 object at `00E198A4` (constructor `006851e0`, vtable `00CF76E4`) has only five vtable
slots: `006852E0` deleting destructor, `00685300` `BSP_LogoSequence_LoadTable`, `00684700` base
activate, `00683AA0` base deactivate, `00684600`. There is no per-frame virtual on it, so nothing
in the screen registry can tick it.

`BSP_LogoSequence_AdvanceOrFinish` (`00685070`), read from the listing:

1. `count = ([this+48h] - [this+44h]) / 8`, `index = [this+60h]`.
2. `index >= count` (unsigned): virtual `+0h` with 1, then `BSP_Game_OnInitOnce(game, 0)`, which
   re-runs `GGame::OnInitTitle` and leaves `game+5D4h = 2`. Return.
3. `[this+60h] = index + 1`.
4. `006850ba` `004f8970([00E18D48], 00685060)` — **installs the end-of-movie callback**.
5. `006850f4` `004f8a20([00E18D48], &entries[index], 1, 0.0f, 0)` starts the movie.
6. `00685101` sets movie-screen bytes `+4h` and `+5h` to 1, `004f83b0`, then its virtual `+18h`
   (the front-end screen enter).
7. `00685117` samples the clock `[01090AB0]` virtual `+20h` and copies all four dwords of the
   returned 16-byte timestamp into `this+68h..+77h`.
8. `0068515c` `this+78h = ((float *)[this+54h])[index]`, the per-entry delay.

`00685060` is a four-instruction static trampoline that Ghidra has **no function for**; from the
disk bytes it is `MOV ECX,[00E198A4]; TEST ECX,ECX; JE ret; JMP 00685070`. That closes the loop:
the movie player calls it when the current logo movie ends, it re-enters the advance with the logo
object in ECX, and the next entry plays. The sequence is driven by movie completion.

`BSP_LogoSequence_PollSkip` (`00685170`) is therefore only the manual skip: `elapsed` from
`00530890` against `this+68h`, reduced by `FILD [EAX] / FILD [EAX+8]`, compared with `FCOMIP` then
`JBE` against `this+78h`, so **strictly** greater; and only then `004d92b0(game, 4Ah)`. Both terms
are required. `004d92b0` reaches `BSP_InputManager_GetSingleton` and
`BSP_SoundRequestQueue_GetSingleton` and has 26 callers, so it is the shared front-end action query
with its UI click; action 0x4A is not identified.

`004f8a20`'s last two arguments are `0.0f, 0` here and `1.0f, 1` in `BSP_AttractScreen_Activate`
with `movies/PacificTheme.bik`. Reading them as volume and loop is a hypothesis; a fade time would
fit the observed values equally well.

This supersedes the **Uncertain** paragraph under "State 1 to state 2" in
`docs/GAME_FRONTEND_STATES.md`. Its reading of `00685170` is correct; the missing automatic advance
is `00685060`, not a timer.

## The title screen's first interactive state

`BSP_TitleScreen_Construct` (`0068d760`) only stores vtable `00CF7A98` and clears `+40h`. The
vtable is `0068D880`, `0068D8D0`, `0068D7B0`, `00683AA0`, `00684600`, so slot `+4h`, the one
`GGame::OnInitTitle` calls right after constructing, is `0068d8d0`.

`0068d8d0` branches on `00E198CC` at `0068d8e6`. With `skipTitle` set it repeats
`BSP_TitleScreen_Skip`'s body: `00bd3450([0109CECC])`, then, when `game+5E8h` is 0, request 4
through `BSP_Game_RequestState` and clear `game+5ECh`. With it clear, at `0068d92f`, it allocates
0x18 bytes, constructs with `0067c840`, stores the result in `title+40h`, runs that screen's
virtual `+10h`, sets its bytes `+4h` and `+5h` to 1, calls `004f83b0` on it and finally its virtual
`+18h`. `OnInitTitle` only ever reaches the second branch, because it constructs the title screen
only when `00E198CC` is clear.

`0067c840` chains `BSP_FrontEndScreen_Construct` and stores vtable `00CF6D90`. That vtable's slot
`+10h` is `0067ca80`, which calls `BSP_FrontEndScreen_Register` and then loads the GUI layout
**`FE_initial`** into `screen+8h`. The literals that follow the vtable in `.rdata` are
`FE_initial`, `press_start_Text`, `globals.saving_xbox`, `L"Session"` and `FE.unitlib_toggledesc`.

So the title screen's first interactive state is the `FE_initial` press-start screen, and the path
out of it is the sign-in handler `0067cc60`, which resets the profile and calls
`BSP_TitleScreen_Skip` to request state 4.

The existing name `BSP_TitleScreen_Advance` on `0068d8d0` describes only its `skipTitle` arm. Its
role at the `OnInitTitle` call site is the post-construct activate. Suggested supersession:
`BSP_TitleScreen_Activate`. Left to the integrator; this packet did not rewrite another packet's
name.

## Callers and callees

Callers of `004c9a70`: `BSP_Game_BeginStartupSequence` (`004e5753`), `BSP_Game_OnInitOnce`
(`004dd5b0`, on both the first-time and the soft path), `BSP_Game_ResetToTitle` (`004db220`, a tail
jump) and `BSP_Game_PollPlatformSessionEvents` (`004db290`, on a profile change).

Callees carrying no reviewed name before this packet: `004c1ac0`, `00518250`, `0067c8f0`,
`0067c970`, `007fdb20`, `00916980`, `004f83b0`. `004f83b0` also has an unnamed sibling `004f8ac0`
that `BSP_AttractScreen_Deactivate` calls next to it; not analysed.

## Uncertainties

- `00517d10` leaves `+0Ch..+160h` of the 0x164 singleton uninitialised, and `00518250` reads
  `+160h` before writing it. Either `00BF681B` zeroes, or the first `select` call is a coin flip.
  Not resolved; `00BF681B` was not read.
- `00518250`'s `RET 8` is inferred from the call site, not read from its epilogue.
- What the eight 0x284-byte records inside `game+21A0h` hold.
- The last two arguments of `004f8a20`.
- Input action 0x4A.
- Whether the 9-entry list `007fdb20` rebuilds at `+18h` is the campaign mission list.

## What each routine reached

| Address | State |
| --- | --- |
| `004c9a70` | reconstructed, build-tested (`bsp::run_title_init`) |
| `00518250` | reconstructed, build-tested (`bsp::select_front_end_frame_set`) |
| `00685070` | reconstructed, build-tested (`bsp::logo_advance_or_finish`) |
| `00685170` | reconstructed, build-tested (`bsp::logo_poll_skip`, `bsp::logo_skip_allowed`) |
| `00685060` | analysed from raw bytes; **no Ghidra function exists at this address** |
| `007fdb20` | analysed, full field map, not reconstructed |
| `004c1ac0`, `0067c8f0`, `0067c970`, `00916980`, `004f83b0` | analysed, not reconstructed |
| `0067c840`, `0067ca80`, `0068d8d0`, `00517d10` | analysed as evidence for the above |

`00685060` has no Ghidra function, so the integrator has to define one at `00685060-0068506F`
before the ledger name can be applied there.

## Follow-up packets proposed

1. `game_profile_reset` — `007fdb20`, `007fee20`, `007ff100`, `00590e60`, `0067cc60`, `00920e10`,
   `007fd780`, `005070c0`. Files `docs/GAME_PROFILE_RESET.md`, `reports/game_profile_reset.json`,
   `include/bsp/profile_reset.hpp`, `src/profile_reset.cpp`. Contract: recover the layout of the
   profile block at `game+650h` from its reset, its save path and its Lua bindings, and name the
   fields the segment keywords already advertise.
2. `game_movie_player` — `004f8af0`, `004f8970`, `004f89d0`, `004f8a20`, `004f8ac0`, `004f83b0`,
   `00E18D48`. Files `docs/GAME_MOVIE_PLAYER.md`, `reports/game_movie_player.json`,
   `include/bsp/movie_player.hpp`, `src/movie_player.cpp`. Contract: recover the 0x34 movie screen,
   the meaning of `004f8a20`'s trailing float and int, the two sinks at `+24h`/`+28h` and the
   completion-callback contract that `00685060` rides on.
3. `game_press_start_screen` — `0067c840`, `0067ca80`, `0067cb40`, `0067cc60`, `0067cfb0`,
   `0067d860`, vtable `00CF6D90`. Files `docs/GAME_PRESS_START_SCREEN.md`, the matching report,
   `include/bsp/press_start_screen.hpp`, `src/press_start_screen.cpp`. Contract: recover the
   `FE_initial` screen, its per-frame update, the sign-in and storage-device flow behind
   `press_start_Text`, and the exit into state 4.

## Correction from docs/PROFILE_UNLOCK_PREDICATE.md

The profile block fields recorded above at `+74h`, `+80h` and `+8Ch` as three `std::list`s are the head fields of three case-insensitive `std::set<NativeString>` trees at `+70h` (the saved `Unlocks` set), `+7Ch` (unlocks granted this session) and `+88h`; the keyed `RANK` map is the named-counter map at `+A0h`. `BSP_Profile_IsUnlockSatisfied` (`007fc4c0`, Lua name `Scoring_IsUnlocked`) splits a requirement string on `" ,"` and, per token, tries the mission-completion map at `+64h`, then the two unlock sets, the counters (strictly `> 0`) and the owned content ids at `+D0h`, all case-insensitively; the first hit wins.
