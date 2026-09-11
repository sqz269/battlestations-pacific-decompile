# Player-profile reset and archive completion
Addresses: 007fdb20, 007fee20, 007ff100, 00590e60, 0067cc60, 00920e10, 007fd780, 005070c0, 007fefe0, 007fa710, 007f9290, 007f9340, 007fa670

Packet `orch2_game_profile_reset`, branch `agent/orch2-profile-20260910`.
Analysis used the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; each `bsp.py ghidra` batch verifies the configured
project/program. This worker made no Ghidra mutations. Raw bytes came from the
configured original executable, never from an edited game installation.

`profile_reset.hpp/.cpp` implement six normal-flow host projections: profile
reset, the two name setters, read request/completion, and conditional write
request. They use `ProfileUnlockState` and `GameSettingsBlock` directly. The
existing sign-in implementation `apply_sign_in` is not duplicated. No function
here is an ABI-compatible replacement, and no real saves were opened or changed.

## Verified field map

Offsets are relative to `game+650h`. Reset is `007fdb20..007fdef9`.

| Offset | Reset | Evidence and meaning |
| --- | --- | --- |
| +08h | empty string list | `007fde11..007fde14`, `004d05e0` |
| +14h | nine bytes: five 1, four 0 | `007fde60..007fdea7`; serialized within `SavedLobbyFilters`, not campaign slots |
| +20h/+24h/+28h/+2Ch | 0/0/0/9 | `filterPlayerCount`, `filterFreeSlots`, `filterLatency`, `arraySize` in `007fdf00` |
| +30h | -1 | `007fdeaf`; overwritten with sum of mission-progress +18h counters at `007fed8c..007fed94`, not selected mission ID |
| +34h | empty | save/archive name, assigned by both `007ff100` and `007fa710` |
| +3Ch | `globals.newplayer` | player name; `007f9290` also updates `game+1FF0h` with display-name precedence |
| +48h/+4Ch | untouched | XUID written at `0067ccdd/0067cce0` after reset |
| +50h | empty | display name; both setters mirror this when its length is nonzero, otherwise +3Ch |
| +59h | 1 | `Voice`, serializer `007fdf99..007fdfb8` |
| +5Ch/+60h | 1/1 | `Difficulty`/`SelectedDifficulty`, `007fdfe2/007fe01b` |
| +64h | destroyed, reallocated as 24h | three-tree mission-progress object; allocator-null result remains null |
| +68h | empty | actual `SelectedMissionID`, `007fe0a9..007fe0c8` |
| +70h/+7Ch/+88h | empty | saved unlocks, pending unlocks, seen unlocks; uses existing unlock projection |
| +94h | empty tree | node key at +0Ch; three scalar words and a second string after the key, `007f89f0`; semantics unresolved |
| +A0h | cleared twice, `RANK=1` | `005070c0` is case-insensitive string/int map `operator[]`, returning node+14h |
| +ACh | empty string vector | serializer's `Bonus` section, `007fe19b..007fe258` |
| +CCh/+D8h | empty content IDs, zero 64-bit mask | `004d05e0` call at `007fdec2`; DLC rebuild in `007fae70` |
| +E0h/+E4h | 1/1 | `JapanNoseArt`/`AlliedNoseArt`, `007fe058/007fe098` |
| +E8h | untouched | `Version`, `007fdf66`; reset does not initialize it |
| +ECh/+F0h | 0/0 | `DropRateTC`/`DropRateDC`, `007fe10f/007fe15a` |

The reset also leaves byte +58h and the vector at +BCh alone. The projection
omits these unconsumed fields rather than inventing a reset or an element type.
Reset does **not** update the separate name buffer at game+1FF0h. Both name
setters choose the nonempty-header display name at +50h, falling back to +3Ch,
then copy until a null byte or 31 bytes. `007f92c7..007f92cf` and
`007f9377..007f937e` prove that changing the player name does not replace an
existing displayed alias, while clearing the display name reveals the player
name. The test is header length, so a nonempty display name beginning with NUL
does not fall back to the player name.

`00920e10` constructs three empty trees at +00h, +0Ch and +18h. The first has
large score records (node sentinel byte +29Dh); the other two have string/int
nodes (sentinel byte +19h). `007fd780` destroys them in reverse order, freeing
each sentinel, and returns at `007fd843`. Its exported listing originally ended
at `free` at `007fd7c5`; raw bytes prove fallthrough, another free at `007fd7f5`,
and the last free at `007fd824`. The primary agent was notified for repair.
The primary subsequently decoded the tail and cleared the three call overrides,
but the saved Ghidra function body still stops at `007fd7c9`; its export remains
incomplete. Raw-byte evidence, not a claimed repaired decompile, supports this
destructor's full extent.
The full score-record payload and native allocation remain required host
operations. The existing projected completion map is cleared by the reset code.

`007fee20` constructs the native strings and container sentinels then calls
reset at `007fef9f`. It never first initializes the +64h pointer, XUID or version.
Its caller therefore needs previously zeroed or otherwise valid storage. A
complete native constructor is not claimed or emulated by a blanket zero-fill.

The final settings calls are `008d4820` then `008d41c0`. The implementation
reuses their existing bodies, preserving control writes before querying Xenon
state/user, the short-circuited second query, and the conditional `008d45d0`
selected-user controls tail. It does not reset audio/video here.

## Read, discard, and write dispatch

`007ff100` is `__thiscall(profile, NativeString* name, callback)`, `RET 8`.
The decompiler incorrectly labels the callback as `unaff_retaddr`; assembly
loads the second real stack argument from `[ESP+18h]` after four register pushes.
It assigns profile+34h, then calls storage virtual +1Ch with `(name, 1)`:

- True: replace `00F87458`, call `00bd3d70(name,1)`, drive storage with `007fefe0` through
  `006adb50` (function address in ECX). The callback can run before this returns;
  an interactive prompt retains it. See `STORAGE_OPERATION_CONTINUATIONS.md`.
- False: reset the profile and invoke the argument callback if non-null. It
  does not clear or replace the global callback slot on this immediate arm.

`007fefe0` uses the global game profile. At storage+8 **exactly 1**, reset it,
take and clear `00F87458`, then invoke the captured callback. Every other state
creates a reader (`00b67980`, `004425c0`), deserializes through `007fdf00`, frees
and clears a non-null storage+30h buffer, closes the archive at storage+38h,
optionally calls manager `00F8A2FC` virtual +A0h, takes and clears the callback
slot, passes that callback to settings restore `008d7a50`, applies settings,
commits the profile through `007fae70`, and destroys the reader. The settings
backend owns the transferred callback; the projection does not invoke it early.
The full serializer, archive reader, save backend and settings restore remain
external contracts. The supplied storage state is not replaced with a guessed
success/error enum. Exception/SEH cleanup is outside this normal-flow projection.

`007fa710` is `__thiscall(profile, name, callback, char force)`, `RET 0Ch`.
It compares the **old** profile+34h and supplied name via `00449af0`, unless
forced. That helper first distinguishes zero/nonzero header lengths at
`00449af0..00449b1e`, then calls `__stricmp` at `00449b29`, stopping at embedded
NUL bytes in both nonempty strings. It always assigns the name and `00F87458`, even when
unchanged. Only force/name difference calls `00bd3dc0(name)` and drives storage with
`007fa670`. That callback and `007fa220` are reconstructed in `profile_write.cpp`;
the text writer and storage driver are bound in `profile_persistence.cpp`. No default completion is fabricated for the unchanged arm.

The earlier press-start public hook labels invert the two routes:
`0067ce4c` calls `007ff100` (read/restore), while `0067cea4` calls `007fa710`
(conditional write). The former's completion invokes the profile serializer
that **reads** values into the profile; the latter proceeds through the separate
settings/write sequence. Native-address API names here remove the ambiguity.

Existing sign-in `0067cc60` resets, sets both names, then stores XUID. Title init
resets at `004c9b00`. The profile-selection caller `00590e60` resets at
`00591042`, sets the name at `00591058`, optionally requests the write at
`005910fb`, then rebuilds the top-level menu. It is analyzed only; its UI flow
is not duplicated. The integrator can connect both existing reset host methods
to `reset_profile_007fdb20` and the two archive hooks to the new request functions.

## Definition and validation boundaries

| Routine | Original ABI | Recovered extent/status |
| --- | --- | --- |
| 007fdb20 | ECX profile, no args, RET | field/reset projection implemented |
| 007f9290 / 007f9340 | ECX profile, string pointer, RET 4 | setters and 31-byte external name mirror implemented |
| 007ff100 | ECX profile, name, callback, RET 8 | request dispatcher implemented |
| 007fefe0 | no arguments, RET | full raw normal-flow callback implemented; no Ghidra function at analysis time |
| 007fa710 | ECX profile, name, callback, force, RET C | conditional write request implemented |
| 007fee20 / 00920e10 | ECX object, return object EAX, RET | native constructors analyzed only |
| 007fd780 | ECX object, RET | destructor analyzed through 007fd843; primary owns tail repair |
| 00590e60 / 0067cc60 | ECX screen, RET | caller evidence; sign-in already reconstructed |
| 005070c0 | ECX map, key pointer, returns int pointer, RET 4 | library operator[] analyzed; use existing C++ map |

Newly named undefined start `007fefe0` has inclusive final instruction
`007ff0f5: RET`, length 1, hence inclusive end `007ff0f5`. Its early return is
`007ff044`. Separately, analyzed callback `007fa670` is also undefined in Ghidra:
last instruction `007fa70c: RET`, length 1, inclusive end `007fa70c`; early return
`007fa6c5`. No guessed function body was installed in Ghidra by this worker.

Validation results are recorded in `reports/game_profile_reset.json`. Build and
fixture checks concern these C++ interfaces, not native ABI, real save handling,
or game startup/runtime validation.
