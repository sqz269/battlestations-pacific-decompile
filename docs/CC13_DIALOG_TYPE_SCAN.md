# Dialog sequence `type` scan (cc13-scripts1, item d for cc9)

Question: does any authored dialog sequence element in this installation carry a `type` the
dialog command factory does not recognise? Such an element would leave the factory's object null
and the image would call through it.

Answer: **none.** Every authored `type` is one of the recognised names; the positive control is
below.

## What the image parses

- The data is Lua tables, not a separate file format. The only script-facing producer is the Lua
  native `StartDialog(id, table)` (`008B0540 BSP_LuaBinding_StartDialog`), which calls 00451A90 on
  `[game+21E4h]` (008B0688); 00451A90 calls the sequence parser 004507D0, which calls the command
  factory 0044BE50 once per `sequence` element (docs/CC13_GUI_SCREENS_3.md, segment 4). The other
  caller of 00451A90 is 00451E50, reached from `00777850 BSP_Session_PumpReceiveQueue`: a client
  rebuilds the host's dialog from a session message, so it carries whatever the host's script
  authored. The other caller of 004507D0, 00452130, has no code or data reference in Ghidra.
- In every script in this installation the table comes from `Mission.Dialogues[dialogID]`
  (`luaStartDialog` in scripts/global/commandhelpers.lua, and per-mission copies such as
  usn_2_java.lua:961 `StartDialog(dialogID, Mission.Dialogues[dialogID])`).
- Factory 0044BE50 (exported, assembly read): it reads `type` through the reader's slot +0Ch,
  read-with-default, with key 00CE4AD0 and default string 00CE4AD8 `"msg"` (pushes at
  0044BE80..0044BEA4, call at 0044BEBA). So an element **without** `type` is a `msg` command.
  Then `_stricmp` against `"msg"` (0044BECA) and `BSP_NativeString_EqualsCStringInsensitive`
  (00425850) against `setpanel`, `hidepanel`, `pause`, `callback`: all five matches are
  case-insensitive. No match falls to `piVar3 = 0` and `CALL [*piVar3 + 8]` at 0044BFBB, a null
  dereference.
- The `type` **key** is a Lua table lookup and therefore case-sensitive: a `["Type"]` key would be
  ignored and the element would default to `msg`. No mission script uses that spelling (below).

## Scan

`local/sc1_dialog_scan.py` (read-only, run with `python -I` over the installation root) walks
every `.lua` file (621 in this installation, none precompiled: no file starts with `\x1bLua`),
blanks comments, finds every `sequence = {` / `["sequence"] = {` block, brace-matches it, and
collects every `type` assignment inside it.

| Measure | Count |
| --- | --- |
| Files with a `sequence` block | 206 |
| `sequence` blocks | 2899 |
| `type = "pause"` | 678 |
| `type = "callback"` | 368 |
| `type = "setpanel"` / `"hidepanel"` / `"msg"` (explicit) | 0 / 0 / 0 |
| `message` keys (elements that take the `msg` default) | 6540 |
| `type` outside the five names, or non-literal | **0** |

Supporting greps over all `*.lua`:

- `"setpanel"`, `"hidepanel"` and `"msg"` as string literals: no live occurrence (the only hits are
  a dialog **id** `["MSG"]` in usn_19_coralus.lua:427 and three commented-out
  `Mission.Dialogues[...]["sequence"][1]["message"]` rewrites in jm03/jm06 variants).
- No script assigns into a sequence element's `type` at run time (`.sequence`/`["sequence"][`
  writes are all comments).
- `["Type"]`/`["TYPE"]` keys occur only in data tables (sound setup, hint messages, class tables,
  effects, achievements), never in a mission file.
- Every direct `StartDialog(` call passes `Mission.Dialogues[dialogID]`.

`.bak` files (four in this installation, e.g. scripts/global/commandhelpers.lua.bak) were not
scanned because the game loads `.lua` names.

## Consequences for the host

- A host factory that treats an unknown `type` as fatal (or as `msg`) diverges from the image only
  for data this installation does not contain; the null-call path is unreachable from these
  scripts. A defensive host fallback is safe to add and changes no authored behaviour.
- `setpanel` and `hidepanel` are never authored, so their command bodies are not exercised by any
  mission here; `msg` is exercised only through the missing-key default.

## Status

Exported (factory assembly read), scan run on this installation (file mtimes 2024-07-13 for the
mission scripts; the installation is modded, see the brief), no C++ written, nothing
game-validated. No Ghidra or ledger changes for this item.
