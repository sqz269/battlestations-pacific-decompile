# Indirect call graph evidence

`python tools/callgraph_indirect.py` reads the configured disk PE and shared
`functions.json`, then atomically writes `<exports>/callgraph_indirect.json`.
`tools/workspace.py` resolves the shared exports directory across worktrees.
The tool does not connect to Ghidra, modify the installation, or run the game.
Dependencies are the existing `pefile` and `capstone` Python packages.

```powershell
python tools/callgraph_indirect.py
python tools/callgraph_indirect.py --config local/target.json --max-candidates 64
# --output DIRECTORY overrides the shared exports directory, including snapshot inputs.
```

The index consumes this evidence separately from the direct-call sweep. Regenerate
the export after a binary or function-snapshot change, then rebuild the index.
Consumers should inspect `kind` before interpreting reachability: a table owns
membership edges, while `vcall?` deliberately retains uncertainty.

## Output and edge kinds

```json
{"meta": {"counts": {"vslot": 0, "vcall?": 0, "lua": 0}},
 "edges": [{"caller": "0x401000", "callee": "0x402000", "site": null,
            "kind": "lua", "detail": "Example"}],
 "vtables": [{"address": "0xd00000", "class": null, "slots": ["0x402000"]}]}
```

Addresses are image virtual addresses, formatted as lowercase `0x...` strings.
Every emitted callee is an exact non-external snapshot function start. Membership
and alias rows retain separate sites, rather than collapsing distinct table entries.

| Kind | Caller | Site | Meaning |
| --- | --- | --- | --- |
| `vslot` | Vtable address | Address of slot dword | The raw pointer at this slot equals the method start; detail is `Class::slot N`, with `<unknown>` for a null class. |
| `vcall?` | Enclosing snapshot function | Indirect call or tail-jump instruction | Method is compatible with the observed slot; this does not establish the receiver's dynamic class. |
| `lua` | Registration table, or function for inline registration | Function-pointer member, or registration call | ASCII Lua name is associated with an exact function start through table/registration evidence. |

`meta` records binary/config/snapshot/functions/tool hashes, dependency versions,
counts, fan-out exclusions, Lua table consumers, elapsed time, and ten reproducible
random `vcall?` samples. The snapshot project/program identity must match the target
configuration when `snapshot.json` is present. The executable must be x86 PE32.

## Vtable discovery and dispatch limits

Discovery begins with aligned `.rdata` runs containing at least two adjacent exact
snapshot starts. At `vtable-4`, the optional MSVC Complete Object Locator must have
signature zero and a valid TypeDescriptor name beginning `.?AV` or `.?AU` and ending
`@@`. Class hierarchy descriptors provide base-type membership. Windows DbgHelp
decodes names, including templates; the portable fallback decodes ordinary
class/namespace names and preserves complex mangling rather than inventing a name.
Without validated RTTI, `class` is null.

Visible `mov dword ptr [reg+offset], vtable` stores establish additional slot-zero
boundaries. This matters because adjacent classes can share one uninterrupted
function-pointer run. At these anchors, executable-pointer gaps preserve their raw
positions when the snapshot lacks a method start; those slots receive **no edges**.
Unanchored arrays use strict exact-start runs, excluding long switch-table runs of
interior code addresses. RTTI-less discovery is still heuristic: a function-pointer
array can resemble a vtable, and an unobserved constructor can leave an uncertain
boundary. The exported `slots` array records actual disk dwords, including the
explicitly counted unindexed executable gaps at anchored tables.

CRT initializer arrays are excluded using their actual consumers: named
`__initterm_e`/`_initterm` calls with literal bounds, and the iterator-load/call,
four-byte stride, and end-bound comparison in `__cinit`'s inlined traversal. On this
image those ranges are `[00CE2734,00CE36E4)` and `[00CE36E8,00CE3708)`, with 200 exact
function-pointer slots. They are initializer callbacks, not virtual methods.

Dispatch recognizes `call/jmp dword ptr [reg+disp]` and `[reg]`, with aligned
nonnegative displacement. It also recognizes the common register form:

```text
mov eax,[object]
mov edx,[eax+disp]
call edx
```

The register form requires the method load to survive to the call in the same
basic block. Register copies are followed; partial-register writes, call barriers,
branch joins, implicit register operations, and disjoint body spans invalidate
evidence. Stack-based and indexed jump-table operands are excluded.

The default limit is 64 unique targets per dispatch site. Sites exceeding the limit
are skipped entirely and counted, avoiding arbitrary truncation. A visible vtable
store followed by a load from the same tracked object narrows the site to that
table and its RTTI-derived classes. This is conservative block-local reasoning,
not a type solver. The checked snapshot produced **zero narrowed sites**; most
constructor stores are outside the methods that later dispatch through the object.

When a snapshot record provides `body_ranges` as inclusive `[start,end]` address
pairs, only those spans are swept. Otherwise the disk sweep runs from a function
start to the next start, stopping at `int3`, matching the original sweep's fallback.
The checked snapshot has no body ranges. Consequently containing-function identity,
embedded data, and reachability retain the linear-sweep limitations; this export
does not claim Ghidra body or runtime control-flow validation.

## Lua tables, inline bindings, and property readers

The scanner searches `.rdata` and `.data` for aligned `{name_pointer,function_pointer}`
rows, decoding bounded ASCII names at the referenced VA. Pooled suffix strings are
valid: there need not be a preceding NUL. Targets must be exact snapshot starts.
The terminator is a null **function pointer**, even when its name pointer is non-null.
Empty names are accepted for `luaopen_*` library opener rows.

A shape-only array is not sufficient. Its members must be referenced by code in a
function that calls a named registration helper (`luaL_register`, `lua_register`,
or `BSP_LuaMachine_RegisterGlobalFunction`), a helper calling both
`lua_pushcclosure`/`lua_setfield`, or an opener consumer calling
`lua_pushcclosure`/`lua_call`. These are local consumer checks rather than complete
argument-flow proofs; the concrete tables below were also checked against their
registration loops. Inline immediate handlers use the game's EDX handler ABI at
`lua_pushcclosure` followed by a bounded `lua_setfield` sequence with an ASCII name,
without intervening calls or branch joins. Pushed name/handler helper arguments
are also recognized.

On the checked image, fourteen confirmed tables contribute 703 rows: the mission
table `00E0B7B8` contributes 560, eleven stock `luaL_Reg` arrays contribute 128,
and opener tables `00CF8350`/`00D62BB8` contribute 7/8. Ten inline registrations
bring the total to 713. The mission table has 557 distinct handlers and retains
the three aliases. All four inline `DoFile` registrations point to `00B69E00`.
One additional shape-only table is counted but excluded.

The `{2,&field}` / `{0,"name"}` records passed to `00BD6830` are transient property
reader descriptors, **not Lua C-function bindings**. For example, `007D697D` writes
type 2 and `007D6989` stores destination `[ESI+9F0]`; `007D699F` supplies
`"pwrInput"`, followed by the reader call at `007D69A6`. They describe data to write,
not a callback. The scanner records 166 direct reader sites as ignored metadata
and emits no callback edges for those destinations. See
[PILOT_COMMAND_PATH.md](PILOT_COMMAND_PATH.md) and
[GUI_LUA_READER.md](GUI_LUA_READER.md).

## Checked image and results (2026-10-18)

Binary SHA-256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The 2026-10-16 snapshot contains 76,332 internal function starts; its functions JSON
SHA-256 is `8cbe5453d72e391cc0b389a3f4b0722d8918ab78f6aae904036f3273fbdfff39`.
Capstone 5.0.7 decoded 2,456,826 instructions in each sweep.

| Evidence | Count |
| --- | ---: |
| `vslot` edges | 21,786 |
| `vcall?` edges | 185,139 |
| Dispatch sites with edges | 5,203 |
| Dispatch sites skipped above 64 targets | 10,823 |
| `lua` edges | 713 |
| Vtables with RTTI | 72 |
| Vtables without RTTI | 1,803 |
| Total vtables | 1,875 |
| Unindexed executable slots, preserved without edges | 1,058 |

Three ledger-known facts now have incoming edges despite missing direct callers:

| Ledger fact | Export evidence |
| --- | --- |
| `0074E1E0 BSP_Plane_SetForwardSpeed_VtableThunk` | `00D05F20 -> 0074E1E0`, `vslot`, slot 15 (`+3Ch`), as in [PLANE_FLIGHT_CORE_LAW.md](PLANE_FLIGHT_CORE_LAW.md). The unindexed slot at `+14h` does not shift this method to slot 9. |
| `00A0FCE0 BSP_AiCommand_NonControl_GetType` | `00D22990 -> 00A0FCE0`, `vslot`, slot 1 (`+4h`). Constructor-store splitting prevents concatenation with the preceding base table. |
| `0088BEC0 BSP_LuaBinding_Log` | `00E0B7B8 -> 0088BEC0`, `lua`, `Log`, function member at `00E0B7BC`; consumer `00884BE0` calls `006B8610`. See [MISSION_LUA_HOST.md](MISSION_LUA_HOST.md). |

Validation checks enforce the exact output keys, target-start membership, unique
`(caller,callee,site,kind)` tuples, the 64-target cap, input/tool hashes, and the three
facts above. Focused in-memory checks exercised receiver aliases, partial-register
invalidation, call barriers, and RTTI descendant selection. These are static
tooling checks, not ABI, gameplay, or actual receiver-type proof.

The final ten deterministic random edges (seed `0x18bc`) were checked against the
listing and their actual slot dwords. The table below records that audit.

An earlier ten-edge audit had 10/10 correct instruction shapes and raw pointer
matches, but only 9/10 valid vtable witnesses: `00CE34E0` was a CRT initializer
array, falsely offering `00CD7B60` at slot 13. Its consumer in `__cinit` proved the
exclusion above. This correction removes that known false candidate while retaining
the legitimate unanchored stream vtable `00D64258` (raw constructor store `00BB8B6F`).

| Caller | Dispatch site | Slot | Candidate | Vtable witness | Independent store |
| --- | --- | ---: | --- | --- | --- |
| `005BEC50` | `005BEFF8` | 13 | `00A9CCE0` | `00D5BBF8` | `00A9DF74` |
| `0057B600` | `0057BB7E` | 33 | `00929E30` | `00D19500` | `00929F1F` |
| `0064DD30` | `0064F841` | 13 | `006CF5B0` | `00CF89F8` | `006CFABA` |
| `0063C620` | `0063CED7` | 13 | `00ADFA60` | `00D5D424` | `00ADFA23` |
| `0066D020` | `0066D051` | 13 | `00B6E870` | `00D62C88` | `00B6F45F` |
| `0054B530` | `0054B5BD` | 13 | `006E1F00` | `00CF95E0` | `006E26CF` |
| `0058F5B0` | `0058FC0E` | 13 | `00B34080` | `00D5F280` | `00B3405E` |
| `0065A740` | `0065A75E` | 13 | `00A7BE80` | `00D5ABB8` | `00A7BDAF` |
| `00675C40` | `0067925C` | 13 | `006E1F00` | `00CF95E0` | `006E26CF` |
| `00BD3580` | `00BD3585` | 17 | `00B6DC00` | `00D62C88` | `00B6F45F` |

All ten pass instruction shape, unchanged method-register value, exact disk-slot
membership, and an independent object/subobject vtable store. Eight sites have
assembly exports; `0066D020` and `0065A740` were decoded from their known disk
function starts. The final row is a tail dispatch: `mov eax,[ecx]`;
`mov edx,[eax+44h]`; `jmp edx`. This is **10/10 static witness precision** for the
sample, not a measurement of actual dynamic-call precision.

The audited tool SHA-256 is
`f07e6ff59da212f6e0f0a05963da0d199762f29b7f17297c321b0ba1ab0b636c`;
the generated JSON SHA-256 is
`0d56f4a21aaa7123e669b09773f2ffd1a82bbb6a805dee6968c27477091d557c`.
Generated UTC/elapsed fields change the JSON hash on a later run. This document
and tool are the only committed artifacts; the export and validation reports
remain in the shared ignored exports/local directories.
