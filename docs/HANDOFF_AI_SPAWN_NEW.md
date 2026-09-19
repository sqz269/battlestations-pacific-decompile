# Handoff: `SpawnNew` as an AI resource request

Written by `agent/cc8-ai-squadron` on 2026-09-19 for whoever takes the packet, because this
session is past two thirds of its context and the packet's first step is a whole-consumer read that
should not be cut off half way. Nothing here is a conclusion; it is the starting position plus the
traps already found.

Tree base: main `5a9f7fe43`.

## What the packet is

`SpawnNew` registers a reinforcement request against a party's resource budget, and the AI planner
consumes it. **Read the consumer side whole before binding anything**: who drains the request, what
it builds, and in which frame step. Claim a lease with the addresses once the band is known.

## The four addresses, looked up rather than assumed

Do this first for anything else you touch: `python tools/bsp.py lookup <addr>`. Two of the four came
back differently from how the packet describes them.

| Address | What the ledger says | Read this before trusting the brief |
| --- | --- | --- |
| `0094C480` | `BSP_LuaBinding_SpawnNew`, **named and already reconstructed**. `lua_CFunction`, `__cdecl int(lua_State*)`, row `00E0BFD0` of the 560-entry binding table at `00E0B7B8`, name pointer `00D0FE58`. | **`callers (0)`** — and that is correct, not a gap: a Lua binding is reached through the table, never by a call. Do not go looking for the caller. |
| `00946FC0` | `BSP_AiParty_AvailableResources`, read whole by packet `cc2_ai_planner_tails`. `__fastcall(ECX = int team)`, float in ST0, body `00946FC0`-`00947084`. Starts from `[00E0CFB4] * 0.5` and subtracts `unit+304h` for every unit of `world+19CCh+58h` owned by the team. | **`callers (1)`: `00A1E250 BSP_AiPlanner_CaptureTargetScore`.** That single caller is the whole planner link and is where the consumer read should start. |
| `00A38DA0` | **No ledger name, `FUN_00a38da0`.** Segment 71, whose keywords are `client`, `online`, `xenonsystemmanager`, `network`, `player`, `mnetworkclientxlive`, `changestate`. Callers `00777850`, `0088C750`, `0088C8E0`; 19 callees including `BSP_Game_GetEffectiveGameMode` and `BSP_InGameInterface_CollapseLevel3WhenSpectating`. | **This does not look like an AI spawn consumer.** It sits in the online/network neighbourhood and its callees are interface and session code. Verify what it is before building on the brief's description of it; it may be the wrong address, or a second role. |
| `*(00F89B3C)` | the spawn manager pointer | Not yet looked up here. A `.data` pointer: read the writer as well as the readers, and remember a BSS address past rawsize is loader zero-fill. |

Start from `docs/LUA_BINDING_SPAWN.md`.

## Traps this session hit that apply directly

These are not general advice; each one cost real time tonight in this exact area.

1. **Search for the concept, not the name a header gives.** A header comment named a resolver
   `resolve_member_units`; that name exists nowhere, so the grep came back empty and a duplicate got
   written. The real function was `GameScriptOrdersHost::resolve_plane_squadron_members`, one
   function further down a file already grepped. Before writing anything, grep for what it would
   *do*, not what something calls it.
2. **An exact numerical coincidence is not evidence until both numbers are in the same space.**
   Tonight one exact match meant everything (`scored` falling by exactly 37600 = the unresolved
   bullet-class lookups) and one meant nothing (sub-type `00h` lookups totalling 38000 against
   `plane_attacker` 38000, which only matches if those attackers carry one barrel each). A peer
   nearly published "the Mavs are ordered against paths" because ids 44, 45 and 46 exist in three
   different id spaces in one log.

   **This one will be directly in your way, so it is spelled out.** At least three id spaces are
   live at once and all three carry small integers: the **scene object** id (what an order's
   `target_object_id` holds), the **scene path** id (`scene path retained: id=44 name=p6de`) and
   the **scene class** id (`scene class Landscape id=44`, `scene class AirField id=45`). **No
   current log prints the scene object id table**, so an object id cannot be turned into a name
   from a log alone, and matching it against either of the other two produces a confident wrong
   answer. `agent/cc8-flyto-solver` ran aground here trying to name what object `45` is; that is
   still open. If `SpawnNew` hands you an id, establish which space it is in before resolving it,
   and consider printing the object table as the first thing you add.
3. **A count that looks wrong may be a correct count of something else.** A diagnostic here flagged
   every correctly grouped one-wing squadron as ungrouped, because "one member" is right when
   `WingCount` is 1. Check the authored value before reporting a defect.
4. **Ordering defects hide behind plausible data explanations.** The squadron residue looked like
   missing data for hours; it was two statements in the wrong order, because `create_units`
   constructs the AI host and runs its census at its own tail, so a record created after
   `create_units` is invisible to it. If something reads empty, ask when the reader runs, not only
   what wrote it.

## Runs

`tools/run_game.ps1` only. Never launch `bsp_game.exe` directly: an untagged run takes the game's
own mutex and has blocked the user's copy of the game.

Since main `af665c062` runs **overlap** — three numbered slots, per-run `--instance-tag` and
`--affinity-core` — so up to three run at once, each needing its own `-Log` path. Merge main before
launching; a launcher from an unmerged tree uses the old single lock and starves.

**Do not pipe the launcher through a short-circuiting filter.** `| Select-Object -First 1` closes
the pipe after one line and kills the run: it exits 0 in under a second and writes no log at all.
`-Last N` consumes the whole stream and is safe. Runs outlast the 600 s foreground cap and nothing
wakes you when they land, so note the expected end time and read the log then.

`BSP_AI_WEIGHT_MODEL=0` keeps the AI target-weight model on its class stand-in and `=1` runs it, so
a before/after pair comes from one binary.

## Baseline warning

Every column in `docs/AI_TARGET_WEIGHT*.md` and `docs/AI_SQUADRON_SERVED.md` predates at least one
of: the `0071EBF0` command-target rule (`0daec4b56`), the `Hidden` hold-back (`67e8ac821`), real
squadron wings (`15563fdf9`), the torpedo goaway binding (`e94f9905d`) and tonight's squadron
ordering fix (`5a9f7fe43`). **Label every column you take with its commit**, and do not compare
across any of them.
