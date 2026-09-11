# Warning message and escape-character tables

Addresses: 009870a0, 0097f570, 00979990, 00979730, 0096dbb0, 008ec460, 008e65e0, 0049c9c0

Packet `orch4_warning_message_table`. Function names below are hypotheses, not recovered
symbols. Evidence is the saved `/battlestationspacific.exe` in `C:/Users/sqz269/bsp.gpr`,
verified by the repository's live export/flow/byte tools, plus capped assembly reads and
read-only disk decoding. Workers made no Ghidra mutations.

## Correction to MISSION_EVENTS_UPDATE.md

`entity` and `playerunit_section` are children of `Warnings.escapecharacters`.
`messages` is a direct child of `Warnings`. In Init, the calls at 009871f3 and
00987244 use the LuaObject at stack+48h as receiver; the call at 0098728d instead
uses stack+5Ch, the `Warnings` object. The stack adjustments in the listing matter.

The destinations at manager+11Ch and +130h are each a 14h-byte section containing
an 8-byte prefix string followed by a 0Ch-byte map. They are not just map headers.
Manager+114h is a third string accumulating both sections' top-level keys. The
message map is string-to-vector-of-strings; its values are not `WarningRecord`.

The six blocks removed from Init's pseudocode are conditional native-string
copy/release paths, not six SEH funclet tails. For example, 00987343 copies the
literal at 00d1b900 and 00987420 copies the literal at 00d1b8ec. These resolve to
`EnvironmentUSHorn` for +190h and `EnvironmentAirRaid` for +194h, respectively.

## Native layout and ABI

| Address | Original interface from listing | Recovered role |
| --- | --- | --- |
| 009870a0 | ECX=manager, no stack arguments, RET | Init; only its table-loading fragment is implemented here |
| 0097f570 | ECX=manager; stack: message map*, LuaObject*, prefix string*; RET Ch | Recursively load message groups |
| 00979990 | ECX=manager; stack: escape section*, LuaObject*; RET 8 | Append section prefixes and load replacements |
| 00979730 | ECX=manager; stack: escape section*, LuaObject*, prefix string*; RET Ch | Recursively load escape replacements |
| 0096dbb0 | ECX=manager; stack: message id string*; bool in AL; RET 4 | Case-insensitive message membership |

The decompiler loses ECX and stack arguments in both recursive loaders; those
signatures follow their callers and `RET` immediates. No x87 arithmetic occurs in
these parsers. Standard library containers remain library contracts.

| Manager offset | Layout |
| --- | --- |
| +108h | Message map: iterator bookkeeping, sentinel at +10Ch, count at +110h |
| +114h | Combined escape prefixes: native string size, char* at +118h |
| +11Ch | Entity section prefix string; map at +124h, sentinel +128h, count +12Ch |
| +130h | Player-unit section prefix string; map +138h, sentinel +13Ch, count +140h |

A message-tree node has a key string at node+0Ch and its value vector at node+14h
(008ec460 returns node+14h). The vector's first/last/capacity pointers are at
node+18h/+1Ch/+20h; element stride is eight bytes, a NativeString. Escape-map
nodes have a key at +0Ch and a replacement NativeString at +14h, returned by
0049c9c0. The comparator is 00443d00, `BSP_NativeString_LessCaseInsensitive`.

## Loading behavior

`0097f570` iterates key/value pairs with 00b67080/00b67190 (`lua_next`). For a
table-valued entry it concatenates the current prefix and that key, with **no
separator**, then recurses. For a scalar it ignores the key and appends the
string-converted value to a temporary vector. At the end of the level it assigns
that vector to the current prefix only if it is nonempty (0097f74b..0097f773).
Mixed groups therefore produce both a parent message and child messages. Empty
groups and groups containing only tables do not independently create entries.

`00979990` appends every top-level key to manager+114h and the destination
section's prefix string, then calls `00979730` with that key as prefix. Its
recursive helper concatenates each key and assigns the scalar string to the
section's map at +8. It checks a null result from `lua_tolstring`: a boolean or
other non-convertible scalar becomes an empty replacement. The other string
reads use unchecked `strlen` loops.

The routines do not clear maps or prefix strings. Equivalent concatenated paths
overwrite in traversal order; a message overwrite replaces the entire vector.
No sorting, random selection, localization, placeholder expansion, voice loading,
or lifetime/priority parsing occurs here. Values remain raw strings such as
`@_ident_dispatcher`. Lua iteration order is retained, with no added promise of
alphabetical or array-index sorting.

`0096dbb0` calls the string-vector map's find adapter 008e65e0 at manager+108h and
compares the returned iterator with sentinel manager+10Ch. It has no side effect
on the map. The native checked-iterator assertions concern library invariants.
Although previously tagged `STL_inst_0096dbb0`, this wrapper embeds the manager's
108h/10Ch offsets and is manager behavior rather than a generic STL routine.

## C++ boundary and limitations

`WarningMessageTables` and `WarningEscapeTable` are new C++ projections using the
existing `NativeStringCaseInsensitiveLess`; they do not duplicate `WarningRecord`
or claim the native allocation/iterator ABI. The four complete parser/lookup
routines use the existing `GuiLuaHost`, whose concrete `GuiLua51Host` links stock
Lua 5.1.1. No Lua, CRT, or STL implementation was ported.

`load_warning_tables_009870a0` models only the three loading calls and associated
Lua lookups at 009871bb..00987307. It accepts the already loaded `Warnings`
LuaObject. Script/VFS ownership, singleton publication, 00973f20's unrelated
manager initialization, suppression/deadline resets, reference-counted effects,
hook installation and the remainder of Init are analyzed external boundaries.
The existing warning report host can use `warning_message_id_known_0096dbb0`.

Supported input is an acyclic, well-formed table hierarchy with string keys where
keys are concatenated, and string/number message scalars. Numeric scalar-list
keys are ignored as in the binary. Converting numeric **path** keys mutates a
native tracked Lua key before `lua_next`; the existing host retains a separate
cursor, so parity for that malformed/unobserved case is not claimed. Host guards
throw `invalid_argument` for a non-table or unchecked-null string precondition;
the binary has no corresponding recovery branch. Native C-string truncation at
embedded NUL is preserved. Non-ASCII locale-dependent comparator parity is not
newly established by this packet.

## Flow gaps and validation

All named routines already have Ghidra function definitions: `no_ghidra_function=[]`.
The integrator should repair these `free` call-site overrides and refresh exports:

| Function | CALL | Missing inclusive byte range | Disk evidence |
| --- | --- | --- | --- |
| 0097f570 | 0097f7bd | 0097f7c2..0097f7c4 | `add esp,4`, then normal epilogue; final `ret 0Ch` at 0097f7d3, length 3 |
| 008ec460 (external STL) | 008ec515 | 008ec51a..008ec51c | `add esp,4`, then iterator checks and node+14h return |
| 00973f20 (external Init callee) | 00973f53 | 00973f58..00973f61 | stack cleanup plus list-loop backedge; not an actual return |

Skipped alignment bytes in 00979730 and 00979990 are jumped over; they are not
missing reachable behavior. Full Init and those external bodies are not claimed
reconstructed here. Build and installed-data fixture results are recorded in
`reports/warning_message_table.json`. File/data tests are not gameplay validation.

Installed input inspected read-only:
`I:/SteamLibrary/steamapps/common/Battlestations Pacific/scripts/datatables/warnings.lua`,
SHA-256 `822a84fd16b6c6f8677892feb7ffe70792d1c12f82aac8ef2a75460cae279ff3`.
Its `~ -> own -> battleship` replacement becomes `~ownbattleship`; the section
path `% -> engineroom` becomes `%engineroom`. `recon -> identified -> enemy -> ship`
contains scalar alternatives and a nested `player` table, providing a real mixed
group fixture. `failure -> HangarFailure` is empty and creates no message entry.
