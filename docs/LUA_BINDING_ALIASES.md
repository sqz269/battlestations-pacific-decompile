# The three doubly-named binding handlers

Addresses: 00896a90 008b0c10 0088e560 006b8610 00e0b7b8

Packet `cc_lua_core`, worktree `agent/cc-lua-core`. Ghidra was read-only. Every name here is a
hypothesis, not a recovered symbol.

`docs/LUA_BINDING_TABLE.md` found that three handler addresses in the 560-row table are each
pointed at by two rows, and left open "whether the two names differ in behaviour by an argument
or are genuinely identical entry points". They are identical entry points. The question is
settled by the registration, not by the bodies.

| Handler | Rows | Names |
| --- | --- | --- |
| `00896A90` | `00E0B9D8`, `00E0B9E8` | `AddAirBaseStock`, `AddAirBasePlanes` |
| `008B0C10` | `00E0C190`, `00E0C198` | `MissionNarrative`, `MissionNarrativeEnqueue` |
| `0088E560` | `00E0C908`, `00E0C910` | `SetMotionBlurParams`, `SetMBP` |

## Why they cannot differ

A `lua_CFunction` is handed the state and nothing else. Three things would let one recover the
name it was called under, and none of them is present:

1. **An upvalue.** `006B8610` registers each row with `lua_pushcclosure(fn, 0)`, which
   `docs/MISSION_LUA_MACHINE.md` established. With `nup = 0` there is no upvalue to read, and
   the two rows produce two closures over the same function with no distinguishing state.
2. **The debug interface.** `lua_getinfo` would report the name at the call site.
   `python tools/bsp.py callees <handler>` over all three lists no debug entry point and no
   upvalue accessor.
3. **An argument the caller supplies.** This would have to come from the script, not from the
   registration, so it could not distinguish the two globals; and the bodies below read their
   arguments positionally with no name-shaped discriminator.

So the pair is one entry point under two globals. A script that calls `SetMBP` and a script
that calls `SetMotionBlurParams` execute the same bytes with the same inputs.

`include/bsp/lua_binding_core.hpp` carries the three rows as `kMissionLuaBindingAliases` and
the claim as `mission_binding_alias_is_indistinguishable`, so a caller states the assumption it
depends on rather than making it silently.

## What each one does

Read far enough to show that no branch depends on anything a name could carry. Coverage is
`partial` for all three: these were read to answer the aliasing question, not to reconstruct
them.

### `00896A90`, `AddAirBaseStock` / `AddAirBasePlanes`

`__fastcall(lua_State* in ECX)`. Entity from argument 0 through `00888AA0`, then `006BCD20`,
then the vehicle-class factory `00964790` over argument 1, then `0095BA60`, then `00964790`
over argument 1 again, then `006CA770` with the first factory result and argument 2. Pushes
nothing. Coverage: partial, the three game callees were not read.

### `008B0C10`, `MissionNarrative` / `MissionNarrativeEnqueue`

`__fastcall(lua_State* in ECX)`, returning the literal 0 rather than a frame result count. It
is the one of the three that does not use the `LuaStateOwner` frame: it goes through
`006B7E30`, `008875F0` and `00887120(-1, 4)`, which build a descriptor array of 14h-byte
entries. Argument 1's string comes from descriptor+1Ch, further arguments are walked from
descriptor+28h at stride 14h through `006EDF00` into a freshly allocated 0Ch-byte vector, and
the call is `00734870(descriptor+8h, &string, vector)`. Coverage: partial, `00734870` was not
read.

That this handler uses a different argument path from the other 557 is worth recording on its
own: a reader who assumes every row opens a `LuaStateOwner` frame would mis-read it.

### `0088E560`, `SetMotionBlurParams` / `SetMBP`

`__fastcall(lua_State* in ECX)`. Three numbers from arguments 0, 1 and 2, then a parameter
block whose third field is `(arg1 - arg0) / arg2` with the rest taken from constants
(`00D7A24C`, `00CE3800`) and zeros. Pushes nothing. Coverage: partial, the block's destination
was not followed.

The shorter name being an abbreviation of the longer one (`SetMBP` for
`SetMotionBlurParams`) is consistent with the aliasing being an authoring convenience rather
than two behaviours; the same reading fits `AddAirBasePlanes` alongside `AddAirBaseStock` and
`MissionNarrativeEnqueue` alongside `MissionNarrative`. That is a reading, not evidence.

## Corrections

None to a prior document's claim. This document answers a question
`docs/LUA_BINDING_TABLE.md` left open in its Uncertainties section, and that entry can be
struck.

## Both names of one pair are live

The sweep ranks by global name, so it can say whether the shipped scripts use both spellings.
They do, for one pair:

| Name | Scripts reaching it at load time |
| --- | --- |
| `AddAirBaseStock` | 17 |
| `AddAirBasePlanes` | 4 |
| `MissionNarrative` | 156 |
| `MissionNarrativeEnqueue` | 0 |
| `SetMotionBlurParams` | 0 |
| `SetMBP` | 0 |

So 21 scripts call `00896A90` under two different globals, and this document's claim is what
tells a reader those 21 all get the same behaviour. `MissionNarrativeClear` (4 scripts) is a
separate row with its own handler, not a third name for `008B0C10`.

## Uncertainties

- The four counts of zero are load-time only. The frame loop reaches rows the sweep cannot see,
  so `SetMBP` and `MissionNarrativeEnqueue` may well be called at run time.
- None of the three bodies was reconstructed. Their contracts are `partial` above.

## Follow-up packets

| Packet | Addresses | Files | Contract |
| --- | --- | --- | --- |
| `lua_binding_narrative` | 008b0c10 00734870 006b7e30 008875f0 00887120 006edf00 | docs/LUA_BINDING_NARRATIVE.md | The descriptor argument path 557 other rows do not use, and the narrative queue behind `00734870` |
| `lua_binding_airbase_stock` | 00896a90 006bcd20 006ca770 | docs/LUA_BINDING_AIRBASE.md | What an air-base stock entry is and why the class factory is called twice |
| `lua_binding_motion_blur` | 0088e560 | docs/LUA_BINDING_MOTION_BLUR.md | Where the parameter block goes and what the three script arguments mean |

## no_ghidra_function

None. All three handlers have Ghidra functions:

| Address | Body range |
| --- | --- |
| `00896A90` | `00896A90` - `00896CB3` |
| `008B0C10` | `008B0C10` - `008B0E07` |
| `0088E560` | `0088E560` - `0088E783` |
