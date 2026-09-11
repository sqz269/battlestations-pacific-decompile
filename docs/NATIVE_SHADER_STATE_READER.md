# Native Lua tracking and shader state-table reader

Addresses: 00b579b0, 00b65f50, 00b66050, 00b66270, 00b66290, 00b669a0, 00b66bd0, 00b66c00, 00b66c60, 00b66de0, 00b67700, 00b67800, 00b67980, 00b68100

The full B579B0 state-table reader now consumes the actual16-byte definitions and appends to
actual8-byte state rows using the previous packet's B567B0. Its Lua objects and state owner also
use their actual20-byte and1224-byte layouts. `native_lua_objects.hpp/.cpp` provides their
tracking, lookup, conversion and cleanup; `native_shader_state_reader.hpp/.cpp` composes them.
These are new C++ interfaces linked to the repository's Lua5.1.1, not original register/EH ABI
replacements. All descriptive names remain hypotheses. Evidence is pinned in
`reports/native_shader_state_reader.json`.

## Correction to earlier Lua object interpretation

Native LuaObject word00 points to a4C8h OWNER, whose word04 is `lua_State*`. Kind2/index08 is a
tracked positive Lua STACK position, not a Lua registry reference. The older `LuaObject` and
`GuiLuaRef` host interfaces remain separate projections; neither is silently substituted here.

| Actual owner offset | Field |
| --- | --- |
| 00 | Owns byte; padding01..03 remains preimage |
| 04 | Interpreter pointer |
| 08 | Opaque word; untouched by both captured constructors |
| 0C | Signed stack-to-slot offset |
| 10 | Opaque word initialized0 |
| 14 | Fifty24-byte slots, each five object pointers and signed count at+14 |
| 4C4 | Signed high-water bound; raised by lookup, never lowered by release |

Constructor B66BD0 writes only owns00,state04,offset0C,word10,the50 counts at28+18*i,and4C4.
The250 pointer cells are not initialized. This corrects interpreting28 as the beginning of each
whole record: it is the first COUNT. Object B65F50 sets owner0,kind0,index-1,tracked byte0;
word0C and padding11..13 retain preimage. The new types have no implicit cleanup/default writes.

## Full routines

| Routine | Coverage | Original ABI and recovered behavior |
| --- | --- | --- |
| B65F50 | complete | ECX fresh14h object, EAX same, RET; four writes above. |
| B66BD0 | complete | ECX fresh4C8h owner, EAX same, RET; initialize bookkeeping only. |
| B66C00 | complete | ECX fresh owner, stack borrowed interpreter, EAX same, RET4. Same bookkeeping, borrowed state, install global DoFile via pushcclosure(callback,0), setfield(globals). |
| B66C60 | complete | ECX owner, stack NativeString, EAX load/pcall status, RET4. Null string data means empty; luaL_loadstring then pcall(0,MULTRET,0) only on load success. Results/errors stay on stack. |
| B669A0 | complete | ECX owner, RET. Close captured nonnull state iff owns byte; then clear state04 regardless. Preserve owns/tracking; no blanket invalidation. |
| B67980 | complete | ECX owner, stack fresh object, EAX out, RET4. owner/kind1/index-10002/word0C0/tracked0; padding untouched. |
| B67800 | complete | ECX table object, stack fresh out/C-string key, EAX out, RET8. Kind3 selects globals; otherwise use current index after key push. Push key, gettable, capture top/owner, publish kind2 tracked result and append its address to the actual slot. |
| B68100 | complete | ECX table, stack fresh out/NativeString key, EAX original out, RET8. Use key.data or the empty0108FF2C fallback; ignore NativeString length for C-string lookup. |
| B66050 | complete | ECX object, AL boolean, RET. Only kind2 calls lua_type(owner.state,index) and compares NUMBER; all other kinds false. |
| B66270 | complete | ECX object, ST0 float32-rounded number, RET. Always call tonumber, FSTP float/reload. |
| B66290 | complete | ECX object, EAX signed result, tail-jump BF7420. Float32 spill precedes runtime integer-mode choice. Reuse the existing verified x87/SSE kernels. |
| B66DE0 | complete | ECX owner, stack object/index/remove-stack byte, RET0C. Untracked skips all owner/Lua access; otherwise first-match last-swap removal, conditional stack removal and tracking compaction described below. |
| B67700 | complete | ECX object, RET. Kind0 skips; otherwise release current object/index/owner with remove1, then clear kind only. |
| B579B0 | complete | Three stack args: output-header/definition-header/LuaObject; RET0C, ECX unused. Ordered current registry traversal, exact gate/fresh coercion, first-state-wins publication. |

B66C00 requires the application's real00B69E00-equivalent callback as an explicit C++ input.
It installs a zero-upvalue callback, matching the original call shape. This packet does not
invent VFS/override behavior or replace PcStorageLuaOwner's existing contextual callback.
Passing a missing callback is rejected at this new host interface. Full native B6A020 bootstrap,
tracked copy/iteration constructors and callback frame-offset management remain separate work.

B67800 ignores lua_checkstack's result and calls lua_gettable, preserving metatables. It does
not add an unbound-kind guard. Slot index is owner.offset+lua_gettop; raise high-water if needed,
then publish the output pointer and increment that slot's count. Valid50-slot/five-reference
backing extents, a live interpreter and stable tracked object addresses are caller contracts.
The native type predicates/getters use the owner's state directly; they never create registry refs.

B66DE0 captures its slot from offset+the supplied index and searches for the object address.
A found pointer is replaced with the last pointer unless already last; count decrements and
the vacated pointer remains stale. If count becomes zero, remove-stack!=0 pops the top via
settop(-2), or removes a non-top index. Except at high-water-1, copy subsequent slot reference
prefixes/counts down, zero the final count, then decrement every remaining moved object's
index08. Bounds/counts are reread where the original does. High-water stays unchanged. With
remove-stack0 bookkeeping still compacts; caller must have handled the Lua stack consistently.
Even an absent pointer in an already-empty slot follows the zero-count path; no new guard exists.

## State conversion and caller evidence

B579B0 reads the live unsigned definition count and recaptures its array each iteration. It copies
the current name,ID and tag before Lua callbacks. First lookup must be exact NUMBER; release it
before deciding whether to proceed. Tag0 does a second lookup and float32-to-native-integer
conversion; tag1 does a second lookup and stores float32 bits. A second lookup may coerce a
numeric string or nil. Unknown tags do not make a second lookup and reuse the previous payload,
initially the source LuaObject pointer's DWORD. Existing state IDs are never replaced, but their
lookup/conversion side effects still occur. Unknown keys are not enumerated.

All four direct callers were checked: B442E9 passes descriptor+B8 and manager+04; B57EDF passes
the sampler-state header and manager+10; B57F8C passes the texture-stage header and manager+1C;
B5A817 passes its previously prepared output and manager+04. B579B0's RET0C establishes all
three arguments despite its pseudocode misidentifying the third as float.

FuncInfo DF9178/mapDF9158 has four states: local definition cleanup B56C10 at state0, first lookup
at state1, integer lookup at state2 and float lookup at state3. Each temporary state returns to0.
The string is always cleaned after the active Lua temporary. C++ uses structured cleanup for
C++/Win32 unwinding; original EH delivery was not executed. Linked stock Lua uses its own error
mechanism; unprotected lookup errors are not converted into successful empty state tables.

## Verification and boundaries

One ignored fixture executes all14 packet bodies plus original B567B0, using current verified
PE bytes and register/stack adapters to the linked Lua5.1.1, actual string storage and existing
numeric/row-allocation dependencies. Whole1224-byte owner and20-byte object snapshots agree
across initialization and eight tracking states after normalizing only known pointer identities.
It covers alias last-swap, non-top removal/index shifts, high-water preservation, offset2,
remove-stack0, kind3 globals, empty-name fallback and untracked cleanup after owner close.

Real Lua metatables verify19 lookup visits for11 definitions, including first/fresh value changes,
wrong first types, nil second values, duplicate/preexisting IDs, infinities,NaN,wide integers,
float32 rounding, both CRT integer modes and unknown-tag payload reuse. Output and string
allocation/release sequences match. Script execution checks multiple results and both load/runtime
errors with their retained stack values. The fixture's generic DoFile callback is an explicit
test dependency, not evidence that the game's VFS callback has been rebuilt.

The same fixture reads the CURRENT installed debugshader.shfx,alphablend.shfx and dx9_lua.inc.
Its asset callback maps that one include to the installed loose file; input selection is explicit
fixture setup, not full descriptor/sampler parsing. With the actual62-definition manager, both
readers produce debug [{14,0},{7,0}], alphablend render [{27,1},{19,5},{20,6}] and sampler
[{1,1},{2,1}], with balanced tracking. Their three file hashes are recorded. No draw/gameplay
claim follows from this table check.

Strict Win32 build and both existing CTests pass; no permanent tests were added. All14 stored
Ghidra body extents are complete. The7-byte B579E9..EF and3-byte B6782D..2F alignment gaps are
left untouched; no flow repairs were needed. Eighteen captures match stored PE bytes; the
four-byte0108FF2C capture matches the PE's zero-filled virtual data tail, explicitly not disk
bytes. Prior names/comments are preserved, annotations saved and affected exports refreshed.

Follow-up packets: full B57B50/B41830 sampler construction and publication into the actual
descriptor; actual B573F0/B419B0 field records; native Lua integer indexing/iteration/copy and
owner bootstrap needed by full B43B00. Later shader creation, rendering and gameplay remain open.
