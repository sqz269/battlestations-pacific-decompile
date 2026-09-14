# Native Lua numeric field setter

Address: `00B67400..00B67455` (86 bytes, complete).

`native_lua_set_number_field_00b67400` supplies the numeric setter required by
`native_unit_killed_base.cpp` for `LastPosition.x/y/z`. It operates on the
existing `NativeLuaObjectStorage`, `NativeString` and linked Lua 5.1.1 stack.
The original is ECX = object, stack = key pointer then **float32**, `RET8`.
The source C++ interface has a different ABI. `BSP_LuaObject_SetNumberField`
is a descriptive hypothesis, not a recovered symbol.

The body ignores `kind04` and the return of `lua_checkstack(L, 2)`. After that
call, it reads the current key length and then data. Null data selects the
empty byte at native `0108FF2C`; length remains unchanged. Each Lua operation
reloads the object's owner and its state. A callback can therefore move later
operations to another interpreter without moving values already pushed.

The numeric argument is loaded with x87 `FLD float`, followed by current
owner/state reads, and widened by `FSTP double` before `lua_pushnumber`.
An ordinary C++ cast can use SSE, changing subnormal and NaN status behavior.
The source preserves the original x87 sequence explicitly. Before the final
`lua_settable`, it reads owner, index, and that owner's state in that order.
There is no metadata update, stack-object registration or cleanup guard.

| Call site | Required library operation | Native ABI |
| --- | --- | --- |
| B6740D | A672F0 `lua_checkstack` | ECX state, EDX 2, RET |
| B6742C | A67A10 `lua_pushlstring` | ECX state, EDX bytes, stack length, RET4 |
| B67440 | A679D0 `lua_pushnumber` | ECX state, stack double, RET8 |
| B6744D | A67E10 `lua_settable` | ECX state, EDX index, RET |

All 30 incoming calls in the 12 current caller bodies were inspected in the
live listings. They pass a four-byte numeric argument and an eight-byte native
string header address. The report checks those calls and the four outgoing
calls against the live Ghidra bodies: 34 rows passed. The string layout is
established by `0041DD40`; the existing Lua owner layout by `00B66BD0`.

The Win32 Release build and both existing CTests passed. Eight native seed
checks passed before the differential probe. The probe links the production
object and Lua 5.1.1 library and executes all 86 original bytes through four
explicit ABI adapters. Only four relative-call operands and the empty-literal
address are relocated; 66 other bytes remain unchanged. No instructions or
exception setup are replaced; this native body has no local EH frame.

There are 27 native/source pairs: six float bit patterns under four rounding
modes, two allocator callback cases, and the null-data/zero-length key. They
cover a finite value, negative zero, a subnormal, infinity, quiet/signaling
NaNs, embedded-NUL keys, unchanged object metadata, and stack effects. One
allocator changes owner and key during stack growth; the other changes owner
and index during string allocation. The latter leaves the key on the first
interpreter and sets the value on the second, preserving the per-call state
captures. Numeric results and the six sticky x87 exception bits match.

The retained proof is `local/number_field_z/manifest.json`, referenced by hash
in `reports/native_lua_number_field.json`. It includes the production object,
compiler-discovered headers, Lua source and library, searched SDK/CRT libraries,
resolved compiler tools, fixture source/executable, embedded manifest, original
and relocated bytes, call contexts, code generation, and run output.

All tested x87 exceptions were masked. Unmasked traps, private frame aliases,
hardware faults, full FPU ABI, Lua longjmp behavior, and invalid pointer/index
domains are unproved. The native Lua library bodies are identified dependencies;
the fixture invokes linked Lua 5.1.1 at their ABI boundaries. This setter is not
yet bound in `GameUnitsHost`, and these fixtures do not establish gameplay
execution of the killed-unit path.
