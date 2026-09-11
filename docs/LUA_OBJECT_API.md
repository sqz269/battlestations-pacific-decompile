# The LuaObject API the datatable loaders share

Addresses: 00b65f50, 00b660a0, 00b66270, 00b66290, 00b662b0, 00b66420, 00b669a0, 00b66bd0,
00b67080, 00b67190, 00b67700, 00b67800, 00b67980, 00b69d40, 00b6a020, 0098d400

Every datatable loader in the executable (decals, foliage types, vehicle classes, dialogs,
awards) reaches Lua through the same small C++ wrapper layer rather than through `lua_*`
directly. This document records the two objects that layer is built from and the three call
shapes every loader repeats. The reconstruction is `include/bsp/lua_object.hpp` and
`src/lua_object.cpp`; the live-reference side was already reconstructed as `bsp::GuiLuaHost`
in `include/bsp/gui_lua_reader.hpp` and is reused rather than duplicated.

All names below are hypotheses, not recovered symbols.

## The 14h LuaObject

`00b65f50 BSP_LuaObject_Construct`, `__thiscall(ECX = object)`, RET. Seven instructions:

| Offset | Written by 00b65f50 | Meaning |
| --- | --- | --- |
| +00 | 0 (00b65f54) | `lua_State*` |
| +04 | 0 (00b65f56) | kind |
| +08 | 0xffffffff (00b65f59) | registry reference, -1 when none |
| +0C | **not written** | left as found; a stack-built object carries stale bytes here |
| +10 | 0, one byte (00b65f60) | flag |

The kind at +04 is the whole type system of the wrapper:

- **0, unbound.** `00b66420 BSP_LuaObject_IsUnbound` is exactly `kind == 0`. Its ABI is
  `__thiscall(ECX ignored, [esp+4] = object)`, RET 4: the `this` is genuinely unused, and both
  call sites in the decal loader (007409a8, 00740d6d) pass the table being iterated, which the
  routine never touches. The iterators leave their pair in this state at the end of a table.
- **1, a borrowed stack or pseudo index.** `00b67980` builds the globals object this way.
  `00b660a0` refuses to type-test kind 1 at all.
- **2, a tracked registry reference.** What `00b67800` and the iterators produce, released
  through `00b66de0` by the destructor `00b67700`.

`00b660a0 BSP_LuaObject_IsString`, `__thiscall(ECX = object)`, RET, bool in AL: false for kind
0, false for every kind other than 2, and for kind 2 `lua_type` (00a675a0) compared with 4,
`LUA_TSTRING`. So a string sitting behind a borrowed stack index answers false.

## The state owner

`00b66bd0 BSP_LuaStateOwner_Construct`, `__thiscall(ECX = owner)`, RET, over a 4C8h object:
the owns byte at +00, the state at +04, then +0Ch and +10h, then fifty 18h-byte slots from
+28h (the 0x31 counter and 18h stride at 00b66be8), then +4C4h. `PcStorageLuaOwner` in
`include/bsp/lua_state_owner.hpp` is the concrete stock-Lua owner for that object; only the
lifetime rule is reconstructed here.

`00b669a0 BSP_LuaStateOwner_Close`, `__thiscall(ECX = owner)`, RET:

```
if (owner->state != null && owner->owns_state) lua_close(owner->state);  // 00a68a90
owner->state = null;                                                      // both paths
```

The owns byte is **not** cleared, so an owner that borrowed its state forgets the state without
closing it, and a second close is harmless only because the state word is already null.

## The three call shapes

**Scan a table.** `00b67080` restarts (`lua_pushnil` then `lua_next`), `00b67190` advances
(`lua_pushvalue` then `lua_next`), and the loop ends when `00b66420` reports the **value**
unbound. Both routines take `ECX` = the table and two stack arguments in the order
(key out, value out); the decal loader's sites are 0074099a and 00740d5f. Testing the value
rather than the key matters: a loader that tested the key would stop one element early on any
table whose last key is unbound-shaped. `LuaTableScan` in `include/bsp/lua_object.hpp` is this
loop, and it releases both temporaries in the native order, value first (00740d7a) then key
(00740d8b).

**Read one field.** `00b67800 BSP_LuaObject_GetByName` (`lua_gettop`, `lua_pushlstring`,
`lua_gettable`) writes a temporary LuaObject through a hidden out pointer and returns that
pointer in `EAX`; the accessor runs on the returned pointer, and `00b67700` releases it. There
is no presence test anywhere in this shape, so a missing key reaches the accessor as nil:
`00b66270` and `00b66290` return zero and `00b662b0` returns what `lua_tolstring` gives for
nil. `lua_field_number_00b66270`, `lua_field_integer_00b66290` and `lua_field_string_00b662b0`
are the three combinations the loaders use.

**Take a global.** `00b67980` then `00b67800` then `00b67700`: build the globals pseudo-index
object, look the name up in it, release the globals object and keep the result
(0074092f..00740965). `lua_global_by_name_00b67980`.

## The localized message table, for completeness

`00996060` is the sibling of this API used by the pre-VFS code: `__fastcall(ECX = LCID,
EDX = message index)` plus varargs, returning `table_00d1e918[row + index * 6]`. The row comes
from `0098d400 BSP_Localization_LanguageRowForLcid`, a pure switch over the LCID: 0x40c = 1,
0x407 = 2, 0x410 = 3, 0x415 = 4, 0x419 = 5, everything else 0. Six languages per message, which
is the stride. `00735450 BSP_LocalizedMessage_Format` then formats the selected string with
`_vswprintf_p_l` into a caller buffer whose wide capacity arrives in `EDX`.

## Uncertainty

- Kind 1 is named from the globals alias in `include/bsp/gui_lua_runtime.hpp` and from
  `00b660a0` refusing it. No call site in this packet produces one directly.
- +0Ch of the LuaObject is never written by the constructor and no routine read in this packet
  reads it. It is modelled as an explicit uninitialised word rather than guessed at.
- The fifty 18h-byte slots of the state owner are not reconstructed. Nothing in the decal path
  touches them.

## Corrections

- The ledger evidence on `00b66420` said 00b67190 "leaves the key unbound when lua_next reports
  the end", citing 00bd5f50. In `00740840` the end test is on the **value**, not the key
  (007409a8 and 00740d6d both pass the value temporary at frame+0x40). Both readings can hold
  if the iterator unbinds both; what is established here is only that the value is unbound at
  the end, because that is what the decal loop tests.

## Follow-up packets

- `lua_object_iterators`: reconstruct 00b67080 and 00b67190 themselves, and settle whether the
  key is unbound at the end as well as the value.
- `lua_state_owner_callbacks`: the fifty 18h-byte slots 00b66bd0 clears at owner+28h, and the
  override contract 00b69d40 applies over them.

## Correction from docs/NATIVE_SHADER_STATE_READER.md

Full native storage and instruction comparisons establish that LuaObject word00 points to
its 4C8h owner, and kind2/index08 is a tracked Lua STACK position. It is not a registry
reference. The owner's fifty24-byte slots begin at14; their counts are at28+18*i. Native
release removes a stack value when its last tracked pointer disappears, shifts subsequent
slots and decrements those objects' indices, leaving the high-water bound and stale pointer
cells intact. The new NativeLuaObjectStorage/NativeLuaStateStorage preserve this behavior;
the existing LuaObject and GuiLuaRef interfaces remain separate host projections. See the
new document and reports/native_shader_state_reader.json for full code, ABI and validation.
