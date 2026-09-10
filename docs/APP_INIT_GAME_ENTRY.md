# Application initialize, game entry tail (0073e474 - 0073e52c)

Addresses: 004e5540, 008d5b50, 007372a0, 004dd5b0, 004c9a70, 004e3aa0, 004dc200, 008fafc0,
004c88a0, 004d3ed0, 004f8af0, 004f8970, 006851e0, 004ceb40, 008d5430, 00737830

Packet `app_init_game_entry`. This covers the last stretch of `cSkeletonAppMidway::Init`
(`0073d410`), from the five-script datatable preload to the closing `RET 0x8`, and the game
startup-mode entry it calls. The flow break at `0073e466` is repaired, so the tail is real code and
not the unreachable region the earlier decompilation suggested.

## The tail in order

`EBP` in the tail is the application object. It is `[ESP+0x150]` on entry, that is stack argument 2
of `0073d410`, but it is reloaded at `0073dbcd` from a local, and at `0073e1af` the newly built game
object is stored into `EBP+0x14`. `docs/APP_RUN_FRAME.md` calls the same field `application+14h` and
passes it in ECX to `BSP_Game_OnMove`, so the two documents describe one object. WinMain
(`008f81f0`) constructs it on its own stack at `008f8419` and calls `Init(this, 0, "cachedload")`
at `008f8429`; the literal `cachedload` at `00ce8168` is stack argument 2 and is consumed early,
not in this tail.

| Address | Step |
| --- | --- |
| `0073e45c` | `if (00e1ae78) { free(00e1ae78); 00e1ae78 = 0; }`, the repaired flow break |
| `0073e474` | `ECX = [EBP+0x14]`, the game object, then `CALL 004e5540` |
| `0073e47c` | `ECX = 0x00f88980`, the settings object |
| `0073e481` | `[EBP+0x4] = 1`, an application byte set once the game is up |
| `0073e485` | `CALL 008d5b50`, apply the whole settings object |
| `0073e48a` | `operator new(0x23c)`, then `00757fe0` on it when non-null, EH state 0x2f |
| `0073e4b1` | The 0x0c object built at `0073d467` by `00be2900`: virtual slot `+4h` with argument 1 |
| `0073e4c9` | `operator new(0xc)`, EH state 0x30 |
| `0073e4e8` | `CALL 007372a0`, then the derived constructor inlined at `0073e4ef` |
| `0073e501` | `ECX = 0x00cff168`, `CALL 004c9c90`, the `End of cSkeletonAppMidway::Init` checkpoint |
| `0073e52a` | `RET 0x8` |

The 0x23c allocation is stored only in the EH cleanup slot and never read again, so `00757fe0`
keeps the object itself. The 0x0c object from the start of `Init` receives a single virtual call
with argument 1 at the end, the shape of a scoped progress object being closed.

## Calling conventions and RET sizes

| Address | Convention | RET |
| --- | --- | --- |
| `0073d410` | `__thiscall`, ECX = application, two stack arguments | `RET 0x8` |
| `004e5540` | `__thiscall`, ECX = game, no stack arguments, void | `RET` |
| `004dd5b0` | `__thiscall`, ECX = game, one stack argument (a `char`) | `RET 0x4` |
| `004c9a70` | `__thiscall`, ECX = game, no stack arguments, void | `RET` |
| `004e3aa0` | `__thiscall`, ECX = game, no stack arguments, void | `RET` |
| `004c88a0` | `__thiscall`, ECX = game, no stack arguments, void | `RET` |
| `004d3ed0` | `__thiscall`, ECX = `game+0x5d8`, one stack argument | `RET 0x4` |
| `004dc200` | no arguments, returns a singleton in EAX | `RET` |
| `008d5b50` | `__thiscall`, ECX = `0x00f88980`, no stack arguments, void | `RET` |
| `008d5430` | `__thiscall`, ECX = `0x00f88980`, no stack arguments, void | `RET` |
| `007372a0` | `__thiscall`, ECX = a 0x0c allocation, returns it in EAX | `RET` |
| `00737830` | `__thiscall`, ECX = a 0x0c allocation, returns it in EAX | `RET` |

Ghidra prints `0073d410` as `__thiscall(int, undefined4)`, which is one stack argument short of the
`RET 0x8`. The second argument is the one the tail uses.

## 004e5540, the game startup-mode entry

The ledger name is now `BSP_Game_BeginStartupSequence`. The earlier provisional name
`BSP_Game_OnInit` was wrong: the literal `GGame::OnInit` belongs to `004e3aa0`, and `004e5540` only
selects which of the three init routines run.

1. `004e555e` stores the address of `004ceb40` into `00f8abec`.
2. `004e556a` `operator new(0x34)`, constructor `004f8af0` into `00e18d48`, then virtual slot
   `+10h` on it. `004f8af0` chains base constructor `004f7180` and installs two vtables
   (`00ceae94` at `+0`, `00ceae7c` at `+8`), so the class uses multiple inheritance. A failed
   allocation stores null and the virtual call still runs.
3. `004e55a5` `004f8970` with `ECX = 00e18d48` and the callback `004f89d0`.
4. `004e55af` reads `0109cee8`. Nonzero clears `00e188ac` and skips the whole command-line scan.
5. Otherwise seven `strstr` tests run against the command line at `game+0x7168`, in this order:

   | Literal | Address | Effect |
   | --- | --- | --- |
   | `skipLogos` | `00ce7fe4` | `00e188ac = 1` |
   | `noskipLogos` | `00ce7fd8` | `00e188ac = 0` |
   | `skipTitle` | `00ce7fcc` | `00e198cc = 1` |
   | `skipBriefings` | `00ce7fbc` | `00e18d91 = 1` |
   | `lockitMark` | `00ce7fb0` | `00f8bc50 = 1` |
   | `lockitRaw` | `00ce7fa4` | `00f8bc50 = 2` |
   | `skipBriefings` | `00ce7fbc` | `00e18d91 = 1`, a duplicate of the fourth test |
   | `.scn` | `00ce7888` | `00e198cc = 1`, `00e188ac = 1`, take the scenario branch |

   Each test is `cmd != 0 && strstr(cmd, lit) != 0 && (strstr(cmd, lit) - cmd) != -1`. The third
   term cannot be false for a non-null result; it is compiled dead code, kept in the
   reconstruction so the guard order stays visible.
6. `004e5748`, reached from `.scn` or from a set `00e188ac`:
   `004dd5b0(game, 1)`, `004c9a70(game)`, then `p = 004dc200(); 008fafc0(p)`.
7. On the `.scn` branch: `004e3aa0(game)`, `004c88a0(game)`, `game+0x5d4 = 10`, and
   `004d3ed0(game+0x5d8, &10)`. Otherwise `game+0x5d4 = 2`.
8. `004e57bc`, the default boot: `game+0x5d4 = 1`, then `operator new(0x80)`, constructor
   `006851e0` into `00e198a4`, then virtual slot `+4h`.

## The OnInitOnce / OnInitTitle / OnInit split

The three literals are scope labels and each appears in exactly one function.

`004dd5b0` `GGame::OnInitOnce(char first_time)`. With `first_time == 0` it only re-runs
`004c9a70`, `004dc200` and `008fafc0`, so it can be called again for a soft restart. With
`first_time != 0` it opens the named block and, in order: `00884be0` runs
`Scripts\fundamentals.lua` and defines `PC=true`; `00b65e20` registers `DoFile` on the Lua state
reached through `***(game+0x1a08)`; `game+0x1a1c` receives `004c6880`; `00b69d40` runs
`Scripts\global\luaMW_init.lua`; `008d44c0(00f88a30)` sets the `X360COMP` Lua global;
`00a917e0(0,1,0)`, `(1,1,0)` and `(2,1,0)` bring up three input slots; `00aa6be0`, `00a9ac60`,
`004c1b90`, `00940ec0`, `00a9ac80` and `00aab5e0` follow; the GUI manager (`004c12b0`) receives
`00696590` at `+4` and `00f8bbfc` receives `006965a0`; `00aa2110(00f88a31)` runs; when `game+0x55c`
is zero `00698a10` loads `Scripts\datatables\Inputs.lua` with the `groups`, `inputs`, `press`,
`InvertPlaneY`, `InvertCameraY`, `SwapStickMap`, `SwapStickGeneral`, `SwapStickPairs` and
`InputModifiers` tables; finally virtual `+10h` on `00425d10()` and on `004c1ac0()`, then
`004c1c50()`. This is the only Lua bring-up in the tail; the five datatable scripts that
`docs/STARTUP_SCRIPT_PRELOAD.md` describes are cached before `004e5540` runs, not executed here.

`004c9a70` `GGame::OnInitTitle`. Opens the named block, runs `007fdb20`, writes `game+0x5d4 = 2`,
loads the atlas `interface/textures/allbutingame.ats`, calls `004c1ac0(0,1)` and `00518250(0,1)`,
lazily builds the 0x4c singleton at `00e198bc` (`00689d90`, virtual `+4h`) and, when `00e198cc` is
clear, the 0x44 singleton at `00e198c8` (`0068d760`, virtual `+4h`); with `skipTitle` set it calls
`0068d8a0` instead. It then runs `00916980` when `*(00e188a8+0x21a0)` is nonzero, clears
`00e198b0`, and when `0067c8f0()` is false runs `00a40020` and `0067c970`. It closes by calling
virtual `+1ch` on `00e18d48` if that object's byte at `+5` is set, then clears its `+4` and `+5`
and runs `004f83b0`.

`004e3aa0` `GGame::OnInit`. Writes `game+0x5d4 = 3`, releases `00e198bc` through its virtual `+0`,
and behind the once-guard at `game+0x719c` preloads the effect roots `Particles\Textures\`,
`Effects\Flares\Textures\`, `Effects\Traceline\` and `Effects\Numbers\` through `00aeff40`, loads
the atlas `interface/textures/common.ats`, allocates the object at `game+0x21d8` through
`006afe80`, and runs roughly thirty further subsystem calls. It is reached from `004e5540` only on
the `.scn` branch.

## Game state and what RunFrame sees

`game+0x5d4` is the state `docs/APP_RUN_FRAME.md` reads as `*(00e188a8)+5D4h`. `00e188a8` is a
mirror of `application+14h`.

| Value | Written at | Meaning |
| --- | --- | --- |
| 1 | `004e57c1` | Logo sequence; the 0x80 object at `00e198a4` plays it |
| 2 | `004c9a70` and `004e5819` | Title screen / front end |
| 3 | `004e3aa0` entry | Mission init in progress, always overwritten before Init returns |
| 10 | `004e578c` | Direct scenario load from a `.scn` command line |

So the frame loop's first pass sees 1, 2 or 10 and never 3. That corrects the reading in
`include/bsp/app_frame.hpp`: `is_mission_game_state` returns true for 1, 2 and 4, and the frame
loop runs its input-action test only when the predicate is false. Since 1 is the logo sequence and
2 is the title screen, that set is the front-end set and not the mission set, and the predicate is
misnamed. `app_frame.cpp` belongs to another packet, so nothing was changed there.

`game+0x5d8` is a ring buffer: `+4` bucket array, `+8` capacity, `+0xc` head, `+0x10` count, 0x10
bytes per element, grown through `004d20d0(1)`. `004d3ed0` pushes and `004c88a0` drains. The
scenario branch drains first and then pushes 10, so the queued request and `game+0x5d4` agree.

## 008d5b50, apply settings

The ledger name is now `BSP_Settings_ApplyAll`; the earlier `BSP_PostEffectSystem_Initialize` is
superseded. ECX is the fixed settings object at `0x00f88980`. There are ten call sites and eight of
them are in the options-menu segment 23 (`005efcf0`, `005f0b90`, `005f5ac0`, `005f5e60`,
`005f65c0`, `005f7c40`, `005f8960`) plus `004de610` and `0067cfb0`, which is why the routine reads
as "apply the current settings", not as one-time initialisation.

In order: `008d5430` applies audio (`settings+0x24` to `00f8bbd8+0x70`, `settings+0x20` through
`00a7a440`, `settings+0x28`/`0x30` into `00f8bbcc+0x218`/`0x21c`, and bus volumes for `Warnings`,
`GUIMusic` and `GUITestSpeech`); `00b1ffb0` receives `2 - settings+0x68`; virtual `+0xf0` on
`00f8d394` receives the float at `settings+0x64`; `004b46e0` receives the four detail bytes at
`settings+0x41..0x44` with `ECX = 00e188a8`; `004dcdf0` follows. It then pushes `settings+0x54`
into the world object at `game+0x19fc`, `settings+0x6c` into `game+0x19f0`, `settings+9` into
`004c1710()+0x14`, reloads `Fonts\` and the lockit tables for the language named by the 0x20-stride
table at `00f88974` indexed by `settings+4`, loads the `globals` table through `00aa0d30`, latches
`settings+0x8d`, `+0x70` and `+0x84` into `00f88a44`, `00f88a40` and `00f88a3d` under the bit flags
in `00f88a48`, refreshes `FoliageGroup`, and finally calls
`BSP_D3D9Renderer_ChangePresentationMode` with `settings+0x14`, `+0x18`, `+0x1e`, `+0x58` and
`+0x60`, virtual `+0xc` on `0109cf04`, `XLiveOnResetDevice`, `00b107f0`, `00b0d080` and a copy of
the string `00439100()` returns.

The four blocks Ghidra removes as unreachable were checked in the listing:

| Block | Content |
| --- | --- |
| `008d5cf8` | `memcpy(buffer, "Fonts\", len+1)` from `00cfefa8` |
| `008d5d34` | Storage-pool release of the language-name string |
| `008d5db2` | Storage-pool release of the lockit-table name string |
| `008d5e64` | Storage-pool release of the `globals` string |

All four are string housekeeping, so no control flow or subsystem call is lost. The `unaff_ESI` the
pseudocode reports is the same `this` pointer: `008d5b6a` sets `ESI = ECX` and never reassigns it.
The pseudocode's closing `ExceptionList = local_3c` is a decompiler artifact of the same reuse.

## 007372a0, the closing lifetime registration

`007372a0` is the base constructor of the last object `Init` builds. It stores the base vtable
`00cfea68`, takes the critical section at `manager+0x10` from `BSP_SingletonLifetime_GetManager`
(`00415350`) and bumps the recursion counter at `+0x18`, publishes the object to `00f88c20`,
registers it through `BSP_SingletonLifetime_Register` (`00bd0c30`), and leaves the section. The
lock is skipped entirely when `manager+0x10` is null. `00736f70` is the matching destructor: it
unregisters, clears `00f88c20` and rebinds the vptr to `00ce3818`.

The caller then completes construction inline: vptr `00cfea98`, a zero byte at `+4` and a zero
float at `+8`. That is byte-for-byte the standalone constructor at `00737830`, so the object is
0x0c bytes with one bool and one float.

The `00cfea98` vtable is six slots, decoded from disk bytes because Ghidra has no functions there:

| Slot | Target | Body |
| --- | --- | --- |
| `+0` | `007378b0` | scalar deleting destructor |
| `+4` | `00737850` | `RET` |
| `+8` | `00737860` | `RET 4` |
| `+0xc` | `00737870` | `fld [esp+4]; push ecx; fstp [esp]; call 008e2560; ret 4` |
| `+0x10` | `00737880` | `RET` |
| `+0x14` | `00737890` | `RET` |

Every override but one is empty, so this is a default implementation of a six-slot interface.
`BSP_Game_Render` (`004ca472`) calls slot `+0xc` with the float at `game+0x21f0` and then slot
`+4`, once per frame, which is why the name recorded is `BSP_FrameHook_ConstructAndRegister`.
`008e2560` forwards that float to `0070aa70` three times and then calls `0070a710` three times;
neither has a string or a second caller, so the subsystem behind it is unidentified. The base
vtable at `00cfea68` has eight slots with purecalls at `+0xc` and `+0x1c`; the derived table sits
immediately before the application vtable at `00cfeab0`, so both classes come from the same
translation unit.

## Callers and callees

`004e5540` has one caller, `0073d410`. `008d5b50` has ten, listed above. `007372a0` has one,
`0073d410`; its sibling `00737830` has none recorded and is reached only through a vtable.

Direct callees of `004e5540`: `004c88a0`, `004c9a70`, `004d3ed0`, `004dc200`, `004dd5b0`,
`004e3aa0`, `004f8970`, `004f8af0`, `006851e0`, `008fafc0`, `00bf681b` (`operator new`),
`00bf9440` (`strstr`).

## Reconstruction

`include/bsp/game_entry.hpp` and `src/game_entry.cpp` reconstruct two things:

- `game_on_init(GameStartupSystems&, GameStartupFlags&)` for `004e5540`. The eleven native call
  sites the routine reaches are declared on `GameStartupSystems` in issue order; none has a default
  implementation. The function returns the `GameStartupState` it writes to `game+0x5d4`. The scan
  order, the duplicated `skipBriefings` test, the dead offset comparison and the `0109cee8`
  override are all preserved.
- `frame_hook_construct(FrameHook&, SingletonLifetimeManager&, FrameHook**)` for `007372a0` plus
  the constructor the caller inlines. The lock, the publish to `00f88c20` and the register call
  keep their native order.

`008d5b50` is not reconstructed. It drives D3D9 presentation mode, XLive and the localization
tables directly, and those interfaces belong to other packets.

## Uncertainties and what remains

- `application+4`, set to 1 at `0073e481`, has no recovered reader. It is modelled nowhere.
- The subsystem behind the `00f88c20` frame hook is unknown. `008e2560` and its two callees carry
  no strings and no other callers.
- `004f8af0` (0x34, `00e18d48`), `006851e0` (0x80, `00e198a4`), `00689d90` (0x4c, `00e198bc`) and
  `0068d760` (0x44, `00e198c8`) are identified only by allocation size, vtable and the branch that
  builds them. `BSP_LogoSequence_Construct` is named from its branch, not from a string.
- `004dc200` is a lifetime-managed singleton getter guarded by `00f8998c`; neither it nor the
  method `008fafc0` that follows it was identified, so neither is named.
- `004e3aa0` was read end to end but its thirty-odd subsystem calls were not opened. Its
  pseudocode carries `unaff_EBX`, `unaff_ESI` and `unaff_EDI` from string-temporary reuse; the
  string literals and the once-guard were confirmed in the listing, the rest was not.
- The `is_mission_game_state` predicate in `include/bsp/app_frame.hpp` is misnamed, as described
  above. It belongs to another packet and was left alone.
- Ghidra was read-only for this packet. No renames, comments or prototypes were applied; the
  ledger names below are records only.

## State reached

| Address | State |
| --- | --- |
| `004e5540` | reconstructed, build-tested, fixture-tested |
| `007372a0` | reconstructed, build-tested |
| `00737830` | analyzed (disk bytes decoded; no Ghidra function exists) |
| `008d5b50` | analyzed (pseudocode and full listing) |
| `008d5430` | analyzed |
| `004dd5b0` | analyzed |
| `004c9a70` | analyzed |
| `004e3aa0` | analyzed, partial |
| `004c88a0` | analyzed |
| `004d3ed0` | analyzed |
| `006851e0` | analyzed |
| `004f8af0` | exported |
| `004dc200` | exported |
| `008fafc0` | exported |
| `004ceb40` | exported |
| `004f8970` | exported |

## Names recorded

| Address | Name | Note |
| --- | --- | --- |
| `004e5540` | `BSP_Game_BeginStartupSequence` | replaces `BSP_Game_OnInit` |
| `004e3aa0` | `BSP_Game_OnInit` | string-proven scope label |
| `004dd5b0` | `BSP_Game_OnInitOnce` | string-proven scope label |
| `004c9a70` | `BSP_Game_OnInitTitle` | string-proven scope label |
| `008d5b50` | `BSP_Settings_ApplyAll` | replaces `BSP_PostEffectSystem_Initialize` |
| `008d5430` | `BSP_Settings_ApplyAudio` | new |
| `007372a0` | `BSP_FrameHook_ConstructAndRegister` | new |
| `00737830` | `BSP_FrameHook_Construct` | new |
| `004d3ed0` | `BSP_Game_EnqueueStateRequest` | new |
| `004c88a0` | `BSP_Game_DrainStateRequests` | new |
| `006851e0` | `BSP_LogoSequence_Construct` | new, provisional |
