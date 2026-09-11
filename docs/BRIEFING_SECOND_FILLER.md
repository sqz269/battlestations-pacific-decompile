# The second route into the briefing screen (packet `cc_menu_detail`)

Addresses: 005E4490 005E4F90 0051DCE0 005D8650 005E3290 005E447C 00CF23B0

The bytes from `005E4479` to `005E4FBF` carry no Ghidra function. Two real
routines live there, and one of them is a second way into the mission briefing:
the multiplayer game lobby's "show briefing" action. Everything below is read
off the disk listing, because `ghidra decompile` cannot run on an undefined
range.

## What is actually at 005E4480

The packet brief put the region at `005E4480..005E4FBF`. That start is four
bytes inside a jump table, not code. `005E3580` ends with `RET` at `005E4478`,
then a three-byte `LEA ECX,[ECX]` aligns to `005E447C`, and the next sixteen
bytes are:

```
005e4470  00 00 5e 5d 5b 8b e5 5d c3 8d 49 00 39 3d 5e 00
005e4480  74 3d 5e 00 af 3d 5e 00 ea 3d 5e 00 cc cc cc cc
```

`005E447C`..`005E448B` is a four-entry jump table whose targets `005E3D39`,
`005E3D74`, `005E3DAF` and `005E3DEA` are all inside `005E3580`'s own body, so
the table belongs to `005E3580`. `CC` padding fills `005E448C`..`005E448F`.
Code resumes at `005E4490`.

## The owner

Both routines are reached only from data. `005E4490` is referenced from
`00CF23B4` and `005E4F90` from `00CF23E0`, and the table they sit in starts at
`00CF23B0`, immediately after the string `players_Group`.

`BSP_MultiGameLobbyScreen_Construct` (`005E3290`) installs three slices of that
table into one object:

```
005e32d3: MOV dword ptr [EBX],0xcf23e4        ; primary vtable, base at +0
005e32d9: MOV dword ptr [EBX + 0x8],0xcf23d0  ; secondary base at +8
005e32e0: MOV dword ptr [EBX + 0xc],0xcf23b0  ; tertiary base at +0Ch
```

So `005E4490` is slot 1 of the `+0Ch` vtable and `005E4F90` is slot 4 of the
`+8` one. The bodies confirm both adjustments: `005E44C7` and `005E456E` recover
the primary object with `LEA ...,[ESI-0Ch]`, and `005E4FAD` does it with
`ADD ECX,-8`. The class is the multiplayer game lobby screen, which is why the
route is "second": the single-player path into the briefing goes through the
mission tree (`docs/MISSION_BRIEFING_START.md`), this one through the lobby.

## 005E4490, the lobby's GUI event handler

`__thiscall(this = lobby + 0Ch, GuiEvent* event)`, `RET 4`, one SEH frame
(`005E4490`..`005E44A5`), 2801 bytes, no jump table of its own. `EDI` holds the
event throughout and `ESI` the adjusted `this`.

It reads the event kind from the event's own virtual `+5Ch`:

- kind 2 (`005E44BA`): a selection change. It pushes the list entry's `+0D8h`
  into the list box at `this+9Ch` through `00A9C7C0`, optionally pokes
  `[00E198B4]`'s sub-object, and ends in `005D67E0`.
- kind 3 (`005E453E`): a command. The command code comes from `[event+0F0h]`, or
  from the global default at `00E19608` when that pointer is null, and is
  compared one byte at a time: `0A2h`, `0B7h`, `0B8h`, `0A5h`, `0B1h` and more.
- kind 12h (`005E4F5A`): forwarded straight to the event's virtual `+84h` with
  the argument 2.

Anything unmatched falls to the common epilogue at `005E4F6D`.

### The briefing arm, 005E4990..005E49E4

```
005e4990: cmp eax, dword ptr [esi + 0x80]     ; the event came from widget this+80h
005e4996: jne 0x5e4f6d
005e499c: mov eax, dword ptr [0xe188a8]       ; the game object
005e49a1: mov edx, dword ptr [eax + 0x18ec]   ; its current entry index
005e49a7: mov eax, dword ptr [eax + edx*4 + 0x18cc]
005e49ae: mov eax, dword ptr [eax + 0x28]     ; that entry's side selector
005e49b1: push eax                            ; -> second argument of 0051DCE0
005e49b2: call 0x5d8650                       ; -> MissionRecord*
005e49b7: mov ecx, dword ptr [0xe198ac]
005e49bd: mov ecx, dword ptr [ecx + 0x64]     ; the briefing screen
005e49c0: push eax                            ; -> first argument
005e49c1: call 0x51dce0                       ; BSP_BriefingScreen_SetMissionData
005e49c6: mov ecx, dword ptr [0xe198b4]
005e49cc: push 3
005e49ce: call 0x688ad0                       ; push interface 3
```

`005D8650` ends in a bare `RET` and takes no stack argument, so the value pushed
at `005E49B1` survives the call and becomes the second argument of `0051DCE0`.
That matches the recorded prototype
`BSP_BriefingScreen_SetMissionData(this, MissionRecord* record, int side)`,
`RET 8`.

`005D8650` itself resolves the record: it takes the mission-tree screen at
`[00E198AC]+5Ch`, its mission vector at `+28h`, and the index
`005D7070([00E188A8]+60Ch)`, with the secure-SCL bound check at `005D8691` and
the `434h` stride at `005D8698`. `[00E198AC]+64h` is the briefing screen of
`include/bsp/main_menu_screens.hpp`, whose interface id is 3
(`kInterfaceBriefing`), which is what `00688AD0` is then asked to push.

So the arm fills the briefing screen with the mission the lobby's current entry
names, plus that entry's side selector at `+28h`, and then raises the briefing
interface. It is the same filler the single-player path uses (`005C5600` is the
only other caller of `0051DCE0`), driven from lobby state instead of from the
mission tree's own selection.

Who calls it: nothing in code. `005E4490` is installed in the lobby's `+0Ch`
vtable at `005E32E0` and reached through the front-end event dispatch.

### The other arms

| Guard | Effect |
| --- | --- |
| `005E44BD` kind 2 | `00A9C7C0(this+9Ch, [event+0D8h])`, then `005D67E0(lobby, [this+98h])` |
| `005E4565` code `0A2h` | `005D5490(lobby)`; if it returns nonzero, `005DA530(lobby)`, else one of `005D9F90`, `005D5870`, `00688AD0(0Bh)` or `00A9BE00` depending on which widget the event names |
| `005E46A5` code `0B7h` | `005D9660(lobby, 0, 1)` |
| `005E46D9` code `0B8h` | falls on to the chain at `005E4873` |
| `005E49F6` code `0A5h` | reads the selected entry, checks `[entry+9]` and `[this+9Ch]+85h`, then `00A3EAC0`/`00A4D440` on `[00F8ABE8]` |
| `005E4AA0` code `0B1h` | `005D71B0(lobby)` |
| `005E4F5A` kind 12h | event virtual `+84h`(2) |

`005E45DF` is worth noting next to the briefing arm: it writes
`[[00E198AC]+70h]+98h` from the same `game+18CCh` entry's `+28h`, sets
`[[00E198AC]+70h]+94h` to 8, and pushes interface `0Bh`. That is the tactical
library taking the same selector, so `entry+28h` is a shared selector field, not
a briefing-only one.

## 005E4F90, the lobby's small comparator

`__thiscall(this = lobby + 8, int unused, int value)`, `RET 8`, 40 bytes, no
frame:

```
005e4f90: mov eax, dword ptr [esp + 8]        ; the second stack argument
005e4f94: cmp eax, dword ptr [ecx + 0x8c]     ; lobby+94h
005e4f9a: je 0x5e4fb5
005e4f9c: mov edx, dword ptr [0xe188a8]
005e4fa2: cmp dword ptr [edx + 0x1fe4], 2
005e4fa9: je 0x5e4fb5
005e4fab: push -1
005e4fad: add ecx, -8                          ; the primary object
005e4fb0: call 0x5e1cf0
005e4fb5: ret 8
```

The first stack argument is never read. `game+1FE4h` is the session-kind word
`00580945` also tests; value 2 suppresses the call.

## Corrections

- The region with no Ghidra function is `005E4490..005E4FBF`, not
  `005E4480..005E4FBF`. `005E447C`..`005E448B` is `005E3580`'s jump table and
  `005E448C`..`005E448F` is `CC` padding. Defining a function at `005E4480`
  would swallow the last three table entries.
- `005E3580`'s body therefore does not end at the `RET` at `005E4478`; its jump
  table extends its extent to `005E448B`.

## Follow-up packets

- `multi_game_lobby_screen`: `005E3290`, the three vtable slices at `00CF23B0`,
  `00CF23D0` and `00CF23E4`, and the `005D....` class the lobby delegates to.
  `005E4490` is its event handler and cannot be named further without it.
- `lobby_entry_table`: `game+18CCh`, `game+18ECh` and `entry+28h`, read by the
  briefing arm, the tactical-library arm and `005D8650`'s neighbour
  `005D7070(game+60Ch)`.
- `front_end_command_codes`: the byte codes `0A2h`, `0A5h`, `0B1h`, `0B7h`,
  `0B8h` and the default table at `00E19608`, which this handler shares with
  every other front-end screen.

## no_ghidra_function

| Start | End (inclusive) | Evidence for the start | Evidence for the end |
| --- | --- | --- | --- |
| `005E4490` | `005E4F80` | `CC CC CC CC` padding at `005E448C`..`005E448F`; `005E4490` is a standard SEH prologue (`MOV EAX,FS:[0] / PUSH -1 / PUSH 0C75018h / PUSH EAX / MOV FS:[0],ESP`); referenced from vtable slot `00CF23B4` | `RET 4` at `005E4F7E`, then fifteen `int3` at `005E4F81`..`005E4F8F`. Every earlier `ret 4` in the range (`005E4532`, `005E45A0`, `005E45D7`, `005E4632`, `005E4678`, `005E4693`, `005E46C7`, `005E47EE`, `005E4870`, `005E48A0`, `005E4965`, `005E498D`, `005E49E4`, `005E4A8E`, `005E4AC2`, `005E4B1D`, `005E4B71`, `005E4BCD`, `005E4C21`, `005E4CAD`, `005E4D15`, `005E4EB9`, `005E4F50`) is an early return: none is followed by padding, and all share the epilogue that restores the SEH link and adds `14h` to `ESP` |
| `005E4F90` | `005E4FB7` | fifteen `int3` at `005E4F81`..`005E4F8F`; `005E4F90` is 16-byte aligned and referenced from vtable slot `00CF23E0` | `RET 8` at `005E4FB5`, then eight `int3` at `005E4FB8`..`005E4FBF` |

The linear decode of `005E4490`..`005E4F80` was checked for desync: it yields
856 instructions with no computed jump, no `jmp dword ptr` and no invalid
opcode, and all 110 branch and call targets that fall inside the range land on a
decoded instruction boundary.
