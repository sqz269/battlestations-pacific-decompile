# The three front-end manager singletons

Addresses: 00689800, 006898C0, 00689A10, 00689820, 00686170, 00686380, 00686C90, 00685820,
006887E0, 00689540, 006888E0, 00689510, 00687800, 00687320, 00687330, 00684E10, 00684FA0,
00684700, 00683AA0, 00683A90, 00684600, 004CC460, 00683E90, 006840F0, 004DA650, 004DB190,
004BAC20, 004BFC70, 00688AD0, 00688B20.

`BSP_Game_EnterFrontEndShell` (004E4000) builds three objects into 00E198B8, 00E198AC and
00E198B4 and then calls each one's vtable `+4h`. They are not three unrelated singletons: all
three derive from one base class whose constructor is 00684E10, and the attract screen at
00E198BC (`docs/GAME_BLOCKING_SCREEN.md`) and the in-mission interface manager at 00E198C4
(`docs/GAME_SIMULATION_GATE.md`) are two more instances of the same base. What the base
provides is a two-record interface-request slot and a registry that makes only one manager
visible at a time.

## Identity

| Global | Size | Constructor | Vtable | Load block | Root interface |
| --- | --- | --- | --- | --- | --- |
| 00E198B8 | 4Ch | 00689800 | 00CF78C4 | `GVOptions` (00CF78DC) | `INTF_OPTIONS` (0Ch) |
| 00E198AC | 78h | 00686170 | 00CF774C | `GVMainMenu` (00CF7754) | `INTF_MAINMENU` (1) or `INTF_REWARDS` (4) |
| 00E198B4 | 68h | 006887E0 | 00CF7820 | `GVMultiMenu::Init` (00CF7834) | `INTF_MULTIMAINMENU` (0Fh) |

The names are the game's own: each Init opens a VFS load block under the label above, and
00689540 references the literal `GVMultiMenu::Init` while 00686380 references `GVMainMenu`.

## The interface id table at 00E08CD8

00688AD0 formats `"GVMultiMenu::PushRequestInterface() req:%s set:%s"` with
`(&PTR_s_INTF_NONE_00E08CD8)[*(this+20h)]` and `(&PTR_s_INTF_NONE_00E08CD8)[argument]`. That
fixes 00E08CD8 as a 45-entry table of `const char*` indexed by the value every manager keeps at
`+04h` and `+20h`, and it is what identifies those two fields. The strings run backwards through
.rdata from 00CF76C4 down to 00CF7370.

| Id | Name | Id | Name |
| --- | --- | --- | --- |
| 00 | INTF_NONE | 17 | INTF_MULTIINGAME |
| 01 | INTF_MAINMENU | 18 | INTF_MULTIFRIENDS |
| 02 | INTF_MISSIONTREE | 19 | INTF_MULTISIGNIN |
| 03 | INTF_BRIEFING | 1A | INTF_MULTISESSIONBROWSER |
| 04 | INTF_REWARDS | 1B | INTF_MULTIERROR |
| 05 | INTF_UNITLIB | 1C | INTF_MULTICREATENEWACCOUNTPC |
| 06 | INTF_VIDEOLIB | 1D | INTF_MULTISESSIONFILTER |
| 07 | INTF_MEDALS | 1E | INTF_MULTITACTSHIT |
| 08 | INTF_CREDITS | 1F | INTF_MULTIPOINTSSHIT |
| 09 | INTF_DOWNLOADEDCONTENT | 20 | INTF_SCENE3D |
| 0A | INTF_LEADERBOARDS | 21 | INTF_MAP |
| 0B | INTF_ACHIEVEMENTS | 22 | INTF_PLANE |
| 0C | INTF_OPTIONS | 23 | INTF_PLANEBOMBER |
| 0D | INTF_CONTROLLAYOUT | 24 | INTF_PLANESPAWN |
| 0E | INTF_KEYBOARDSETUP | 25 | INTF_CAPTAIN |
| 0F | INTF_MULTIMAINMENU | 26 | INTF_BOMBVIEW |
| 10 | INTF_MULTIMODESELECTOR | 27 | INTF_TBOATHELMSMAN |
| 11 | INTF_MULTILIVERANKEDSELECTOR | 28 | INTF_SUBMARINE |
| 12 | INTF_MULTILAN | 29 | INTF_FREECAMERA |
| 13 | INTF_MULTICREATELANSERVER | 2A | INTF_IDLECAMERA |
| 14 | INTF_MULTIGAMELOBBY | 2B | INTF_MOVIECAMERA |
| 15 | INTF_MULTISELECTPLAYER | 2C | INTF_MOVIECAMERANEW |
| 16 | INTF_MULTIPLAYERS | | |

Ids 00h..1Fh are front-end interfaces and 20h..2Ch are in-session HUD interfaces. 00683E90 splits
them at exactly that boundary, so the split is the binary's, not an inference from the names.

**`*(00E198AC)+4h` is this id, not a count.** The packet contract asked what it counts; it counts
nothing. 00686380 writes it with 4 (`INTF_REWARDS`) when `DAT_00E198B0` is 1 and with 1
(`INTF_MAINMENU`) otherwise, at 006868AD..006868BD, and the shell reads the same field back at
004E4250: when it is not 4 the shell pushes `INTF_MAINMENU` at 004E4257. In other words the shell
is asking "is the main menu already showing the post-mission rewards page?" and only forces the
root page when it is not.

## The base class, 00684E10

40h bytes. Two identical 1Ch-byte records followed by one byte:

| Offset | Field | Evidence |
| --- | --- | --- |
| +00h | vtable | each derived constructor overwrites it after 00684E10 returns |
| +04h | applied interface id | 00684600 writes it from +20h; 006840F0 compares it against +20h |
| +08h | embedded object, vtable 00CE3CF0 | `param_1[2] = &PTR_..._0042EC80_00CE3CF0` |
| +0Ch, +10h, +14h | zeroed | 00684E10 |
| +18h | byte, set to 1 | 00684E10 |
| +1Ch | applied payload, refcounted | 00684600 swaps it from +38h through 006952A0/00694A60 |
| +20h | pending interface id | 004CC460 and 00684600 write it |
| +24h | embedded object, vtable 00CE3CF0 | `param_1[9] = &PTR_..._0042EC80_00CE3CF0` |
| +28h, +2Ch, +30h | zeroed | 00684E10 |
| +34h | byte, set to 1 | 00684E10 |
| +38h | pending payload, refcounted | 004CC460 swaps it |
| +3Ch | `bool active` | set at 0068477A, cleared at 00683AA2, tested at 006840FE |

The base constructor ends by inserting `this` into the `std::map` at 00E19898, whose head node
pointer is 00E1989C, with `this` as both key and value (00684C40, a red-black insert). The base
destructor 00684FA0 restores the base vtable, erases the entry through 00684EF0 and releases the
two payloads. `docs/GAME_BLOCKING_SCREEN.md` records the same list at 00E19898 as unrecovered;
this packet recovers it as the front-end manager registry.

Base vtable at 00CF76D0, five slots. Every derived vtable has the same five:

| Slot | Base | Meaning |
| --- | --- | --- |
| +00h | 00685040 | scalar deleting destructor, `__thiscall(this, flags)`, RET 4 |
| +04h | 00683A90 | `Init`. The base body is a bare RET, which is why the shell can call it unconditionally |
| +08h | 00684700 | `Activate` |
| +0Ch | 00683AA0 | `Deactivate` |
| +10h | 00684600 | `ApplyPendingInterface`, `__thiscall(this, id, payload)`, RET 8 |

| Manager | +00h | +04h | +08h | +0Ch | +10h |
| --- | --- | --- | --- | --- | --- |
| 00E198B8 | 00689BA0 | 006898C0 | 00684700 | 00683AA0 | 00689820 |
| 00E198AC | 00687300 | 00686380 | 00684700 | 00683AA0 | 00685820 |
| 00E198B4 | 006892F0 | 00689540 | 00689510 | 00687320 | 00687800 |
| 00E198BC (attract) | 00689E60 | 00689BD0 | 00689E80 | 00689C00 | 00684600 |

00687320 is a five-byte `jmp 00683AA0`, so all three managers reach the same deactivate body.

### Activate, 00684700, `__thiscall(this)`, RET

Walks the registry from its leftmost node and calls virtual `+0Ch` on every entry that is not
`this`, advancing with the iterator increment 00683D10. It then sets `+3Ch` and, unless `this`
is the in-mission interface manager at 00E198C4 (compared at 0068477E), calls its own virtual `+10h` with
`*(this+20h)` and `*(this+38h)`. That replay is how a manager that was lowered comes back showing
the page it had. This is the front end's only mutual exclusion.

### Deactivate, 00683AA0, `__thiscall(this)`, RET

Three instructions of substance: `*(this+3Ch) = 0`, `004F8710(0)`, `004D8C00(DAT_00E188A8, 0)`.
Both callees are varargs terminated by a zero id, so an empty list means "show nothing". Nothing
is freed, nothing is unloaded and both records survive.

### The request pair, 004CC460 and 00684600

`004CC460 PushRequestInterface(this, id, payload)`, `__thiscall`, RET 8: it writes the pending
record only. `00684600 ApplyPendingInterface(this, id, payload)`, `__thiscall`, RET 8, returns 1:
it writes the pending record the same way and then copies it into the applied record. Both build
a counted temporary from the payload with 004BEF00 and drop it with 0042BCA0, and both swap the
stored handle with 006952A0 release / 00694A60 retain.

Both are gated by the byte at 00E19894. 00683E90, `__stdcall(id)`, RET 4, returns 1 when the
request must be dropped: with the byte clear nothing is rejected; with it set, id 2Ch
(`INTF_MOVIECAMERANEW`) passes untouched, any id >= 20h is rejected, and any id < 20h clears the
byte and passes. 00684600 inlines the same test at 0068461C..00684636. So while the byte is set
the in-session HUD cannot take the screen and the first front-end request lifts the block.
Writers of the byte outside this packet: 004DAB5D, 004D2C5F, 004BC493, 005CD1E9.

### Servicing, 006840F0 `BSP_MenuInterface_ServicePendingRequests`

For 00E198AC, then 00E198B4, then 00E198B8: skip a null manager, skip one whose `+3Ch` is clear,
and otherwise compare `+04h` against `+20h` and `+1Ch` against `+38h`. When either pair differs it
calls virtual `+10h` with `+20h` and `+38h` and writes 1 through ECX. The redundant second compare
at 00684116 cannot change the outcome. Because an inactive manager is skipped, a request pushed at
a lowered manager waits until it is raised again, at which point 00684700's replay delivers it.

Each derived `+10h` (00689820, 00685820, 00687800) has the same shape: call 00684600 first, return
false if it rejected the request, then map the id to a GUI screen id and hand it to 004F8710
followed by 004D8C00. 00687800 does the mapping through 00687330, a jump-table for ids 3..1Fh.

## The three layouts

### 00E198B8, the options manager, 4Ch

00689800 is `__thiscall(this)`, RET: it calls 00684E10 and stores the vtable. It initialises none
of its own fields; Init does. 006898C0 opens the `GVOptions` block, writes `DAT_00E198B8 = this`
at 0068991E before it builds anything, and then:

| Offset | Size | Constructor | Loading progress before it |
| --- | --- | --- | --- |
| +40h | 270h | 005F6030 | 0.35 (00CF6560) |
| +44h | 30h | 00527D00 | 0.37 (00CF78D8) |
| +48h | 250h | 00555770 | none |

Each screen gets its virtual `+10h` called immediately after construction. Init closes with
`00684600(this, 0Ch, 0)` at 006899DA and `BSP_FileBlock_Destroy`. The destructor 00689A10 hides
then destroys the three screens and clears 00E198B8 at 00689B53.

### 00E198AC, the main menu manager, 78h

00686170 is `__thiscall(this)`, RET: base, vtable, then it zeroes +40h..+70h and the byte at +74h.

| Offset | Field |
| --- | --- |
| +40h | native string `{length, data}` = `sound/music/titlescreen.fsb` |
| +48h | native string = `sound/music/creditsfinal.fsb` |
| +50h, +54h | refcounted handles, released in 00686C90 with `InterlockedDecrement(handle+4h)` and the handle's virtual +00h. Provisional: nothing in this packet writes them, so they are the streams the two paths open into |
| +58h | screen, 578h, constructor 005902E0. The shell reaches it directly at 004E4271 (`005884A0` with ECX = `*(00E198AC)+58h`) |
| +5Ch | screen, 34h, 005CA880 |
| +60h | screen, 260h, 00626630 |
| +64h | screen, 138h, 0051E4D0 |
| +68h | screen, 5Ch, 0052FCE0 |
| +6Ch | screen, 16Ch, 00563370 |
| +70h | screen, 5D0h, 005098B0 |
| +74h | byte, zeroed by the constructor, not written by Init |

00686380 is the longest of the three. It opens `GVMainMenu`, stores both music paths in the
manager rather than in locals, and caches each one twice through the FileStore: a sibling built by
replacing the last four characters with the literal `.def` at 00CF779C, at priority 32h, then the
`.fsb` itself at priority 2. It registers the localisation table `globals` and reloads the tables.
If `*(game+1EE1h)` is set it clears `DAT_00E198B0` and raises `DAT_00E08874`. It writes
`DAT_00E198AC = this`, builds the seven screens with loading progress 0.40, 0.45, 0.50, 0.55,
0.80, 0.94, 0.96 and reports 1.0 after the last, then commits `INTF_REWARDS` or `INTF_MAINMENU`,
calls its own virtual `+8h` at 006868C9, enables GUI layer 1 at 006868CC and closes the block.

The destructor 00686C90 hides and destroys the seven screens, clears 00E198AC, releases the two
handles at +50h and +54h and removes both music files and their `.def` siblings from the
FileStore.

### 00E198B4, the multiplayer menu manager, 68h

006887E0 is `__thiscall(this)`, RET, wrapped in an SEH frame for the base subobject: base, vtable,
then it zeroes +5Ch, +60h and +64h only.

| Offset | Field |
| --- | --- |
| +40h..+54h | six screens: F8h/005E6D90, 100h/005EA8C0, 300h/005E3290, 2D0h/00574240, 5Ch/005D2F90, 90h/005CFDA0 |
| +58h | **unidentified.** Neither the constructor, nor 00689540, nor 006888E0 touches it |
| +5Ch..+64h | a vector `{first, last, end}`, zeroed by the constructor and freed by the destructor. Nothing this packet traced fills it |

00689540 opens its block through `BSP_FileBlock_Construct` with the literal `GVMultiMenu::Init`,
runs 00576B10, then 00687C00 when `DAT_00F88950` is set, 00576B10 again, 008D2F50, and 0076F0E0
when the inline strlen of `*(game+1FF0h)` finds an empty profile name. It writes
`DAT_00E198B4 = this` at 00689685, builds the six screens with no progress reporting, and commits
`INTF_MULTIMAINMENU` at 006897D3. It is the only one of the three that never touches the loading
screen. The destructor 006888E0 collects the six screens into a scratch vector, hides each one
whose byte `+5` is set through its virtual `+1Ch`, destroys them all through virtual `+0Ch(1)`,
clears 00E198B4 and frees the member vector at +5Ch.

00689510 overrides `Activate`: base 00684700, then 00689310 when the online object at 00F8A2FC
answers its virtual `+68h`.

## Lifetime: who builds and who tears down

Construction happens in three places. The shell builds all three in the order 00E198B8 (004E4171),
00E198AC (004E41B0), 00E198B4 (004E41F0), calling virtual `+4h` after each; 004BAC20 builds the
options manager lazily; 004BFC70 builds the multiplayer menu lazily. Both lazy paths run
`006B8AD0("collectgarbage(\"collect\")", 0, 0, 2)` when `*(game+1A08h)+4h` is set, but only after a
fresh construction. Each Init publishes its own global before it builds anything, so the shell's
own store of the same pointer is a second write.

### Why 16h, 06h and 09h are separate requests

They **do not destroy anything.** The drain table in `docs/GAME_FRAME_CONTROL.md` describes them
as destroying 00E198B8, 00E198AC and 00E198B4; what each one actually does is load the global and
call virtual `+0Ch`, which is 00683AA0 for all three (00E198B4 through the thunk at 00687320).
That clears the manager's `+3Ch` and hides both screen sets. The object, its screens, its cached
music and both interface records stay exactly as they were.

They are three requests rather than one because `Activate` already deactivates every other
manager: the only thing a caller can ask for that `Activate` does not provide is "lower this one
and raise nothing". Lowering the multiplayer menu while the main menu stays down is a different
front-end state from lowering the main menu, and the drain has one request per manager so any
caller can name the one it means. The state that must survive is the pair `+04h/+20h`: the
manager comes back through 00684700, whose replay re-issues `+20h` and `+38h` and puts the same
page on screen. If `+0Ch` freed anything, that replay would have nothing to show.

Actual destruction is two routines, both `__cdecl`, no arguments, RET, both calling virtual `+00h`
with the delete flag and clearing the global:

- **004DA650** runs 004D95F0(0) and then destroys 00E198AC, 00E198B4, 00E198B8 and the
  in-mission interface manager at 00E198C4, in that order. It does not touch the attract screen.
- **004DB190** raises 00E08874, lowers the in-mission interface manager with 004D7970(0) and
  004DA780 without destroying it, then destroys 00E198B4, 00E198B8 and 00E198AC, which is the reverse of 004DA650
  for the last two. It ends with 004C1710, 00A4C2C0 and a fresh
  `BSP_Game_DrainStateRequests`.

### The two raise requests

- **14h, 004BAC20**, `__thiscall(this = game)`, RET: build 00E198B8 if absent, Init, garbage
  collect, virtual `+8h`, `game+5D4h = 15h`.
- **07h, 004BFC70**, `__thiscall(this = game)`, RET: build 00E198B4 if absent, Init, garbage
  collect; push `INTF_MULTIMAINMENU` at 004BFCFC when `*(00F8A2FC)+4Ch` is clear; virtual `+8h` at
  004BFD0E; then, when `*(game+218Ch)` is set, clear `DAT_00E19610`, push
  `INTF_MULTIMODESELECTOR` at 004BFD1F and apply it immediately at 004BFD46 because `Activate`
  has already replayed the old record; `game+5D4h = 8`. The shell also runs this routine at
  004E428A when 0067D6E0 is true.

## Readers

`python tools/bsp.py ghidra xrefs` on the three globals gives 40, 40 and 40 sites at the query cap.
The shape is consistent: 006840F0 and the drain touch all three; 004DA650 and 004DB190 destroy all
three; and each manager is then read by its own screen family. 00E198B8 is read by the options
screens at 0055Fxxx, 0056xxxx and 005Fxxxx; 00E198AC by `BSP_Game_Render` at 004CA63B,
`BSP_Settings_ApplyAudio` at 008D5498, 004D8000 (request 02h) and the 005Dxxxx/005Exxxx screen
family; 00E198B4 by `BSP_Game_UpdateMultiplayerInterface`, `BSP_Game_UpdatePresenceContext` at
004C020E and the 0077xxxx multiplayer screens. None of those readers were analysed here.

## Calling conventions and RET sizes

| Address | Convention | RET |
| --- | --- | --- |
| 00689800, 00686170, 006887E0 | `__thiscall(this)`, returns this in EAX | RET |
| 006898C0, 00686380, 00689540 | `__thiscall(this)` | RET |
| 00689A10, 00686C90, 006888E0 | `__thiscall(this)` | RET |
| 00689BA0, 00687300, 006892F0 | `__thiscall(this, unsigned flags)` | RET 4 |
| 00684E10, 00684FA0 | `__thiscall(this)` | RET |
| 00684700, 00683AA0, 00683A90, 00689510 | `__thiscall(this)` | RET |
| 00684600, 004CC460 | `__thiscall(this, int id, void* payload)` | RET 8 |
| 00689820, 00685820, 00687800 | `__thiscall(this, int id, void* payload)` -> bool in AL | RET 8 |
| 00683E90, 00687330 | `__stdcall(int id)` | RET 4 |
| 006840F0 | `__thiscall(this = out byte)` | RET |
| 004DA650, 004DB190 | `__cdecl()` | RET |
| 004BAC20, 004BFC70 | `__thiscall(this = game)` | RET |
| 00687320 | five-byte `jmp 00683AA0` | n/a |
| 004F8710, 004D8C00 | `__cdecl` varargs, zero-terminated id list | RET |

## Uncertainties

- The embedded objects at +08h and +24h (vtable 00CE3CF0, scalar deleting destructor 0042EC80)
  are zero-initialised by 00684E10 and never read by anything traced here. Their type is unknown.
- 00E198B4+58h is written by nothing in this packet. It may be a member some untraced method
  fills, or padding.
- 00E198AC+50h and +54h are identified as refcounted handles from the destructor's release
  sequence only; that they are the two music streams is an inference from the two adjacent paths.
- 00E198AC+74h is zeroed by the constructor and never written by Init.
- The member vector at 00E198B4+5Ch has an unknown element type.
- 00E19894 is read here but its four writers were not analysed, so what raises the lock in the
  first place is open.
- The four Init callees 00576B10, 00687C00, 008D2F50 and 0076F0E0 were identified by call site
  only; their bodies were not read.

## Ghidra function coverage

Four addresses this document names have **no Ghidra function** and need one defined before a name
can be applied: 00689820, 00685820 and 00687800 (the three `+10h` overrides, each `ret 8`) and
00687320 (the five-byte deactivate thunk). Ghidra folds 00687320 into the body of the scalar
deleting destructor at 00687300, which really ends at 0068731B. 00687330 does have a function.
Everything else named here has one.

## State reached

| Address | State |
| --- | --- |
| 00684E10, 00684700, 00683AA0, 00683A90, 00684600, 004CC460, 00683E90, 006840F0 | reconstructed, build-tested |
| 00689800, 006898C0, 00689A10 | reconstructed, build-tested |
| 00686170, 00686380, 00686C90 | reconstructed, build-tested |
| 006887E0, 00689540, 006888E0 | reconstructed, build-tested |
| 004DA650, 004DB190, 004BAC20, 004BFC70 | reconstructed, build-tested |
| 00684FA0, 00689510, 00687330, 00688AD0, 00688B20 | analysed; body read, not reconstructed |
| 00689820, 00685820, 00687800, 00687320 | analysed from raw bytes; no Ghidra function |
| 00684C40, 00684EF0, 00683D10, 004BEF00, 0042BCA0, 00694A60, 006952A0 | identified by role only |

Nothing here is ABI-compatible or game-validated.

## Follow-up packets

- `front_end_screen_sets`: 004F8710, 004D8C00, 004F83B0, 00687330, files
  `docs/FRONT_END_SCREEN_SETS.md`, `include/bsp/front_end_screen_sets.hpp`. Contract: the two
  zero-terminated varargs screen-set calls every Activate and every `+10h` override makes, the
  jump-table id mapping at 006873EC, and what "hide everything" does to the 95-slot screen
  registry in `docs/GAME_FRONTEND_STATES.md`.
- `front_end_interface_lock`: 00E19894 and its writers 004DAB5D (004DA780), 004D2C5F (004D2BB0),
  004BC493 (004BC410), 005CD1E9 (005CD1A0), files `docs/FRONT_END_INTERFACE_LOCK.md`. Contract:
  what raises the lock that blocks every in-session HUD interface, and why 2Ch is exempt.
- `main_menu_screens`: 005902E0, 005CA880, 00626630, 0051E4D0, 0052FCE0, 00563370, 005098B0,
  files `docs/MAIN_MENU_SCREENS.md`. Contract: the seven objects at 00E198AC+58h..+70h, which
  interface id each serves, and the one the shell drives directly through 005884A0.
- `multi_menu_screens`: 005E6D90, 005EA8C0, 005E3290, 00574240, 005D2F90, 005CFDA0, 00689310,
  files `docs/MULTI_MENU_SCREENS.md`. Contract: the six objects at 00E198B4+40h..+54h, the
  unidentified +58h member and the vector at +5Ch, and the online refresh 00689510 gates on
  `*(00F8A2FC)` virtual +68h.
- `front_end_manager_registry`: 00684C40, 00684EF0, 00683D10, 00683C80, 00684990, files
  `docs/FRONT_END_MANAGER_REGISTRY.md`. Contract: the `std::map` at 00E19898 keyed by manager
  pointer, its node layout, and whether anything other than 00684700 iterates it.
