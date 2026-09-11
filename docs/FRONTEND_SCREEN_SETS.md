# The front-end screen sets (packet `front_end_screen_sets`)

Addresses: 004f8710, 004d8c00, 004f83b0, 00687330, 004f7620, 004f8530, 004f85d0, 004f8670,
004f87b0, 004f7210, 004f81d0, 004f7e80, 004f8480, 004c4300, 004d6410, 004d6840, 004d3740, 00a92290,
00a933f0, 00685820, 00687800, 00689820, 00683aa0, 004da780.

Every front-end manager publishes what it wants on screen through two `__cdecl` varargs calls,
`004F8710(ids..., 0)` and `004D8C00(game, ids..., 0)`. Neither call touches a screen directly.
Each one replaces one level of a small layered stack and then re-derives, from all levels at
once, which of the 95 registered screens is requested. `docs/GAME_FRONTEND_STATES.md` covers what
happens afterwards: the pump `004F8830` reconciles each screen's applied byte `+5h` with the
requested byte `+4h` this packet writes.

## `004F8710`, the level-4 screen set

`__cdecl(int id, ...)`, RET, no ECX, SEH handler `00C68558`. The list is terminated by a zero id
and the terminator is not stored. From the listing:

```
004f8737  EAX = [ESP+28h]          ; the first vararg
004f8745  if (EAX == 0) skip the loop
004f8748  ESI = &[ESP+28h]         ; the argument list
004f8750  push &idTemp; ECX = &localVector; call 004f8480   ; vector<int>::push_back
004f875e  EAX = [ESI+4]; ESI += 4; idTemp = EAX; loop while EAX != 0
004f876d  push &localVector; ECX = 00E18D28; call 004f81d0  ; vector<int>::operator=
004f8777  byte [00E18CDC] = 1
004f8783  call 004f7620
004f8788  free(localVector._Myfirst) if non-null
```

The local container is the 16-byte MSVC `_SECURE_SCL` `std::vector<int>`: a 4-byte iterator-debug
head at `+0h`, then `_Myfirst`/`_Mylast`/`_Myend` at `+4h`/`+8h`/`+0Ch`. Only the last three are
zeroed on entry, and the freed pointer is the one at `+4h`. `004F7210` is that vector's `size()`
(`__thiscall`, RET), `004F8480` its `push_back` and `004F81D0` its `operator=`; when the source is
empty `004F81D0` falls through to `004F7E80`, which is `erase(begin, end)`. **An empty list
therefore clears the level.** That is exactly what `00683AA0` (deactivate) and the default arms of
two `+10h` overrides use it for.

`004F8710` is one of five identical setters, one per level. The levels are five consecutive
16-byte vectors and the recompute walks them from the top down:

| Setter | Level | Vector |
| --- | --- | --- |
| 004F8530 | 1 | 00E18CF8 |
| 004F85D0 | 2 | 00E18D08 |
| 004F8670 | 3 | 00E18D18 |
| **004F8710** | **4** | **00E18D28** |
| 004F87B0 | 5 | 00E18D38 |

Each of the five loads its own `ECX` (`004F8592`, `004F8632`, `004F86D2`, `004F8772`, `004F87F6`),
sets the same byte `00E18CDC` and tail-calls the same `004F7620`. The level is not an argument; it
is baked into the entry point. Every manager calls the level-4 form only.

## `004F7620`, the recompute

`__cdecl(void)`, RET, 60h bytes of frame. This is where a list of ids becomes a set of requested
screens, and it is the only writer of the `+4h` byte outside the screens themselves.

```
004f7627  memset(bitmap[5Fh], 0, 5Fh)       ; one byte per registry slot
004f7638  [00E08310] = 1                    ; the occlusion floor
004f7642  EBX = 5; ESI = 00E18D38
004f7650  for each id in level EBX:
004f767e    screen = [00E18B60 + id*4]; skip if null
004f7689    if (EBX > [00E08310] && screen->vtable[8]())  [00E08310] = EBX
004f76a2    bitmap[id] |= (EBX >= [00E08310])
004f76bf  ESI -= 10h; EBX -= 1; loop while ESI >= 00E18CF8
004f76d0  for slot 0..5Eh: if (screen && !screen->vtable[4]() && !bitmap[slot]) screen->+4h = 0
004f7700  for slot 0..5Eh: if (screen && !screen->vtable[4]() &&  bitmap[slot]) screen->+4h = 1
```

Three recovered rules:

- **Layering with occlusion.** The walk starts at level 5. The first listed screen in a level whose
  virtual `+8h` returns true raises the floor `00E08310` to that level, and from then on every id in
  a lower level fails `level >= floor` and is not requested. The guard `EBX > [00E08310]` means the
  predicate is asked only while the level can still raise the floor, so within one level the first
  opaque screen decides and the rest are not consulted.
- **Exemption.** A screen whose virtual `+4h` returns true is skipped by both passes, so neither the
  clear nor the set touches its `+4h` byte. Such a screen owns its own visibility.
- **Clear before set.** The two passes are separate full sweeps of all 95 slots, not one sweep with
  two arms. Both are guarded by the same `+4h` predicate and read the same bitmap, so the split is
  only observable through the `+4h` virtual, which is called twice per live slot.

The base vtable `00CEAE54` supplies `004F7570` for `+4h` and `004F7580` for `+8h`; both are
`xor al,al; ret`. So by default nothing is exempt and nothing occludes, and a screen opts in by
overriding. **Which leaf types override them was not established** and is the main gap here.

`00E08310` outlives the call: `BSP_Game_Render` compares it against 1 at `004CA521` before taking a
branch, and `005B5757` reads it too. Level 1 means "nothing is covering the scene".

## `004D8C00`, the level-4 input-context set

`__cdecl(void* game, int id, ...)`, RET, SEH handler `00C66568`. Same varargs shape with the game
object as an extra leading argument; the caller pops 0Ch for the two-id form. It is the fourth of
another family of four, this one on the game object `00E188A8`:

| Setter | Level | Vector |
| --- | --- | --- |
| 004D8A50 | 1 | game+570h |
| 004D8AE0 | 2 | game+580h |
| 004D8B70 | 3 | game+590h |
| **004D8C00** | **4** | **game+5A0h** |

```
004d8c5c  ESI = [ESP+2Ch]              ; the game object
004d8c60  push &localVector; ECX = ESI+5A0h; call 004d3740   ; vector<int>::operator=
004d8c70  push 4; ECX = ESI; call 004c4300
004d8c79  EDX = 4; EAX = ESI+5A4h
004d8c84  walk down by 10h while the level's vector is empty and EDX > 1
```

That last walk finds the topmost non-empty level and then **discards the result**; nothing after
`004D8C9F` reads `EDX` or `EAX`. It reads as an inlined accessor whose value the source dropped.
It is not reproduced in the reconstruction.

The three lower setters do not inline any of that. Each builds the same local vector and then calls
one shared routine, `004D6410(level, &list)`, with 1, 2 and 3 at `004D8AB6`, `004D8B46` and
`004D8BD6`. `004D6410` is `__thiscall(game, int level, vector* ids)` and its body is
`ECX = (level + 56h) * 10h + game; 004D3740(ECX, ids); 004C4300(game, level);` followed by the same
dead backwards scan from `game+5A4h`. So `004D8C00` is `004D6410` inlined for level 4, and the
level-to-offset mapping is recovered twice over.

`004C4300` is `__thiscall(game, int level)`, RET 4, and it addresses the same table as
`this + (level + 56h) * 10h`, which is what fixes the level-to-offset mapping above. For `level`
down to 1 it clears every input context currently sitting at that level and then raises each context
listed at that level:

```
004c4310  manager = 004BEC00()                       ; the input manager singleton
004c4318  for id in 1..19h: if 00a92290(id) == level then 00a933f0(id, 0)
004c4340  for each id in this-level's vector: if 00a92290(id) < level then 00a933f0(id, level)
004c4380  level -= 1; table -= 10h; loop while level >= 1
```

`00A92290` is `get([manager+10h][id])` and `00A933F0` is `set([manager+10h][id]) = level` plus a
rescan that refreshes the cached maximum at `manager+20h`. **So the ids in a `004D8C00` list are
input-context indices in the range 1..19h, not screen registry slots.** The two varargs calls carry
two different id spaces, which the existing `FrontEndScreenSetHost` in `include/bsp/frontend_managers.hpp`
does not distinguish; its comment calling both "interface" sets should be corrected.

Every manager call site passes the single id `1`, so in the front end this call reduces to "give
input context 1 priority level 4". `00683AA0` passes the bare terminator, which clears the level.

## The call sites

`004F8710` and `004D8C00` have exactly five callers each, and they are the same five.

| Caller | Screen-set call | Input-context call |
| --- | --- | --- |
| 00683AA0 `BSP_FrontEndManager_Deactivate` | `004F8710(0)` at 00683AA6 | `004D8C00(game, 0)` at 00683AB3 |
| 00685820 `BSP_MainMenu_ApplyPendingInterface` | `004F8710(screen, 0)` | `004D8C00(game, 1, 0)` |
| 00687800 `BSP_MultiMenu_ApplyPendingInterface` | `004F8710(00687330(id), 0)` at 0068783E | `004D8C00(game, 1, 0)` at 0068784E |
| 00689820 `BSP_OptionsMenu_ApplyPendingInterface` | `004F8710(screen, 0)` | `004D8C00(game, 1, 0)` |
| 004DA780 | `004F8710(0)` at 004DA9CA | `004D8C00(game, 0)` at 004DA9B2 |

`004DA780` is the interesting one: at `004DA99D..004DA9CA` it calls all eight setters of both
families in sequence with an empty list each (`add esp, 30h` afterwards covers four two-argument
and four one-argument `__cdecl` calls), so it tears the whole layered state down, not just level 4.

The three `+10h` overrides share one shape: call `00684600` and return false if it rejected the
request, map the interface id to a screen id, publish the one-element screen set, publish the
one-element input-context set `{1}`, return true in AL, `ret 8`. The mapping differs per manager.

**Main menu, `00685820`.** `lea eax,[esi-1]; cmp eax,0Ah; ja 00685938; jmp [eax*4+00685948]`, an
eleven-entry table for interface ids 1..0Bh. Every arm pushes the id unchanged, so the map is the
identity over `INTF_MAINMENU`..`INTF_ACHIEVEMENTS`. The default arm at `00685938` pushes only the
terminator, calls `004F8710` and **skips `004D8C00` entirely**, then still returns true.

**Options menu, `00689820`.** A three-way compare chain, not a table: 0Ch (`INTF_OPTIONS`) to
screen 0Dh, 0Dh (`INTF_CONTROLLAYOUT`) to 0Eh, 0Eh (`INTF_KEYBOARDSETUP`) to 0Fh. The default arm
at `0068984B` is the same bare-terminator call. Note the push order at `0068983D`: the terminator
is pushed before the compare chain, so every arm pushes only its id, which is what a right-to-left
`__cdecl` list looks like when the tail is constant.

**Multi menu, `00687800`.** It first prints the interface name (`[EDI*4 + 00E08CD8]` into the
formatter `004254B0`), then calls `00684600`, then `00687330` for the mapping.

## `00687330`, the multi-menu id map

`RET 4` with the id at `[ESP+4]`. The caller at `00687838` also loads `ECX` with the manager, so
this is more likely a `__thiscall` member that ignores `this` than a true `__stdcall`;
`docs/FRONTEND_MANAGERS.md` records it as `__stdcall(int id)`, which is indistinguishable from the
callee side. The body is `EAX = id - 3; if (EAX > 1Ch) return 0; jmp [EAX*4 + 006873EC]`, so the
table covers ids 3..1Fh and every arm is a two-instruction `mov eax, imm; ret 4`.

Table read back from `006873EC` (1Dh dwords) with the arm each entry reaches, and the interface
names from `00E08CD8`:

| Interface id | Name | Arm | Screen id |
| --- | --- | --- | --- |
| 03h | INTF_BRIEFING | 006873BF | 03h |
| 04h | INTF_REWARDS | 006873C7 | 04h |
| 05h..0Ah | INTF_UNITLIB .. INTF_LEADERBOARDS | 006873E7 | 0 |
| 0Bh | INTF_ACHIEVEMENTS | 006873DF | 0Bh |
| 0Ch..0Eh | INTF_OPTIONS .. INTF_KEYBOARDSETUP | 006873E7 | 0 |
| 0Fh | INTF_MULTIMAINMENU | 00687347 | 10h |
| 10h | INTF_MULTIMODESELECTOR | 0068734F | 11h |
| 11h | INTF_MULTILIVERANKEDSELECTOR | 00687357 | 12h |
| 12h | INTF_MULTILAN | 0068735F | 13h |
| 13h | INTF_MULTICREATELANSERVER | 00687367 | 14h |
| 14h | INTF_MULTIGAMELOBBY | 0068736F | 16h |
| 15h | INTF_MULTISELECTPLAYER | 00687377 | 17h |
| 16h | INTF_MULTIPLAYERS | 0068737F | 18h |
| 17h | INTF_MULTIINGAME | 00687387 | 19h |
| 18h | INTF_MULTIFRIENDS | 006873AF | 1Ah |
| 19h | INTF_MULTISIGNIN | 0068738F | 1Bh |
| 1Ah | INTF_MULTISESSIONBROWSER | 0068739F | 1Ch |
| 1Bh | INTF_MULTIERROR | 006873B7 | 1Dh |
| 1Ch | INTF_MULTICREATENEWACCOUNTPC | 00687397 | 1Eh |
| 1Dh | INTF_MULTISESSIONFILTER | 006873A7 | 1Fh |
| 1Eh | INTF_MULTITACTSHIT | 006873CF | 54h |
| 1Fh | INTF_MULTIPOINTSSHIT | 006873D7 | 5Eh |

Every unmapped id returns 0, and `004F8710(0)` is the empty list, so an interface the multi menu
does not own hides everything rather than leaving the previous screen up.

Putting the three managers together gives the screen-id space of the registry:

| Screen ids | Owner |
| --- | --- |
| 01h..0Bh | main menu, identity with the interface id |
| 0Ch | **no manager maps to it** |
| 0Dh..0Fh | options menu, interface id + 1 |
| 10h..14h | multi menu, interface ids 0Fh..13h |
| 15h | **no manager maps to it** |
| 16h..1Fh | multi menu, interface ids 14h..1Dh |
| 54h, 5Eh | multi menu, the two `SHIT`-suffixed debug interfaces |

The two holes are why the multi menu's map is a table and not an offset. The 95-slot registry has
room for 5Fh ids and the highest one any manager reaches is 5Eh, the last valid slot.

## `004F83B0`, the per-screen commit

`__thiscall(screen)`, RET, SEH handler `00C684D8`. `config/names/004f0000.jsonl` already carried
this behaviour under the name `BSP_FrontEndScreen_n`; this packet confirms it and supersedes the
name. The screen's virtual `+24h` fills a heap vector of child pointers, and each non-null child
gets its own virtual `+34h` called with the byte at `screen+5h`, the **applied** state, not the
requested one. Then the vector is freed. The interleaved `00BF6713` calls are `_SECURE_SCL` range
assertions. The base `+24h` at `004F75D0` is `ret 4`, so a screen with no children commits nothing.

It has 35 callers, which is why it is not reconstructed as part of the set operation: the pump
`004F8830` calls it on both the exit and the enter pass, `BSP_FrontEndScreen_Close` (004B6E50)
tail-jumps to it, and a dozen individual screens call it directly after writing their own bytes.
This packet models it as one function over an injected host and leaves the call sites alone.

## The whole sequence

1. A manager's `+10h` override accepts a request through `00684600`.
2. It maps the interface id to a screen id (identity, +1, or the `00687330` table).
3. `004F8710(screen, 0)` replaces level 4 of `00E18CF8..00E18D38` and raises `00E18CDC`.
4. `004F7620` re-derives the `+4h` byte of all 95 registered screens from the five levels, applying
   the occlusion floor `00E08310` and skipping self-managed screens.
5. `004D8C00(game, 1, 0)` replaces level 4 of `game+560h + level*10h` and `004C4300` pushes the
   resulting input-context priorities into the input manager.
6. On the next front-end frame the pump `004F8830` walks the registry, calls the exit and enter
   virtuals where `+5h` disagrees with `+4h`, and calls `004F83B0` to push the new applied byte down
   to each screen's GUI children.

## Calling conventions and RET sizes

| Address | Convention | Stack args | RET |
| --- | --- | --- | --- |
| 004F8710, 004F8530, 004F85D0, 004F8670, 004F87B0 | `__cdecl` varargs | `id..., 0` | RET |
| 004D8C00, 004D8A50, 004D8AE0, 004D8B70 | `__cdecl` varargs | `game, id..., 0` | RET |
| 004F7620 | `__cdecl` | none | RET |
| 004F83B0 | `__thiscall(screen)` | none | RET |
| 00687330 | `ret 4` with the id at `[ESP+4]`; `ECX` is loaded but unused | `int id` | RET 4 |
| 004C4300 | `__thiscall(game, int level)` | `int level` | RET 4 |
| 004D6410 | `__thiscall(game, int level, vector*)` | two | RET 8 |
| 004F7210 | `__thiscall(vector)` | none | RET |
| 004F81D0, 004D3740 | `__thiscall(dst, src)` | `src` | RET 4 |
| 004F8480, 004D6840 | `__thiscall(vector, const int*)` | `const int*` | RET 4 |
| 00A92290 | `__thiscall(inputManager, int id)` | `int id` | RET 4 |
| 00A933F0 | `__thiscall(inputManager, int id, int level)` | two | RET 8 |

## Uncertainties

- **No leaf override of `+4h` or `+8h` was identified.** The occlusion and exemption rules are read
  from the recompute, not from a screen that uses them. Until one is found, "level 5 covers level 4"
  is a mechanism with no observed instance. Verified later by packets `cc_main_menu_path` and
  `cc_frontend_states` (docs/MAIN_MENU_PATH.md): the level-5 setter `004F87B0` has zero
  references and no paired input-context setter, and `004DA780` skips it, so the vector at
  `00E18D38` is only ever touched by the setter and the recompute's descent at `004F7647`; the
  recompute's top level is always empty and the walk effectively starts at level 4. Level 5 is
  unexercised in this build. Level ownership from the call sites: 1 = in-mission HUD interface
  (18 sites, the only level with multi-element sets), 2 = in-mission overlays (5), 3 =
  in-mission transients (17), 4 = the front-end managers (5), 5 = nothing.
- The five levels have no recovered names. Level 4 is the manager level and level 5 is above it;
  what uses levels 1..3 and 5 was not traced (only `004DA780` clears them).
- `00E18CDC` is raised by every setter and cleared by the pump at `004F8881`. Nothing was found that
  reads it between those two points, so "dirty" is a hypothesis about its role, not an observation.
- The dead backwards scan at `004D8C84` is unexplained; it computes the topmost non-empty level and
  drops it.
- The 16-byte vector's `+0h` word is never initialised by these callers. It is consistent with the
  VS2005 `_SECURE_SCL` container-proxy head, given the `00BF6713` assertions everywhere, but the
  field itself was not traced to a writer.
- `005B5757` reads `00E08310` and was not examined; it may be a second consumer of the occlusion
  level or an unrelated alias.

## What remains

- Find the screens that override `+8h` (occluding) and `+4h` (self-managed), which would turn the
  layering from a mechanism into observed behaviour.
- Identify the screen types behind ids 0Ch and 15h, the two registry slots no manager maps to.
- Trace the callers of `004F8530`, `004F85D0`, `004F8670` and `004F87B0` to name levels 1, 2, 3, 5.
- Name the input contexts 1..19h that `004C4300` prioritises.

## State reached

| Address | State |
| --- | --- |
| 004F8710, 004F7620, 004D8C00, 004C4300, 00687330, 004F83B0 | analyzed, reconstructed, build-tested |
| 004F8530, 004F85D0, 004F8670, 004F87B0 | analyzed; level variants of 004F8710, each with its own `ECX` constant |
| 004D8A50, 004D8AE0, 004D8B70, 004D6410 | analyzed; levels 1..3 through the shared routine 004D8C00 inlines |
| 004F7210, 004F8480, 004F81D0, 004F7E80, 004D6840, 004D3740 | analyzed; identified as the `std::vector<int>` members |
| 00685820, 00689820, 00687800 | analyzed from raw bytes; no Ghidra function |
| 00A92290, 00A933F0 | analyzed from raw bytes; accessor pair only |
| 004DA780 | identified by role only; the eight-call teardown block was read, the rest was not |

Nothing here is ABI-compatible or game-validated. `bsp::FrontEndScreenSetStack` and
`bsp::GameInputContextSetStack` are C++ models with `std::vector` members, not the native layout.

## Follow-up packets

- `front_end_screen_levels`: 004f8530, 004f85d0, 004f8670, 004f87b0, 004da780, files
  `docs/FRONTEND_SCREEN_LEVELS.md`, `reports/frontend_screen_levels.json`. Contract: who publishes
  levels 1, 2, 3 and 5 of `00E18CF8..00E18D38`, what `004DA780` is (it clears all eight vectors of
  both families in one block), and whether any of them ever publishes more than one id.
- `front_end_screen_occlusion`: 004f7570, 004f7580, 00ceae54, 004ca4f0, 005b5710, files
  `docs/FRONTEND_SCREEN_OCCLUSION.md`. Contract: the leaf overrides of the screen base's `+4h` and
  `+8h`, and the two readers of the occlusion level `00E08310` outside the recompute.
- `game_input_context_levels`: 004c4300, 00a92290, 00a933f0, 004bec00, files
  `docs/GAME_INPUT_CONTEXT_LEVELS.md`. Contract: the input-context id space 1..19h, the level array
  at `manager+10h`, and what the cached maximum at `manager+20h` gates.
